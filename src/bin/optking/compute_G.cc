// This function computes G via BuB^t where u is a diagonal matrix
// of inverse masses.

#include <cmath>

extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <libciomr/libciomr.h>
}

#define EXTERN
#include "opt.h"
#undef EXTERN
#include "cartesians.h"

double **compute_G(double **B, int num_intcos, cartesians &carts) {
  double **u, **G, **temp_mat, *masses;
  int i, dim_carts;

  // dim_carts = 3*carts.get_natom();
  dim_carts = 3*optinfo.nallatom;
  masses = carts.get_fmass();
  u = mass_mat(masses);
  free(masses);

  G = block_matrix(num_intcos,num_intcos);
  temp_mat = block_matrix(dim_carts,num_intcos);

  mmult(u,0,B,1,temp_mat,0,dim_carts,dim_carts,num_intcos,0);
  mmult(B,0,temp_mat,0,G,0,num_intcos,dim_carts,num_intcos,0);

  free_block(u);
  free_block(temp_mat);

  return G;
}
