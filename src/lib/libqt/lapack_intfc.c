/*!
** \file lapack_intfc.c
** \brief Interface to LAPACK routines
** \ingroup (QT)
**
** Rollin A. King and T. Daniel Crawford
** August 2001 - January 2002
**
** 03/08/2002 EFV Added DGETRF since DGETRI isn't useful without it
**
** Written to work similarly to the BLAS C interface in blas_intfc.c
*/

#if FC_SYMBOL==2
#define F_DGEEV dgeev_
#define F_DGESV dgesv_
#define F_DGETRF dgetrf_
#define F_DGETRI dgetri_
#define F_DGESVD dgesvd_
#define F_DSYEV dsyev_
#elif FC_SYMBOL==1
#define F_DGEEV dgeev
#define F_DGESV dgesv
#define F_DGETRF dgetrf
#define F_DGETRI dgetri
#define F_DGESVD dgesvd
#define F_DSYEV dsyev
#elif FC_SYMBOL==3
#define F_DGEEV DGEEV
#define F_DGESV DGESV
#define F_DGETRF DGETRF
#define F_DGETRI DGETRI
#define F_DGESVD DGESVD
#define F_DSYEV DSYEV
#elif FC_SYMBOL==4
#define F_DGEEV DGEEV_
#define F_DGESV DGESV_
#define F_DGETRF DGETRF_
#define F_DGETRI DGETRI_
#define F_DGESVD DGESVD_
#define F_DSYEV DSYEV_
#endif


/* this should be a diagonalizer */
int C_DGEEV(int n, double **a, int lda,
  double *wr, double *wi, double **vl, int ldvl, double **vr,
  int ldvr, double *work, int lwork, int info)
{
  char jobvl, jobvr;
  jobvl = 'V';
  jobvr = 'V';
  F_DGEEV(&jobvl, &jobvr, &n, &(a[0][0]), &lda, &(wr[0]), &(wi[0]),
       &(vl[0][0]), &ldvl, &(vr[0][0]), &ldvr, &(work[0]), &lwork, &info);

  return info;
}

/*!
** C_DGESV()
**
** This function solves a system of linear equations A * X = B, where
** A is an n x n matrix and X and B are n x nrhs matrices.
**
** Arguments:
** \param int n: The number of linear equations, i.e., the order of
** the matrix A.  n >= 0.
**
** \param int nrhs: The number of right hand sides, i.e., the number
** of columns of the matrix B.  nrhs >= 0.
**
** \param double *A: On entry, the n-by-n coefficient matrix A.  On
** exit, the factors L and U from the factorization A = P*L*U; the
** unit diagonal elements of L are not stored.
**
** \params int lda: The leading dimension of the array A. lda >=
** max(1,n).
**
** \param int *ipiv: An integer array of length n.  The pivot indices
** that define the permutation matrix P; row i of the matrix was
** interchanged with row ipiv(i).
**
** \param double *B: On entry, the n-by-nrhs matrix of right hand side
** matrix B.  On exit, if info = 0, the n-by-nrhs solution matrix X.
**
** \param int ldb: The leading dimension of the array B.  ldb >=
** max(1,n).
**
** Returns:
** 
** \param int info: = 0: successful exit < 0: if info = -i, the i-th
** argument had an illegal value > 0: if info = i, U(i,i) is exactly
** zero.  The factorization has been completed, but the factor U is
** exactly singular, so the solution could not be computed.
**
** \ingroup (QT)
*/
int C_DGESV(int n, int nrhs, double *a, int lda, int *ipiv, double *b, int ldb)
{
  int info;

  F_DGESV(&n, &nrhs, &(a[0]), &lda, &(ipiv[0]), &(b[0]), &ldb, &info);

  return info;
}

/* 
   lda >= ncol
 */
int C_DGETRF(int nrow, int ncol, double *a, int lda, int *ipiv)
{
  int info;

  F_DGETRF(&ncol, &nrow, &(a[0]), &lda, &(ipiv[0]), &info);

  return info;
}

int C_DGETRI(int n, double *a, int lda, int *ipiv, double *work, int lwork)
{
  int info;

  F_DGETRI(&n, &(a[0]), &lda, &(ipiv[0]), &(work[0]), &lwork, &info);

  return info;
}

int C_DGESVD(char jobu, char jobvt, int m, int n, double *A, int lda, double *s, int lds, 
	     double *u, int ldu, double *vt, int ldvt, double *work, int lwork)
{
  int info;

  F_DGESVD(&jobvt, &jobu, &n, &m, A, &lda, s, vt, &ldvt, u, &ldu, work, &lwork, &info);

  return info;
}

/*!
** C_DSYEV()
** This function computes all eigenvalues and, optionally, eigenvectors of a real 
** symmetric matrix A.
**
** These arguments mimic their Fortran counterparts.
**
** \param char jobz:    'N' or 'n' = compute eigenvalues only;
**                      'V' or 'v' = compute both eigenvalues and eigenvectors.
**
** \param char uplo:    'U' or 'u' = A contains the upper triangular part of the matrix;
**                      'L' or 'l' = A contains the lower triangular part of the matrix.
**
** \param int n:        The order of the matrix A.
**
** \param double *A:    On entry, the two-dimensional array with dimensions n by lda.
**                      On exit, if jobz = 'V', the matrix contains the eigenvectors of A, 
**                      but if jobz = 'N', the contents of the matrix are destroyed.
**
** \param int lda:      The second dimension of A (i.e., the number of columns allocated for A).
**
** \param double *w:    The computed eigenvalues in ascending order.
**
** \param double *work: An array of length lwork.  On exit, if the return value is 0, work[0]
**                      contains the optimal value of lwork.
**
** \param int lwork:    The length of the array work.  A useful value of lwork seems to be 3*N.
**
** Returns:  0 = successful exit
**          <0 = the value of the i-th argument to the function was illegal
**          >0 = the algorithm failed to converge.
**
** Interface written by TDC, 10/2002
** \ingroup(QT)
*/

int C_DSYEV(char jobz, char uplo, int n, double *A, int lda, double *w, double *work, int lwork)
{
  int info;

  F_DSYEV(&jobz, &uplo, &n, A, &lda, w, work, &lwork, &info);

  return info;
}
