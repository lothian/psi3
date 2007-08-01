/*! \file read_gen_opdm.c
    \ingroup (CINTS)
    \brief Enter brief description of file here 
*/
#include<stdio.h>
#include<stdlib.h>
#include<libciomr/libciomr.h>
#include<libpsio/psio.h>
#include<libint/libint.h>

#include"defines.h"
#define EXTERN
#include"global.h"
#include"prints.h"

void read_gen_opdm()
{ 
  int natri = ioff[BasisSet.num_ao];
  PSI_FPTR next;
  double *dens, *lagr;

  dens = init_array(natri);
  lagr = init_array(natri);

  psio_open(IOUnits.itapD, PSIO_OPEN_OLD);
  psio_read_entry(IOUnits.itapD, "AO-basis OPDM", (char *) dens, sizeof(double)*natri);
  psio_read_entry(IOUnits.itapD, "AO-basis Lagrangian", (char *) lagr, sizeof(double)*natri);
  psio_close(IOUnits.itapD, 1);

  /* convert to square forms */
  Dens = block_matrix(BasisSet.num_ao,BasisSet.num_ao);
  Lagr = block_matrix(BasisSet.num_ao,BasisSet.num_ao);
  tri_to_sq(dens,Dens,BasisSet.num_ao);
  tri_to_sq(lagr,Lagr,BasisSet.num_ao);
  free(dens);
  free(lagr);

  print_opdm();

  return;
}

