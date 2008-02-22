/*! \file
    \ingroup CCSORT
    \brief Enter brief description of file here 
*/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libiwl/iwl.h>
#include <ccfiles.h>
#include <psifiles.h>
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

namespace psi { namespace ccsort {

void idx_permute_presort(dpdfile4 *File, int this_bucket,
			   int **bucket_map, int **bucket_offset,
			   int p, int q, int r, int s,
			   double value, FILE *outfile);

int file_build_presort(dpdfile4 *File, int inputfile, double tolerance, int keep)
{
  struct iwlbuf InBuf;
  int lastbuf;
  long int memoryb, memoryd, core_left, row_length;
  int h, nirreps, n, row, nump, numq, nbuckets;
  int **bucket_map, **bucket_offset, **bucket_rowdim;
  int **bucket_size;
  Value *valptr;
  Label *lblptr;
  int idx, p, q, r, s;
  double value;
  struct iwlbuf *SortBuf;
  psio_address next;

  nirreps = File->params->nirreps;

  fndcor(&memoryb, infile, outfile);
  memoryd = memoryb/sizeof(double);

  /* It's annoying that I have to compute this here */
  for(h=0,nump=0,numq=0; h < File->params->nirreps; h++) {
    nump += File->params->ppi[h];
    numq += File->params->qpi[h];
  }
  bucket_map = init_int_matrix(nump,numq);

  /* Room for one bucket to begin with */
  bucket_offset = (int **) malloc(sizeof(int *));
  bucket_offset[0] = init_int_array(nirreps);
  bucket_rowdim = (int **) malloc(sizeof(int *));
  bucket_rowdim[0] = init_int_array(nirreps);
  bucket_size = (int **) malloc(sizeof(int *));
  bucket_size[0] = init_int_array(nirreps);
    
  /* Figure out how many passes we need and where each p,q goes */
  for(h=0,core_left=memoryd,nbuckets=1; h < nirreps; h++) {

    row_length = (long int) File->params->coltot[h^(File->my_irrep)];
	       
    for(row=0; row < File->params->rowtot[h]; row++) {

      if((core_left - row_length) >= 0) {
	core_left -= row_length;
	bucket_rowdim[nbuckets-1][h]++;
	bucket_size[nbuckets-1][h] += row_length;
      }
      else {
	nbuckets++;
	core_left = memoryd - row_length;

	/* Make room for another bucket */
	bucket_offset = (int **) realloc((void *) bucket_offset,
					 nbuckets * sizeof(int *));
	bucket_offset[nbuckets-1] = init_int_array(nirreps);
	bucket_offset[nbuckets-1][h] = row;

	bucket_rowdim = (int **) realloc((void *) bucket_rowdim,
					 nbuckets * sizeof(int *));
	bucket_rowdim[nbuckets-1] = init_int_array(nirreps);
	bucket_rowdim[nbuckets-1][h] = 1;

	bucket_size = (int **) realloc((void *) bucket_size,
					    nbuckets * sizeof(int *));
	bucket_size[nbuckets-1] = init_int_array(nirreps);
	bucket_size[nbuckets-1][h] = row_length;
      }

      p = File->params->roworb[h][row][0];
      q = File->params->roworb[h][row][1];
      bucket_map[p][q] = nbuckets - 1;
    }
  }

  fprintf(outfile, "\tSorting File: %s nbuckets = %d\n", File->label, nbuckets);
  fflush(outfile);

  next = PSIO_ZERO;
  for(n=0; n < nbuckets; n++) { /* nbuckets = number of passes */

    /* Prepare target matrix */
    for(h=0; h < nirreps; h++) {
      File->matrix[h] = block_matrix(bucket_rowdim[n][h], File->params->coltot[h]);
    }

    iwl_buf_init(&InBuf, inputfile, tolerance, 1, 1);

    lblptr = InBuf.labels;
    valptr = InBuf.values;
    lastbuf = InBuf.lastbuf;

    for(idx=4*InBuf.idx; InBuf.idx < InBuf.inbuf; InBuf.idx++) {
      p = abs((int) lblptr[idx++]);
      q = (int) lblptr[idx++];
      r = (int) lblptr[idx++];
      s = (int) lblptr[idx++];

      value = (double) valptr[InBuf.idx];

/*       fprintf(outfile, "%d %d %d %d %20.12f\n", p, q, r, s, value); */

      idx_permute_presort(File,n,bucket_map,bucket_offset,p,q,r,s,value,outfile);

    } /* end loop through current buffer */

    /* Now run through the rest of the buffers in the file */
    while (!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      for (idx=4*InBuf.idx; InBuf.idx < InBuf.inbuf; InBuf.idx++) {
	p = abs((int) lblptr[idx++]);
	q = (int) lblptr[idx++];
	r = (int) lblptr[idx++];
	s = (int) lblptr[idx++];

	value = (double) valptr[InBuf.idx];

/* 	fprintf(outfile, "%d %d %d %d %20.12f\n", p, q, r, s, value); */

	idx_permute_presort(File,n,bucket_map,bucket_offset,p,q,r,s,value,outfile);
      
      } /* end loop through current buffer */
    } /* end loop over reading buffers */

    iwl_buf_close(&InBuf, 1); /* close buffer for next pass */

    for(h=0; h < nirreps;h++) {
      if(bucket_size[n][h])
	psio_write(File->filenum, File->label, (char *) File->matrix[h][0],
		   bucket_size[n][h]*((long int) sizeof(double)), next, &next);
      free_block(File->matrix[h]);
    }

  } /* end loop over buckets/passes */

  /* Get rid of the input integral file */
  psio_open(inputfile, PSIO_OPEN_OLD);
  psio_close(inputfile, keep);

  free_int_matrix(bucket_map);

  for(n=0; n < nbuckets; n++) {
    free(bucket_offset[n]);
    free(bucket_rowdim[n]);
    free(bucket_size[n]);
  }
  free(bucket_offset);
  free(bucket_rowdim);
  free(bucket_size);

  return 0;
}

}} // namespace psi::ccsort
