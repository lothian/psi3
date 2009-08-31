/*
 *  hf.h
 *  matrix
 *
 *  Created by Justin Turney on 4/9/08.
 *  Copyright 2008 by Justin M. Turney, Ph.D.. All rights reserved.
 *
 */

#ifndef HF_H
#define HF_H

#include <libpsio/psio.hpp>
#include <libmints/wavefunction.h>

 using namespace psi;
 
class HF : public Wavefunction {
protected:
    Matrix H_;
    Matrix S_;
    Matrix Shalf_;
    Matrix Sphalf_;
    
    // Previous iteration's energy and current energy
    double Eold_;
    double E_;
    
    // Max number of iterations for HF
    int maxiter_;
    
    // Nuclear repulsion energy
    double nuclearrep_;
    
    // Number of atoms and their Z value (used in find_occupation)
    int natom_;
    double *zvals_;
    
    // DOCC vector from input (if found)
    int doccpi_[8];
    bool input_docc_;
    
    // SOCC vector from input (if found)
    int soccpi_[8];
    bool input_socc_;
    
    // Number of alpha and beta electrons
    int nalpha_, nbeta_;
    // Number of alpha and beta electrons per irrep
    int nalphapi_[8], nbetapi_[8];
    
    // Mapping arrays
    int *so2symblk_;
    int *so2index_;
    
    // Pairs needed for PK supermatrix
    size_t pk_pairs_;
    size_t pk_size_;
    int *pk_symoffset_;
    
    // Perturb the Hamiltonian?
    int perturb_h_;
    // How big of a perturbation
    double lambda_;
    // With what...
    enum perturb { nothing, dipole_x, dipole_y, dipole_z };
    perturb perturb_;

    // Using direct integrals?
    int direct_integrals_;

public:    
    // Exactly what their name says
    SimpleMatrix** Dipole_;
    SimpleMatrix** Quadrupole_;
    
    // Nuclear contributions
    SimpleVector nuclear_dipole_contribution_;
    SimpleVector nuclear_quadrupole_contribution_;
    
    // Formation of H is the same regardless of RHF, ROHF, UHF
    void form_H();
    
    // Formation of S^+1/2 and S^-1/2 are the same
    void form_Shalf();

protected:
    // Common initializer
    void common_init();
    
    // Compute multipole integrals
    void form_multipole_integrals();
    
    // Determine how many core and virtual orbitals to freeze
    int *compute_fcpi(int nfzc, Vector& eigvalues);
    int *compute_fvpi(int nfvc, Vector& eigvalues);
    
    // Forms the _so2* mapping arrays and determines _pk_pairs
    void form_indexing();
    
    // Prints some opening information
    void print_header();
    
    inline int integral_type(int i, int j, int k, int l)
    {
        int type;
        
        if (i == j && i == k && i == l)     // (ij|kl)  (11|11)
            type = 1;
        else if (i == j && k == l && i > k) // (ij|kl)  (22|11)
            type = 2;
        else if (i == j && i == k && i > l) // (ij|kl)  (22|21)
            type = 3;
        else if (j == k && j == l && i > j) // (ij|kl)  (21|11)
            type = 4;
        else if (i == k && j == l)          // (ij|kl)  (21|21)
            type = 5;
        else if (i == j)                    // (ij|kl)  (33|21)
            type = 6;
        else if (j >  k && k == l)          // (ij|kl)  (32|11)
            type = 7;
        else if (k == l)                    // (ij|kl)  (31|22)
            type = 8;
        else if (i == k)                    // (ij|kl)  (32|31)
            type = 9;
        else if (j == k)                    // (ij|kl)  (32|21)
            type = 10;
        else if (j == l)                    // (ij|kl)  (31|21)
            type = 11;
        else if (j >  k)                    // (ij|kl)  (43|21)
            type = 12;
        else if (j >  l)                    // (ij|kl)  (42|31)
            type = 13;
        else                                // (ij|kl)  (41|32)
            type = 14;
        
        return type;
    }
public:
    HF(psi::PSIO *psio, psi::Chkpt *chkpt = 0);
    HF(psi::PSIO &psio, psi::Chkpt &chkpt);
    
    virtual ~HF();
};

#endif
