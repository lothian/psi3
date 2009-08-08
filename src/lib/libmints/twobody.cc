#include <stdexcept>

#include <libqt/qt.h>

#include <libmints/twobody.h>
#include <libmints/basisset.h>
#include <libmints/gshell.h>
#include <libmints/integral.h>

using namespace psi;

static void transform2e_1(int, SphericalTransformIter&, double*, double*, int);
static void transform2e_2(int, SphericalTransformIter&, double*, double*, int, int, int);
static void transform2e_3(int, SphericalTransformIter&, double*, double*, int, int, int);
static void transform2e_4(int, SphericalTransformIter&, double*, double*, int, int);

TwoBodyInt::TwoBodyInt(IntegralFactory *integral,
                       BasisSet* bs1,
                       BasisSet* bs2,
                       BasisSet* bs3,
                       BasisSet* bs4,
                       int deriv)
    : integral_(integral), original_bs1_(bs1), original_bs2_(bs2), original_bs3_(bs3),
      original_bs4_(bs4), deriv_(deriv)
{
    target_ = 0;
    tformbuf_ = 0;
    source_ = 0;
    natom_ = original_bs1_->molecule()->natom();  // This assumes the 4 bases come from the same molecule.
    
    // compute the maximum number of unique quartets needed for a given i, by 
    // using the max_stability_index_ of j, k, and l.
    max_unique_quartets_ = original_bs2_->max_stability_index() * original_bs3_->max_stability_index() * original_bs4_->max_stability_index();
}

TwoBodyInt::~TwoBodyInt()
{

}

BasisSet* TwoBodyInt::basis()
{
    return original_bs1_;
}

BasisSet* TwoBodyInt::basis1()
{
    return original_bs1_;
}

BasisSet* TwoBodyInt::basis2()
{
    return original_bs2_;
}

BasisSet* TwoBodyInt::basis3()
{
    return original_bs3_;
}

BasisSet* TwoBodyInt::basis4()
{
    return original_bs4_;
}

bool TwoBodyInt::cloneable()
{
    return false;
}

TwoBodyInt* TwoBodyInt::clone()
{
    throw std::runtime_error("TwoBodyInt::clone() not implemented");
}

void TwoBodyInt::normalize_am(GaussianShell* s1, GaussianShell* s2, GaussianShell* s3, GaussianShell* s4, int nchunk)
{
#ifdef MINTS_TIMER
    timer_on("Angular momentum normalization");
#endif
    int am1 = s1->am(0);
    int am2 = s2->am(0);
    int am3 = s3->am(0);
    int am4 = s4->am(0);
    int length = INT_NCART(am1) * INT_NCART(am2) * INT_NCART(am3) * INT_NCART(am4);
    
    // Need to go through and grab all the integrals for this given shell and add them
    // to the running totals.
    int nprim = 0;
    for (int i = 0; i <= am1; i++) {
        int l1 = am1 - i;
        for (int j = 0; j <= i; j++) {
            int m1 = i - j;
            int n1 = j;
            double norm_a = s1->normalize(l1, m1, n1);

            for (int k = 0; k <= am2; k++) {
                int l2 = am2 - k;
                for (int l = 0; l <= k; l++) {
                    int m2 = k - l;
                    int n2 = l;
                    double norm_b = s2->normalize(l2, m2, n2);

                    for (int m = 0; m <= am3; m++) {
                        int l3 = am3 - m;
                        for (int n = 0; n <= m; n++) {
                            int m3 = m - n;
                            int n3 = n;
                            double norm_c = s3->normalize(l3, m3, n3);

                            for (int o = 0; o <= am4; o++) {
                                int l4 = am4 - o;
                                for (int p = 0; p <= o; p++) {
                                    int m4 = o - p;
                                    int n4 = p;
                                    double norm_d = s4->normalize(l4, m4, n4);

                                    // printf("normalization %f %f %f %f\n", norm_a, norm_b, norm_c, norm_d);
                                    for (int chunk=0; chunk < nchunk; ++chunk) {
                                        source_[nprim+(chunk*length)] *= norm_a * norm_b * norm_c * norm_d;
                                    }
                                    nprim++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
#ifdef MINTS_TIMER
    timer_off("Angular momentum normalization");
#endif
}

void TwoBodyInt::permute_target(double *s, double *t, int sh1, int sh2, int sh3, int sh4, bool p12, bool p34, bool p13p24)
{
#ifdef MINTS_TIMER
    timer_on("Permute target");
#endif
    GaussianShell *s1, *s2, *s3, *s4;

    s1 = bs1_->shell(sh1);
    s2 = bs2_->shell(sh2);
    s3 = bs3_->shell(sh3);
    s4 = bs4_->shell(sh4);
      
    int nbf1 = s1->nfunction(0);
    int nbf2 = s2->nfunction(0);
    int nbf3 = s3->nfunction(0);
    int nbf4 = s4->nfunction(0);
    
    if (!p13p24) {
        if (p12) {
            if (p34) {
                permute_1234_to_2143(s, t, nbf1, nbf2, nbf3, nbf4);
            } else {
                permute_1234_to_2134(s, t, nbf1, nbf2, nbf3, nbf4);
            }
        } else {
            permute_1234_to_1243(s, t, nbf1, nbf2, nbf3, nbf4);
        }
    } else {
        if (p12) {
            if (p34) {
                permute_1234_to_4321(s, t, nbf1, nbf2, nbf3, nbf4);
            } else {
                permute_1234_to_4312(s, t, nbf1, nbf2, nbf3, nbf4);
            }
        } else {
            if (p34) {
                permute_1234_to_3421(s, t, nbf1, nbf2, nbf3, nbf4);
            } else {
                permute_1234_to_3412(s, t, nbf1, nbf2, nbf3, nbf4);
            }
        }
    }
#ifdef MINTS_TIMER
    timer_off("Permute target");
#endif
}

void TwoBodyInt::permute_1234_to_1243(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf1;
    int f2=nbf2;
    int f3=nbf4;
    int f4=nbf3;
    for (int bf1=0; bf1<f1; bf1++) {
        for (int bf2=0; bf2<f2; bf2++) {
            for (int bf4=0; bf4<f4; bf4++) {
                for (int bf3=0; bf3<f3; bf3++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }    
}

void TwoBodyInt::permute_1234_to_2134(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf2;
    int f2=nbf1;
    int f3=nbf3;
    int f4=nbf4;    
    for (int bf2=0; bf2<f2; bf2++) {
        for (int bf1=0; bf1<f1; bf1++) {
            for (int bf3=0; bf3<f3; bf3++) {
                for (int bf4=0; bf4<f4; bf4++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }    
}

void TwoBodyInt::permute_1234_to_2143(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf2;
    int f2=nbf1;
    int f3=nbf4;
    int f4=nbf3;    
    for (int bf2=0; bf2<f2; bf2++) {
        for (int bf1=0; bf1<f1; bf1++) {
            for (int bf4=0; bf4<f4; bf4++) {
                for (int bf3=0; bf3<f3; bf3++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }    
}

void TwoBodyInt::permute_1234_to_3412(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf3;
    int f2=nbf4;
    int f3=nbf1;
    int f4=nbf2;    
    for (int bf3=0; bf3<f3; bf3++) {
        for (int bf4=0; bf4<f4; bf4++) {
            for (int bf1=0; bf1<f1; bf1++) {
                for (int bf2=0; bf2<f2; bf2++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }
}

void TwoBodyInt::permute_1234_to_4312(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf4;
    int f2=nbf3;
    int f3=nbf1;
    int f4=nbf2;
    for (int bf3=0; bf3<f3; bf3++) {
        for (int bf4=0; bf4<f4; bf4++) {
            for (int bf2=0; bf2<f2; bf2++) {
                for (int bf1=0; bf1<f1; bf1++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }
}

void TwoBodyInt::permute_1234_to_3421(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf3;
    int f2=nbf4;
    int f3=nbf2;
    int f4=nbf1;
    for (int bf4=0; bf4<f4; bf4++) {
        for (int bf3=0; bf3<f3; bf3++) {
            for (int bf1=0; bf1<f1; bf1++) {
                for (int bf2=0; bf2<f2; bf2++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }    
}

void TwoBodyInt::permute_1234_to_4321(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4)
{
    int f1=nbf4;
    int f2=nbf3;
    int f3=nbf2;
    int f4=nbf1;
    for (int bf4=0; bf4<f4; bf4++) {
        for (int bf3=0; bf3<f3; bf3++) {
            for (int bf2=0; bf2<f2; bf2++) {
                for (int bf1=0; bf1<f1; bf1++) {
                    double *t_ptr = t + ((bf1*f2 + bf2)*f3 + bf3)*f4 + bf4;
                    *(t_ptr) = *(s++);
                }
            }
        }
    }    
}

void TwoBodyInt::pure_transform(int sh1, int sh2, int sh3, int sh4, int nchunk)
{
#ifdef MINTS_TIMER
    timer_on("Pure transformation");
#endif
    GaussianShell *s1, *s2, *s3, *s4;
    
    s1 = bs1_->shell(sh1);
    s2 = bs2_->shell(sh2);
    s3 = bs3_->shell(sh3);
    s4 = bs4_->shell(sh4);
    
    // Get the transforms from the basis set
    SphericalTransformIter trans1(bs1_->spherical_transform(s1->am(0)));
    SphericalTransformIter trans2(bs2_->spherical_transform(s2->am(0)));
    SphericalTransformIter trans3(bs3_->spherical_transform(s3->am(0)));
    SphericalTransformIter trans4(bs4_->spherical_transform(s4->am(0)));
    
    // Get the angular momentum for each shell
    int am1 = s1->am(0);
    int am2 = s2->am(0);
    int am3 = s3->am(0);
    int am4 = s4->am(0);
    
    // Get number of Cartesian functions for each shell
    int nao1 = s1->ncartesian();
    int nao2 = s2->ncartesian();
    int nao3 = s3->ncartesian();
    int nao4 = s4->ncartesian();

    int nbf1 = s1->nfunction(0);
    int nbf2 = s2->nfunction(0);
    int nbf3 = s3->nfunction(0);
    int nbf4 = s4->nfunction(0);
    
    // Get if each shell has pure functions
    bool is_pure1 = s1->is_pure(0);
    bool is_pure2 = s2->is_pure(0);
    bool is_pure3 = s3->is_pure(0);
    bool is_pure4 = s4->is_pure(0);

    for (int ichunk=0; ichunk < nchunk; ++ichunk) {
        // Compute the offset in source_, and target
        size_t sourcechunkoffset = ichunk * (nao1 * nao2 * nao3 * nao4);
        size_t targetchunkoffset = ichunk * (nbf1 * nbf2 * nbf3 * nbf4);
        double *source1, *target1;
        double *source2, *target2;
        double *source3, *target3;
        double *source4, *target4;
        double *source = source_+sourcechunkoffset;
        double *target = target_+targetchunkoffset;
        double *tmpbuf = tformbuf_;

        int transform_index = 8*is_pure1 + 4*is_pure2 + 2*is_pure3 + is_pure4;
        switch (transform_index) {
    	    case 0:
                break;
            
    	    case 1:
                source4 = source;
                target4 = target;
                break;
            
    	    case 2:
                source3 = source;
                target3 = target;
                break;
            
    	    case 3:
                source4 = source;
                target4 = tmpbuf;
                source3 = tmpbuf;
                target3 = target;
                break;
            
    	    case 4:
                source2 = source;
                target2 = target;
                break;
            
    	    case 5:
                source4 = source;
                target4 = tmpbuf;
                source2 = tmpbuf;
                target2 = target;
                break;
            
    	    case 6:
                source3 = source;
                target3 = tmpbuf;
                source2 = tmpbuf;
                target2 = target;
                break;
            
    	    case 7:
                source4 = source;
                target4 = tmpbuf;
                source3 = tmpbuf;
                target3 = source;
                source2 = source;
                target2 = target;
                break;
            
    	    case 8:
                source1 = source;
                target1 = target;
                break;
            
    	    case 9:
                source4 = source;
                target4 = tmpbuf;
                source1 = tmpbuf;
                target1 = target;
                break;
            
    	    case 10:
                source3 = source;
                target3 = tmpbuf;
                source1 = tmpbuf;
                target1 = target;
                break;
            
    	    case 11:
                source4 = source;
                target4 = tmpbuf;
                source3 = tmpbuf;
                target3 = source;
                source1 = source;
                target1 = target;
                break;
            
    	    case 12:
                source2 = source;
                target2 = tmpbuf;
                source1 = tmpbuf;
                target1 = target;
                break;
            
    	    case 13:
                source4 = source;
                target4 = tmpbuf;
                source2 = tmpbuf;
                target2 = source;
                source1 = source;
                target1 = target;
                break;
            
    	    case 14:
                source3 = source;
                target3 = tmpbuf;
                source2 = tmpbuf;
                target2 = source;
                source1 = source;
                target1 = target;
                break;
            
    	    case 15:
                source4 = source;
                target4 = tmpbuf;
                source3 = tmpbuf;
                target3 = source;
                source2 = source;
                target2 = tmpbuf;
                source1 = tmpbuf;
                target1 = target;
                break;
        }
    
        size_t size = 1;
        if (is_pure4) {
            transform2e_4(am4, trans4, source4, target4, nao1*nao2*nao3,nao4);
            size *= nbf4;
        }
        if (is_pure3) {
            transform2e_3(am3, trans3, source3, target3, nao1*nao2,nao3,nbf4);
            size *= nbf3;
        }
        if (is_pure2) {
            transform2e_2(am2, trans2, source2, target2, nao1,nao2,nbf3*nbf4);
            size *= nbf2;
        }
        if (is_pure1) {
            transform2e_1(am1, trans1, source1, target1, nbf2*nbf3*nbf4);
            size *= nbf1;
        }
    
        // The permute indices routines depend on the integrals being in source_
        if (is_pure1 || is_pure2 || is_pure3 || is_pure4)
            memcpy(source, target, size * sizeof(double));
    }
#ifdef MINTS_TIMER
    timer_off("Pure transformation");
#endif
}

static void transform2e_1(int am, SphericalTransformIter& sti, double *s, double *t, int njkl)
{
    memset(t,0,INT_NPURE(am)*njkl*sizeof(double));

    for (sti.first(); sti.is_done(); sti.next()) {
        double *sptr = s + sti.cartindex()*njkl;
        double *tptr = t + sti.pureindex()*njkl;
        double coef = sti.coef();
        for(int jkl=0; jkl<njkl; jkl++)
            *(tptr++) += coef * *(sptr++);
    }
}

static void transform2e_2(int am, SphericalTransformIter& sti, double *s, double *t, int ni, int nj, int nkl)
{
    int sj = INT_NPURE(am);
    const int sjkl = nj*nkl;
    const int tjkl = sj*nkl;

    memset(t,0,ni*tjkl*sizeof(double));

    for (sti.first(); sti.is_done(); sti.next()) {
        double *sptr = s + sti.cartindex()*nkl;
        double *tptr = t + sti.pureindex()*nkl;
        double coef = sti.coef();
        for(int i=0; i<ni; i++,sptr+=sjkl,tptr+=tjkl) {
            for(int kl=0; kl<nkl; kl++)
                tptr[kl] += coef * sptr[kl];
        }
    }
}

static void transform2e_3(int am, SphericalTransformIter& sti, double *s, double *t, int nij, int nk, int nl)
{
    int sk = INT_NPURE(am);
    const int skl = nk*nl;
    const int tkl = sk*nl;

    memset(t,0,nij*tkl*sizeof(double));

    for (sti.first(); sti.is_done(); sti.next()) {
        double *sptr = s + sti.cartindex()*nl;
        double *tptr = t + sti.pureindex()*nl;
        // printf("cartindex = %d, pureindex = %d\n", sti.cartindex(), sti.pureindex());
        double coef = sti.coef();
        for(int ij=0; ij<nij; ij++,sptr+=skl,tptr+=tkl) {
            for(int l=0; l<nl; l++)
                tptr[l] += coef * sptr[l];
        }
    }
}

// am => angular momentum of l
// sti => spherical tranformation iterator
// s => source integrals buffer
// t => target buffer
// nijk => how many i, j, k combinations are there?
// nl => how man l's are there?
static void transform2e_4(int am, SphericalTransformIter& sti, double *s, double *t, int nijk, int nl)
{
    // Protect ourselves
    const int sl = nl;
    const int tl = INT_NPURE(am);
    
    // Clear out target memory
    memset(t, 0, nijk*tl*sizeof(double));
    
    for (sti.first(); sti.is_done(); sti.next()) {
        // Starting point in source and target buffers
        double *sptr = s + sti.cartindex();
        double *tptr = t + sti.pureindex();
        
        // What's the coefficient we're using
        double coef = sti.coef();
        for (int ijk=0; ijk<nijk; ++ijk) {
            // Add contribution of the source to the target
            *(tptr) += coef * *(sptr);
            
            // skip ahead to the next ijk
            sptr += sl;
            tptr += tl;
        }
    }
}
