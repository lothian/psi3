/*!
  \file contr_full.c
*/

#include <stdio.h>
#include <libciomr/libciomr.h>
#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>

/*!
** chkpt_rd_contr_full(): Reads in the normalized contraction coefficients.
**
**  takes no arguments.
**
**  returns: double **contr Normalized contraction coefficients are
**  returned as a matrix of doubles.
*/


double **chkpt_rd_contr_full(void)
{
  double **contr, *temp_contr;
  int nprim, i, j, ij = 0;

  nprim = chkpt_rd_nprim();

  temp_contr = init_array(MAXANGMOM*nprim);
  contr = block_matrix(nprim,MAXANGMOM);

  psio_read_entry(PSIF_CHKPT, "::Contraction coefficients", (char *) temp_contr,
		  MAXANGMOM*nprim*sizeof(double));

  /* Picking non-zero coefficients to the "master" array contr */
  for(i=0,ij=0; i < MAXANGMOM; i++) 
    for(j=0; j < nprim; j++, ij++) {
      contr[j][i] = temp_contr[ij];
    }

  free(temp_contr);

  return contr;
}

