/*! \defgroup QT libqt: The Quantum-Trio Miscellaneous Library */

/*! \file blas_intfc.c
    \ingroup (QT)
    \brief The PSI3 BLAS interface routines

 Interface to the BLAS routines

 C. David Sherrill
 Anna I. Krylov

 May 1998

 Additions by TD Crawford and EF Valeev, June 1999.
*/

#include <stdio.h>

extern "C" {

#if FC_SYMBOL==2
#define F_DAXPY daxpy_
#define F_DCOPY dcopy_
#define F_DGEMM dgemm_
#define F_DROT drot_
#define F_DSCAL dscal_
#define F_DGEMV dgemv_
#define F_DSPMV dspmv_
#define F_DDOT  ddot_
#elif FC_SYMBOL==1
#define F_DAXPY daxpy
#define F_DCOPY dcopy
#define F_DGEMM dgemm
#define F_DROT drot
#define F_DSCAL dscal
#define F_DGEMV dgemv
#define F_DSPMV dspmv
#define F_DDOT  ddot
#elif FC_SYMBOL==3
#define F_DAXPY DAXPY
#define F_DCOPY DCOPY
#define F_DGEMM DGEMM
#define F_DROT DROT
#define F_DSCAL DSCAL
#define F_DGEMV DGEMV
#define F_DSPMV DSPMV
#define F_DDOT  DDOT
#elif FC_SYMBOL==4
#define F_DAXPY DAXPY_
#define F_DCOPY DCOPY_
#define F_DGEMM DGEMM_
#define F_DROT DROT_
#define F_DSCAL DSCAL_
#define F_DGEMV DGEMV_
#define F_DSPMV DSPMV_
#define F_DDOT  DDOT_
#endif

extern void F_DAXPY(int *length, double *a, double *x, int *inc_x, 
                    double *y, int *inc_y);
extern void F_DCOPY(int *length, double *x, int *inc_x, 
                    double *y, int *inc_y);
extern void F_DGEMM(char *transa, char *transb, int *m, int *n, int *k, 
                    double *alpha, double *A, int *lda, double *B, int *ldb, 
                    double *beta, double *C, int *ldc);
extern void F_DROT(int *ntot,double *x, int *incx,double *y, int *incy,
                  double *cotheta,double *sintheta);
extern void F_DSCAL(int *n, double *alpha, double *vec, int *inc);
extern void F_DGEMV(char *transa, int *m, int *n, double *alpha, double *A, 
                    int *lda, double *X, int *inc_x, double *beta, 
                    double *Y, int *inc_y);
extern void F_DSPMV(char *uplo,int *n,double *alpha,double *A,double *X,int *inc_x,double *beta,double *Y,int *inc_y);
extern double F_DDOT(int *n, double *x, int *incx, double *y, int *incy);

/*! 
** C_DAXPY()
** This function performs y = a * x + y. 
** Steps every inc_x in x and every inc_y in y (normally both 1).
** \ingroup (QT)
*/
void C_DAXPY(int length, double a, double *x, int inc_x, 
             double *y, int inc_y)
{
  F_DAXPY(&length, &a, x, &inc_x, y, &inc_y);
}

/*!
** C_DCOPY()
** This function copies x into y.
** Steps every inc_x in x and every inc_y in y (normally both 1).
** \ingroup (QT)
*/
void C_DCOPY(int length, double *x, int inc_x, 
             double *y, int inc_y)
{
  F_DCOPY(&length, x, &inc_x, y, &inc_y);
}


/*!
** C_DSCAL()
** This function scales a vector by a real scalar.
** \ingroup (QT)
*/
void C_DSCAL(int n, double alpha, double *vec, int inc)
{
  F_DSCAL(&n, &alpha, vec, &inc);
}


/*!
** C_DROT()
** This function calculates plane Givens rotation for vectors
** x,y and angle theta.  x = x*cos + y*sin, y = -x*sin + y*cos.
**
**  \param x: vector x
**  \param y: vector Y
**  \param tot: length of x,y
**  \param incx: increment for x
**  \param incy: increment for y
** \ingroup (QT)
*/
void C_DROT(int ntot, double *x, int incx, double *y, int incy,
            double costheta, double sintheta)
{

  F_DROT(&ntot,x,&incx,y,&incy,&costheta,&sintheta);
}


/*!
** C_DGEMM()
**
** This function calculates C(m,n)=alpha*(opT)A(m,k)*(opT)B(k,n)+ beta*C(m,n)
**
** These arguments mimic their Fortran conterparts; parameters have been 
** reversed nca, ncb, ncc, A, B, C,  to make it correct for C.
**  
** \param char transa: On entry, specifies the form of (op)A used in the
**                    matrix multiplication:
**                    If transa = 'N' or 'n', (op)A = A.
**                    If transa = 'T' or 't', (op)A = transp(A).
**                    If transa = 'R' or 'r', (op)A = conjugate(A).
**                    If transa = 'C' or 'c', (op)A = conjug_transp(A).
**                    On exit, transa is unchanged.
**
** \param char transb: On entry, specifies the form of (op)B used in the
**                    matrix multiplication:
**                    If transb = 'N' or 'n', (op)B = B.
**                    If transb = 'T' or 't', (op)B = transp(B).
**                    If transb = 'R' or 'r', (op)B = conjugate(B)
** 
** \param int m:      On entry, the number of rows of the matrix (op)A and of
**                    the matrix C; m >= 0. On exit, m is unchanged.
**
** \param int n:      On entry, the number of columns of the matrix (op)B and
**                    of the matrix C; n >= 0. On exit, n is unchanged.
**
** \param int k:      On entry, the number of columns of the matrix (op)A and
**                    the number of rows of the matrix (op)B; k >= 0. On exit,
**                    k is unchanged.
** 
** \param double alpha:  On entry, specifies the scalar alpha. On exit, alpha is
**                       unchanged.
** 
** \param double* A:  On entry, a two-dimensional array A with dimensions ka
**                    by nca. For (op)A = A  or  conjugate(A), nca >= k and the
**                    leading m by k portion of the array A contains the matrix
**                    A. For (op)A = transp(A) or conjug_transp(A), nca >= m
**                    and the leading k by m part of the array A contains the
**                    matrix A. On exit, a is unchanged.
** 
** \param int nca:    On entry, the second dimension of array A.
**                    For (op)A = A  or conjugate(A), nca >= MAX(1,k).
**                    For (op)A=transp(A) or conjug_transp(A), nca >= MAX(1,m).
**                    On exit, nca is unchanged.
**
** \param double* B:  On entry, a two-dimensional array B with dimensions kb
**                  by ncb. For (op)B = B or conjugate(B), kb >= k and the
**                  leading k by n portion of the array contains the matrix
**                  B. For (op)B = transp(B) or conjug_transp(B), ncb >= k and
**                  the leading n by k part of the array contains the matrix
**                  B. On exit, B is unchanged.
**
** \param int ncb:    On entry, the second dimension of array B.
**                    For (op)B = B or <conjugate(B), ncb >= MAX(1,n).
**                    For (op)B = transp(B) or conjug_transp(B), ncb >=
**                    MAX(1,k). On exit, ncb is unchanged.
**
** \param double beta: On entry, specifies the scalar beta. On exit, beta is
**                    unchanged.
**
** \param double* C:  On entry, a two-dimensional array with the dimension
**                    at least m by ncc. On exit,  the leading  m by n part of
**                    array C is overwritten by the matrix alpha*(op)A*(op)B +
**                    beta*C.
**
** \param int ncc:    On entry, the second dimension  of array C; 
**                    ncc >=MAX(1,n).  On exit, ncc is unchanged.
** \ingroup (QT)
*/
void C_DGEMM(char transa, char transb, int m, int n, int k, double alpha,
           double *A, int nca, double *B, int ncb, double beta, double *C,
           int ncc)
{

  /* the only strange thing we need to do is reverse everything
     since the stride runs differently in C vs. Fortran
   */
  
  /* also, do nothing if a dimension is 0 */
  if (m == 0 || n == 0 || k == 0) return;

  F_DGEMM(&transb,&transa,&n,&m,&k,&alpha,B,&ncb,A,&nca,&beta,C,&ncc);

}

/*!
**  C_DGEMV()
**  This function calculates the matrix-vector product:
**
**  Y = alpha * A * X + beta * Y
**
** where X and Y are vectors, A is a matrix, and alpha and beta are
** constants. 
**
** \param char transa:     Indicates whether the matrix A should be
**                         transposed ('t') or left alone ('n').
** \param int m:           The row dimension of A (regardless of transa).
** \param int n:           The column dimension of A (regardless of transa).
** \param double alpha:    The scalar alpha.
** \param double* A:       A pointer to the beginning of the data in A.
** \param int nca:         The number of columns *actually* in A.  This is
**                         useful if one only wishes to multiply the first
**                         n columns of A times X even though A
**                         contains nca columns.
** \param double* X:       A pointer to the beginning of the data in X.
** \param int inc_x:       The desired stride for X.  Useful for skipping
**                         sections of data to treat only one column of a
**                         complete matrix.  Usually 1, though.
** \param double beta:     The scalar beta.
** \param double* Y:       A pointer to the beginning of the data in Y.
** \param int inc_y:       The desired stride for Y.
** 
** Interface written by TD Crawford and EF Valeev.
** June 1999.
** \ingroup (QT)
*/

void C_DGEMV(char transa, int m, int n, double alpha, double *A, 
             int nca, double *X, int inc_x, double beta, double *Y,
             int inc_y)
{
  if (m == 0 || n == 0) return;

  if(transa == 'n') transa = 't';
  else transa = 'n';

  F_DGEMV(&transa,&n,&m,&alpha,A,&nca,X,&inc_x,&beta,Y,&inc_y);

}


/*!
**  C_DSPMV()
**  This function calculates the matrix-vector product:
**
**  Y = alpha * A * X + beta * Y
**
** where X and Y are vectors, A is a matrix, and alpha and beta are
** constants. 
**
** \param char uplo:       Indicates whether the matrix A is packed in
**                         upper ('U' or 'u') or lower ('L' or 'l')
**                         triangular form.  We reverse what is passed
**                         before sending it on to Fortran because of 
**                         the different Fortran/C conventions
** \param int n:           The order of the matrix A (number of rows/columns)
** \param double alpha:    The scalar alpha.
** \param double* A:       A pointer to the beginning of the data in A.
** \param double* X:       A pointer to the beginning of the data in X.
** \param int inc_x:       The desired stride for X.  Useful for skipping
**                         sections of data to treat only one column of a
**                         complete matrix.  Usually 1, though.
** \param double beta:     The scalar beta.
** \param double* Y:       A pointer to the beginning of the data in Y.
** \param int inc_y:       The desired stride for Y.
** 
** Interface written by CD Sherrill
** July 2003
** \ingroup (QT)
*/

void C_DSPMV(char uplo, int n, double alpha, double *A, 
             double *X, int inc_x, double beta, double *Y,
             int inc_y)
{
  if (n == 0) return;

  if (uplo != 'U' && uplo != 'u' && uplo != 'L' && uplo != 'l')
    fprintf(stderr, "C_DSPMV: called with unrecognized option for uplo!\n");

  if (uplo == 'U' || uplo == 'u') uplo = 'L';
  else uplo = 'U';

  F_DSPMV(&uplo,&n,&alpha,A,X,&inc_x,&beta,Y,&inc_y);

}

/*!
** C_DDOT()
** This function returns the dot product of two vectors, X and Y.
**
** \param n:      Number of elements in X and Y.
** \param X:      A pointer to the beginning of the data in X.
**                Must be of at least length (1+(N-1)*abs(inc_x).
** \param inc_x:  The desired stride of X. Useful for skipping
**                    around to certain data stored in X.
** \param Y:      A pointer to the beginning of the data in Y.
** \param inc_y:  The desired stride for Y.
**
** Interface written by ST Brown.
** July 2000
** \ingroup (QT)
*/

double C_DDOT(int n, double *X, int inc_x, double *Y, int inc_y)
{
   if(n == 0) return 0.0;

   return F_DDOT(&n,X,&inc_x,Y,&inc_y);
}

} /* extern "C" */