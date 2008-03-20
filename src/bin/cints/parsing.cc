/*! \file
    \ingroup CINTS
    \brief Parse the input file and command line for CINTS specific options.
*/
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<cmath>
#include<vector>
#include<libipv1/ip_lib.h>
#include<libint/libint.h>
#include<libciomr/libciomr.h>
#include <psifiles.h>

#include"defines.h"
#define EXTERN
#include"global.h"
#include <stdexcept>
#ifdef INCLUDE_Fock
 #include"scf_parsing.h"
#endif

namespace psi { namespace CINTS {
  //! Main parsing routine for input.
  void parsing()
  {
    int errcod;
    int cutoff_exp;
    long int max_bytes;
  
    UserOptions.print_lvl = 1;
    errcod = ip_data("PRINT","%d",&(UserOptions.print_lvl),0);
    
    /*--- This piece of code from CPHF by Ed Seidl ---*/
    if (ip_exist("MEMORY", 0)) {
      fndcor(&max_bytes, infile, outfile);
      UserOptions.max_memory = max_bytes / sizeof(double);
    }
    else
      UserOptions.max_memory = MAX_NUM_DOUBLES;
    UserOptions.memory = UserOptions.max_memory;
    
    cutoff_exp = CUTOFF;
    errcod = ip_data("CUTOFF","%d",&cutoff_exp,0);
    UserOptions.cutoff = 1.0/pow(10.0,(double)cutoff_exp);

    UserOptions.fine_structure_alpha = 1.0;
    errcod = ip_data("FINE_STRUCTURE_ALPHA","%lf",&(UserOptions.fine_structure_alpha),0);

    UserOptions.make_oei = 1;
    UserOptions.make_fock = 0;
    UserOptions.make_eri = 1;
    UserOptions.symm_ints = 1;
    errcod = ip_boolean("MAKE_ERI",&(UserOptions.make_eri),0);
    
    errcod = ip_data("S_FILE","%d",&(IOUnits.itapS),0);
    errcod = ip_data("T_FILE","%d",&(IOUnits.itapT),0);
    errcod = ip_data("V_FILE","%d",&(IOUnits.itapV),0);
    if (UserOptions.make_eri)
      errcod = ip_data("ERI_FILE","%d",&(IOUnits.itap33),0);
    
    UserOptions.empirical_dispersion = 0;
    errcod = ip_boolean("EMPIRICAL_DISPERSION", 
      &UserOptions.empirical_dispersion, 0);
    
    UserOptions.scf_only = 0;
    errcod = ip_string("WFN",&UserOptions.wfn,0);
    if (UserOptions.wfn == NULL)
      throw std::domain_error("Keyword WFN is missing");
    if ((!strcmp("SCF",UserOptions.wfn)) || 
        (!strcmp("SCF_MVD",UserOptions.wfn)))
      UserOptions.scf_only = 1;
    
    UserOptions.num_threads = 1;
    errcod = ip_data("NUM_THREADS","%d",&(UserOptions.num_threads),0);
    if (UserOptions.num_threads < 1)
      UserOptions.num_threads = 1;
    
    UserOptions.restart = 0;
    errcod = ip_boolean("RESTART",&UserOptions.restart,0);
    if (UserOptions.restart) {
      errcod = ip_data("RESTART_TASK","%d",&UserOptions.restart_task,0);
      if (UserOptions.restart_task < 0)
	throw std::domain_error("RESTART_TASK < 0");
    }
    
    std::vector<double> temp(3);
    errcod = ip_double_array("ORIGIN", &temp[0], 3);
    if(errcod == IPE_OK) {
      UserOptions.origin.x = temp[0];
      UserOptions.origin.y = temp[1];
      UserOptions.origin.z = temp[2];
    }
    else {
      UserOptions.origin.x = 0;
      UserOptions.origin.y = 0;
      UserOptions.origin.z = 0;
    }
    UserOptions.origin.Z_nuc = 0;

    UserOptions.E_given = false;
    if (ip_exist("EFIELD",0)) {
      UserOptions.E_given = true;
      errcod = ip_double_array("EFIELD", UserOptions.E, 3);
      if(errcod != IPE_OK)
        throw std::runtime_error("Could not read EFIELD");
      else {
        // if the field is specified also need to query the frame        
        char* frame;
        errcod = ip_string("EFIELD_FRAME",&frame,0);
        if (errcod == IPE_OK) {
          if (!strcmp(frame,"CANONICAL"))
            UserOptions.E_frame = canonical;
          else if (!strcmp(frame,"REFERENCE"))
            UserOptions.E_frame = reference;
          else
            throw std::invalid_argument("Invalid value for keyword EFIELD_FRAME");
          free(frame);
        }
        else {
          UserOptions.E_frame = canonical;
        }
        
      }
    }

    return;
    
  }
  
  //! Parses the command line for options.
  void parsing_cmdline(int argc, char *argv[])
  {
    int i, errcod;
    char *refstring;
    
    for (i=1; i<argc; i++) {
      
      /*--- build Fock option ---*/
      if (strcmp(argv[i], "--fock") == 0) {
#ifdef INCLUDE_Fock
	scf_parsing();
	/*UserOptions.make_oei = 0;
	  UserOptions.make_eri = 0;
	  UserOptions.make_fock = 1;
	  UserOptions.print_lvl = 0;
	  UserOptions.symm_ints = 0;
	  UserOptions.make_dft = 0;
	  errcod = ip_string("REFERENCE",&refstring,0);
	  if (errcod != IPE_OK)
	  throw std::domain_error("REFERENCE keyword is missing");
	  else if (!strcmp(refstring,"RHF") || !strcmp(refstring,""))
	  UserOptions.reftype = rhf;
	  else if (!strcmp(refstring,"ROHF"))
	  UserOptions.reftype = rohf;
	  else if (!strcmp(refstring,"UHF"))
	  UserOptions.reftype = uhf;
	  else if (!strcmp(refstring,"RKS")){
	  UserOptions.reftype = rhf;
	  UserOptions.make_dft = 1;
	  }
	  else if (!strcmp(refstring,"UKS")){
	  UserOptions.reftype = uhf;
	  UserOptions.make_dft = 1;
	  }
	  else
	  throw std::domain_error("The specified REFERENCE not implemented");*/
#else
	throw std::domain_error("--fock option is not supported by your CINTS executable.\nRecompile the code including files in Fock subdirectory.");
#endif
	return;
      }
      
      /*--- build oeints option ---*/
      if (strcmp(argv[i], "--oeints") == 0) {
#ifdef INCLUDE_Default_Ints
	UserOptions.make_oei = 1;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.print_lvl = 0;
	UserOptions.symm_ints = 1;
	UserOptions.num_threads = 1;
#else
	throw std::domain_error("--oeints option is not supported by your CINTS executable.\nRecompile the code including files in Default_Ints subdirectory.");
#endif
	return;
      }
      
      /*--- build ERIs option ---*/
      if (strcmp(argv[i], "--teints") == 0) {
#ifdef INCLUDE_Default_Ints
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 1;
	UserOptions.make_fock = 0;
	UserOptions.print_lvl = 0;
	UserOptions.symm_ints = 1;
	UserOptions.num_threads = 1;
#else
	throw std::domain_error("--teints option is not supported by your CINTS executable.\nRecompile the code including files in Default_Ints subdirectory.");
#endif
	return;
      }
      
      /*--- compute 1st derivatives option ---*/
      if (strcmp(argv[i], "--deriv1") == 0) {
#ifdef INCLUDE_Default_Deriv1
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.make_deriv1 = 1;
        UserOptions.make_deriv1_mvd = 0;
        if (!strcmp("SCF_MVD",UserOptions.wfn))
          UserOptions.make_deriv1_mvd = 1;
	UserOptions.symm_ints = 0;
	UserOptions.dertype = strdup("FIRST");
	if (!strcmp("SCF",UserOptions.wfn)) {
	  errcod = ip_string("REFERENCE",&refstring,0);
	  if (errcod != IPE_OK)
	    throw std::domain_error("REFERENCE keyword is missing");
	  else if (!strcmp(refstring,"RHF") || !strcmp(refstring,""))
	    UserOptions.reftype = rhf;
	  else if (!strcmp(refstring,"ROHF"))
	    UserOptions.reftype = rohf;
	  else if (!strcmp(refstring,"UHF"))
	    UserOptions.reftype = uhf;
	  else if (!strcmp(refstring,"TWOCON"))
	    UserOptions.reftype = twocon;
	  else
	    throw std::domain_error("SCF gradients with specified REFERENCE not implemented");
	}
    if (!strcmp("SCF_MVD",UserOptions.wfn)) {
	  if (UserOptions.reftype != rhf)
	    throw std::domain_error("SCF_MVD gradients with specified REFERENCE not implemented");
    }
	if ((strcmp("SCF",UserOptions.wfn)) && (strcmp("SCF_MVD",UserOptions.wfn))) 
	  UserOptions.num_threads = 1;
#else
	throw std::domain_error("--deriv1 option is not supported by your CINTS executable.\nRecompile the code including files in Default_Deriv1 subdirectory.");
#endif
	return;
      }
      
      /*--- compute 1st derivative integrals option ---*/
      if (strcmp(argv[i], "--deriv1_ints") == 0) {
#ifdef INCLUDE_Default_Deriv1
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.symm_ints = 1;
	UserOptions.make_deriv1 = 1;
	UserOptions.num_threads = 1;
        UserOptions.make_mkpt2_ints = 0;
#else
	throw std::domain_error("--deriv1_ints option is not supported by your CINTS executable.\nRecompile the code including files in Default_Deriv1 subdirectory.");
#endif
	return;
      }
      
      /*--- compute 2nd derivatives ---*/
      if(!strcmp(argv[i], "--deriv2")) {
#ifdef INCLUDE_Default_Deriv2
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.symm_ints = 0;
	UserOptions.make_deriv2 = 1;
        UserOptions.make_mkpt2_ints = 0;
#else
	throw std::domain_error("--deriv2 option is not supported by your CINTS executable.\nRecompile the code including files in Default_Deriv2 subdirectory.");
#endif
	return;
      }
      
      /*--- compute one-electron property integrals option ---*/
      if (strcmp(argv[i], "--oeprop") == 0) {
#ifdef INCLUDE_OEProp_Ints
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.make_oeprop = 1;
	UserOptions.symm_ints = 0;
	UserOptions.print_lvl = 0;
        UserOptions.make_mkpt2_ints = 0;
#else
	throw std::domain_error("--oeprop option is not supported by your CINTS executable.\nRecompile the code including files in OEProp_Ints subdirectory.");
#endif
	return;
      }
      
      /*--- compute MP2 energy option ---*/
      if (strcmp(argv[i], "--mp2") == 0) {
#ifdef INCLUDE_MP2
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.make_deriv1 = 0;
	UserOptions.make_mp2 = 1;
	UserOptions.make_r12ints = 0;
	UserOptions.make_mp2r12 = 0;
	UserOptions.make_cc_bt2 = 0;
	UserOptions.symm_ints = 0;
        UserOptions.make_mkpt2_ints = 0;
	
	errcod = ip_string("REFERENCE",&refstring,0);
	if (errcod != IPE_OK)
	  throw std::domain_error("REFERENCE keyword is missing");
	else if (!strcmp(refstring,"RHF") || !strcmp(refstring,""))
	  UserOptions.reftype = rhf;
	else if (!strcmp(refstring,"UHF")) {
	  UserOptions.reftype = uhf;
	  throw std::domain_error("UMP2 energy evaluation is not yet implemented");
	}
	else
	  throw std::domain_error("MP2 energy evaluation with specified REFERENCE not implemented");
#else
	throw std::domain_error("--mp2 option is not supported by your CINTS executable.\nRecompile the code including files in MP2 subdirectory.");
#endif
	return;
      }
      
      /*--- build te integrals for R12 methods option ---*/
      if (strcmp(argv[i], "--r12ints") == 0) {
#ifdef INCLUDE_R12_Ints
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.make_deriv1 = 0;
	UserOptions.make_mp2 = 0;
	UserOptions.make_r12ints = 1;
	UserOptions.make_mp2r12 = 0;
	UserOptions.make_cc_bt2 = 0;
	UserOptions.symm_ints = 1;
	UserOptions.num_threads = 1;
        UserOptions.make_mkpt2_ints = 0;
#else
	throw std::domain_error("--r12ints option is not supported by your CINTS executable.\nRecompile the code including files in R12_Ints subdirectory.");
#endif
	return;
      }
      
      /*--- compute MP2-R12 energy option ---*/
      if (strcmp(argv[i], "--mp2r12") == 0) {
#ifdef INCLUDE_MP2R12
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.make_deriv1 = 0;
	UserOptions.make_mp2 = 0;
	UserOptions.make_r12ints = 0;
	UserOptions.make_mp2r12 = 1;
	UserOptions.make_cc_bt2 = 0;
	UserOptions.symm_ints = 0;
        UserOptions.make_mkpt2_ints = 0;
	
	errcod = ip_string("REFERENCE",&refstring,0);
	if (errcod != IPE_OK)
	  throw std::domain_error("REFERENCE keyword is missing");
	else if (!strcmp(refstring,"RHF") || !strcmp(refstring,""))
	  UserOptions.reftype = rhf;
	else
	  throw std::domain_error("Direct MP2-R12/A integrals transformation with specified REFERENCE not implemented");
#else
	throw std::domain_error("--mp2r12 option is not supported by your CINTS executable.\nRecompile the code including files in MP2R12 subdirectory.");
#endif
	return;
      }
      
      /*--- compute MkPT2 integrals option ---*/
      if (strcmp(argv[i], "--mkpt2") == 0) {
#ifdef INCLUDE_MkPT2
        UserOptions.make_oei = 0;
        UserOptions.make_eri = 0;
        UserOptions.make_fock = 0;
        UserOptions.make_deriv1 = 0;
        UserOptions.make_mp2 = 0;
        UserOptions.make_r12ints = 0;
        UserOptions.make_mp2r12 = 0;
        UserOptions.make_cc_bt2 = 0;
        UserOptions.symm_ints = 0;
        UserOptions.make_mkpt2_ints = 1;

        errcod = ip_string("REFERENCE",&refstring,0);
        if (errcod != IPE_OK)
          throw std::domain_error("REFERENCE keyword is missing");
        else if (!strcmp(refstring,"RHF") || !strcmp(refstring,""))
          UserOptions.reftype = rhf;
        else if (!strcmp(refstring,"ROHF"))
          UserOptions.reftype = rohf;
        else if (!strcmp(refstring,"TWOCON"))
          UserOptions.reftype = twocon;
        else
          throw std::domain_error("Direct MkPT2 integral computation with specified REFERENCE not implemented");
#else
        throw std::domain_error("--mkpt2 option is not supported by your CINTS executable.\nRecompile the code including files in MKPT2 subdirectory.");
#endif
        return;
      }


      /*--- compute 4-virtuals T2 contribution to CC equations ---*/
      if (strcmp(argv[i], "--cc_bt2") == 0) {
#ifdef INCLUDE_CC
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.make_deriv1 = 0;
	UserOptions.make_mp2 = 0;
	UserOptions.make_mp2r12 = 0;
	UserOptions.make_cc_bt2 = 1;
	UserOptions.symm_ints = 0;
	UserOptions.print_lvl = 0;
        UserOptions.make_mkpt2_ints = 0;
	
	errcod = ip_string("REFERENCE",&refstring,0);
	if (errcod != IPE_OK)
	  throw std::domain_error("REFERENCE keyword is missing");
	else if (!strcmp(refstring,"RHF") || !strcmp(refstring,""))
	  UserOptions.reftype = rhf;
	else
	  throw std::domain_error("Direct CC four-virtuals term computation with specified REFERENCE not implemented");
#else
	throw std::domain_error("--cc_bt2 option is not supported by your CINTS executable.\nRecompile the code including files in CC subdirectory.");
#endif
	return;
      }
      
      /*--- compute derivatives integrals over GIAOs ---*/
      if(!strcmp(argv[i], "--giao_deriv")) {
#ifdef INCLUDE_GIAO_Deriv
	UserOptions.make_oei = 0;
	UserOptions.make_eri = 0;
	UserOptions.make_fock = 0;
	UserOptions.symm_ints = 0;
	UserOptions.make_giao_deriv = 1;
        UserOptions.make_mkpt2_ints = 0;
#else
	throw std::domain_error("--giao_deriv option is not supported by your CINTS executable.\nRecompile the code including files in GIAO_Deriv subdirectory.");
#endif
	return;
      }
      
    }
    
    return;
  }
};
};
