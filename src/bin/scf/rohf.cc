#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <utility>

#include <psifiles.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.hpp>
#include <libchkpt/chkpt.hpp>
#include <libipv1/ip_lib.h>
#include <libiwl/iwl.hpp>
#include <libqt/qt.h>

#include <libmints/matrix.h>
#include "rohf.h"

extern FILE *outfile;

using namespace std;
using namespace psi;

ROHF::ROHF(PSIO *psio, Chkpt *chkpt) : HF(psio, chkpt)
{
    common_init();
}

ROHF::ROHF(Ref<PSIO> &psio, Ref<Chkpt> &chkpt) : HF(psio, chkpt)
{
    common_init();
}

ROHF::~ROHF() {
	free_block(diis_B_);
	if (pk_)
		delete[](pk_);
	if (k_)
		delete[](k_);
}

void ROHF::common_init()
{
    diis_enabled_ = true;
	use_out_of_core_ = false;
    num_diis_vectors_ = 6;

	Fc_     = factory_.create_matrix("F closed");
	Fo_     = factory_.create_matrix("F open");
	Feff_   = factory_.create_matrix("F effective (MO basis)");
	C_      = factory_.create_matrix("C");
	Dc_     = factory_.create_matrix("D closed");
	Do_     = factory_.create_matrix("D open");
	Dc_old_ = factory_.create_matrix("D closed old");
	Do_old_ = factory_.create_matrix("D open old");
	Gc_     = factory_.create_matrix("G closed");
	Go_     = factory_.create_matrix("G open");
	epsilon_ = factory_.create_vector();
	
	pk_ = NULL;
	k_ = NULL;
	
    diis_B_ = NULL;
	current_diis_fock_ = 0;
	
	ip_data(const_cast<char*>("DIIS_VECTORS"), const_cast<char*>("%d"), &num_diis_vectors_, 0);
    ip_boolean(const_cast<char*>("DIIS"), &diis_enabled_, 0);
    
    // Don't perform DIIS is less than 2 vectors requested, or user requested a negative number
    if (num_diis_vectors_ < 2) {
        // disable diis
        diis_enabled_ = false;
    }
    
    if (diis_enabled_ == true) {
        // Allocate the memory
        diis_B_ = block_matrix(num_diis_vectors_, num_diis_vectors_);
        
        // Allocate space for diis_F_ and diis_E_
        diis_F_ = new RefMatrix[num_diis_vectors_];
        diis_E_ = new RefMatrix[num_diis_vectors_];
        
        for (int i=0; i < num_diis_vectors_; ++i) {
            diis_F_[i] = factory_.create_matrix();
            diis_E_[i] = factory_.create_matrix();
        }
    }

	charge_ = 0;
	ip_data(const_cast<char*>("CHARGE"), const_cast<char*>("%d"), &(charge_), 0);
	multiplicity_ = 0;
	
	ip_data(const_cast<char*>("MULTP"), const_cast<char*>("%d"), &(multiplicity_), 0);
	ip_boolean(const_cast<char*>("OUT_OF_CORE"), &(use_out_of_core_), 0);

	// Disable the use of the out of core algorithm. Hasn't been modified for ROHF
	use_out_of_core_ = false;

	fprintf(outfile, "  DIIS %s.\n\n", diis_enabled_ ? "enabled" : "disabled");

	allocate_PK();
}

void ROHF::initial_guess()
{
    RefMatrix temp   = factory_.create_matrix("Temporary Matrix");
	RefVector values = factory_.create_vector();
	
    // In ROHF the creation of the C matrix depends on the previous iteration's C
	// matrix. Here we use H to generate the first C.
	temp.copy(H_);
	temp.transform(Shalf_);
	temp.diagonalize(C_, values);
	temp.gemm(false, false, 1.0, Shalf_, C_, 0.0);
	C_.copy(temp);
}

double ROHF::compute_energy() {
	bool converged = false, diis_iter=false;
	int iter = 0;
	
	// Do the initial work to give the iterations a starting point.
	form_H();
	find_occupation(H_);

	if (use_out_of_core_ == false)
		form_PK();

	form_Shalf();
    initial_guess();
	form_D();
	
	// Compute an initial energy using H and D
	E_ = compute_initial_E();

	do {
		iter++;

		Dc_old_ = Dc_; // save previous density
		Do_old_ = Do_; // save previous density
		Eold_ = E_; // save previous energy

		if (use_out_of_core_ == false)
			form_G_from_PK();
		else
			form_G();

		form_F(); // Forms: Fc_, Fo_, Feff_
		
		if (diis_enabled_)
			save_fock(); // Save the effective Fock for diis
		
		// Compute total energy
		E_ = compute_E();

		if (diis_enabled_ == true && iter >= num_diis_vectors_ && iter % 6 == 0) {
			diis();
			diis_iter = true;
		} else {
			diis_iter = false;
		}
		fprintf(outfile,
				"  @ROHF iteration %3d energy: %20.14f    %20.14f %s\n",
				iter, E_, E_ - Eold_, diis_iter == false ? " " : "DIIS");
		fflush(outfile);

		form_C(); 	// Uses Feff_ to form C_.
		//	find_occupation(_F);
		form_D();

		converged = test_convergency();
	} while (!converged && iter < maxiter_);

	// Return the final ROHF energy
	if (converged) {
		fprintf(outfile, "\n  Energy converged.\n");
		save_information();
		return E_;
	} else {
		fprintf(outfile, "\n  Failed to converge.\n");
		return 0.0;
	}
}

void ROHF::save_information()
{
	// Print the final docc vector
	char **temp2 = chkpt_->rd_irr_labs();
	int nso = chkpt_->rd_nso();

	fprintf(outfile, "\n  Final DOCC vector = (");
	for (int h=0; h<factory_.nirreps(); ++h) {
		fprintf(outfile, "%2d %3s ", doccpi_[h], temp2[h]);
	}
	fprintf(outfile, ")\n");
	
	fprintf(outfile, "  Final SOCC vector = (");
	for (int h=0; h<factory_.nirreps(); ++h) {
		fprintf(outfile, "%2d %3s ", soccpi_[h], temp2[h]);
	}
	fprintf(outfile, ")\n");

	int print_mos = false;
	ip_boolean(const_cast<char*>("PRINT_MOS"), &(print_mos), 0);
	if (print_mos) {
		fprintf(outfile, "\n  Molecular orbitals:\n");

		C_.eivprint(epsilon_);
	}

	// Print out orbital energies.
	std::vector<std::pair<double, int> > pairs;
	for (int h=0; h<epsilon_.nirreps(); ++h) {
		for (int i=0; i<epsilon_.dimpi()[h]; ++i)
			pairs.push_back(make_pair(epsilon_.get(h, i), h));
	}
	sort(pairs.begin(), pairs.end());
	int ndocc = 0, nsocc = 0;
	for (int i=0; i<epsilon_.nirreps(); ++i) {
		ndocc += doccpi_[i];
		nsocc += soccpi_[i];
	}

	fprintf(outfile,
			"\n  Orbital energies (a.u.):\n    Doubly occupied orbitals\n      ");
	for (int i=1; i<=ndocc; ++i) {
		fprintf(outfile, "%12.6f %3s  ", pairs[i-1].first,
				temp2[pairs[i-1].second]);
		if (i % 4 == 0)
			fprintf(outfile, "\n      ");
	}
	fprintf(outfile, "\n");
	fprintf(outfile, "\n    Singly occupied orbitals\n      ");
	for (int i=ndocc+1; i<=ndocc+nsocc; ++i) {
		fprintf(outfile, "%12.6f %3s  ", pairs[i-1].first,
			temp2[pairs[i-1].second]);
		if ((i-ndocc) % 4 == 0)
			fprintf(outfile, "\n      ");
	}
	fprintf(outfile, "\n");
	fprintf(outfile, "\n    Unoccupied orbitals\n      ");
	for (int i=ndocc+nsocc+1; i<=nso; ++i) {
		fprintf(outfile, "%12.6f %3s  ", pairs[i-1].first,
				temp2[pairs[i-1].second]);
		if ((i-ndocc-nsocc) % 4 == 0)
			fprintf(outfile, "\n      ");
	}
	fprintf(outfile, "\n");

	for (int i=0; i<epsilon_.nirreps(); ++i)
		free(temp2[i]);
	free(temp2);

	chkpt_->wt_nmo(nso);
	chkpt_->wt_ref(2); // ROHF
	chkpt_->wt_etot(E_);
	chkpt_->wt_escf(E_);
	chkpt_->wt_eref(E_);
	chkpt_->wt_clsdpi(doccpi_);
	chkpt_->wt_orbspi(epsilon_.dimpi());
	chkpt_->wt_openpi(soccpi_);
	chkpt_->wt_phase_check(0);

	// Figure out frozen core orbitals
	int nfzc = chkpt_->rd_nfzc();
	int nfzv = chkpt_->rd_nfzv();
	int *frzcpi = compute_fcpi(nfzc, epsilon_);
	int *frzvpi = compute_fvpi(nfzv, epsilon_);
	chkpt_->wt_frzcpi(frzcpi);
	chkpt_->wt_frzvpi(frzvpi);
	delete[](frzcpi);
	delete[](frzvpi);

	int nopenirreps = 0;
	for (int i=0; i<epsilon_.nirreps(); ++i)
		if (soccpi_[i])
			nopenirreps++;
	
	// This code currently only handles ROHF
	chkpt_->wt_iopen(nopenirreps * (nopenirreps + 1));

	// Write eigenvectors and eigenvalue to checkpoint 
	double *values = epsilon_.to_block_vector();
	chkpt_->wt_evals(values);
	free(values);
	double **vectors = C_.to_block_matrix();
	chkpt_->wt_scf(vectors);
	free_block(vectors);
}

void ROHF::save_fock() {
#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "  Saving current Fock matrix to %d.\n", current_diis_fock_);
	}
#endif

    // Save the effective Fock, back transform to AO, and orthonormalize
	diis_F_[current_diis_fock_].copy(Feff_);
	diis_F_[current_diis_fock_].back_transform(C_);
	diis_F_[current_diis_fock_].transform(Sphalf_);
	
	// Determine error matrix for this Fock
	diis_E_[current_diis_fock_].copy(Feff_);
    diis_E_[current_diis_fock_].zero_diagonal();
	diis_E_[current_diis_fock_].back_transform(C_);
	diis_E_[current_diis_fock_].transform(Sphalf_);

#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "  New error matrix:\n");
		diis_E_[current_diis_fock_].print(outfile);
	}
#endif
	current_diis_fock_++;
	if (current_diis_fock_ == num_diis_vectors_)
		current_diis_fock_ = 0;
}

void ROHF::diis() {
	int i, j, matrix_size;
	
    matrix_size = num_diis_vectors_ + 1;
    
	// Construct the B matrix
	// Assumes all the error matrices are available
	RefMatrix temp = factory_.create_matrix();
	for (i=0; i<num_diis_vectors_; ++i) {
        // diis_E_[i].print(outfile);
		for (j=0; j<num_diis_vectors_; ++j) {
//			temp.gemm(false, true, 1.0, diis_E_[i], diis_E_[j], 0.0);
//			diis_B_[i][j] = temp.trace();
            diis_B_[i][j] = diis_E_[i].vector_dot(diis_E_[j]);
		}
	}

#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "  B matrix:\n");
		print_mat(diis_B_, num_diis_vectors_, num_diis_vectors_, outfile);
	}
#endif

	double **A = block_matrix(matrix_size, matrix_size);
	double *b = init_array(matrix_size);
	int *ipiv = init_int_array(matrix_size);

	A[0][0] = 0.0;
	b[0] = -1.0;
	for (i=1; i<matrix_size; ++i) {
		A[0][i] = -1.0;
		A[i][0] = -1.0;
		b[i] = 0.0;
		for (j=1; j<matrix_size; ++j) {
			A[i][j] = diis_B_[i-1][j-1];
		}
	}

#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "  A:\n");
		print_mat(A, matrix_size, matrix_size, outfile);
	}
#endif

	// Solve A * x = b
	int errcode = C_DGESV(matrix_size, 1, &(A[0][0]), matrix_size, ipiv, b, matrix_size);

#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "  A:\n");
		print_mat(A, matrix_size, matrix_size, outfile);
		fprintf(outfile, "  x:\n");
		for (i=0; i<matrix_size; ++i)
		fprintf(outfile, "    %d: %20.16f\n", i, b[i]);
	}
#endif

	// Extrapolate a new Fock matrix.
    if (errcode == 0) {
    	Feff_.zero();
    	for (i=0; i<num_diis_vectors_; ++i)
    		Feff_.add(diis_F_[i] * b[i+1]);
    		
		// Feff_ is now in the orthogonal AO basis
    	// Transform back to MO basis for C construction
    	Feff_.back_transform(Sphalf_);
    	Feff_.transform(C_);

        #ifdef _DEBUG
        	if (debug_) {
        		fprintf(outfile, "  Extrapolated Fock:\n");
        		Feff_.print(outfile);
        	}
        #endif
    } else if (errcode > 0) {
    	fprintf(outfile, "  DIIS: singularity detected, DIIS skipped this iteration (errcode = %d).\n", errcode);
    } else {
    	fprintf(outfile, "  DIIS: DGESV argument #%d is illegal, DIIS skipped this iteration.\n", -errcode);
    }

	free_block(A);
	free(b);
	free(ipiv);
}

bool ROHF::test_convergency() {
	double ediff = E_ - Eold_;

	if (fabs(ediff) < energy_threshold_)
		return true;
	else
		return false;
}

void ROHF::allocate_PK() {
	// Figure out how many pair combinations yield A1 symmetry (done in above loop)
	//   num_pair_combinations_of_A1 = ioff[_opi[0]] + ioff[_opi[1]] + ioff[_opi[2]] + ...
	// Allocate memory for the PK matrix (using a vector)
	if (pk_size_ < (memory_ / sizeof(double) / 2)) {
		pk_ = new double[pk_size_];
		k_ = new double[pk_size_];

		if (pk_ == NULL || k_ == NULL) {
			fprintf(outfile, "  Insufficient free system memory for in-core PK implementation.\n");
			fprintf(outfile, "  Switching to out-of-core algorithm.\n");
			use_out_of_core_ = true;
		} else {
			// Zero out PK and K
			memset(pk_, 0, pk_size_*sizeof(double));
			memset(k_, 0, pk_size_*sizeof(double));

			fprintf(outfile,
				"  Allocated %lu elements (%lu pairs) for PK. (%5f MiB)\n",
				(unsigned long)pk_size_, (unsigned long)pk_pairs_, pk_size_ * 8.0 / 1048576.0);
			fprintf(outfile,
				"  Allocated %lu elements (%lu pairs) for K.  (%5f MiB)\n\n",
				(unsigned long)pk_size_, (unsigned long)pk_pairs_, pk_size_ * 8.0 / 1048576.0);
		}
	} else {
		fprintf(outfile,
				"  Insufficient memory for in-core PK implementation.\n");
		fprintf(outfile,
				"  Would need %lu elements of double memory. (%5f MiB)\n",
				(unsigned long)pk_size_*2, pk_size_ * 8.0 / 1048576.0 * 2.0);
		fprintf(outfile, "  Switching to out-of-core algorithm.\n");
		use_out_of_core_ = true;
	}
}

void ROHF::find_occupation(RefMatrix& mat) {
	if (input_docc_ && input_socc_)
		return;
	else {
		fprintf(stderr, "ROHF: Occupation guessing not implemented yet.\n");
		abort();
		return;
	}
	RefMatrix eigvector = factory_.create_matrix();
	RefVector eigvalues = factory_.create_vector();

	mat.diagonalize(eigvector, eigvalues);
	std::vector<std::pair<double, int> > pairs;
	for (int h=0; h<eigvalues.nirreps(); ++h) {
		for (int i=0; i<eigvalues.dimpi()[h]; ++i)
			pairs.push_back(make_pair(eigvalues.get(h, i), h));
	}
	sort(pairs.begin(), pairs.end());

	memset(doccpi_, 0, sizeof(int) * eigvalues.nirreps());
	int n2elec = 0, n1elec;
	for (int i=0; i<natom_; ++i)
		n2elec += (int)zvals_[i];
	n2elec /= 2;
	n1elec = n2elec % 2;

	for (int i=0; i<n2elec; ++i)
		doccpi_[pairs[i].second]++;
}

void ROHF::form_initialF() {
	// Form the initial Fock matrix, closed and open variants
	Fc_.copy(H_);
	Fo_.copy(0.5 * H_);

	// Transform the Focks
	Fc_.transform(Shalf_);
	Fo_.transform(Shalf_);
	
#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "Initial closed Fock matrix:\n");
		Fc_.print(outfile);
		fprintf(outfile, "Initial open Fock matrix:\n");
		Fo_.print(outfile);
	}
#endif
}

void ROHF::form_F() {
	RefMatrix Fct = factory_.create_matrix("Fock closed transformed");
	RefMatrix Fot = factory_.create_matrix("Fock open transformed");

	// Form Fc_ and Fo_. See derivation notebook for equations.
	Fc_.copy(H_ + Gc_);
	Fo_.copy(0.5 * H_ + Go_);

	// Transform Fc_ and Fo_ to MO basis
	Fct.transform(Fc_, C_);
	Fot.transform(Fo_, C_);
	
	// Form the effective Fock matrix, too
	// The effective Fock matrix has the following structure
	//  closed   | open     | virtual
	//  Fc         2(Fc-Fo)   Fc
	//  2(Fc-Fo)   Fc         2Fo
	//  Fc         2Fo         Fc
	int *opi = Fc_.rowspi();
	Feff_.copy(Fct);
	for (int h=0; h<Feff_.nirreps(); ++h) {
		for (int i=doccpi_[h]; i<doccpi_[h]+soccpi_[h]; ++i) {
			// Set the open/closed portion
			for (int j=0; j<doccpi_[h]; ++j) {
				double val = 2.0 * (Fct.get(h, i, j) - Fot.get(h, i, j));
				Feff_.set(h, i, j, val);
				Feff_.set(h, j, i, val);
			}
			// Set the open/virtual portion
			for (int j=doccpi_[h]+soccpi_[h]; j<opi[h]; ++j) {
				double val = 2.0 * Fot.get(h, i, j);
				Feff_.set(h, i, j, val);
				Feff_.set(h, j, i, val);
			}
			// Set the open/open portion
//			for (int j=doccpi_[h]; j<doccpi_[h]+soccpi_[h]; ++j) {
//				double val = Fot.get(h, i, j);
//				Feff_.set(h, i, j, val);
//				Feff_.set(h, j, i, val);
//			}
		}
	}
#ifdef _DEBUG
	if (debug_) {
		Fc_.print(outfile);
		Fo_.print(outfile);
		Fct.print(outfile);
		Fot.print(outfile);
		Feff_.print(outfile);
	}
#endif
}

void ROHF::form_C() {
	RefMatrix temp   = factory_.create_matrix("temp matrix");
	RefMatrix eigvec = factory_.create_matrix("Effective Fock Eigenvectors");
	
	// Obtain new eigenvectors
	Feff_.diagonalize(eigvec, epsilon_);

#ifdef _DEBUG
	if (debug_) {
		eigvec.eivprint(epsilon_);
	}
#endif
	// Perform a half-transform using the previous iteration's C
	temp.gemm(false, false, 1.0, C_, eigvec, 0.0);
	C_.copy(temp);
	
#ifdef _DEBUG
	if (debug_) {
		C_.print(outfile);
	}
#endif
}

void ROHF::form_D() {
	int h, i, j, m;
	int *opi = Dc_.rowspi();
	int nirreps = Dc_.nirreps();
	double val;
	for (h=0; h<nirreps; ++h) {
		for (i=0; i<opi[h]; ++i) {
			for (j=0; j<opi[h]; ++j) {
				val = 0.0;
				for (m=0; m<doccpi_[h]; ++m)
					val += C_.get(h, i, m) * C_.get(h, j, m);
				Dc_.set(h, i, j, val);

				val = 0.0;
				for (m=doccpi_[h]; m<doccpi_[h]+soccpi_[h]; ++m)
					val += C_.get(h, i, m) * C_.get(h, j, m);
				Do_.set(h, i, j, val);
			}
		}
	}

#ifdef _DEBUG
	if (debug_) {
		Dc_.print(outfile);
		Do_.print(outfile);
	}
#endif
}

double ROHF::compute_initial_E() {
	RefMatrix Ho = 0.5 * H_;

	double Etotal = nuclearrep_ + Dc_.vector_dot(H_) + Do_.vector_dot(Ho);
	fprintf(outfile, "\n  Initial ROHF energy: %20.14f\n\n", Etotal);
	fflush(outfile);
	return Etotal;
}

double ROHF::compute_E() {
	RefMatrix HFc = H_ + Fc_;
	RefMatrix HFo = 0.5 * H_ + Fo_;
	double Etotal = nuclearrep_ + Dc_.vector_dot(HFc) + Do_.vector_dot(HFo);
	return Etotal;
}

void ROHF::form_PK() {
    // struct iwlbuf ERIIN;
	int ilsti, nbuf;
	int i, j, k, l;
	int ii, jj, kk, ll;
	int is, js, ks, ls;
	int fi;
	size_t bra, ket, braket;
	int idx;
	int counter = 0;
	double value;

	// PK zeroed out during allocation
	fprintf(outfile, "  Forming PK and K matrices.\n");
	fflush(outfile);

	IWL ERIIN(psio_.pointer(), PSIF_SO_TEI, 0.0, 1, 1);

	do {
		ilsti = ERIIN.last_buffer();
		nbuf = ERIIN.buffer_count();

		fi = 0;
		for (idx=0; idx<nbuf; ++idx) {
			i = ERIIN.labels()[fi] > 0 ? ERIIN.labels()[fi] : -ERIIN.labels()[fi];
			j = ERIIN.labels()[fi+1];
			k = ERIIN.labels()[fi+2];
			l = ERIIN.labels()[fi+3];
			value = ERIIN.values()[idx];
			fi += 4;

			// Get the symmetries
			is = so2symblk_[i];
			js = so2symblk_[j];
			ks = so2symblk_[k];
			ls = so2symblk_[l];

			// Get the offset of the SO index in its symblock
			ii = so2index_[i];
			jj = so2index_[j];
			kk = so2index_[k];
			ll = so2index_[l];

			// J
			if ((is == js) && (ks == ls)) {
				bra = INDEX2(ii, jj);
				ket = INDEX2(kk, ll);
				// pk_symoffset_ corrects for the symmetry offset in the pk_ vector
				braket = INDEX2(bra + pk_symoffset_[is], ket + pk_symoffset_[ks]);
				pk_[braket] += value;

				// K/2 (2nd sort)
				if ((ii != jj) && (kk != ll)) {
					if ((is == ls) && (js == ks)) {
						bra = INDEX2(ii, ll);
						ket = INDEX2(jj, kk);
						braket = INDEX2(bra + pk_symoffset_[is], ket + pk_symoffset_[js]);
						if ((ii == ll) || (jj == kk)) {
							pk_[braket] -= 0.5 * value;
							k_[braket] -= 0.5 * value;
						} else {
							pk_[braket] -= 0.25 * value;
							k_[braket] -= 0.25 * value;
						}
					}
				}
			}

			// K/2 (1st sort)
			if ((is == ks) && (js == ls)) {
				bra = INDEX2(ii, kk);
				ket = INDEX2(jj, ll);
				braket = INDEX2(bra + pk_symoffset_[is], ket + pk_symoffset_[js]);
				if ((ii == kk) || (jj == ll)) {
					pk_[braket] -= 0.5 * value;
					k_[braket] -= 0.5 * value;
				} else {
					pk_[braket] -= 0.25 * value;
					k_[braket] -= 0.25 * value;
				}
			}
			counter++;
		}

		if (!ilsti)
			ERIIN.fetch();
	} while (!ilsti);

    // Going out of scope will close the buffer
    // iwl_buf_close(&ERIIN, 1);

	// After stage two is complete, the elements of P must be halved for the case IJ=KL.
	for (size_t ij=0; ij < pk_pairs_; ++ij) {
		pk_[INDEX2(ij,ij)] *= 0.5;
		k_[INDEX2(ij,ij)] *= 0.5;
	}
	
	fprintf(outfile, "  Processed %d two-electron integrals.\n", counter);
#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "pk_:\n");
		print_array(pk_, pk_pairs_, outfile);
		fprintf(outfile, "k_:\n");
		print_array(k_, pk_pairs_, outfile);
	}
#endif
}

void ROHF::form_G_from_PK() {
	int nirreps = factory_.nirreps();
	int *opi = factory_.rowspi();
	size_t ij;
	double *Do_vector = new double[pk_pairs_];
	double *Dc_vector = new double[pk_pairs_];
	double *Gc_vector = new double[pk_pairs_];
	double *Go_vector = new double[pk_pairs_];

	Gc_.zero();
	Go_.zero();

	memset(Do_vector, 0, sizeof(double) * pk_pairs_);
	memset(Dc_vector, 0, sizeof(double) * pk_pairs_);
	memset(Gc_vector, 0, sizeof(double) * pk_pairs_);
	memset(Go_vector, 0, sizeof(double) * pk_pairs_);

	ij=0;
	for (int h=0; h<nirreps; ++h) {
		for (int p=0; p<opi[h]; ++p) {
			for (int q=0; q<=p; ++q) {
				if (p != q) {
					Dc_vector[ij] = 2.0 * Dc_.get(h, p, q);
					Do_vector[ij] = 2.0 * Do_.get(h, p, q);
				} else {
					Dc_vector[ij] = Dc_.get(h, p, q);
					Do_vector[ij] = Do_.get(h, p, q);
				}
				ij++;
			}
		}
	}

#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "PK: ij = %lu\n", (unsigned long)ij);
		fflush(outfile);
		Dc_.print(outfile);
		fprintf(outfile, "PK: Dc vector:\n");
		for (ij=0; ij<pk_pairs_; ++ij)
			fprintf(outfile, "PK: Dc vector [%lu] = %20.14f\n", (unsigned long)ij, Dc_vector[ij]);
		Do_.print(outfile);
		fprintf(outfile, "PK: Do vector:\n");
		for (ij=0; ij<pk_pairs_; ++ij)
			fprintf(outfile, "PK: Do vector [%lu] = %20.14f\n", (unsigned long)ij, Do_vector[ij]);
	}
#endif

	/* 
	 * This code goes through the densities (Dc_ and Do_), PK, and K to form
	 * two G matrices. One G matrix is for Fc_ and the other for Fo_.
	 * See derivation notebook for equations.
	 */
	double Gc_pq, Dc_pq;
	double Go_pq, Do_pq;
	double* Dc_rs;
	double* Gc_rs;
	double* Do_rs;
	double* Go_rs;
	int pq, rs;
	double* PK_block = pk_;
	double* K_block = k_;
	int ts_pairs = pk_pairs_;
	for (pq = 0; pq < ts_pairs; ++pq) {
		Gc_pq = 0.0;
		Dc_pq = Dc_vector[pq];
		Dc_rs = &Dc_vector[0];
		Gc_rs = &Gc_vector[0];
		Go_pq = 0.0;
		Do_pq = Do_vector[pq];
		Do_rs = &Do_vector[0];
		Go_rs = &Go_vector[0];
		for (rs = 0; rs <= pq; ++rs) {
			// D_{rs}^{c} * PK_{pqrs}         Also found in RHF
			Gc_pq  += *PK_block * (*Dc_rs);
			*Gc_rs += *PK_block * Dc_pq;
			// D_{rs}^{o} * PK_{pqrs} / 2     Yes, open D adds to closed G
			Gc_pq  += *PK_block * (*Do_rs) * 0.5;
			*Gc_rs += *PK_block * Do_pq    * 0.5;
			// D_{rs}^{c} * PK_{pqrs} / 2     Yes, closed D adds to open G
			Go_pq  += *PK_block * (*Dc_rs) * 0.5;
			*Go_rs += *PK_block * Dc_pq    * 0.5;
			// D_{rs}^{o} * (PK_{pqrs} + K_{pqrs}) / 4
			Go_pq  += (*PK_block + *K_block) * (*Do_rs) * 0.25;
			*Go_rs += (*PK_block + *K_block) * Do_pq    * 0.25;
			++Dc_rs;
			++Gc_rs;
			++Do_rs;
			++Go_rs;
			++PK_block;
			++K_block;
		}
		Gc_vector[pq] += Gc_pq;
		Go_vector[pq] += Go_pq;
	}

	// Convert G to a matrix
	ij = 0;
	for (int h = 0; h < nirreps; ++h) {
		for (int p = 0; p < opi[h]; ++p) {
			for (int q = 0; q <= p; ++q) {
				Gc_.set(h, p, q, 2.0 * Gc_vector[ij]);
				Gc_.set(h, q, p, 2.0 * Gc_vector[ij]);
				Go_.set(h, p, q, 2.0 * Go_vector[ij]);
				Go_.set(h, q, p, 2.0 * Go_vector[ij]);
				ij++;
			}
		}
	}

#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "Gc from PK:\n");
		Gc_.print(outfile);
		fprintf(outfile, "Go from PK:\n");
		Go_.print(outfile);
	}
#endif

	delete[](Dc_vector);
	delete[](Do_vector);
	delete[](Gc_vector);
	delete[](Go_vector);
}

void ROHF::form_G() {
#if 0
	struct iwlbuf ERIIN;
	int ilsti, nbuf;
	int i, j, k, l;
	int ii, jj, kk, ll;
	int is, js, ks, ls;
	int fi;
	double value;
	double temp1, temp2, temp3, temp4, temp5, temp6;
	int idx;
	int itype;
	int counter = 0;
#endif
	
	fprintf(stderr, "ROHF out-of-core algorithm is not implemented yet!\n");
	abort();
	
#if 0
	// Zero out the G matrix
	G_.zero();

	iwl_buf_init(&ERIIN, PSIF_SO_TEI, 0.0, 1, 1);

	do {
		ilsti = ERIIN.lastbuf;
		nbuf = ERIIN.inbuf;

		fi = 0;
		for (idx=0; idx<nbuf; ++idx) {
			i = ERIIN.labels[fi] > 0 ? ERIIN.labels[fi] : -ERIIN.labels[fi];
			j = ERIIN.labels[fi+1];
			k = ERIIN.labels[fi+2];
			l = ERIIN.labels[fi+3];
			value = ERIIN.values[idx];
			fi += 4;

			//fprintf(outfile, "  (%4d,%4d|%4d,%4d) = %20.16lf\n", i, j, k, l, value);

			itype = integral_type(i, j, k, l);
			switch (itype) {
			case 1:
				ii = so2index_[i];
				is = so2symblock_[i];
				temp1 = D_.get(is, ii, ii) * value;

				G_.add(is, ii, ii, temp1);

#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 1:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, ii, temp1);
				}
#endif
				break;

			case 2:
				ii = so2index_[i];
				kk = so2index_[k];
				is = so2symblock_[i];
				ks = so2symblock_[k];

				temp1 = D_.get(ks, kk, kk) * 2.0 * value;
				temp2 = 0.0;
				temp3 = D_.get(is, ii, ii) * 2.0 * value;

				G_.add(is, ii, ii, temp1);
				G_.add(ks, kk, kk, temp3);

				if (is == ks) {
					temp2 = D_.get(is, ii, kk) * value;
					G_.add(is, ii, kk, -temp2);
					G_.add(is, kk, ii, -temp2);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 2:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, ii, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, kk, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", ks, kk, kk, temp3);
				}
#endif
				break;

			case 3:
				ii = so2index_[i];
				ll = so2index_[l];
				is = so2symblock_[i];
				ls = so2symblock_[l];

				temp1 = temp2 = 0.0;
				if (is == ls) {
					temp1 = D_.get(is, ii, ii) * value;
					temp2 = D_.get(is, ii, ll) * value * 2.0;

					G_.add(is, ii, ll, temp1);
					G_.add(is, ll, ii, temp1);
					G_.add(is, ii, ii, temp2);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 3:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, ll, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, ii, temp2);
				}
#endif
				break;

			case 4:
				ii = so2index_[i];
				jj = so2index_[j];
				is = so2symblock_[i];
				js = so2symblock_[j];

				temp1 = temp2 = 0.0;
				if (is == js) {
					temp1 = D_.get(js, jj, jj) * value;
					temp2 = D_.get(is, ii, jj) * value * 2.0;

					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
					G_.add(js, jj, jj, temp2);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 4:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", js, jj, jj, temp2);
				}
#endif
				break;

			case 5:
				ii = so2index_[i];
				jj = so2index_[j];
				is = so2symblock_[i];
				js = so2symblock_[j];

				if (is == js) {
					temp1 = D_.get(is, ii, jj) * value * 3.0;
					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
				}

				temp2 = D_.get(is, ii, ii) * value;
				temp3 = D_.get(js, jj, jj) * value;
				G_.add(js, jj, jj, -temp2);
				G_.add(is, ii, ii, -temp3);
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 5:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, jj, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, ii, temp3);
				}
#endif
				break;

			case 6:
				ii = so2index_[i];
				kk = so2index_[k];
				ll = so2index_[l];
				is = so2symblock_[i];
				ks = so2symblock_[k];
				ls = so2symblock_[l];

				temp1 = temp2 = temp3 = temp4 = 0.0;
				if (ks == ls)
					temp1 = D_.get(ks, kk, ll) * value * 4.0;
				if (is == ls)
					temp2 = D_.get(is, ii, ll) * value;
				temp3 = D_.get(is, ii, ii) * value * 2.0;
				if (is == ks)
					temp4 = D_.get(is, ii, kk) * value;

				G_.add(is, ii, ii, temp1);
				if (is == ks) {
					G_.add(is, ii, kk, -temp2);
					G_.add(is, kk, ii, -temp2);
				}
				if (ks == ls) {
					G_.add(ks, kk, ll, temp3);
					G_.add(ks, ll, kk, temp3);
				}
				if (is == ls) {
					G_.add(is, ii, ll, -temp4);
					G_.add(is, ll, ii, -temp4);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 6:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, ii, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, kk, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", ks, kk, ll, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, ll, temp4);
				}
#endif
				break;

			case 7:
				kk = so2index_[k];
				ii = so2index_[i];
				jj = so2index_[j];
				ks = so2symblock_[k];
				is = so2symblock_[i];
				js = so2symblock_[j];

				temp1 = temp2 = temp3 = temp4 = 0.0;
				if (is == js)
					temp1 = D_.get(is, ii, jj) * value * 4.0;
				if (js == ks)
					temp2 = D_.get(js, jj, kk) * value;
				if (is == ks)
					temp3 = D_.get(is, ii, kk) * value;
				temp4 = D_.get(ks, kk, kk) * value * 2.0;

				G_.add(ks, kk, kk, temp1);
				if (is == ks) {
					G_.add(is, ii, kk, -temp2);
					G_.add(is, kk, ii, -temp2);
				}
				if (js == ks) {
					G_.add(js, jj, kk, -temp3);
					G_.add(js, kk, jj, -temp3);
				}
				if (is == js) {
					G_.add(is, ii, jj, temp4);
					G_.add(is, jj, ii, temp4);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 7:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", ks, kk, kk, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, kk, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, kk, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp4);
				}
#endif
				break;

			case 8:
				kk = so2index_[k];
				ii = so2index_[i];
				jj = so2index_[j];
				ks = so2symblock_[k];
				is = so2symblock_[i];
				js = so2symblock_[j];

				temp1 = temp2 = temp3 = temp4 = 0.0;
				temp1 = D_.get(ks, kk, kk) * value * 2.0;
				if (is == js)
					temp2 = D_.get(is, ii, jj) * value * 4.0;
				if (js == ks)
					temp3 = D_.get(js, jj, kk) * value;
				if (is == ks)
					temp4 = D_.get(is, ii, kk) * value;

				if (is == js) {
					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
				}
				G_.add(ks, kk, kk, temp2);
				if (is == ks) {
					G_.add(is, ii, kk, -temp3);
					G_.add(is, kk, ii, -temp3);
				}
				if (js == ks) {
					G_.add(js, jj, kk, -temp4);
					G_.add(js, kk, jj, -temp4);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 8:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", ks, kk, kk, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, kk, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, kk, temp4);
				}
#endif
				break;

			case 9:
				ii = so2index_[i];
				jj = so2index_[j];
				ll = so2index_[l];
				is = so2symblock_[i];
				js = so2symblock_[j];
				ls = so2symblock_[l];

				temp1 = temp2 = temp3 = temp4 = 0.0;
				if (is == ls)
					temp1 = D_.get(is, ii, ll) * value * 3.0;
				if (is == js)
					temp2 = D_.get(is, ii, jj) * value * 3.0;
				if (js == ls)
					temp3 = D_.get(js, jj, ll) * value * 2.0;
				temp4 = D_.get(is, ii, ii) * value;

				if (is == js) {
					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
				}
				if (is == ls) {
					G_.add(is, ii, ll, temp2);
					G_.add(is, ll, ii, temp2);
				}
				G_.add(is, ii, ii, -temp3);
				if (js == ls) {
					G_.add(js, jj, ll, -temp4);
					G_.add(js, ll, jj, -temp4);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 9:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, ll, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, ii, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, ll, temp4);
				}
#endif
				break;

			case 10:
				ii = so2index_[i];
				jj = so2index_[j];
				ll = so2index_[l];
				is = so2symblock_[i];
				js = so2symblock_[j];
				ls = so2symblock_[l];

				temp1 = temp2 = temp3 = temp4 = 0.0;
				if (js == ls)
					temp1 = D_.get(js, jj, ll) * value * 3.0;
				if (is == js)
					temp2 = D_.get(is, ii, jj) * value * 3.0;
				temp3 = D_.get(js, jj, jj) * value;
				if (is == ls)
					temp4 = D_.get(is, ii, ll) * value * 2.0;

				if (is == js) {
					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
				}
				if (js == ls) {
					G_.add(js, jj, ll, temp2);
					G_.add(js, ll, jj, temp2);
				}
				if (is == ls) {
					G_.add(is, ii, ll, -temp3);
					G_.add(is, ll, ii, -temp3);
				}
				G_.add(js, jj, jj, -temp4);
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 10:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", js, jj, ll, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, ll, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, jj, temp4);
				}
#endif
				break;

			case 11:
				ii = so2index_[i];
				kk = so2index_[k];
				jj = so2index_[j];
				is = so2symblock_[i];
				ks = so2symblock_[k];
				js = so2symblock_[j];

				temp1 = temp2 = temp3 = temp4 = 0.0;
				if (ks == js)
					temp1 = D_.get(ks, kk, jj) * value * 3.0;
				if (is == js)
					temp2 = D_.get(is, ii, jj) * value * 3.0;
				temp3 = D_.get(js, jj, jj) * value;
				if (is == ks)
					temp4 = D_.get(is, ii, kk) * value * 2.0;

				if (is == js) {
					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
				}
				if (ks == js) {
					G_.add(ks, kk, jj, temp2);
					G_.add(ks, jj, kk, temp2);
				}
				if (is == ks) {
					G_.add(is, ii, kk, -temp3);
					G_.add(is, kk, ii, -temp3);
				}
				G_.add(js, jj, jj, -temp4);
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 11:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", ks, kk, jj, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, kk, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, jj, temp4);
				}
#endif
				break;

			case 12:
			case 13:
			case 14:
				ii = so2index_[i];
				jj = so2index_[j];
				kk = so2index_[k];
				ll = so2index_[l];
				is = so2symblock_[i];
				js = so2symblock_[j];
				ks = so2symblock_[k];
				ls = so2symblock_[l];

				temp1 = temp2 = temp3 = temp4 = temp5 = temp6 = 0.0;
				if (ks == ls)
					temp1 = D_.get(ks, kk, ll) * value * 4.0;
				if (is == js)
					temp2 = D_.get(is, ii, jj) * value * 4.0;
				if (js == ls)
					temp3 = D_.get(js, jj, ll) * value;
				if (is == ks)
					temp4 = D_.get(is, ii, kk) * value;
				if (js == ks)
					temp5 = D_.get(js, jj, kk) * value;
				if (is == ls)
					temp6 = D_.get(is, ii, ll) * value;

				if (is == js) {
					G_.add(is, ii, jj, temp1);
					G_.add(is, jj, ii, temp1);
				}
				if (ks == ls) {
					G_.add(ks, kk, ll, temp2);
					G_.add(ks, ll, kk, temp2);
				}
				if (is == ks) {
					G_.add(is, ii, kk, -temp3);
					G_.add(is, kk, ii, -temp3);
				}
				if (js == ls) {
					G_.add(js, jj, ll, -temp4);
					G_.add(js, ll, jj, -temp4);
				}
				if (is == ls) {
					G_.add(is, ii, ll, -temp5);
					G_.add(is, ll, ii, -temp5);
				}
				if (js == ks) {
					G_.add(js, jj, kk, -temp6);
					G_.add(js, kk, jj, -temp6);
				}
#ifdef _DEBUG
				if (debug_) {
					fprintf(outfile, "INTEGRAL CASE 12,13,14:\n\tModifying G[%d][%d][%d] by +%20.15lf\n", is, ii, jj, temp1);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by +%20.15lf\n", ks, kk, ll, temp2);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, kk, temp3);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, ll, temp4);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", is, ii, ll, temp5);
					fprintf(outfile, "\tModifying G[%d][%d][%d] by -%20.15lf\n", js, jj, kk, temp6);
				}
#endif
				break;
			};
			counter++;
		}

		if (!ilsti)
			iwl_buf_fetch(&ERIIN);
	} while (!ilsti);

	iwl_buf_close(&ERIIN, 1);
#ifdef _DEBUG
	if (debug_) {
		fprintf(outfile, "  Processed %6d two-electron integrals.\n", counter);
		fprintf(outfile, "G:\n");
		G_.print(outfile);
	}
#endif
	
#endif // 0
}
