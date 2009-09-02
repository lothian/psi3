#include <utility>
#include <algorithm>
#include <cstdio>

#include <cmath>

#include <libchkpt/chkpt.hpp>
#include <libmoinfo/libmoinfo.h>
#include <liboptions/liboptions.h>

#include "scf.h"

namespace psi{ namespace mcscf{

using namespace std;

void SCF::save_info()
{
  // Writes out the total number of irreps in the point group in which the molecule is being considered which have non-zero number of basis functions.
  int n_so_typs = 0;
  for(int h = 0; h < nirreps; ++h){
    if( docc[h] + actv[h] > 0 ) n_so_typs++;
  }
  _default_chkpt_lib_->wt_nsymhf(n_so_typs);

  // Write out the total number of molecular orbitals.
  _default_chkpt_lib_->wt_nmo(nso); // TODO: find nmo

  // Write out the dimensionality of ALPHA and BETA vectors of two-electron coupling coefficients for open shells.
  int tmp_iopen = ioff[moinfo_scf->get_nactv()];
  if(reference == tcscf) tmp_iopen = -tmp_iopen;
  _default_chkpt_lib_->wt_iopen(tmp_iopen);

  // Write open-shell coupling coefficients
  if(moinfo_scf->get_nactv() > 0){
    double** ccvecs;
    allocate2(double,ccvecs,2,ioff[moinfo_scf->get_nactv()]);
    for(int i=0; i < ioff[reference == tcscf]; i++) {
      ccvecs[0][i] = 0.0;
      ccvecs[1][i] = 0.0;
    }
    _default_chkpt_lib_->wt_ccvecs(ccvecs);
    release2(ccvecs);
  }

  // Writes out the total energy.
  _default_chkpt_lib_->wt_etot(total_energy);
  _default_chkpt_lib_->wt_escf(total_energy);
  _default_chkpt_lib_->wt_eref(total_energy);

  // Write the orbitals per irreps arrays
  _default_chkpt_lib_->wt_orbspi(&sopi[0]);
  _default_chkpt_lib_->wt_clsdpi(&docc[0]);
  _default_chkpt_lib_->wt_openpi(&actv[0]);


  // Read the number of frozen MOs
  int nfrzc = _default_chkpt_lib_->rd_nfzc();

  int* frz = new int[nirreps];
  for(int h = 0; h < nirreps; ++h) frz[h] = 0;
  _default_chkpt_lib_->wt_frzvpi(frz);

  vector<std::pair<double, int> > sorted_evals;

  for(int h = 0; h < nirreps; ++h)
    for(int i = 0; i < sopi[h]; ++i)
      sorted_evals.push_back( make_pair(epsilon->get(h,i),h) );

  // Sort the eigenvalues by energy
  sort(sorted_evals.begin(),sorted_evals.end());

  for(int i = 0; i < nfrzc; ++i){
    frz[sorted_evals[i].second]++;
  }

  _default_chkpt_lib_->wt_frzcpi(frz);

  delete[] frz;

  // Save the eigenvectors after rotating them
  if(options_get_int("ROTATE_MO_ANGLE") != 0){
    int mo_rotate_angle = options_get_int("ROTATE_MO_ANGLE");
    int p = options_get_int("ROTATE_MO_P") -1;  // P, Q and IRREPS are one-based
    int q = options_get_int("ROTATE_MO_Q") -1;
    int h = options_get_int("ROTATE_MO_IRREP") - 1;

    fprintf(outfile,"\n\n  Rotating MOs %d and %d of irrep %d by %d degrees",
                    p,q,h,mo_rotate_angle);
    double angle = static_cast<double>(mo_rotate_angle) * acos(-1.0) / 180.0;
    for(int i = 0; i < sopi[h]; ++i){
      double Cp = cos(angle) * C->get(h,i,p) + sin(angle) * C->get(h,i,q);
      double Cq = cos(angle) * C->get(h,i,q) - sin(angle) * C->get(h,i,p);
      C->set(h,i,p,Cp);
      C->set(h,i,q,Cq);
    }
  }

  double** C_save;
  allocate2(double,C_save,nso,nso);

  for(int h = 0; h < nirreps; ++h)
    for(int i = 0; i < sopi[h]; ++i)
      for(int j = 0; j < sopi[h]; ++j)
        C_save[i + block_offset[h]][j + block_offset[h]] = C->get(h,i,j);
  _default_chkpt_lib_->wt_scf(C_save);

  release2(C_save);

  int k = 0;
  double* evals = new double[nso];
  for(int h = 0; h < nirreps; ++h){
    for(int i = 0; i < sopi[h]; ++i){
      evals[k] = epsilon->get(h,i);
      k++;
    }
  }
  _default_chkpt_lib_->wt_evals(evals);
  delete[] evals;
}

}} /* End Namespaces */
