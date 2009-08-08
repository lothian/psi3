#ifndef _psi_src_lib_libmints_twobody_h
#define _psi_src_lib_libmints_twobody_h

/*!
    \file libmints/twobody.h
    \ingroup MINTS
*/

#include <libmints/ref.h>
#include <libmints/matrix.h>

namespace psi {
    
class IntegralFactory;
class BasisSet;
class GaussianShell;

//! Two body integral base class.
class TwoBodyInt
{
protected:
    IntegralFactory *integral_;
    BasisSet *bs1_;
    BasisSet *bs2_;
    BasisSet *bs3_;
    BasisSet *bs4_;

    BasisSet *original_bs1_;
    BasisSet *original_bs2_;
    BasisSet *original_bs3_;
    BasisSet *original_bs4_;
    
    /// Buffer to hold the final integrals.
    double *target_;
    /// Buffer to hold the transformation intermediates.
    double *tformbuf_;
    /// Buffer to hold the initially computed integrals.
    double *source_;
    /// Maximum number of unique quartets needed to compute a set of SO's
    int max_unique_quartets_;
    /// Number of atoms.
    int natom_;
    /// Derivative level.
    int deriv_;
    
    void permute_target(double *s, double *t, int sh1, int sh2, int sh3, int sh4, bool p12, bool p34, bool p13p24);
    void permute_1234_to_1243(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    void permute_1234_to_2134(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    void permute_1234_to_2143(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    void permute_1234_to_3412(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    void permute_1234_to_4312(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    void permute_1234_to_3421(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    void permute_1234_to_4321(double *s, double *t, int nbf1, int nbf2, int nbf3, int nbf4);
    
    TwoBodyInt(IntegralFactory *integral,
               BasisSet* bs1,
               BasisSet* bs2,
               BasisSet* bs3,
               BasisSet* bs4,
               int deriv = 0);
               
public:
    virtual ~TwoBodyInt();
    
    /// Basis set on center one
    BasisSet* basis();
    /// Basis set on center one
    BasisSet* basis1();
    /// Basis set on center two
    BasisSet* basis2();
    /// Basis set on center three
    BasisSet* basis3();
    /// Basis set on center four
    BasisSet* basis4();

    /// Buffer where the integrals are placed
    const double *buffer() const { return target_; };
    
    /// Compute the integrals
    virtual void compute_shell(int, int, int, int) = 0;
    
    /// Integral object that created me.
    IntegralFactory *integral() const { return integral_; }
    
    /// Normalize Cartesian functions based on angular momentum
    void normalize_am(GaussianShell*, GaussianShell*, GaussianShell*, GaussianShell*, int nchunk=1);
        
    /// Return true if the clone member can be called. By default returns false.
    virtual bool cloneable();
    
    /// Returns a clone of this object. By default throws an exception
    virtual TwoBodyInt* clone();
    
    /// Results go back to buffer_
    void pure_transform(int, int, int, int, int nchunk);
};

}

#endif
