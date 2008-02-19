/***************************************************************************
 *  PSIMRCC : Copyright (C) 2007 by Francesco Evangelista and Andrew Simmonett
 *  frank@ccc.uga.edu   andysim@ccc.uga.edu
 *  A multireference coupled cluster code
 ***************************************************************************/
#include "calculation_options.h"
#include "moinfo.h"
#include "memory_manager.h"`
#include "mrcc.h"
#include "matrix.h"
#include "blas.h"
#include "debugging.h"
#include "utilities.h"

extern FILE* outfile;

using namespace std;

#include <libpsio/psio.h>

namespace psi{ namespace psimrcc{


void CCMRCC::update_amps_mkccsd_wrapper()
{
  ptr->update_amps_mkccsd();
}

void CCMRCC::update_amps_mkccsd()
{
//   update_t1_amps_mkccsd();
//   update_t2_amps_mkccsd();

  update_t1_t2_amps_mkccsd();
  if(triples_type>ccsd)
    update_t3_amps_mkccsd();
  zero_internal_delta_amps();


  blas->solve("||Delta_t1||{u}  = t1_delta[o][v]{u} . t1_delta[o][v]{u}");
  blas->solve("||Delta_t1||{u} += t1_delta[O][V]{u} . t1_delta[O][V]{u}");

  blas->solve("||Delta_t2||{u}  = t2_delta[oo][vv]{u} . t2_delta[oo][vv]{u}");
  blas->solve("||Delta_t2||{u} += t2_delta[oO][vV]{u} . t2_delta[oO][vV]{u}");
  blas->solve("||Delta_t2||{u} += t2_delta[OO][VV]{u} . t2_delta[OO][VV]{u}");

  // Compute the T-AMPS difference
  delta_t1_amps=0.0;
  delta_t2_amps=0.0;
  for(int n=0;n<moinfo->get_ref_size("a");n++){
    delta_t1_amps+=blas->get_scalar("||Delta_t1||",moinfo->get_ref_number("a",n));
    delta_t2_amps+=blas->get_scalar("||Delta_t2||",moinfo->get_ref_number("a",n));
  }
  delta_t1_amps=pow(delta_t1_amps,0.5)/((double)moinfo->get_nrefs());
  delta_t2_amps=pow(delta_t2_amps,0.5)/((double)moinfo->get_nrefs());
}


void CCMRCC::update_t1_t2_amps_mkccsd()
{
  Timer timer;
  DEBUGGING(1,
    fprintf(outfile,"\n\tUpdating the t_ia,t_IA,t_ijab,t_iJaB,t_IJAB amps using the MkCCSD equations ...");
    fflush(outfile);
  );
  blas->solve("d'1[o][v]{u}  = d1[o][v]{u}");
  blas->solve("d'1[O][V]{u}  = d1[O][V]{u}");

  blas->solve("d'2[oo][vv]{u}  = d2[oo][vv]{u}");
  blas->solve("d'2[oO][vV]{u}  = d2[oO][vV]{u}");
  blas->solve("d'2[OO][VV]{u}  = d2[OO][VV]{u}");

  for(int n=0;n<moinfo->get_nunique();n++){
    int m = moinfo->get_ref_number("u",n);
    string shift = to_string(current_energy-Heff[m][m]);
    blas->solve("d'1[o][v]{" + to_string(m) + "} += " + shift);
    blas->solve("d'1[O][V]{" + to_string(m) + "} += " + shift);
    blas->solve("d'2[oo][vv]{" + to_string(m) + "} += " + shift);
    blas->solve("d'2[oO][vV]{" + to_string(m) + "} += " + shift);
    blas->solve("d'2[OO][VV]{" + to_string(m) + "} += " + shift);
  }

  for(int i=0;i<moinfo->get_nunique();i++){
    int unique_i = moinfo->get_ref_number("u",i);
    string i_str = to_string(unique_i);
    // Form the coupling terms
    for(int j=0;j<moinfo->get_nrefs();j++){
      int unique_j = moinfo->get_ref_number("a",j);
      string j_str = to_string(unique_j);
      double term = eigenvector[j]/eigenvector[unique_i];
      if(fabs(term)>1.0e5) term = 0.0;
      blas->set_scalar("factor_mk",unique_j,Heff[unique_i][j]*term);
      if(unique_i!=j){
        if(j==unique_j){
          blas->solve("t1_eqns[o][v]{" + i_str + "} += factor_mk{" + j_str + "} t1[o][v]{" + j_str + "}");
          blas->solve("t1_eqns[O][V]{" + i_str + "} += factor_mk{" + j_str + "} t1[O][V]{" + j_str + "}");
        }else{
          blas->solve("t1_eqns[o][v]{" + i_str + "} += factor_mk{" + j_str + "} t1[O][V]{" + j_str + "}");
          blas->solve("t1_eqns[O][V]{" + i_str + "} += factor_mk{" + j_str + "} t1[o][v]{" + j_str + "}");
        }
      }
    }
    if(fabs(eigenvector[unique_i])>1.0e-6){
    // Update t1 for reference i
    blas->solve("t1_delta[o][v]{" + i_str + "}  =   t1_eqns[o][v]{" + i_str + "} / d'1[o][v]{" + i_str + "} - t1[o][v]{" + i_str + "}");
    blas->solve("t1_delta[O][V]{" + i_str + "}  =   t1_eqns[O][V]{" + i_str + "} / d'1[O][V]{" + i_str + "} - t1[O][V]{" + i_str + "}");

    blas->solve("t1[o][v]{" + i_str + "} = t1_eqns[o][v]{" + i_str + "} / d'1[o][v]{" + i_str + "}");
    blas->solve("t1[O][V]{" + i_str + "} = t1_eqns[O][V]{" + i_str + "} / d'1[O][V]{" + i_str + "}");
    }
    zero_internal_amps();

    // Add the contribution from the other references
    for(int j=0;j<moinfo->get_nrefs();j++){
      int unique_j = moinfo->get_ref_number("a",j);
      string j_str = to_string(unique_j);
      double term = eigenvector[j]/eigenvector[unique_i];
      if(fabs(term)>1.0e5) term = 0.0;
      blas->set_scalar("factor_mk",unique_j,Heff[unique_i][j]*term);
      if(unique_i!=j){
        if(j==unique_j){
          // aaaa case
          // + t_ij^ab(nu/mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "}  = t2[oo][vv]{" + j_str + "}");

          // P(ij)t_i^a(nu/mu)t_j^b(nu/mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1324#   t1[o][v]{" + j_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2314# - t1[o][v]{" + j_str + "} X t1[o][v]{" + j_str + "}");

          // -P(ij)P(ab)t_i^a(mu)t_j^b(nu/mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1324# - t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2314#   t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1423#   t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2413# - t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");

          // P(ij)t_i^a(mu)t_j^b(mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1324#   t1[o][v]{" + i_str + "} X t1[o][v]{" + i_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2314# - t1[o][v]{" + i_str + "} X t1[o][v]{" + i_str + "}");
  
          blas->solve("t2_eqns[oo][vv]{" + i_str + "} += factor_mk{" + j_str + "} Mk2[oo][vv]{" + i_str + "}");
  
          // abab case
          // + t_ij^ab(nu/mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "}  = t2[oO][vV]{" + j_str + "}");
  
          // P(ij)t_i^a(nu/mu)t_J^B(nu/mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324#   t1[o][v]{" + j_str + "} X t1[O][V]{" + j_str + "}");
  
          // -P(iJ)P(aB)t_i^a(mu)t_J^B(nu/mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324# - t1[o][v]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324# - t1[o][v]{" + j_str + "} X t1[O][V]{" + i_str + "}");
  
          // P(iJ)t_i^a(mu)t_J^B(mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324#   t1[o][v]{" + i_str + "} X t1[O][V]{" + i_str + "}");
  
          blas->solve("t2_eqns[oO][vV]{" + i_str + "} += factor_mk{" + j_str + "} Mk2[oO][vV]{" + i_str + "}");
  
          // bbbb case
          // + t_ij^ab(nu/mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "}  = t2[OO][VV]{" + j_str + "}");
  
          // P(ij)t_i^a(nu/mu)t_j^b(nu/mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1324#   t1[O][V]{" + j_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2314# - t1[O][V]{" + j_str + "} X t1[O][V]{" + j_str + "}");
  
          // -P(ij)P(ab)t_i^a(mu)t_j^b(nu/mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1324# - t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2314#   t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1423#   t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2413# - t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
  
          // P(ij)t_i^a(mu)t_j^b(mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1324#   t1[O][V]{" + i_str + "} X t1[O][V]{" + i_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2314# - t1[O][V]{" + i_str + "} X t1[O][V]{" + i_str + "}");
  
          blas->solve("t2_eqns[OO][VV]{" + i_str + "} += factor_mk{" + j_str + "} Mk2[OO][VV]{" + i_str + "}");
        }else{
          // aaaa case
          // + t_ij^ab(nu/mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "}  = t2[OO][VV]{" + j_str + "}");
  
          // P(ij)t_i^a(nu/mu)t_j^b(nu/mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1324#   t1[O][V]{" + j_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2314# - t1[O][V]{" + j_str + "} X t1[O][V]{" + j_str + "}");
  
          // -P(ij)P(ab)t_i^a(mu)t_j^b(nu/mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1324# - t1[o][v]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2314#   t1[o][v]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1423#   t1[o][v]{" + i_str + "} X t1[O][V]{" + j_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2413# - t1[o][v]{" + i_str + "} X t1[O][V]{" + j_str + "}");
  
          // P(ij)t_i^a(mu)t_j^b(mu)
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #1324#   t1[o][v]{" + i_str + "} X t1[o][v]{" + i_str + "}");
          blas->solve("Mk2[oo][vv]{" + i_str + "} += #2314# - t1[o][v]{" + i_str + "} X t1[o][v]{" + i_str + "}");
  
          blas->solve("t2_eqns[oo][vv]{" + i_str + "} += factor_mk{" + j_str + "} Mk2[oo][vv]{" + i_str + "}");
  
          // abab case
          // + t_ij^ab(nu/mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "}  = #2143# t2[oO][vV]{" + j_str + "}");
  
          // P(ij)t_i^a(nu/mu)t_J^B(nu/mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324#   t1[O][V]{" + j_str + "} X t1[o][v]{" + j_str + "}");
  
          // -P(iJ)P(aB)t_i^a(mu)t_J^B(nu/mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324# - t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324# - t1[O][V]{" + j_str + "} X t1[O][V]{" + i_str + "}");
  
          // P(iJ)t_i^a(mu)t_J^B(mu)
          blas->solve("Mk2[oO][vV]{" + i_str + "} += #1324#   t1[o][v]{" + i_str + "} X t1[O][V]{" + i_str + "}");
  
          blas->solve("t2_eqns[oO][vV]{" + i_str + "} += factor_mk{" + j_str + "} Mk2[oO][vV]{" + i_str + "}");
  
          // bbbb case
          // + t_ij^ab(nu/mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "}  = t2[oo][vv]{" + j_str + "}");
  
          // P(ij)t_i^a(nu/mu)t_j^b(nu/mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1324#   t1[o][v]{" + j_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2314# - t1[o][v]{" + j_str + "} X t1[o][v]{" + j_str + "}");
  
          // -P(ij)P(ab)t_i^a(mu)t_j^b(nu/mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1324# - t1[O][V]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2314#   t1[O][V]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1423#   t1[O][V]{" + i_str + "} X t1[o][v]{" + j_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2413# - t1[O][V]{" + i_str + "} X t1[o][v]{" + j_str + "}");
  
          // P(ij)t_i^a(mu)t_j^b(mu)
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #1324#   t1[O][V]{" + i_str + "} X t1[O][V]{" + i_str + "}");
          blas->solve("Mk2[OO][VV]{" + i_str + "} += #2314# - t1[O][V]{" + i_str + "} X t1[O][V]{" + i_str + "}");
  
          blas->solve("t2_eqns[OO][VV]{" + i_str + "} += factor_mk{" + j_str + "} Mk2[OO][VV]{" + i_str + "}");
        }

      }
    }
    blas->solve("t2_delta[oo][vv]{" + i_str + "} = t2_eqns[oo][vv]{" + i_str + "} / d'2[oo][vv]{" + i_str + "} - t2[oo][vv]{" + i_str + "}");
    blas->solve("t2_delta[oO][vV]{" + i_str + "} = t2_eqns[oO][vV]{" + i_str + "} / d'2[oO][vV]{" + i_str + "} - t2[oO][vV]{" + i_str + "}");
    blas->solve("t2_delta[OO][VV]{" + i_str + "} = t2_eqns[OO][VV]{" + i_str + "} / d'2[OO][VV]{" + i_str + "} - t2[OO][VV]{" + i_str + "}");

    if(fabs(eigenvector[unique_i])>1.0e-6){
      string damp = to_string(double(options->get_int_option("DAMPING_FACTOR"))/1000.0);
      string one_minus_damp = to_string(1.0-double(options->get_int_option("DAMPING_FACTOR"))/1000.0);
      blas->solve("t2[oo][vv]{" + i_str + "} = " + one_minus_damp + " t2_eqns[oo][vv]{" + i_str + "} / d'2[oo][vv]{" + i_str + "}");
      blas->solve("t2[oO][vV]{" + i_str + "} = " + one_minus_damp + " t2_eqns[oO][vV]{" + i_str + "} / d'2[oO][vV]{" + i_str + "}");
      blas->solve("t2[OO][VV]{" + i_str + "} = " + one_minus_damp + " t2_eqns[OO][VV]{" + i_str + "} / d'2[OO][VV]{" + i_str + "}");
      blas->solve("t2[oo][vv]{" + i_str + "} += " + damp + " t2_old[oo][vv]{" + i_str + "}");
      blas->solve("t2[oO][vV]{" + i_str + "} += " + damp + " t2_old[oO][vV]{" + i_str + "}");
      blas->solve("t2[OO][VV]{" + i_str + "} += " + damp + " t2_old[OO][VV]{" + i_str + "}");
      zero_internal_amps();
      blas->solve("t2_old[oo][vv]{" + i_str + "} = t2[oo][vv]{" + i_str + "}");
      blas->solve("t2_old[oO][vV]{" + i_str + "} = t2[oO][vV]{" + i_str + "}");
      blas->solve("t2_old[OO][VV]{" + i_str + "} = t2[OO][VV]{" + i_str + "}");
    }
  }



//   blas->solve("t1_norm{u}  = t1[o][v]{u} . t1[o][v]{u}");
//   blas->solve("t1_norm{u} += t1[O][V]{u} . t1[O][V]{u}");
// 
//   blas->solve("t2[oo][vv]{c} = t2[oO][vV]{c}");
//   blas->solve("t2[oo][vv]{c} += #2134# - t2[oO][vV]{c}");
// 
//   blas->solve("t2[OO][VV]{c} = t2[oO][vV]{c}");
//   blas->solve("t2[OO][VV]{c} += #2134# - t2[oO][vV]{c}");
//   zero_internal_amps();


  DEBUGGING(3,
//     blas->print("t2_eqns[oo][vv]{u}");
    blas->print("t2[oo][vv]{u}");
//     blas->print("t2_eqns[oO][vV]{u}");
    blas->print("t2[oO][vV]{u}");
//     blas->print("t2_eqns[OO][VV]{u}");
    blas->print("t2[OO][VV]{u}");
  );
}
















void CCMRCC::update_t1_amps_mkccsd()
{
  blas->solve("d'1[o][v]{u}  = d1[o][v]{u}");
  blas->solve("d'1[O][V]{u}  = d1[O][V]{u}");

  for(int n=0;n<moinfo->get_nunique();n++){
    int m = moinfo->get_ref_number("u",n);
    string n_str = to_string(m);
    string shift = to_string(current_energy-Heff[m][m]);
    blas->solve("d'1[o][v]{" + n_str + "} += " + shift);
    blas->solve("d'1[O][V]{" + n_str + "} += " + shift);
  }

  // Add the contribution from the other references
  for(int i=0;i<moinfo->get_nunique();i++){
    int unique_i = moinfo->get_ref_number("u",i);
    string i_str = to_string(unique_i);
    for(int j=0;j<moinfo->get_nunique();j++){
      int unique_j = moinfo->get_ref_number("u",j);
      string j_str = to_string(unique_j);
      if(i!=j){
        double term = eigenvector[unique_j]/eigenvector[unique_i];
        if(fabs(term)>1.0e5) term = 0.0;
        string factor = to_string(Heff[unique_i][unique_j]*term);
        blas->solve("t1_eqns[o][v]{" + i_str + "} += " + factor + " t1[o][v]{" + j_str + "}");
        //blas->solve("t1_eqns[o][v]{" + i_str + "} += factor_mk{u} t1[o][v]{u}");
        blas->solve("t1_eqns[O][V]{" + i_str + "} += " + factor + " t1[O][V]{" + j_str + "}");
      }
    }
    blas->solve("t1_delta[o][v]{" + i_str + "}  =   t1_eqns[o][v]{" + i_str + "} / d'1[o][v]{" + i_str + "} - t1[o][v]{" + i_str + "}");
    blas->solve("t1_delta[O][V]{" + i_str + "}  =   t1_eqns[O][V]{" + i_str + "} / d'1[O][V]{" + i_str + "} - t1[O][V]{" + i_str + "}");
  
    blas->solve("t1[o][v]{" + i_str + "} = t1_eqns[o][v]{" + i_str + "} / d'1[o][v]{" + i_str + "}");
    blas->solve("t1[O][V]{" + i_str + "} = t1_eqns[O][V]{" + i_str + "} / d'1[O][V]{" + i_str + "}");
    zero_internal_amps();
  }

//   blas->solve("t1_delta[o][v]{u}  =   t1_eqns[o][v]{u} / d'1[o][v]{u} - t1[o][v]{u}");
//   blas->solve("t1_delta[O][V]{u}  =   t1_eqns[O][V]{u} / d'1[O][V]{u} - t1[O][V]{u}");
// 
//   blas->solve("t1[o][v]{u} = t1_eqns[o][v]{u} / d'1[o][v]{u}");
//   blas->solve("t1[O][V]{u} = t1_eqns[O][V]{u} / d'1[O][V]{u}");

  blas->solve("t1_norm{u}  = t1[o][v]{u} . t1[o][v]{u}");
  blas->solve("t1_norm{u} += t1[O][V]{u} . t1[O][V]{u}");


  DEBUGGING(3,
    blas->print("t1[o][v]{u}");
    blas->print("t1[O][V]{u}");
  )
}

void CCMRCC::update_t2_amps_mkccsd()
{
  blas->solve("d'2[oo][vv]{u}  = d2[oo][vv]{u}");
  blas->solve("d'2[oO][vV]{u}  = d2[oO][vV]{u}");
  blas->solve("d'2[OO][VV]{u}  = d2[OO][VV]{u}");

  for(int n=0;n<moinfo->get_nunique();n++){
    int m = moinfo->get_ref_number("u",n);
    string shift = to_string(current_energy-Heff[m][m]); 
    string n_str = to_string(m);
    blas->solve("d'2[oo][vv]{" + n_str + "} += " + shift);
    blas->solve("d'2[oO][vV]{" + n_str + "} += " + shift);
    blas->solve("d'2[OO][VV]{" + n_str + "} += " + shift);
  }

  // Add the contribution from the other references
  for(int i=0;i<moinfo->get_nunique();i++){
    int unique_i = moinfo->get_ref_number("u",i);
    string i_str = to_string(unique_i);
    for(int j=0;j<moinfo->get_nunique();j++){
      int unique_j = moinfo->get_ref_number("u",j);
      string j_str = to_string(unique_j);
      if(i!=j){
        double term = eigenvector[unique_j]/eigenvector[unique_i];
        if(fabs(term)>1.0e5) term = 0.0;
        string     factor = to_string(Heff[unique_i][unique_j]*term);
        string neg_factor = to_string(-Heff[unique_i][unique_j]*term);

        // aaaa case
        // + t_ij^ab(nu/mu)
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += " + factor + " t2[oo][vv]{" + j_str + "}");

        // P(ij)t_i^a(nu/mu)t_j^b(nu/mu)
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #1324# " + factor     + " t1[o][v]{" + j_str + "} X t1[o][v]{" + j_str + "}");
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #2314# " + neg_factor + " t1[o][v]{" + j_str + "} X t1[o][v]{" + j_str + "}");

        // -P(ij)P(ab)t_i^a(mu)t_j^b(nu/mu) = - t_i^a(mu)t_j^b(nu/mu)
        //                                    + t_j^a(mu)t_i^b(nu/mu)
        //                                    + t_i^b(mu)t_j^a(nu/mu)
        //                                    - t_j^b(mu)t_i^a(nu/mu)
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #1324# " + neg_factor + " t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #2314# " +     factor + " t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #1423# " +     factor + " t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #2413# " + neg_factor + " t1[o][v]{" + i_str + "} X t1[o][v]{" + j_str + "}");

        // P(ij)t_i^a(mu)t_j^b(mu)
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #1324# " + factor     + " t1[o][v]{" + i_str + "} X t1[o][v]{" + i_str + "}");
        blas->solve("t2_eqns[oo][vv]{" + i_str + "} += #2314# " + neg_factor + " t1[o][v]{" + i_str + "} X t1[o][v]{" + i_str + "}");


        // abab case
        // + t_ij^ab(nu/mu)
        blas->solve("t2_eqns[oO][vV]{" + i_str + "} += " + factor + " t2[oO][vV]{" + j_str + "}");

        // P(ij)t_i^a(nu/mu)t_J^B(nu/mu)    =   t_i^a(nu/mu)t_J^B(nu/mu)
        blas->solve("t2_eqns[oO][vV]{" + i_str + "} += #1324# " + factor     + " t1[o][v]{" + j_str + "} X t1[O][V]{" + j_str + "}");

        // -P(iJ)P(aB)t_i^a(mu)t_J^B(nu/mu) = - t_i^a(mu)t_J^B(nu/mu) - t_J^B(mu)t_i^a(nu/mu)
        blas->solve("t2_eqns[oO][vV]{" + i_str + "} += #1324# " + neg_factor + " t1[o][v]{" + i_str + "} X t1[O][V]{" + j_str + "}");
        blas->solve("t2_eqns[oO][vV]{" + i_str + "} += #1324# " + neg_factor + " t1[o][v]{" + j_str + "} X t1[O][V]{" + i_str + "}");
        //blas->solve("t2_eqns[oO][vV]{" + i_str + "} += #2413# " + neg_factor + " t1[O][V]{" + i_str + "} X t1[o][v]{" + j_str + "}");

        // P(iJ)t_i^a(mu)t_J^B(mu)          =   t_i^a(mu)t_J^B(mu)
        blas->solve("t2_eqns[oO][vV]{" + i_str + "} += #1324# " + factor     + " t1[o][v]{" + i_str + "} X t1[O][V]{" + i_str + "}");


        // bbbb case
        // + t_ij^ab(nu/mu)
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += " + factor + " t2[OO][VV]{" + j_str + "}");

        // P(ij)t_i^a(nu/mu)t_j^b(nu/mu)
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #1324# " + factor     + " t1[O][V]{" + j_str + "} X t1[O][V]{" + j_str + "}");
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #2314# " + neg_factor + " t1[O][V]{" + j_str + "} X t1[O][V]{" + j_str + "}");

        // -P(ij)P(ab)t_i^a(mu)t_j^b(nu/mu)
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #1324# " + neg_factor + " t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #2314# " +     factor + " t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #1423# " +     factor + " t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #2413# " + neg_factor + " t1[O][V]{" + i_str + "} X t1[O][V]{" + j_str + "}");

        // P(ij)t_i^a(mu)t_j^b(mu)
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #1324# " + factor     + " t1[O][V]{" + i_str + "} X t1[O][V]{" + i_str + "}");
        blas->solve("t2_eqns[OO][VV]{" + i_str + "} += #2314# " + neg_factor + " t1[O][V]{" + i_str + "} X t1[O][V]{" + i_str + "}");
      }
    }
    blas->solve("t2_delta[oo][vv]{" + i_str + "} = t2_eqns[oo][vv]{" + i_str + "} / d'2[oo][vv]{" + i_str + "} - t2[oo][vv]{" + i_str + "}");
    blas->solve("t2_delta[oO][vV]{" + i_str + "} = t2_eqns[oO][vV]{" + i_str + "} / d'2[oO][vV]{" + i_str + "} - t2[oO][vV]{" + i_str + "}");
    blas->solve("t2_delta[OO][VV]{" + i_str + "} = t2_eqns[OO][VV]{" + i_str + "} / d'2[OO][VV]{" + i_str + "} - t2[OO][VV]{" + i_str + "}");
  
    blas->solve("t2[oo][vv]{" + i_str + "} = t2_eqns[oo][vv]{" + i_str + "} / d'2[oo][vv]{" + i_str + "}");
    blas->solve("t2[oO][vV]{" + i_str + "} = t2_eqns[oO][vV]{" + i_str + "} / d'2[oO][vV]{" + i_str + "}");
    blas->solve("t2[OO][VV]{" + i_str + "} = t2_eqns[OO][VV]{" + i_str + "} / d'2[OO][VV]{" + i_str + "}");
    zero_internal_amps();
  }

//   blas->solve("t2_delta[oo][vv]{u} = t2_eqns[oo][vv]{u} / d'2[oo][vv]{u} - t2[oo][vv]{u}");
//   blas->solve("t2_delta[oO][vV]{u} = t2_eqns[oO][vV]{u} / d'2[oO][vV]{u} - t2[oO][vV]{u}");
//   blas->solve("t2_delta[OO][VV]{u} = t2_eqns[OO][VV]{u} / d'2[OO][VV]{u} - t2[OO][VV]{u}");
// 
//   blas->solve("t2[oo][vv]{u} = t2_eqns[oo][vv]{u} / d'2[oo][vv]{u}");
//   blas->solve("t2[oO][vV]{u} = t2_eqns[oO][vV]{u} / d'2[oO][vV]{u}");
//   blas->solve("t2[OO][VV]{u} = t2_eqns[OO][VV]{u} / d'2[OO][VV]{u}");

  DEBUGGING(3,
//     blas->print("t2_eqns[oo][vv]{u}");
    blas->print("t2[oo][vv]{u}");
//     blas->print("t2_eqns[oO][vV]{u}");
    blas->print("t2[oO][vV]{u}");
//     blas->print("t2_eqns[OO][VV]{u}");
    blas->print("t2[OO][VV]{u}");
  );
}

void CCMRCC::update_t3_amps_mkccsd()
{
  update_t3_ijkabc_amps_mkccsd();
  update_t3_ijKabC_amps_mkccsd();
  update_t3_iJKaBC_amps_mkccsd();
  update_t3_IJKABC_amps_mkccsd();
}

void CCMRCC::update_t3_ijkabc_amps_mkccsd()
{
  // Loop over references
  for(int mu=0;mu<moinfo->get_nunique();mu++){
    int unique_mu  = moinfo->get_ref_number("u",mu);

    // Grab the temporary matrices
    CCMatTmp  TijkabcMatTmp = blas->get_MatTmp("t3[ooo][vvv]",unique_mu,none);
    CCMatTmp  HijkabcMatTmp = blas->get_MatTmp("t3_eqns[ooo][vvv]",unique_mu,none);

    double*** Tijkabc_matrix = TijkabcMatTmp->get_matrix();
    double*** Hijkabc_matrix = HijkabcMatTmp->get_matrix();

    // Add the Mk coupling terms
    for(int nu=0;nu<moinfo->get_nrefs();nu++){
      if(unique_mu!=nu){
        int unique_nu = moinfo->get_ref_number("a",nu);
        double factor = Heff[unique_mu][nu]*eigenvector[nu]/eigenvector[unique_mu];
  
        // Linear Coupling Terms
        if(triples_coupling_type>=linear){
          double*** Tijkabc_nu_matrix;
          // No spin-flip
          if(nu==unique_nu){
            Tijkabc_nu_matrix = blas->get_MatTmp("t3[ooo][vvv]",unique_nu,none)->get_matrix();
          }else{    // Spin-flip
            Tijkabc_nu_matrix = blas->get_MatTmp("t3[OOO][VVV]",unique_nu,none)->get_matrix();
          }
          for(int h =0; h < moinfo->get_nirreps();h++){
            for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
              for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
                Hijkabc_matrix[h][ijk][abc]+=Tijkabc_nu_matrix[h][ijk][abc]*factor;
              }
            }
          }
        }
        // Quadratic Coupling Terms
        if(triples_coupling_type>=quadratic){
          // Form DELTA_t1 and DELTA_t2
          if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[o][v] = t1[o][v]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oo][vv] = t2[oo][vv]{" + to_string(unique_nu) + "} - t2[oo][vv]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[o][v] = t1[O][V]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oo][vv] = t2[OO][VV]{" + to_string(unique_nu) + "} - t2[oo][vv]{" + to_string(unique_mu) + "}");
          }
          CCMatTmp DELTA_t1 = blas->get_MatTmp("DELTA_t1[o][v]",none);
          CCMatTmp DELTA_t2 = blas->get_MatTmp("DELTA_t2[oo][vv]",none);

          short**   jk_tuples = DELTA_t2->get_left()->get_tuples();
          short**   bc_tuples = DELTA_t2->get_right()->get_tuples();

          for(int h =0; h < moinfo->get_nirreps();h++){
            size_t i_offset  = DELTA_t1->get_left()->get_first(h);
            size_t a_offset  = DELTA_t1->get_right()->get_first(h);     
            for(int a = 0;a <DELTA_t1->get_right_pairpi(h);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1->get_left_pairpi(h);i++){
                int i_abs = i + i_offset;
                for(int jk_sym =0; jk_sym < moinfo->get_nirreps();jk_sym++){
                  size_t jk_offset = DELTA_t2->get_left()->get_first(jk_sym);
                  size_t bc_offset = DELTA_t2->get_right()->get_first(jk_sym);
                  for(int bc = 0;bc <DELTA_t2->get_right_pairpi(jk_sym);bc++){
                    int b = bc_tuples[bc_offset + bc][0];
                    int c = bc_tuples[bc_offset + bc][1];
                    for(int jk = 0;jk <DELTA_t2->get_left_pairpi(jk_sym);jk++){
                      int j = jk_tuples[jk_offset + jk][0];
                      int k = jk_tuples[jk_offset + jk][1];
                      double value = DELTA_t1->get_two_address_element(i_abs,a_abs)*DELTA_t2->get_four_address_element(j,k,b,c)*factor;
                      HijkabcMatTmp->add_six_address_element_Pi_jk_Pa_bc(i_abs,j,k,a_abs,b,c,value);
                    }
                  }
                }
              }
            }
          }
        }
        // Cubic Coupling Terms
        if(triples_coupling_type>=cubic){
          // Form DELTA_t1
          if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[o][v] = t1[o][v]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[o][v] = t1[O][V]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
          }
          CCMatTmp DELTA_t1 = blas->get_MatTmp("DELTA_t1[o][v]",none);

          for(int h_ia =0; h_ia < moinfo->get_nirreps();h_ia++){
            size_t i_offset  = DELTA_t1->get_left()->get_first(h_ia);
            size_t a_offset  = DELTA_t1->get_right()->get_first(h_ia);     
            for(int a = 0;a <DELTA_t1->get_right_pairpi(h_ia);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1->get_left_pairpi(h_ia);i++){
                int i_abs = i + i_offset;
                for(int h_jb =0; h_jb < moinfo->get_nirreps();h_jb++){
                  size_t j_offset  = DELTA_t1->get_left()->get_first(h_jb);
                  size_t b_offset  = DELTA_t1->get_right()->get_first(h_jb);     
                  for(int b = 0;b <DELTA_t1->get_right_pairpi(h_jb);b++){
                    int b_abs = b + b_offset;
                    for(int j = 0;j<DELTA_t1->get_left_pairpi(h_jb);j++){
                      int j_abs = j + j_offset;
                      for(int h_kc =0; h_kc < moinfo->get_nirreps();h_kc++){
                        size_t k_offset  = DELTA_t1->get_left()->get_first(h_kc);
                        size_t c_offset  = DELTA_t1->get_right()->get_first(h_kc);     
                        for(int c = 0;c <DELTA_t1->get_right_pairpi(h_kc);c++){
                          int c_abs = c + c_offset;
                          for(int k = 0;k<DELTA_t1->get_left_pairpi(h_kc);k++){
                            int k_abs = k + k_offset;
                            double value = DELTA_t1->get_two_address_element(i_abs,a_abs) *
                                           DELTA_t1->get_two_address_element(j_abs,b_abs) *
                                           DELTA_t1->get_two_address_element(k_abs,c_abs) * factor;
                            HijkabcMatTmp->add_six_address_element_Pijk(i_abs,j_abs,k_abs,a_abs,b_abs,c_abs,value);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    if(!options->get_bool_option("DIIS_TRIPLES")){
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_vvv[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_ooo[mu][h][ijk];
            if(delta_ijk-delta_abc < 1.0e50)
              Tijkabc_matrix[h][ijk][abc] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift);
          }
        }
      }
    }else{
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        double* diis_error;
        size_t  k = 0;
        allocate1(double,diis_error,TijkabcMatTmp->get_block_sizepi(h));
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_vvv[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_ooo[mu][h][ijk];
              diis_error[k]=0.0;
              if(delta_ijk-delta_abc < 1.0e50){
                diis_error[k] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift) - Tijkabc_matrix[h][ijk][abc];
                Tijkabc_matrix[h][ijk][abc] += diis_error[k];
              }
              k++;
          }
        }
        char data_label[80];
        sprintf(data_label,"%s%d%s_%s_%d_%d","t3_delta[ooo][vvv]{",unique_mu,"}","DIIS",h,diis_step);
        psio_write_entry(MRCC_ON_DISK,data_label,(char*)(diis_error),TijkabcMatTmp->get_block_sizepi(h)*sizeof(double));
        release1(diis_error);
      }
    }
  }
}

void CCMRCC::update_t3_ijKabC_amps_mkccsd()
{
  // Loop over references
  for(int mu=0;mu<moinfo->get_nunique();mu++){
    int unique_mu  = moinfo->get_ref_number("u",mu);

    // Grab the temporary matrices
    CCMatTmp  TijkabcMatTmp = blas->get_MatTmp("t3[ooO][vvV]",unique_mu,none);
    CCMatTmp  HijkabcMatTmp = blas->get_MatTmp("t3_eqns[ooO][vvV]",unique_mu,none);

    double*** Tijkabc_matrix = TijkabcMatTmp->get_matrix();
    double*** Hijkabc_matrix = HijkabcMatTmp->get_matrix();

    // Add the Mk coupling terms
    for(int nu=0;nu<moinfo->get_nrefs();nu++){
      if(unique_mu!=nu){
        int unique_nu = moinfo->get_ref_number("a",nu);
        double factor = Heff[unique_mu][nu]*eigenvector[nu]/eigenvector[unique_mu];

        // Linear Coupling Terms
        if(triples_coupling_type>=linear){
          // No spin-flip
          if(nu==unique_nu){
            double*** Tijkabc_nu_matrix = blas->get_MatTmp("t3[ooO][vvV]",unique_nu,none)->get_matrix();
            for(int h =0; h < moinfo->get_nirreps();h++){
              for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
                for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
                  Hijkabc_matrix[h][ijk][abc]+=Tijkabc_nu_matrix[h][ijk][abc]*factor;
                }
              }
            }
          }else{
            CCMatTmp Tijkabc_nu_MatTmp = blas->get_MatTmp("t3[oOO][vVV]",unique_nu,none);
            short**   ooo_tuples  = TijkabcMatTmp->get_left()->get_tuples();
            short**   vvv_tuples  = TijkabcMatTmp->get_right()->get_tuples();
            for(int h =0; h < moinfo->get_nirreps();h++){
              size_t ooo_offset = TijkabcMatTmp->get_left()->get_first(h);
              size_t vvv_offset = TijkabcMatTmp->get_right()->get_first(h);
              for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
                int a = vvv_tuples[vvv_offset + abc][0];
                int b = vvv_tuples[vvv_offset + abc][1];
                int c = vvv_tuples[vvv_offset + abc][2];
                for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
                  int i = ooo_tuples[ooo_offset + ijk][0];
                  int j = ooo_tuples[ooo_offset + ijk][1];
                  int k = ooo_tuples[ooo_offset + ijk][2];
                  Hijkabc_matrix[h][ijk][abc]+=Tijkabc_nu_MatTmp->get_six_address_element(k,i,j,c,a,b)*factor;
                }
              }
            }
          }
        }
        // Quadratic Coupling Terms
        if(triples_coupling_type>=quadratic){
          // Form DELTA_t1 and DELTA_t2
          if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[o][v] = t1[o][v]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[O][V]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oo][vv] = t2[oo][vv]{" + to_string(unique_nu) + "} - t2[oo][vv]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oO][vV] = t2[oO][vV]{" + to_string(unique_nu) + "} - t2[oO][vV]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[o][v] = t1[O][V]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[o][v]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oo][vv] = t2[OO][VV]{" + to_string(unique_nu) + "} - t2[oo][vv]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oO][vV] = #2143# t2[oO][vV]{" + to_string(unique_nu) + "} - t2[oO][vV]{" + to_string(unique_mu) + "}");
          }
          CCMatTmp DELTA_t1_ov = blas->get_MatTmp("DELTA_t1[o][v]",none);
          CCMatTmp DELTA_t1_OV = blas->get_MatTmp("DELTA_t1[O][V]",none);
          CCMatTmp DELTA_t2_oovv = blas->get_MatTmp("DELTA_t2[oo][vv]",none);
          CCMatTmp DELTA_t2_oOvV = blas->get_MatTmp("DELTA_t2[oO][vV]",none);

          short**   jk_tuples = DELTA_t2_oOvV->get_left()->get_tuples();
          short**   bc_tuples = DELTA_t2_oOvV->get_right()->get_tuples();

          for(int h =0; h < moinfo->get_nirreps();h++){
            size_t i_offset  = DELTA_t1_ov->get_left()->get_first(h);
            size_t a_offset  = DELTA_t1_ov->get_right()->get_first(h);     
            for(int a = 0;a <DELTA_t1_ov->get_right_pairpi(h);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1_ov->get_left_pairpi(h);i++){
                int i_abs = i + i_offset;
                for(int jk_sym =0; jk_sym < moinfo->get_nirreps();jk_sym++){
                  size_t jk_offset = DELTA_t2_oovv->get_left()->get_first(jk_sym);
                  size_t bc_offset = DELTA_t2_oovv->get_right()->get_first(jk_sym);
                  for(int bc = 0;bc <DELTA_t2_oovv->get_right_pairpi(jk_sym);bc++){
                    int b = bc_tuples[bc_offset + bc][0];
                    int c = bc_tuples[bc_offset + bc][1];
                    for(int jk = 0;jk <DELTA_t2_oovv->get_left_pairpi(jk_sym);jk++){
                      int j = jk_tuples[jk_offset + jk][0];
                      int k = jk_tuples[jk_offset + jk][1];
                      double value = DELTA_t1_ov->get_two_address_element(i_abs,a_abs)*
                                      DELTA_t2_oOvV->get_four_address_element(j,k,b,c)*
                                      factor;
                      HijkabcMatTmp->add_six_address_element_Pij_Pab(i_abs,j,k,a_abs,b,c,value);
                      value = DELTA_t1_OV->get_two_address_element(i_abs,a_abs)*
                                     DELTA_t2_oovv->get_four_address_element(j,k,b,c)*
                                     factor;
                      HijkabcMatTmp->add_six_address_element(j,k,i_abs,b,c,a_abs,value);
                    }
                  }
                }
              }
            }
          }
        }
        // Cubic Coupling Terms
        if(triples_coupling_type>=cubic){

          // Form DELTA_t1
           if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[o][v] = t1[o][v]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[O][V]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[o][v] = t1[O][V]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[o][v]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
          }

          CCMatTmp DELTA_t1_ov = blas->get_MatTmp("DELTA_t1[o][v]",none);
          CCMatTmp DELTA_t1_OV = blas->get_MatTmp("DELTA_t1[O][V]",none);

          for(int h_ia =0; h_ia < moinfo->get_nirreps();h_ia++){
            size_t i_offset  = DELTA_t1_ov->get_left()->get_first(h_ia);
            size_t a_offset  = DELTA_t1_ov->get_right()->get_first(h_ia);     
            for(int a = 0;a <DELTA_t1_ov->get_right_pairpi(h_ia);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1_ov->get_left_pairpi(h_ia);i++){
                int i_abs = i + i_offset;
                for(int h_jb =0; h_jb < moinfo->get_nirreps();h_jb++){
                  size_t j_offset  = DELTA_t1_ov->get_left()->get_first(h_jb);
                  size_t b_offset  = DELTA_t1_ov->get_right()->get_first(h_jb);     
                  for(int b = 0;b <DELTA_t1_ov->get_right_pairpi(h_jb);b++){
                    int b_abs = b + b_offset;
                    for(int j = 0;j<DELTA_t1_ov->get_left_pairpi(h_jb);j++){
                      int j_abs = j + j_offset;
                      for(int h_kc =0; h_kc < moinfo->get_nirreps();h_kc++){
                        size_t k_offset  = DELTA_t1_ov->get_left()->get_first(h_kc);
                        size_t c_offset  = DELTA_t1_ov->get_right()->get_first(h_kc);     
                        for(int c = 0;c <DELTA_t1_ov->get_right_pairpi(h_kc);c++){
                          int c_abs = c + c_offset;
                          for(int k = 0;k<DELTA_t1_ov->get_left_pairpi(h_kc);k++){
                            int k_abs = k + k_offset;
                            double value = DELTA_t1_ov->get_two_address_element(i_abs,a_abs) *
                                           DELTA_t1_ov->get_two_address_element(j_abs,b_abs) *
                                           DELTA_t1_OV->get_two_address_element(k_abs,c_abs) * factor;
                            HijkabcMatTmp->add_six_address_element(i_abs,j_abs,k_abs,a_abs,b_abs,c_abs,value);
                            HijkabcMatTmp->add_six_address_element(j_abs,i_abs,k_abs,a_abs,b_abs,c_abs,-value);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    if(!options->get_bool_option("DIIS_TRIPLES")){
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_vvV[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_ooO[mu][h][ijk];
            if(delta_ijk-delta_abc < 1.0e50)
              Tijkabc_matrix[h][ijk][abc] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift);
          }
        }
      }
    }else{
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        double* diis_error;
        size_t  k = 0;
        allocate1(double,diis_error,TijkabcMatTmp->get_block_sizepi(h));
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_vvV[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_ooO[mu][h][ijk];
              diis_error[k]=0.0;
              if(delta_ijk-delta_abc < 1.0e50){
                diis_error[k] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift) - Tijkabc_matrix[h][ijk][abc];
                Tijkabc_matrix[h][ijk][abc] += diis_error[k];
              }
              k++;
          }
        }
        char data_label[80];
        sprintf(data_label,"%s%d%s_%s_%d_%d","t3_delta[ooO][vvV]{",unique_mu,"}","DIIS",h,diis_step);
        psio_write_entry(MRCC_ON_DISK,data_label,(char*)(diis_error),TijkabcMatTmp->get_block_sizepi(h)*sizeof(double));
        release1(diis_error);
      }
    }
  }
}


void CCMRCC::update_t3_iJKaBC_amps_mkccsd()
{
  // Loop over references
  for(int mu=0;mu<moinfo->get_nunique();mu++){
    int unique_mu  = moinfo->get_ref_number("u",mu);

    // Grab the temporary matrices
    CCMatTmp  TijkabcMatTmp = blas->get_MatTmp("t3[oOO][vVV]",unique_mu,none);
    CCMatTmp  HijkabcMatTmp = blas->get_MatTmp("t3_eqns[oOO][vVV]",unique_mu,none);

    double*** Tijkabc_matrix = TijkabcMatTmp->get_matrix();
    double*** Hijkabc_matrix = HijkabcMatTmp->get_matrix();

    // Add the Mk coupling terms
    for(int nu=0;nu<moinfo->get_nrefs();nu++){
      if(unique_mu!=nu){
        int unique_nu = moinfo->get_ref_number("a",nu);
        double factor = Heff[unique_mu][nu]*eigenvector[nu]/eigenvector[unique_mu];

        // Linear Coupling Terms
        if(triples_coupling_type>=linear){
          // No spin-flip
          if(nu==unique_nu){
            double*** Tijkabc_nu_matrix = blas->get_MatTmp("t3[oOO][vVV]",unique_nu,none)->get_matrix();
            for(int h =0; h < moinfo->get_nirreps();h++){
              for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
                for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
                  Hijkabc_matrix[h][ijk][abc]+=Tijkabc_nu_matrix[h][ijk][abc]*factor;
                }
              }
            }
          }else{
            // iJKaBC  = IjkAbc = jkI bcA
            CCMatTmp Tijkabc_nu_MatTmp = blas->get_MatTmp("t3[ooO][vvV]",unique_nu,none);
            short**   ooo_tuples  = TijkabcMatTmp->get_left()->get_tuples();
            short**   vvv_tuples  = TijkabcMatTmp->get_right()->get_tuples();
            for(int h =0; h < moinfo->get_nirreps();h++){
              size_t ooo_offset = TijkabcMatTmp->get_left()->get_first(h);
              size_t vvv_offset = TijkabcMatTmp->get_right()->get_first(h);
              for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
                int a = vvv_tuples[vvv_offset + abc][0];
                int b = vvv_tuples[vvv_offset + abc][1];
                int c = vvv_tuples[vvv_offset + abc][2];
                for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
                  int i = ooo_tuples[ooo_offset + ijk][0];
                  int j = ooo_tuples[ooo_offset + ijk][1];
                  int k = ooo_tuples[ooo_offset + ijk][2];
                  Hijkabc_matrix[h][ijk][abc]+=Tijkabc_nu_MatTmp->get_six_address_element(j,k,i,b,c,a)*factor;
                }
              }
            }
          }
        }

        // Quadratic Coupling Terms
        if(triples_coupling_type>=quadratic){
          // Form DELTA_t1 and DELTA_t2
          if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[o][v] = t1[o][v]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[O][V]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oO][vV] = t2[oO][vV]{" + to_string(unique_nu) + "} - t2[oO][vV]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[OO][VV] = t2[OO][VV]{" + to_string(unique_nu) + "} - t2[OO][VV]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[o][v] = t1[O][V]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[o][v]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[oO][vV] = #2143# t2[oO][vV]{" + to_string(unique_nu) + "} - t2[oO][vV]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[OO][VV] = t2[oo][vv]{" + to_string(unique_nu) + "} - t2[OO][VV]{" + to_string(unique_mu) + "}");
          }
          CCMatTmp DELTA_t1_ov = blas->get_MatTmp("DELTA_t1[o][v]",none);
          CCMatTmp DELTA_t1_OV = blas->get_MatTmp("DELTA_t1[O][V]",none);
          CCMatTmp DELTA_t2_oOvV = blas->get_MatTmp("DELTA_t2[oO][vV]",none);
          CCMatTmp DELTA_t2_OOVV = blas->get_MatTmp("DELTA_t2[OO][VV]",none);

          short**   jk_tuples = DELTA_t2_oOvV->get_left()->get_tuples();
          short**   bc_tuples = DELTA_t2_oOvV->get_right()->get_tuples();

          for(int h =0; h < moinfo->get_nirreps();h++){
            size_t i_offset  = DELTA_t1_ov->get_left()->get_first(h);
            size_t a_offset  = DELTA_t1_ov->get_right()->get_first(h);     
            for(int a = 0;a <DELTA_t1_ov->get_right_pairpi(h);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1_ov->get_left_pairpi(h);i++){
                int i_abs = i + i_offset;
                for(int jk_sym =0; jk_sym < moinfo->get_nirreps();jk_sym++){
                  size_t jk_offset = DELTA_t2_OOVV->get_left()->get_first(jk_sym);
                  size_t bc_offset = DELTA_t2_OOVV->get_right()->get_first(jk_sym);
                  for(int bc = 0;bc <DELTA_t2_OOVV->get_right_pairpi(jk_sym);bc++){
                    int b = bc_tuples[bc_offset + bc][0];
                    int c = bc_tuples[bc_offset + bc][1];
                    for(int jk = 0;jk <DELTA_t2_OOVV->get_left_pairpi(jk_sym);jk++){
                      int j = jk_tuples[jk_offset + jk][0];
                      int k = jk_tuples[jk_offset + jk][1];
                      double value = DELTA_t1_OV->get_two_address_element(i_abs,a_abs)*
                                     DELTA_t2_oOvV->get_four_address_element(j,k,b,c)*
                                     factor;
                      HijkabcMatTmp->add_six_address_element_Pjk_Pbc(j,i_abs,k,b,a_abs,c,value);
                      value = DELTA_t1_ov->get_two_address_element(i_abs,a_abs)*
                                      DELTA_t2_OOVV->get_four_address_element(j,k,b,c)*
                                      factor;
                      HijkabcMatTmp->add_six_address_element(i_abs,j,k,a_abs,b,c,value);
                    }
                  }
                }
              }
            }
          }
        }

        // Cubic Coupling Terms
        if(triples_coupling_type>=cubic){

          // Form DELTA_t1
           if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[o][v] = t1[o][v]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[O][V]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[o][v] = t1[O][V]{" + to_string(unique_nu) + "} - t1[o][v]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t1[O][V] = t1[o][v]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
          }

          CCMatTmp DELTA_t1_ov = blas->get_MatTmp("DELTA_t1[o][v]",none);
          CCMatTmp DELTA_t1_OV = blas->get_MatTmp("DELTA_t1[O][V]",none);

          for(int h_ia =0; h_ia < moinfo->get_nirreps();h_ia++){
            size_t i_offset  = DELTA_t1_ov->get_left()->get_first(h_ia);
            size_t a_offset  = DELTA_t1_ov->get_right()->get_first(h_ia);     
            for(int a = 0;a <DELTA_t1_ov->get_right_pairpi(h_ia);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1_ov->get_left_pairpi(h_ia);i++){
                int i_abs = i + i_offset;
                for(int h_jb =0; h_jb < moinfo->get_nirreps();h_jb++){
                  size_t j_offset  = DELTA_t1_ov->get_left()->get_first(h_jb);
                  size_t b_offset  = DELTA_t1_ov->get_right()->get_first(h_jb);     
                  for(int b = 0;b <DELTA_t1_ov->get_right_pairpi(h_jb);b++){
                    int b_abs = b + b_offset;
                    for(int j = 0;j<DELTA_t1_ov->get_left_pairpi(h_jb);j++){
                      int j_abs = j + j_offset;
                      for(int h_kc =0; h_kc < moinfo->get_nirreps();h_kc++){
                        size_t k_offset  = DELTA_t1_ov->get_left()->get_first(h_kc);
                        size_t c_offset  = DELTA_t1_ov->get_right()->get_first(h_kc);     
                        for(int c = 0;c <DELTA_t1_ov->get_right_pairpi(h_kc);c++){
                          int c_abs = c + c_offset;
                          for(int k = 0;k<DELTA_t1_ov->get_left_pairpi(h_kc);k++){
                            int k_abs = k + k_offset;
                            double value = DELTA_t1_ov->get_two_address_element(i_abs,a_abs) *
                                           DELTA_t1_OV->get_two_address_element(j_abs,b_abs) *
                                           DELTA_t1_OV->get_two_address_element(k_abs,c_abs) * factor;
                            HijkabcMatTmp->add_six_address_element(i_abs,j_abs,k_abs,a_abs,b_abs,c_abs,value);
                            HijkabcMatTmp->add_six_address_element(i_abs,k_abs,j_abs,a_abs,b_abs,c_abs,-value);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    if(!options->get_bool_option("DIIS_TRIPLES")){
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_vVV[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_oOO[mu][h][ijk];
            if(delta_ijk-delta_abc < 1.0e50)
              Tijkabc_matrix[h][ijk][abc] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift);
          }
        }
      }
    }else{
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        double* diis_error;
        size_t  k = 0;
        allocate1(double,diis_error,TijkabcMatTmp->get_block_sizepi(h));
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_vVV[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_oOO[mu][h][ijk];
              diis_error[k]=0.0;
              if(delta_ijk-delta_abc < 1.0e50){
                diis_error[k] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift) - Tijkabc_matrix[h][ijk][abc];
                Tijkabc_matrix[h][ijk][abc] += diis_error[k];
              }
              k++;
          }
        }
        char data_label[80];
        sprintf(data_label,"%s%d%s_%s_%d_%d","t3_delta[oOO][vVV]{",unique_mu,"}","DIIS",h,diis_step);
        psio_write_entry(MRCC_ON_DISK,data_label,(char*)(diis_error),TijkabcMatTmp->get_block_sizepi(h)*sizeof(double));
        release1(diis_error);
      }
    }
  }
}

void CCMRCC::update_t3_IJKABC_amps_mkccsd()
{
  // Loop over references
  for(int mu=0;mu<moinfo->get_nunique();mu++){
    int unique_mu  = moinfo->get_ref_number("u",mu);

    // Grab the temporary matrices
    CCMatTmp  TijkabcMatTmp = blas->get_MatTmp("t3[OOO][VVV]",unique_mu,none);
    CCMatTmp  HijkabcMatTmp = blas->get_MatTmp("t3_eqns[OOO][VVV]",unique_mu,none);

    double*** Tijkabc_matrix = TijkabcMatTmp->get_matrix();
    double*** Hijkabc_matrix = HijkabcMatTmp->get_matrix();



    // Add the Mk coupling terms
    for(int nu=0;nu<moinfo->get_nrefs();nu++){
      if(unique_mu!=nu){
        int unique_nu = moinfo->get_ref_number("a",nu);
        double factor = Heff[unique_mu][nu]*eigenvector[nu]/eigenvector[unique_mu];

        // Linear Coupling Terms
        if(triples_coupling_type>=linear){
          double*** Tijkabc_nu_matrix;
          // No spin-flip
          if(nu==unique_nu){
            Tijkabc_nu_matrix = blas->get_MatTmp("t3[OOO][VVV]",unique_nu,none)->get_matrix();
          }else{    // Spin-flip
            Tijkabc_nu_matrix = blas->get_MatTmp("t3[ooo][vvv]",unique_nu,none)->get_matrix();
          }
          for(int h =0; h < moinfo->get_nirreps();h++){
            for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
              for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
                Hijkabc_matrix[h][ijk][abc]+=Tijkabc_nu_matrix[h][ijk][abc]*factor;
              }
            }
          }
        }
        // Quadratic Coupling Terms
        if(triples_coupling_type>=quadratic){
          // Form DELTA_t1 and DELTA_t2
          if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[O][V] = t1[O][V]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[OO][VV] = t2[OO][VV]{" + to_string(unique_nu) + "} - t2[OO][VV]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[O][V] = t1[o][v]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
            blas->solve("DELTA_t2[OO][VV] = t2[oo][vv]{" + to_string(unique_nu) + "} - t2[OO][VV]{" + to_string(unique_mu) + "}");
          }
          CCMatTmp DELTA_t1 = blas->get_MatTmp("DELTA_t1[O][V]",none);
          CCMatTmp DELTA_t2 = blas->get_MatTmp("DELTA_t2[OO][VV]",none);
          short**   jk_tuples = DELTA_t2->get_left()->get_tuples();
          short**   bc_tuples = DELTA_t2->get_right()->get_tuples();

          for(int h =0; h < moinfo->get_nirreps();h++){
            size_t i_offset  = DELTA_t1->get_left()->get_first(h);
            size_t a_offset  = DELTA_t1->get_right()->get_first(h);     
            for(int a = 0;a <DELTA_t1->get_right_pairpi(h);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1->get_left_pairpi(h);i++){
                int i_abs = i + i_offset;
                for(int jk_sym =0; jk_sym < moinfo->get_nirreps();jk_sym++){
                  size_t jk_offset = DELTA_t2->get_left()->get_first(jk_sym);
                  size_t bc_offset = DELTA_t2->get_right()->get_first(jk_sym);
                  for(int bc = 0;bc <DELTA_t2->get_right_pairpi(jk_sym);bc++){
                    int b = bc_tuples[bc_offset + bc][0];
                    int c = bc_tuples[bc_offset + bc][1];
                    for(int jk = 0;jk <DELTA_t2->get_left_pairpi(jk_sym);jk++){
                      int j = jk_tuples[jk_offset + jk][0];
                      int k = jk_tuples[jk_offset + jk][1];
                      double value = DELTA_t1->get_two_address_element(i_abs,a_abs)*DELTA_t2->get_four_address_element(j,k,b,c)*factor;
                       HijkabcMatTmp->add_six_address_element_Pi_jk_Pa_bc(i_abs,j,k,a_abs,b,c,value);
                    }
                  }
                }
              }
            }
          }
        }
        // Cubic Coupling Terms
        if(triples_coupling_type>=cubic){
          // Form DELTA_t1
          if(nu==unique_nu){
            // No spin-flip 
            blas->solve("DELTA_t1[O][V] = t1[O][V]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
          }else{
            blas->solve("DELTA_t1[O][V] = t1[o][v]{" + to_string(unique_nu) + "} - t1[O][V]{" + to_string(unique_mu) + "}");
          }
          CCMatTmp DELTA_t1 = blas->get_MatTmp("DELTA_t1[o][v]",none);

          for(int h_ia =0; h_ia < moinfo->get_nirreps();h_ia++){
            size_t i_offset  = DELTA_t1->get_left()->get_first(h_ia);
            size_t a_offset  = DELTA_t1->get_right()->get_first(h_ia);     
            for(int a = 0;a <DELTA_t1->get_right_pairpi(h_ia);a++){
              int a_abs = a + a_offset;
              for(int i = 0;i<DELTA_t1->get_left_pairpi(h_ia);i++){
                int i_abs = i + i_offset;
                for(int h_jb =0; h_jb < moinfo->get_nirreps();h_jb++){
                  size_t j_offset  = DELTA_t1->get_left()->get_first(h_jb);
                  size_t b_offset  = DELTA_t1->get_right()->get_first(h_jb);     
                  for(int b = 0;b <DELTA_t1->get_right_pairpi(h_jb);b++){
                    int b_abs = b + b_offset;
                    for(int j = 0;j<DELTA_t1->get_left_pairpi(h_jb);j++){
                      int j_abs = j + j_offset;
                      for(int h_kc =0; h_kc < moinfo->get_nirreps();h_kc++){
                        size_t k_offset  = DELTA_t1->get_left()->get_first(h_kc);
                        size_t c_offset  = DELTA_t1->get_right()->get_first(h_kc);     
                        for(int c = 0;c <DELTA_t1->get_right_pairpi(h_kc);c++){
                          int c_abs = c + c_offset;
                          for(int k = 0;k<DELTA_t1->get_left_pairpi(h_kc);k++){
                            int k_abs = k + k_offset;
                            double value = DELTA_t1->get_two_address_element(i_abs,a_abs) *
                                           DELTA_t1->get_two_address_element(j_abs,b_abs) *
                                           DELTA_t1->get_two_address_element(k_abs,c_abs) * factor;
                            HijkabcMatTmp->add_six_address_element_Pijk(i_abs,j_abs,k_abs,a_abs,b_abs,c_abs,value);
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
    if(!options->get_bool_option("DIIS_TRIPLES")){
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_VVV[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_OOO[mu][h][ijk];
            if(delta_ijk-delta_abc < 1.0e50)
              Tijkabc_matrix[h][ijk][abc] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc]) / (delta_ijk-delta_abc + shift);
          }
        }
      }
    }else{
      // Update the equations
      double shift = current_energy - Heff[unique_mu][unique_mu];
      for(int h =0; h < moinfo->get_nirreps();h++){
        double* diis_error;
        size_t  k = 0;
        allocate1(double,diis_error,TijkabcMatTmp->get_block_sizepi(h));
        for(int abc = 0;abc<TijkabcMatTmp->get_right_pairpi(h);abc++){
          double delta_abc = d3_VVV[mu][h][abc];
          for(int ijk = 0;ijk<TijkabcMatTmp->get_left_pairpi(h);ijk++){
            double delta_ijk = d3_OOO[mu][h][ijk];
              diis_error[k]=0.0;
              if(delta_ijk-delta_abc < 1.0e50){
                diis_error[k] = (Tijkabc_matrix[h][ijk][abc]*(delta_ijk-delta_abc) + Hijkabc_matrix[h][ijk][abc])/(delta_ijk-delta_abc + shift) - Tijkabc_matrix[h][ijk][abc];
                Tijkabc_matrix[h][ijk][abc] += diis_error[k];
              }
              k++;
          }
        }
        char data_label[80];
        sprintf(data_label,"%s%d%s_%s_%d_%d","t3_delta[OOO][VVV]{",unique_mu,"}","DIIS",h,diis_step);
        psio_write_entry(MRCC_ON_DISK,data_label,(char*)(diis_error),TijkabcMatTmp->get_block_sizepi(h)*sizeof(double));
        release1(diis_error);
      }
    }
  }
}


}} /* End Namespaces */