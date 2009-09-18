#include <cstdlib>
#include <iostream>
#include <cmath>
#include <cstdio>

#include <liboptions/liboptions.h>
#include <libutil/libutil.h>

#include "scf.h"

extern FILE* outfile;

namespace psi{ namespace mcscf{

SCF::SCF()
{
  startup();
}

SCF::~SCF()
{
  cleanup();
}

void SCF::startup()
{
  ioff    = moinfo_scf->get_ioff();
  nirreps = moinfo_scf->get_nirreps();
  nso     = moinfo_scf->get_nso();
  sopi    = moinfo_scf->get_sopi();

  docc    = moinfo_scf->get_docc();
  actv    = moinfo_scf->get_actv();

  // Compute the block_offset for the so basis
  allocate1(double,block_offset,nirreps);
  block_offset[0] = 0;
  for(int h=1; h < nirreps; ++h){
    block_offset[h] = block_offset[h-1] + sopi[h-1];
  }

  // Allocate the pairs
  allocate1(int,pairpi,nirreps);
  allocate1(int,pair_offset,nirreps);
  allocate2(int,pair,nso,nso);
  allocate2(int,pair_sym,nso,nso);
  pairs = 0;
  PK = 0;
  K = 0;

  if(options_get_str("REFERENCE")=="RHF"){
    reference = rhf;
  }
  else if(options_get_str("REFERENCE")=="ROHF"){
    reference = rohf;
  }
  else if(options_get_str("REFERENCE")=="UHF"){
    reference = uhf;
  }
  else if(options_get_str("REFERENCE")=="TWOCON"){
    reference = tcscf;
    if(moinfo_scf->get_guess_occupation()){
      printf("\n  ERROR:  MCSCF cannot guess the active orbital occupation\n");
      fprintf(outfile,"\n\n  MCSCF cannot guess the active orbital occupation\n");
      fflush(outfile);
      exit(1);
    }
  }

  root = options_get_int("ROOT") - 1;

  // OUT OF CORE ALGORITHM
  out_of_core  = false;

  // DIIS
  use_diis = options_get_bool("USE_DIIS");
  ndiis    = options_get_int("NDIIS");
  current_diis = 0;

  turn_on_actv = options_get_int("TURN_ON_ACTV");

  epsilon     .allocate("epsilon",nirreps,sopi);

  e           .allocate("e",nirreps,sopi,sopi);
  C           .allocate("C",nirreps,sopi,sopi);
  C_t         .allocate("C_t",nirreps,sopi,sopi);
  C_T         .allocate("C_T",nirreps,sopi,sopi);
  Dc          .allocate("Dc",nirreps,sopi,sopi);
  Feff_t      .allocate("Feff_t",nirreps,sopi,sopi);
  Feff_t_old  .allocate("Feff_t",nirreps,sopi,sopi);
  Feff_oAO    .allocate("Feff_oAO",nirreps,sopi,sopi);
  Fc          .allocate("Fc",nirreps,sopi,sopi);
  Fc_t        .allocate("Fc_t",nirreps,sopi,sopi);
  G           .allocate("G",nirreps,sopi,sopi);
  H           .allocate("H",nirreps,sopi,sopi);
  O           .allocate("O",nirreps,sopi,sopi);
  S           .allocate("S",nirreps,sopi,sopi);
  S_sqrt_inv  .allocate("S^-1/2",nirreps,sopi,sopi);
  S_sqrt      .allocate("S^1/2",nirreps,sopi,sopi);
  T           .allocate("T",nirreps,sopi,sopi);

  for(int i = 0; i < ndiis; ++i){
    diis_F[i] .allocate("diis_F[" + to_string(i) + "]",nirreps,sopi,sopi);
    diis_e[i] .allocate("diis_e[" + to_string(i) + "]",nirreps,sopi,sopi);
  }

  if(reference == rohf){
    Do        .allocate("Do",nirreps,sopi,sopi);
    Fo        .allocate("Fo",nirreps,sopi,sopi);
    Fo_t      .allocate("Fo_t",nirreps,sopi,sopi);
  }
  if(reference == tcscf){
    int count = 0;
    for(int h = 0; h < nirreps; ++h){
      for(int n = 0; n < actv[h]; ++n){
        tcscf_sym[count] = h;
        tcscf_mos[count] = docc[h] + n;
        count++;
      }
    }

    nci = count;
    fprintf(outfile,"\n  TWOCON MOs = [");
    for(int I = 0; I < nci; ++I)
      fprintf(outfile,"%d (%s)%s",tcscf_mos[I] + block_offset[tcscf_sym[I]],
                                 moinfo_scf->get_irr_labs(tcscf_sym[I]),
                                 I != nci - 1 ? "," : "");
    fprintf(outfile,"]");

    Favg      .allocate("Favg",nirreps,sopi,sopi);
    Favg_t    .allocate("Favg_t",nirreps,sopi,sopi);

    allocate1(double,ci,nci);
    allocate1(double,ci_grad,nci);
    allocate2(double,H_tcscf,nci,nci);
    for(int I = 0; I < nci; ++I){
      Dtc[I]    .allocate("Dtc[" + to_string(I) + "]",nirreps,sopi,sopi);
      Dsum[I]   .allocate("Dsum[" + to_string(I) + "]",nirreps,sopi,sopi);
      Ftc[I]    .allocate("Ftc[" + to_string(I) + "]",nirreps,sopi,sopi);
      Ftc_t[I]  .allocate("Ftc_t[" + to_string(I) + "]",nirreps,sopi,sopi);
      ci[I] = 0.0;// (I == 0 ? 0.7071067811865475244 : -0.7071067811865475244) ;
    }
    if(options_get_bool("FORCE_TWOCON")){
      ci[0] =   0.7071067811865475244;
      ci[1] = - 0.7071067811865475244;
    }
  }
}

void SCF::cleanup()
{
  release1(block_offset);
  release1(pairpi);
  release1(pair_offset);
  release1(pairs);
  release2(pair);
  release2(pair_sym);

  if(reference == tcscf){
    release1(ci);
    release1(ci_grad);
    release2(H_tcscf);
  }

  release1(PK);
  release1(K);
}

void SCF::transform(SBlockMatrix& Initial, SBlockMatrix& Final, SBlockMatrix& Transformation){
  T.multiply(false,false,Initial,Transformation);
  Final.multiply(true,false,Transformation,T);
}

}} /* End Namespaces */
