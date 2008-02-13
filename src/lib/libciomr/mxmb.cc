/*!
** \file mxmb.cc
** \brief Wrapper for the mmult function
** \ingroup CIOMR
*/
 
#include <stdio.h>
#include <stdlib.h>
#include "libciomr.h"

extern "C" {

  static void mxmbol(double **a, int ia, int ja, double **b, int ib, int jb, 
    double **c, int ic, int jc, int nrow, int nlnk, int ncol) 
  {
  abort();
  }

  
/*!
** mxmb: multiplies two rectangular matrices together (wrapper for mmult).  
** Deprecated; use C_DGEMM instead.
**
** \param a    = first matrix to multiply
** \param ia   = if 1, normal multiplication of a
** \param ja   = if 1, transpose a before multiplication
** \param b    = second matrix to multiply
** \param ib   = if 1, normal multiplication of b
** \param jb   = if 1, transpose b before multiplication
** \param c    = matrix to store the result
** \param ic   = if 1, normal multiplication into c
** \param jb   = if 1, transpose c after multiplication
** \param nrow = number of rows of a
** \param nlnk = number of columns of a and rows of b
** \param ncol = number of columns of b
** 
** Returns: none
**
** \ingroup CIOMR
*/
void mxmb(double **a, int ia, int ja, double **b, int ib, int jb, 
          double **c, int ic, int jc, int nrow, int nlnk, int ncol)
   {
      if (ic == 1) {
         if (ia == 1) {
            if (ib == 1) {
               mmult(a,0,b,0,c,0,nrow,nlnk,ncol,0);
               }
            else {
               if (jb == 1) {
                  mmult(a,0,b,1,c,0,nrow,nlnk,ncol,0);
                  }
               else {
                  mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
                  }
               }
            }
         else {
            if (ja == 1) {
               if (ib == 1) {
                  mmult(a,1,b,0,c,0,nrow,nlnk,ncol,0);
                  }
               else {
                  if (jb == 1) {
                     mmult(a,1,b,1,c,0,nrow,nlnk,ncol,0);
                     }
                  else {
                     mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
                     }
                  }
               }
            else {
               mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
               }
            }
         }
      else {
         if (jc == 1) {
            if (ia == 1) {
               if (ib == 1) {
                  mmult(a,0,b,0,c,1,nrow,nlnk,ncol,0);
                  }
               else {
                  if (jb == 1) {
                     mmult(a,0,b,1,c,1,nrow,nlnk,ncol,0);
                     }
                  else {
                     mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
                     }
                  }
               }
            else {
               if (ja == 1) {
                  if (ib == 1) {
                     mmult(a,1,b,0,c,1,nrow,nlnk,ncol,0);
                     }
                  else {
                     if (jb == 1) {
                        mmult(a,1,b,1,c,1,nrow,nlnk,ncol,0);
                        }
                     else {
                        mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
                        }
                     }
                  }
               else {
                  mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
                  }
               }
            }
         else {
            mxmbol(a,ia,ja,b,ib,jb,c,ic,jc,nrow,nlnk,ncol);
            }
         }
      }

} /* extern "C" */
