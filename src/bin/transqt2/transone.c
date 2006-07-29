#include <stdio.h>
#include <stdlib.h>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>

/*
** transone(): Transform a packed symmetric matrix.
**
** int m: input matrix row dimension
** int n: output matrix row dimension
** double *input: pointer to input integrals (the lower-triange of a symmetric matrix)
** double *output: pointer to output integrals (the lower-triange of a symmetric matrix)
** double **C: transformation matrix (rectangular)
** int nc: column dimension of C
** int *order: reordering array for transformed indices
**
** Written for new transqt module
** TDC, 7/06
*/

#define INDEX(i,j) ((i>j) ? (ioff[(i)]+(j)) : (ioff[(j)]+(i)))

void transone(int m, int n, double *input, double *output, double **C, int nc, int *order, int *ioff)
{
  int p, q, pq;
  double **TMP0, **TMP1;

  TMP0 = block_matrix(m,m);
  TMP1 = block_matrix(m,m);

  for(p=0,pq=0; p < m; p++)
    for(q=0; q <= p; q++,pq++) 
      TMP0[p][q] = TMP0[q][p] = input[pq];

  if(m && n) {
    C_DGEMM('n','n',m,n,m,1.0,TMP0[0],m,C[0],nc,0.0,TMP1[0],m);
    C_DGEMM('t','n',n,n,m,1.0,C[0],nc,TMP1[0],m,0.0,TMP0[0],m);
  }

  for(p=0; p < n; p++)
    for(q=0; q <= p; q++) {
      pq = INDEX(order[p],order[q]);
      output[pq] = TMP0[p][q];
    }

  free_block(TMP0);
  free_block(TMP1);
}