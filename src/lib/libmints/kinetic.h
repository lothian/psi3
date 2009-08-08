#ifndef _psi_src_lib_libmints_kinetic_h_
#define _psi_src_lib_libmints_kinetic_h_

/*!
    \file libmints/kinetic.h
    \ingroup MINTS
*/

#include <libmints/ref.h>

#include <libmints/basisset.h>
#include <libmints/gshell.h>
#include <libmints/osrecur.h>
#include <libmints/onebody.h>
#include <libmints/integral.h>

namespace psi {

//! Computes kinetic integrals.
//! Use an IntegralFactory to create this object.
class KineticInt : public OneBodyInt
{
    //! Obara and Saika recursion object to be used.
    ObaraSaikaTwoCenterRecursion overlap_recur_;
    
    //! Computes the kinetic integral between two gaussian shells.
    void compute_pair(GaussianShell*, GaussianShell*);
    //! Computes the kinetic derivatve between two gaussian shells.
    void compute_pair_deriv1(GaussianShell*, GaussianShell*);
    
public:
    //! Constructor. Do not call directly, use an IntegralFactory.
    KineticInt(IntegralFactory*, BasisSet*, BasisSet*, int deriv=0);
    //! Virtual destructor.
    virtual ~KineticInt();
    
    //! Compute kinetic integral between two shells, result stored in buffer_.
    void compute_shell(int, int);
    //! Compute kinetic derivative between two shells, result stored in buffer_.
    void compute_shell_deriv1(int, int);
    
    /// Does the method provide first derivatives?
    bool has_deriv1() { return true; }
};

}

#endif
