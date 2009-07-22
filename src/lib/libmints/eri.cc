#include <stdexcept>
#include <string>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>

#include <libmints/basisset.h>
#include <libmints/gshell.h>
#include <libmints/overlap.h>
#include <libmints/eri.h>
#include <libmints//wavefunction.h>
#include <physconst.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define EPS 1.0e-17
#define TABLESIZE 121

using namespace psi;

void calc_f(double *F, int n, double t)
{
    int i, m, k;
    int m2;
    double t2;
    double num;
    double sum;
    double term1, term2;
    static double K = 1.0/M_2_SQRTPI;
    double et;

    if (t>20.0){
        t2 = 2*t;
        et = exp(-t);
        t = sqrt(t);
        F[0] = K*erf(t)/t;
        for(m=0; m<=n-1; m++){
            F[m+1] = ((2*m + 1)*F[m] - et)/(t2);
        }
    }
    else {
        et = exp(-t);
        t2 = 2*t;
        m2 = 2*n;
        num = df[m2];
        i=0;
        sum = 1.0/(m2+1);
        do{
            i++;
            num = num*t2;
            term1 = num/df[m2+2*i+2];
            sum += term1;
        } while (fabs(term1) > EPS && i < MAX_FAC);
        F[n] = sum*et;
        for(m=n-1;m>=0;m--){
            F[m] = (t2*F[m+1] + et)/(2*m+1);
        }
    }
}

ERI::ERI(IntegralFactory* integral, Ref<BasisSet> &bs1, Ref<BasisSet> &bs2, Ref<BasisSet> &bs3, Ref<BasisSet> &bs4, int deriv)
    : TwoBodyInt(integral, bs1, bs2, bs3, bs4, deriv)
{
    // Initialize libint static data
    init_libint_base();
    if (deriv_)
        init_libderiv_base();

    // Figure out some information to initialize libint/libderiv with
    // 1. Maximum angular momentum
    int max_am = MAX(MAX(bs1->max_am(), bs2->max_am()), MAX(bs3->max_am(), bs4->max_am()));
    // 2. Maximum number of primitive combinations
    int max_nprim = bs1->max_nprimitive() * bs2->max_nprimitive() * bs3->max_nprimitive() * bs4->max_nprimitive();
    // 3. Maximum Cartesian class size
    max_cart_ = ioff[bs1->max_am()] * ioff[bs2->max_am()] * ioff[bs3->max_am()] * ioff[bs4->max_am()] +1;

    // Make sure libint is compiled to handle our max AM
    if (max_am >= LIBINT_MAX_AM) {
        fprintf(stderr, "ERROR: ERI - libint cannot handle angular momentum this high.\n       Recompile libint for higher angular momentum, then recompile this program.\n");
        abort();
    }
    if (deriv_ == 1 && max_am >= LIBDERIV_MAX_AM1) {
        fprintf(stderr, "ERROR: ERI - libderiv cannot handle angular momentum this high.\n     Recompile libderiv for higher angular momentum, then recompile this program.\n");
        abort();
    }

    // Initialize libint
    init_libint(&libint_, max_am, max_nprim);
    // and libderiv, if needed
    if (deriv_)
        init_libderiv1(&libderiv_, max_am, max_nprim, max_cart_-1);

    size_t size = INT_NCART(bs1->max_am()) * INT_NCART(bs2->max_am()) *
                  INT_NCART(bs3->max_am()) * INT_NCART(bs4->max_am());

    // Used in pure_transform
    tformbuf_ = new double[size];
    memset(tformbuf_, 0, sizeof(double)*size);

    if (deriv_ == 1)
        size *= 3*natom_;

    target_ = new double[size];
    memset(target_, 0, sizeof(double)*size);

    source_ = new double[size];
    memset(source_, 0, sizeof(double)*size);

    init_fjt(4*max_am + DERIV_LVL);
}

ERI::~ERI()
{
    delete[] tformbuf_;
    delete[] target_;
    delete[] source_;
    delete[] denom_;
    free_block(d_);
    free_libint(&libint_);
    if (deriv_)
        free_libderiv(&libderiv_);
}

// Taken from CINTS
void ERI::init_fjt(int max)
{
    int i,j;
    double denom,d2jmax1,r2jmax1,wval,d2wval,sum,term,rexpw;

    int n1 = max+7;
    int n2 = TABLESIZE;
    d_ = block_matrix(n1, n2);

    /* Tabulate the gamma function for t(=wval)=0.0. */
    denom = 1.0;
    for (i=0; i<n1; i++) {
        d_[i][0] = 1.0/denom;
        denom += 2.0;
    }

    /* Tabulate the gamma function from t(=wval)=0.1, to 12.0. */
    d2jmax1 = 2.0*(n1-1) + 1.0;
    r2jmax1 = 1.0/d2jmax1;
    for (i=1; i<TABLESIZE; i++) {
        wval = 0.1 * i;
        d2wval = 2.0 * wval;
        term = r2jmax1;
        sum = term;
        denom = d2jmax1;
        for (j=2; j<=200; j++) {
            denom = denom + 2.0;
            term = term * d2wval / denom;
            sum = sum + term;
            if (term <= 1.0e-15) break;
        }
        rexpw = exp(-wval);

      /* Fill in the values for the highest j gtable entries (top row). */
        d_[n1-1][i] = rexpw * sum;

      /* Work down the table filling in the rest of the column. */
        denom = d2jmax1;
        for (j=n1 - 2; j>=0; j--) {
            denom = denom - 2.0;
            d_[j][i] = (d_[j+1][i]*d2wval + rexpw)/denom;
        }
    }

    /* Form some denominators, so divisions can be eliminated below. */
    denom_ = new double[max+1];
    denom_[0] = 0.0;
    for (i=1; i<=max; i++) {
        denom_[i] = 1.0/(2*i - 1);
    }

    wval_infinity_ = 2*max + 37.0;
    itable_infinity_ = (int) (10 * wval_infinity_);
}

void ERI::int_fjt(double *F, int J, double wval)
{
    const double sqrpih =  0.886226925452758;
    const double coef2 =  0.5000000000000000;
    const double coef3 = -0.1666666666666667;
    const double coef4 =  0.0416666666666667;
    const double coef5 = -0.0083333333333333;
    const double coef6 =  0.0013888888888889;
    const double gfac30 =  0.4999489092;
    const double gfac31 = -0.2473631686;
    const double gfac32 =  0.321180909;
    const double gfac33 = -0.3811559346;
    const double gfac20 =  0.4998436875;
    const double gfac21 = -0.24249438;
    const double gfac22 =  0.24642845;
    const double gfac10 =  0.499093162;
    const double gfac11 = -0.2152832;
    const double gfac00 = -0.490;

    double wdif, d2wal, rexpw, /* denom, */ gval, factor, rwval, term;
    int i, itable, irange;

    /* Compute an index into the table. */
    /* The test is needed to avoid floating point exceptions for
    * large values of wval. */
    if (wval > wval_infinity_) {
        itable = itable_infinity_;
    }
    else {
        itable = (int) (10.0 * wval);
    }

    /* If itable is small enough use the table to compute int_fjttable. */
    if (itable < TABLESIZE) {

        wdif = wval - 0.1 * itable;

      /* Compute fjt for J. */
        F[J] = (((((coef6 * d_[J+6][itable]*wdif
            + coef5 * d_[J+5][itable])*wdif
            + coef4 * d_[J+4][itable])*wdif
            + coef3 * d_[J+3][itable])*wdif
            + coef2 * d_[J+2][itable])*wdif
            -  d_[J+1][itable])*wdif
            +  d_[J][itable];

      /* Compute the rest of the fjt. */
        d2wal = 2.0 * wval;
        rexpw = exp(-wval);
      /* denom = 2*J + 1; */
        for (i=J; i>0; i--) {
        /* denom = denom - 2.0; */
            F[i-1] = (d2wal*F[i] + rexpw)*denom_[i];
        }
    }
    /* If wval <= 2*J + 36.0, use the following formula. */
    else if (itable <= 20*J + 360) {
        rwval = 1.0/wval;
        rexpw = exp(-wval);

      /* Subdivide wval into 6 ranges. */
        irange = itable/30 - 3;
        if (irange == 1) {
            gval = gfac30 + rwval*(gfac31 + rwval*(gfac32 + rwval*gfac33));
            F[0] = sqrpih*sqrt(rwval) - rexpw*gval*rwval;
        }
        else if (irange == 2) {
            gval = gfac20 + rwval*(gfac21 + rwval*gfac22);
            F[0] = sqrpih*sqrt(rwval) - rexpw*gval*rwval;
        }
        else if (irange == 3 || irange == 4) {
            gval = gfac10 + rwval*gfac11;
            F[0] = sqrpih*sqrt(rwval) - rexpw*gval*rwval;
        }
        else if (irange == 5 || irange == 6) {
            gval = gfac00;
            F[0] = sqrpih*sqrt(rwval) - rexpw*gval*rwval;
        }
        else {
            F[0] = sqrpih*sqrt(rwval);
        }

      /* Compute the rest of the int_fjttable from table->d[0]. */
        factor = 0.5 * rwval;
        term = factor * rexpw;
        for (i=1; i<=J; i++) {
            F[i] = factor * F[i-1] - term;
            factor = rwval + factor;
        }
    }
    /* For large values of wval use this algorithm: */
    else {
        rwval = 1.0/wval;
        F[0] = sqrpih*sqrt(rwval);
        factor = 0.5 * rwval;
        for (i=1; i<=J; i++) {
            F[i] = factor * F[i-1];
            factor = rwval + factor;
        }
    }
}

void ERI::compute_shell(int sh1, int sh2, int sh3, int sh4)
{
#ifdef MINTS_TIMER
    timer_on("ERI::compute_shell");
#endif
    // Need to ensure the ordering asked by the user is valid for libint
    // compute_quartet does NOT check this. SEGFAULTS should occur if order
    // is not guaranteed.
    int s1, s2, s3, s4;
    int am1, am2, am3, am4, temp;
    Ref<BasisSet> bs_temp;
    
    bool p13p24 = false, p12 = false, p34 = false;

    // AM used for ordering
    am1 = original_bs1_->shell(sh1)->am(0);
    am2 = original_bs2_->shell(sh2)->am(0);
    am3 = original_bs3_->shell(sh3)->am(0);
    am4 = original_bs4_->shell(sh4)->am(0);

    int n1 = original_bs1_->shell(sh1)->nfunction(0);
    int n2 = original_bs2_->shell(sh2)->nfunction(0);
    int n3 = original_bs3_->shell(sh3)->nfunction(0);
    int n4 = original_bs4_->shell(sh4)->nfunction(0);

    // l(a) >= l(b), l(c) >= l(d), and l(c) + l(d) >= l(a) + l(b).
    if (am1 >= am2) {
        s1 = sh1;
        s2 = sh2;
        
        bs1_ = original_bs1_;
        bs2_ = original_bs2_;
    } else {
        s1 = sh2;
        s2 = sh1;
        
        bs1_ = original_bs2_;
        bs2_ = original_bs1_;
        
        p12 = true;
    }

    if (am3 >= am4) {
        s3 = sh3;
        s4 = sh4;
        
        bs3_ = original_bs3_;
        bs4_ = original_bs4_;
        
    } else {
        s3 = sh4;
        s4 = sh3;
        
        bs3_ = original_bs4_;
        bs4_ = original_bs3_;
        
        p34 = true;
    }

    if ((am1 + am2) > (am3 + am4)) {
        // Swap s1 and s2 with s3 and s4
        temp = s1;
        s1 = s3;
        s3 = temp;

        temp = s2;
        s2 = s4;
        s4 = temp;
        
        bs_temp = bs1_;
        bs1_ = bs3_;
        bs3_ = bs_temp;
        
        bs_temp = bs2_;
        bs2_ = bs4_;
        bs4_ = bs_temp;
        
        p13p24 = true;
    }
    
    // s1, s2, s3, s4 contain the shells to do in libint order
    compute_quartet(s1, s2, s3, s4);

    // Permute integrals back, if needed
    if (p12 || p34 || p13p24)
        permute_target(source_, target_, s1, s2, s3, s4, p12, p34, p13p24);
    else {
        // copy the integrals to the target_
        memcpy(target_, source_, n1 * n2 * n3 * n4 *sizeof(double));
    }
#ifdef MINTS_TIMER
    timer_off("ERI::compute_shell");
#endif
}

void ERI::compute_quartet(int sh1, int sh2, int sh3, int sh4)
{
    Ref<GaussianShell> s1, s2, s3, s4;

    s1 = bs1_->shell(sh1);
    s2 = bs2_->shell(sh2);
    s3 = bs3_->shell(sh3);
    s4 = bs4_->shell(sh4);

    int am1 = s1->am(0);
    int am2 = s2->am(0);
    int am3 = s3->am(0);
    int am4 = s4->am(0);
    int am = am1 + am2 + am3 + am4; // total am
    int nprim1 = s1->nprimitive();
    int nprim2 = s2->nprimitive();
    int nprim3 = s3->nprimitive();
    int nprim4 = s4->nprimitive();
    size_t nprim, nprim_combination = nprim1 * nprim2 * nprim3 * nprim4;
    double A[3], B[3], C[3], D[3];
    
    A[0] = s1->center()[0];
    A[1] = s1->center()[1];
    A[2] = s1->center()[2];
    B[0] = s2->center()[0];
    B[1] = s2->center()[1];
    B[2] = s2->center()[2];
    C[0] = s3->center()[0];
    C[1] = s3->center()[1];
    C[2] = s3->center()[2];
    D[0] = s4->center()[0];
    D[1] = s4->center()[1];
    D[2] = s4->center()[2];

    // compute intermediates
    double AB2 = 0.0;
    AB2 += (A[0] - B[0]) * (A[0] - B[0]);
    AB2 += (A[1] - B[1]) * (A[1] - B[1]);
    AB2 += (A[2] - B[2]) * (A[2] - B[2]);
    double CD2 = 0.0;
    CD2 += (C[0] - D[0]) * (C[0] - D[0]);
    CD2 += (C[1] - D[1]) * (C[1] - D[1]);
    CD2 += (C[2] - D[2]) * (C[2] - D[2]);

    libint_.AB[0] = A[0] - B[0];
    libint_.AB[1] = A[1] - B[1];
    libint_.AB[2] = A[2] - B[2];
    libint_.CD[0] = C[0] - D[0];
    libint_.CD[1] = C[1] - D[1];
    libint_.CD[2] = C[2] - D[2];

#ifdef MINTS_TIMER
    timer_on("Primitive setup");
#endif
    
    // Prepare all the data needed by libint
    nprim = 0;
    for (int p1=0; p1<nprim1; ++p1) {
        double a1 = s1->exp(p1);
        double c1 = s1->coef(0, p1);
        for (int p2=0; p2<nprim2; ++p2) {
            double a2 = s2->exp(p2);
            double c2 = s2->coef(0, p2);
            double zeta = a1 + a2;
            double ooz = 1.0/zeta;
            double oo2z = 1.0/(2.0 * zeta);

            double PA[3], PB[3];
            double P[3];

            P[0] = (a1*A[0] + a2*B[0])*ooz;
            P[1] = (a1*A[1] + a2*B[1])*ooz;
            P[2] = (a1*A[2] + a2*B[2])*ooz;
            PA[0] = P[0] - A[0];
            PA[1] = P[1] - A[1];
            PA[2] = P[2] - A[2];
            PB[0] = P[0] - B[0];
            PB[1] = P[1] - B[1];
            PB[2] = P[2] - B[2];

            double Sab = pow(M_PI*ooz, 3.0/2.0) * exp(-a1*a2*ooz*AB2) * c1 * c2;

            for (int p3=0; p3<nprim3; ++p3) {
                double a3 = s3->exp(p3);
                double c3 = s3->coef(0, p3);
                for (int p4=0; p4<nprim4; ++p4) {
                    double a4 = s4->exp(p4);
                    double c4 = s4->coef(0, p4);
                    double nu = a3 + a4;
                    double oon = 1.0/nu;
                    double oo2n = 1.0/(2.0*nu);
                    double oo2zn = 1.0/(2.0*(zeta+nu));
                    double rho = (zeta*nu)/(zeta+nu);
                    double oo2rho = 1 / (2.0*rho);

                    double QC[3], QD[3], WP[3], WQ[3], PQ[3];
                    double Q[3], W[3];

                    Q[0] = (a3*C[0] + a4*D[0])*oon;
                    Q[1] = (a3*C[1] + a4*D[1])*oon;
                    Q[2] = (a3*C[2] + a4*D[2])*oon;
                    QC[0] = Q[0] - C[0];
                    QC[1] = Q[1] - C[1];
                    QC[2] = Q[2] - C[2];
                    QD[0] = Q[0] - D[0];
                    QD[1] = Q[1] - D[1];
                    QD[2] = Q[2] - D[2];
                    PQ[0] = P[0] - Q[0];
                    PQ[1] = P[1] - Q[1];
                    PQ[2] = P[2] - Q[2];

                    double PQ2 = 0.0;
                    PQ2 += (P[0] - Q[0]) * (P[0] - Q[0]);
                    PQ2 += (P[1] - Q[1]) * (P[1] - Q[1]);
                    PQ2 += (P[2] - Q[2]) * (P[2] - Q[2]);

                    W[0] = (zeta*P[0] + nu*Q[0]) / (zeta + nu);
                    W[1] = (zeta*P[1] + nu*Q[1]) / (zeta + nu);
                    W[2] = (zeta*P[2] + nu*Q[2]) / (zeta + nu);
                    WP[0] = W[0] - P[0];
                    WP[1] = W[1] - P[1];
                    WP[2] = W[2] - P[2];
                    WQ[0] = W[0] - Q[0];
                    WQ[1] = W[1] - Q[1];
                    WQ[2] = W[2] - Q[2];

                    for (int i=0; i<3; ++i) {
                        libint_.PrimQuartet[nprim].U[0][i] = PA[i];
                        libint_.PrimQuartet[nprim].U[2][i] = QC[i];
                        libint_.PrimQuartet[nprim].U[4][i] = WP[i];
                        libint_.PrimQuartet[nprim].U[5][i] = WQ[i];
                    }
                    libint_.PrimQuartet[nprim].oo2z = oo2z;
                    libint_.PrimQuartet[nprim].oo2n = oo2n;
                    libint_.PrimQuartet[nprim].oo2zn = oo2zn;
                    libint_.PrimQuartet[nprim].poz = rho * ooz;
                    libint_.PrimQuartet[nprim].pon = rho * oon;
                    libint_.PrimQuartet[nprim].oo2p = oo2rho;

                    double T = rho * PQ2;
                    calc_f(libint_.PrimQuartet[nprim].F, am+1, T);

                    // Modify F to include overlap of ab and cd, eqs 14, 15, 16 of libint manual
                    double Scd = pow(M_PI*oon, 3.0/2.0) * exp(-a3*a4*oon*CD2) * c3 * c4;
                    double val = 2.0 * sqrt(rho * M_1_PI) * Sab * Scd;
                    for (int i=0; i<=am; ++i) {
                        libint_.PrimQuartet[nprim].F[i] *= val;
                    }
                    nprim++;
                }
            }
        }
    }

#ifdef MINTS_TIMER
    timer_off("Primitive setup");
#endif

    // How many are there?
    size_t size = INT_NCART(am1) * INT_NCART(am2) * INT_NCART(am3) * INT_NCART(am4);

#ifdef MINTS_TIMER
    timer_on("libint overhead");
#endif

    // Compute the integral
    if (am) {
        REALTYPE *target_ints;

        target_ints = build_eri[am1][am2][am3][am4](&libint_, nprim);

        memcpy(source_, target_ints, sizeof(double)*size);
    } else {
        // Handle (ss|ss)
        double temp = 0.0;
        for (size_t i=0; i<nprim_combination; ++i)
            temp += (double)libint_.PrimQuartet[i].F[0];
        source_[0] = temp;
    }

#ifdef MINTS_TIMER
    timer_off("libint overhead");
#endif

    // The following two functions time themselves.
    
    // Normalize the integrals for angular momentum
    normalize_am(s1, s2, s3, s4);

    // Transform the integrals to the spherical basis
    pure_transform(sh1, sh2, sh3, sh4, 1);

    // Results are in source_
}

void ERI::compute_shell_deriv1(int sh1, int sh2, int sh3, int sh4)
{
    if (deriv_ >= 1) {
        fprintf(stderr, "ERROR - ERI: ERI object not initialized to handle derivatives.\n");
        abort();
    }
    // Need to ensure the ordering asked by the user is valid for libint
    // compute_quartet does NOT check this. SEGFAULTS should occur if order
    // is not guaranteed.
    int s1, s2, s3, s4;
    int am1, am2, am3, am4, temp;
    Ref<BasisSet> bs_temp;
    bool p13p24 = false, p12 = false, p34 = false;

    // AM used for ordering
    am1 = original_bs1_->shell(sh1)->am(0);
    am2 = original_bs2_->shell(sh2)->am(0);
    am3 = original_bs3_->shell(sh3)->am(0);
    am4 = original_bs4_->shell(sh4)->am(0);

    int n1 = original_bs1_->shell(sh1)->nfunction(0);
    int n2 = original_bs2_->shell(sh2)->nfunction(0);
    int n3 = original_bs3_->shell(sh3)->nfunction(0);
    int n4 = original_bs4_->shell(sh4)->nfunction(0);

    // l(a) >= l(b), l(c) >= l(d), and l(c) + l(d) >= l(a) + l(b).
    if (am1 >= am2) {
        s1 = sh1;
        s2 = sh2;
        
        bs1_ = original_bs1_;
        bs2_ = original_bs2_;
    } else {
        s1 = sh2;
        s2 = sh1;
        
        bs1_ = original_bs2_;
        bs2_ = original_bs1_;
        
        p12 = true;
    }

    if (am3 >= am4) {
        s3 = sh3;
        s4 = sh4;
        
        bs3_ = original_bs3_;
        bs4_ = original_bs4_;
        
    } else {
        s3 = sh4;
        s4 = sh3;
        
        bs3_ = original_bs4_;
        bs4_ = original_bs3_;
        
        p34 = true;
    }

    if ((am1 + am2) > (am3 + am4)) {
        // Swap s1 and s2 with s3 and s4
        temp = s1;
        s1 = s3;
        s3 = temp;

        temp = s2;
        s2 = s4;
        s4 = temp;
        
        bs_temp = bs1_;
        bs1_ = bs3_;
        bs3_ = bs_temp;
        
        bs_temp = bs2_;
        bs2_ = bs4_;
        bs4_ = bs_temp;
        
        p13p24 = true;
    }
    
    // s1, s2, s3, s4 contain the shells to do in libderive order
    compute_quartet_deriv1(s1, s2, s3, s4);    // compute 9 sets of integral derivatives

    size_t size = n1 * n2 * n3 * n4;
    // Permute integrals back, if needed
    if (p12 || p34 || p13p24) {
        // 3n of them
        for (int i=0; i<3*natom_; ++i)
            permute_target(source_+(i*size), target_+(i*size), s1, s2, s3, s4, p12, p34, p13p24);
    }
    else {
        // copy the integrals to the target_, 3n of them
        memcpy(target_, source_, 3 * natom_ * size *sizeof(double));
    }
}

void ERI::compute_quartet_deriv1(int sh1, int sh2, int sh3, int sh4)
{
    Ref<GaussianShell> s1, s2, s3, s4;

    s1 = bs1_->shell(sh1);
    s2 = bs2_->shell(sh2);
    s3 = bs3_->shell(sh3);
    s4 = bs4_->shell(sh4);

    int am1 = s1->am(0);
    int am2 = s2->am(0);
    int am3 = s3->am(0);
    int am4 = s4->am(0);
    int am = am1 + am2 + am3 + am4; // total am
    int nprim1 = s1->nprimitive();
    int nprim2 = s2->nprimitive();
    int nprim3 = s3->nprimitive();
    int nprim4 = s4->nprimitive();
    size_t nprim;
    double A[3], B[3], C[3], D[3];
    A[0] = s1->center()[0];
    A[1] = s1->center()[1];
    A[2] = s1->center()[2];
    B[0] = s2->center()[0];
    B[1] = s2->center()[1];
    B[2] = s2->center()[2];
    C[0] = s3->center()[0];
    C[1] = s3->center()[1];
    C[2] = s3->center()[2];
    D[0] = s4->center()[0];
    D[1] = s4->center()[1];
    D[2] = s4->center()[2];

    // compute intermediates
    double AB2 = 0.0;
    AB2 += (A[0] - B[0]) * (A[0] - B[0]);
    AB2 += (A[1] - B[1]) * (A[1] - B[1]);
    AB2 += (A[2] - B[2]) * (A[2] - B[2]);
    double CD2 = 0.0;
    CD2 += (C[0] - D[0]) * (C[0] - D[0]);
    CD2 += (C[1] - D[1]) * (C[1] - D[1]);
    CD2 += (C[2] - D[2]) * (C[2] - D[2]);

    libderiv_.AB[0] = A[0] - B[0];
    libderiv_.AB[1] = A[1] - B[1];
    libderiv_.AB[2] = A[2] - B[2];
    libderiv_.CD[0] = C[0] - D[0];
    libderiv_.CD[1] = C[1] - D[1];
    libderiv_.CD[2] = C[2] - D[2];

    // Prepare all the data needed by libint
    nprim = 0;
    for (int p1=0; p1<nprim1; ++p1) {
        double a1 = s1->exp(p1);
        double c1 = s1->coef(0, p1);
        for (int p2=0; p2<nprim2; ++p2) {
            double a2 = s2->exp(p2);
            double c2 = s2->coef(0, p2);
            double zeta = a1 + a2;
            double ooz = 1.0/zeta;
            double oo2z = 1.0/(2.0 * zeta);

            double PA[3], PB[3];
            double P[3];

            P[0] = (a1*A[0] + a2*B[0])*ooz;
            P[1] = (a1*A[1] + a2*B[1])*ooz;
            P[2] = (a1*A[2] + a2*B[2])*ooz;
            PA[0] = P[0] - A[0];
            PA[1] = P[1] - A[1];
            PA[2] = P[2] - A[2];
            PB[0] = P[0] - B[0];
            PB[1] = P[1] - B[1];
            PB[2] = P[2] - B[2];

            double Sab = pow(M_PI*ooz, 3.0/2.0) * exp(-a1*a2*ooz*AB2) * c1 * c2;

            for (int p3=0; p3<nprim3; ++p3) {
                double a3 = s3->exp(p3);
                double c3 = s3->coef(0, p3);
                for (int p4=0; p4<nprim4; ++p4) {
                    double a4 = s4->exp(p4);
                    double c4 = s4->coef(0, p4);
                    double nu = a3 + a4;
                    double oon = 1.0/nu;
                    double oo2n = 1.0/(2.0*nu);
                    double oo2zn = 1.0/(2.0*(zeta+nu));
                    double rho = (zeta*nu)/(zeta+nu);
                    double oo2rho = 1 / (2.0*rho);

                    double QC[3], QD[3], WP[3], WQ[3], PQ[3];
                    double Q[3], W[3];

                    Q[0] = (a3*C[0] + a4*D[0])*oon;
                    Q[1] = (a3*C[1] + a4*D[1])*oon;
                    Q[2] = (a3*C[2] + a4*D[2])*oon;
                    QC[0] = Q[0] - C[0];
                    QC[1] = Q[1] - C[1];
                    QC[2] = Q[2] - C[2];
                    QD[0] = Q[0] - D[0];
                    QD[1] = Q[1] - D[1];
                    QD[2] = Q[2] - D[2];
                    PQ[0] = P[0] - Q[0];
                    PQ[1] = P[1] - Q[1];
                    PQ[2] = P[2] - Q[2];

                    double PQ2 = 0.0;
                    PQ2 += (P[0] - Q[0]) * (P[0] - Q[0]);
                    PQ2 += (P[1] - Q[1]) * (P[1] - Q[1]);
                    PQ2 += (P[2] - Q[2]) * (P[2] - Q[2]);

                    W[0] = (zeta*P[0] + nu*Q[0]) / (zeta + nu);
                    W[1] = (zeta*P[1] + nu*Q[1]) / (zeta + nu);
                    W[2] = (zeta*P[2] + nu*Q[2]) / (zeta + nu);
                    WP[0] = W[0] - P[0];
                    WP[1] = W[1] - P[1];
                    WP[2] = W[2] - P[2];
                    WQ[0] = W[0] - Q[0];
                    WQ[1] = W[1] - Q[1];
                    WQ[2] = W[2] - Q[2];

                    for (int i=0; i<3; ++i) {
                        libderiv_.PrimQuartet[nprim].U[0][i] = PA[i];
                        libderiv_.PrimQuartet[nprim].U[1][i] = PB[i];
                        libderiv_.PrimQuartet[nprim].U[2][i] = QC[i];
                        libderiv_.PrimQuartet[nprim].U[3][i] = QD[i];
                        libderiv_.PrimQuartet[nprim].U[4][i] = WP[i];
                        libderiv_.PrimQuartet[nprim].U[5][i] = WQ[i];
                    }
                    libderiv_.PrimQuartet[nprim].oo2z = oo2z;
                    libderiv_.PrimQuartet[nprim].oo2n = oo2n;
                    libderiv_.PrimQuartet[nprim].oo2zn = oo2zn;
                    libderiv_.PrimQuartet[nprim].poz = rho * ooz;
                    libderiv_.PrimQuartet[nprim].pon = rho * oon;
                    // libderiv_.PrimQuartet[nprim].oo2p = oo2rho;   // NOT SET IN CINTS
                    libderiv_.PrimQuartet[nprim].twozeta_a = 2.0 * a1;
                    libderiv_.PrimQuartet[nprim].twozeta_b = 2.0 * a2;
                    libderiv_.PrimQuartet[nprim].twozeta_c = 2.0 * a3;
                    libderiv_.PrimQuartet[nprim].twozeta_d = 2.0 * a4;

                    double T = rho * PQ2;
                    int_fjt(libderiv_.PrimQuartet[nprim].F, am+DERIV_LVL, T);

                    // Modify F to include overlap of ab and cd, eqs 14, 15, 16 of libint manual
                    double Scd = pow(M_PI*oon, 3.0/2.0) * exp(-a3*a4*oon*CD2) * c3 * c4;
                    double val = 2.0 * sqrt(rho * M_1_PI) * Sab * Scd;
                    for (int i=0; i<=am+DERIV_LVL; ++i) {
                        libderiv_.PrimQuartet[nprim].F[i] *= val;
                    }

                    nprim++;
                }
            }
        }
    }

    // How many are there?
    size_t size = INT_NCART(am1) * INT_NCART(am2) * INT_NCART(am3) * INT_NCART(am4);

    // Compute the integral
    build_deriv1_eri[am1][am2][am3][am4](&libderiv_, nprim);

    // Sum results into derivs_
    int center_i = s1->ncenter()*3*size;
    int center_j = s2->ncenter()*3*size;
    int center_k = s3->ncenter()*3*size;
    int center_l = s4->ncenter()*3*size;

    // Zero out memory
    memset(source_, 0, sizeof(double) * size * 3 * natom_);

    for (size_t i=0; i < size; ++i) {
        source_[center_i+(0*size)+i] += libderiv_.ABCD[0][i];
        source_[center_i+(1*size)+i] += libderiv_.ABCD[1][i];
        source_[center_i+(2*size)+i] += libderiv_.ABCD[2][i];

        // Use translational invariance to determine center_j derivatives
        source_[center_j+(0*size)+i] -= (libderiv_.ABCD[0][i] + libderiv_.ABCD[6][i] + libderiv_.ABCD[9][i]);
        source_[center_j+(1*size)+i] -= (libderiv_.ABCD[1][i] + libderiv_.ABCD[7][i] + libderiv_.ABCD[10][i]);
        source_[center_j+(2*size)+i] -= (libderiv_.ABCD[2][i] + libderiv_.ABCD[8][i] + libderiv_.ABCD[11][i]);

        source_[center_k+(0*size)+i] += libderiv_.ABCD[6][i];
        source_[center_k+(1*size)+i] += libderiv_.ABCD[7][i];
        source_[center_k+(2*size)+i] += libderiv_.ABCD[8][i];

        source_[center_l+(0*size)+i] += libderiv_.ABCD[9][i];
        source_[center_l+(1*size)+i] += libderiv_.ABCD[10][i];
        source_[center_l+(2*size)+i] += libderiv_.ABCD[11][i];
    }

    // Normalize the 3n sets of integrals
    normalize_am(s1, s2, s3, s4, 3*natom_);

    // Transform the integrals to the spherical basis
    pure_transform(sh1, sh2, sh3, sh4, 3*natom_);

    // Results are in source_
}

