extern "C" {
#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<libciomr/libciomr.h>
#include<libint/libint.h>

#include"defines.h"
#define EXTERN
#include"global.h"
}

/*-------------------------------------
  Swap ket and bra of a 4-index buffer
 -------------------------------------*/
extern "C" void ijkl_to_klij(double *ijkl_buf, double *klij_buf, int nij, int nkl)
{
  int ijkl = 0;
  for(int ij=0; ij<nij; ij++) {

    int klij = ij;
    for(int kl=0; kl<nkl; kl++,ijkl++,klij+=nij)
	klij_buf[klij] = ijkl_buf[ijkl];

  }
  return;
}

extern "C" void ijkl_to_jikl(double *ijkl_buf, double *jikl_buf, int ni, int nj, int nkl)
{
  double* ijkl_ptr = ijkl_buf;
  for(int i=0; i<ni; i++) {
    for(int j=0; j<nj; j++) {      
      int ji = j*ni+i;
      double* jikl_ptr = jikl_buf + ji*nkl;
      for(int kl=0;kl<nkl;kl++,ijkl_ptr++,jikl_ptr++)
	*jikl_ptr = *ijkl_ptr;
    }
  }
}

extern "C" void ijkl_to_ijlk(double *ijkl_buf, double *ijlk_buf, int nij, int nk, int nl)
{
  double* ijkl_ptr = ijkl_buf;
  for(int ij=0; ij<nij; ij++) {
    double* ij_offset_ptr = ijlk_buf + ij*nk*nl;
    for(int k=0; k<nk; k++) {
      double* ijlk_ptr = ij_offset_ptr + k;
      for(int l=0;l<nl;l++,ijkl_ptr++,ijlk_ptr+=nk)
	*ijlk_ptr = *ijkl_ptr;
    }
  }
}
