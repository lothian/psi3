/*! \file 
    \ingroup (INPUT)
    \brief Enter brief description of file here 
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libciomr/libciomr.h>

#include "defines.h"
#define EXTERN
#include "global.h"

static double **init_bf_norm(int);

void init_gto(max_angmom)
{
  if (max_angmom > GTOs.max_angmom || GTOs.bf_norm == NULL) {
    GTOs.max_angmom = max_angmom;
    GTOs.bf_norm = init_bf_norm(max_angmom+1);
  }
  return;
}


void cleanup_gto()
{
  free_matrix(GTOs.bf_norm,GTOs.max_angmom+1);
  return;
}

/*---------------------------------------------------------
  Computes normalization constants for cartesian Gaussians
 ---------------------------------------------------------*/
double **init_bf_norm(int max_am)
{
  static int use_cca_integrals_standard = (INTEGRALS_STANDARD == 1);
  double **bf_norm;
  int am,bf,i,j,l1,m1,n1;

  bf_norm = (double **) malloc(sizeof(double *)*max_am);
  for(am=0; am<max_am; am++) {
    bf = 0;
    bf_norm[am] = init_array(ioff[am+1]);
    for(i=0; i<=am; i++) {
      l1 = am - i;
      for(j=0; j<=i; j++) {
	m1 = i-j;
	n1 = j;
	if (use_cca_integrals_standard)
	  bf_norm[am][bf++] = 1.0;
	else
	  bf_norm[am][bf++] = sqrt(df[2*am]/(df[2*l1]*df[2*m1]*df[2*n1]));
      }
    }
  }

  return bf_norm;
}
