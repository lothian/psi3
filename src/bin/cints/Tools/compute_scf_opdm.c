#include<stdio.h>
#include<string.h>
#include<libciomr.h>
#include<file30.h>
#include<math.h>
#include<libint.h>

#include"defines.h"
#define EXTERN
#include"global.h"
#include"prints.h"
#include"small_fns.h"

#define VIRT_SHELL -1000000       /* This should cause program to segfault if arrays are overrun */

void compute_scf_opdm()
{
  int i,j,k,l,mo,open_1st,openshell_count,closedmos;
  int *symblk_offset;                           /* The pointer to the first MO in the symmetry block */
  int *mo2moshell;                              /* array maping MO number to the mo shell */
  double *occ;                                  /* occupation vector - num_so long */
  double *shell_occ;                            /* occupation numbers for MO shells */
  double **mo_lagr;                                /* MO lagrangian */
  double tmp;
  
  /*--- Compute offsets of symmetry blocks ---*/
  symblk_offset = init_int_array(Symmetry.nirreps);
  symblk_offset[0] = 0;
  for(i=0;i<Symmetry.nirreps-1;i++)
    symblk_offset[i+1] = symblk_offset[i] + MOInfo.orbspi[i];

  /*----------------------------------------------------
    Compute occupation vector for spin-restricted cases
   ----------------------------------------------------*/
  if (UserOptions.reftype != uhf) {
    occ = init_array(MOInfo.num_mo);
    mo2moshell = init_int_array(MOInfo.num_mo);
    closedmos = (MOInfo.num_moshells != MOInfo.num_openmoshells);
    shell_occ = init_array(MOInfo.num_moshells);
    if (closedmos > 0)
      shell_occ[0] = 2.0;
    if (UserOptions.reftype != twocon) {	/* single-reference case */
      openshell_count = (closedmos > 0);
      for (i=0;i<Symmetry.nirreps;i++) {
	mo = symblk_offset[i];
	for (j=0;j<MOInfo.clsdpi[i];j++) {
	  mo2moshell[mo] = 0;
	  occ[mo] = 2.0;
	  mo++;
	}
	if (MOInfo.openpi[i] > 0) {
	  for (j=0;j<MOInfo.openpi[i];j++) {
	    mo2moshell[mo] = openshell_count;
	    occ[mo] = 1.0;
	    mo++;
	  }
	  shell_occ[openshell_count] = 1.0;
	  openshell_count++;
	}
	for (j=0;j<MOInfo.orbspi[i]-MOInfo.clsdpi[i]-MOInfo.openpi[i];j++) {
	  mo2moshell[mo] = VIRT_SHELL;
	  mo++;
	}
      }
    }
    else {		/* TCSCF for closed shells */
      openshell_count = (closedmos > 0);
      k = 0;
      for (i=0;i<Symmetry.nirreps;i++) {
	mo = symblk_offset[i];
	for (j=0;j<MOInfo.clsdpi[i];j++) {
	  mo2moshell[mo] = 0;
	  occ[mo] = 2.0;
	  mo++;
	}
	if (MOInfo.openpi[i] > 0) {
	  for (j=0;j<MOInfo.openpi[i];j++) {
	    mo2moshell[mo] = openshell_count;
	    occ[mo] = MOInfo.tcscf_occ[k];
	    mo++;
	  }
	  shell_occ[openshell_count] = MOInfo.tcscf_occ[k];
	  openshell_count++;
	  k++;
	}
	for (j=0;j<MOInfo.orbspi[i]-MOInfo.clsdpi[i]-MOInfo.openpi[i];j++) {
	  mo2moshell[mo] = VIRT_SHELL;
	  mo++;
	}
      }
    }
  }  /*--- Done with occupations ---*/


  /*-----------------------------------
    Allocate and compute total denisty
   -----------------------------------*/
  Dens = block_matrix(BasisSet.num_ao,BasisSet.num_ao);
  if (UserOptions.reftype != uhf) {
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<=i;j++) {
	tmp = 0.0;
	for(k=0;k<Symmetry.nirreps;k++) {
	  mo = symblk_offset[k];
	  for(l=0; l < (MOInfo.clsdpi[k]+MOInfo.openpi[k]); l++) {
	    tmp += occ[mo]*MOInfo.scf_evec[0][mo][i]*MOInfo.scf_evec[0][mo][j];
	    mo++;
	  }
	}
	Dens[j][i] = Dens[i][j] = tmp;
      }
  }
  else {
    /* UHF case : Alpha and Beta eigenvectors */
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<=i;j++) {
	tmp = 0.0;
	for(k=0;k<Symmetry.nirreps;k++) {
	  mo = symblk_offset[k];
	  /*--- Doubly-occupied MOs : Alpha and Beta ---*/
	  for(l=0; l < MOInfo.clsdpi[k]; l++) {
	    tmp += MOInfo.scf_evec[0][mo][i]*
		   MOInfo.scf_evec[0][mo][j]
		+  MOInfo.scf_evec[1][mo][i]*
		   MOInfo.scf_evec[1][mo][j];
	    mo++;
	  }
	  /*--- Singly-occupied MOs : Alpha only ---*/
	  for(l=0; l < MOInfo.openpi[k]; l++) {
	    tmp += MOInfo.scf_evec[0][mo][i]*
		   MOInfo.scf_evec[0][mo][j];
	    mo++;
	  }
	}
	Dens[j][i] = Dens[i][j] = tmp;
      }
  }

  
  /*--------------------------------
    Compute energy-weighted density
   --------------------------------*/
  Lagr = block_matrix(BasisSet.num_ao,BasisSet.num_ao);
  if (UserOptions.reftype == rhf) {
    /*--- In closed-shell case compute energy-weighted density from SCF eigenvalues ---*/
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<=i;j++) {
	tmp = 0.0;
	for(k=0;k<Symmetry.nirreps;k++) {
	  mo = symblk_offset[k];
	  for(l=0; l < (MOInfo.clsdpi[k]+MOInfo.openpi[k]); l++) {
	    tmp += occ[mo]*MOInfo.scf_evals[0][mo]*
		   MOInfo.scf_evec[0][mo][i]*
		   MOInfo.scf_evec[0][mo][j];
	    mo++;
	  }
	}
	Lagr[j][i] = Lagr[i][j] = tmp;
      }
  }
  else if (UserOptions.reftype == uhf) {
    /*--- Use both alpha and beta eigenvalues in UHF case ---*/
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<=i;j++) {
	tmp = 0.0;
	for(k=0;k<Symmetry.nirreps;k++) {
	  mo = symblk_offset[k];
	  /*--- Doubly-occupied MOs : Alpha and Beta ---*/
	  for(l=0; l < MOInfo.clsdpi[k]; l++) {
	    tmp += MOInfo.scf_evals[0][mo]*
		   MOInfo.scf_evec[0][mo][i]*
		   MOInfo.scf_evec[0][mo][j]
		+  MOInfo.scf_evals[1][mo]*
		   MOInfo.scf_evec[1][mo][i]*
		   MOInfo.scf_evec[1][mo][j];
	    mo++;
	  }
	  /*--- Singly-occupied MOs : Alpha only ---*/
	  for(l=0; l < MOInfo.openpi[k]; l++) {
	    tmp += MOInfo.scf_evals[0][mo]*
		   MOInfo.scf_evec[0][mo][i]*
		   MOInfo.scf_evec[0][mo][j];
	    mo++;
	  }
	}
	Lagr[j][i] = Lagr[i][j] = tmp;
      }
  }
  else if (UserOptions.reftype == rohf || UserOptions.reftype == twocon) {
    /*--- In a restricted open-shell case just transform the lagrangian to AO basis ---*/
    mo_lagr = file30_rd_lagr();
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<=i;j++) {
	tmp = 0.0;
	for(k=0;k<MOInfo.num_mo;k++)
	  for(l=0;l<MOInfo.num_mo;l++)
	    tmp += mo_lagr[k][l]*MOInfo.scf_evec[0][k][i]*MOInfo.scf_evec[0][l][j];
	Lagr[j][i] = Lagr[i][j] = tmp;
      }
  }

  /*----------------------------------------------
    Compute MO shell densities for ROHF and TCSCF
   ----------------------------------------------*/
  if (UserOptions.reftype == rohf || UserOptions.reftype == twocon) {
    ShDens = (double ***) malloc(sizeof(double **)*MOInfo.num_moshells);
    for(i=0;i<MOInfo.num_moshells;i++)
      ShDens[i] = block_matrix(BasisSet.num_ao,BasisSet.num_ao);
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<=i;j++) {
	for(k=0;k<Symmetry.nirreps;k++) {
	  mo = symblk_offset[k];
	  for(l=0; l < (MOInfo.clsdpi[k]+MOInfo.openpi[k]); l++) {
	    ShDens[mo2moshell[mo]][j][i] = (ShDens[mo2moshell[mo]][i][j] +=
						MOInfo.scf_evec[0][mo][i]*MOInfo.scf_evec[0][mo][j]);
	    mo++;
	  }
	}
      }
    
    /* Check if the total density is a sum of shell densities - REMOVE AFTER DONE TESTING */
    for(i=0;i<BasisSet.num_ao;i++)
      for(j=0;j<BasisSet.num_ao;j++) {
	tmp = 0.0;
	for(k=0;k<MOInfo.num_moshells;k++)
	  tmp += ShDens[k][i][j]*shell_occ[k];
	if (fabs(tmp - Dens[i][j]) > 1.0e-12)
	  punt("Total density is not a sum of shell densities");
      }

    /*--- Do this dirty trick because some people decided to keep
          open-shells of diff irreps separate ---*/
    /*--- form a single open-shell if rohf and high-spin ---*/
/*    if (UserOptions.reftype == rohf) {
      i = (closedmos > 0) ? 1 : 0;
      for(j=i+1;j<MOInfo.num_moshells;j++) {
	for(k=0;k<BasisSet.num_ao;k++)
	  for(l=0;l<BasisSet.num_ao;l++)
	    ShDens[i][k][l] += ShDens[j][k][l];
	free_block(ShDens[j]);
      }
      BasisSet.num_moshells = 1 + i;
    }*/

    if (UserOptions.print_lvl >= PRINT_OPDM) {
      for(i=0;i<MOInfo.num_moshells;i++) {
	fprintf(outfile,"  -Density matrix for shell %d in AO basis :\n",i);
	print_mat(ShDens[i],BasisSet.num_ao,BasisSet.num_ao,outfile);
	fprintf(outfile,"\n\n");
      }
    }
  }

  print_opdm();

		/* Cleaning up */

  free(symblk_offset);
  if (UserOptions.reftype != uhf) {
    free(occ);
    free(mo2moshell);
    free(shell_occ);
  }
  if (UserOptions.reftype == rohf || UserOptions.reftype == twocon)
    free_block(mo_lagr);
  
  return;
}
