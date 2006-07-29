#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <physconst.h>
#include <masses.h>
#define EXTERN
#include "globals.h"

void transpert(char *pert);
void sort_pert(char *pert, double **pertX, double **pertY, double **pertZ,
	       int irrep_x, int irrep_y, int irrep_z);
void pertbar(char *pert, int irrep_x, int irrep_y, int irrep_z, int anti);
void compute_X(char *pert, char *cart, int irrep, double omega);
void linresp(double **tensor, double A, double B,
	     char *pert_x, int *x_irreps, double omega_x,
	     char *pert_y, int *y_irreps, double omega_y);

void optrot(void)
{
  double ***tensor_rl, ***tensor_pl, ***tensor_rp, **tensor0;
  double **tensor_rl0, **tensor_rl1, **tensor_pl0, **tensor_pl1;
  char **cartcomp;
  int alpha, i, j, k;
  double TrG_rl, TrG_pl, M, nu, bohr2a4, m2a, hbar, prefactor;
  double *rotation_rl, *rotation_pl, *rotation_rp, *rotation_mod, **delta;
  char lbl1[32], lbl2[32], lbl3[32];
  int compute_rl=0, compute_pl=0;

  /* Booleans for convenience */
  if(!strcmp(params.gauge,"LENGTH") || !strcmp(params.gauge,"BOTH")) compute_rl=1;
  if(!strcmp(params.gauge,"VELOCITY") || !strcmp(params.gauge,"BOTH")) compute_pl=1;

  cartcomp = (char **) malloc(3 * sizeof(char *));
  cartcomp[0] = strdup("X");
  cartcomp[1] = strdup("Y");
  cartcomp[2] = strdup("Z");

  tensor_rl = (double ***) malloc(params.nomega * sizeof(double **));
  tensor_pl = (double ***) malloc(params.nomega * sizeof(double **));
  tensor_rp = (double ***) malloc(params.nomega * sizeof(double **));
  for(i=0; i < params.nomega; i++) {
    tensor_rl[i] = block_matrix(3,3);
    tensor_pl[i] = block_matrix(3,3);
    tensor_rp[i] = block_matrix(3,3);
  }
  tensor0 = block_matrix(3,3);
  tensor_rl0 = block_matrix(3,3);
  tensor_rl1 = block_matrix(3,3);
  tensor_pl0 = block_matrix(3,3);
  tensor_pl1 = block_matrix(3,3);
  rotation_rl = init_array(params.nomega);
  rotation_pl = init_array(params.nomega);
  rotation_rp = init_array(params.nomega);
  rotation_mod = init_array(params.nomega);
  delta = block_matrix(params.nomega,3);

  if(compute_pl) {

    /* compute the zero-frequency Rosenfeld tensor for Koch's modified
       velocity optical rotation */
    /* NOTE!!!  The complex conjugation required for the response
       function is handled *implicitly* in the code below because the
       velocity-gauge version of the optical rotation tensor contains
       *two* pure-imaginary operators, p and m.  Thus, even though the
       corresponding perturbed wave functions change sign when we take
       complex conjugates of the p and m operators, the signs cancel,
       giving an identical contribution to the total *zero-frequency*
       Rosenfeld tensor as the original non-complex-conjugate term. */

    sprintf(lbl1, "<<P;L>>_(%5.3f)", 0.0);
    if(!params.restart || !psio_tocscan(CC_INFO, lbl1)) {

      transpert("P");
      sort_pert("P", moinfo.PX, moinfo.PY, moinfo.PZ,
		moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z);
      pertbar("P", moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z, 1);

      /* prepare the magnetic-dipole integrals */
      transpert("L");
      sort_pert("L", moinfo.LX, moinfo.LY, moinfo.LZ,
		moinfo.irrep_Rx, moinfo.irrep_Ry, moinfo.irrep_Rz);
      pertbar("L", moinfo.irrep_Rx, moinfo.irrep_Ry, moinfo.irrep_Rz, 1);

      /* Compute the magnetic-dipole and electric-dipole CC wave functions */
      for(alpha=0; alpha < 3; alpha++) {
	compute_X("P", cartcomp[alpha], moinfo.mu_irreps[alpha], 0);
	compute_X("L", cartcomp[alpha], moinfo.l_irreps[alpha], 0);
      }

      fprintf(outfile, "\n\tComputing %s tensor.\n", lbl1); fflush(outfile);
      linresp(tensor0, -1.0, 0.0, "P", moinfo.mu_irreps, 0.0, "L", moinfo.l_irreps, 0.0);
      psio_write_entry(CC_INFO, lbl1, (char *) tensor0[0], 9*sizeof(double));

      /* Clean up disk space */
      psio_close(CC_LR, 0);
      psio_open(CC_LR, 0);

      for(j=CC_TMP; j <= CC_TMP11; j++) {
	psio_close(j,0);
	psio_open(j,0);
      }
    }
    else {
      fprintf(outfile, "Using %s tensor found on disk.\n", lbl1);
      psio_read_entry(CC_INFO, lbl1, (char *) tensor0[0], 9*sizeof(double));
    }

    if (!strcmp(params.wfn,"CC2")) 
      fprintf(outfile, "\n     CC2 Optical Rotation Tensor (Velocity Gauge): %s\n", lbl1);
    else if(!strcmp(params.wfn,"CCSD"))
      fprintf(outfile, "\n    CCSD Optical Rotation Tensor (Velocity Gauge): %s\n", lbl1);

    fprintf(outfile, "  -------------------------------------------------------------------------\n");
    fprintf(outfile,   "   Evaluated at omega = 0.00 E_h (Inf nm, 0.0 eV, 0.0 cm-1)\n");
    fprintf(outfile, "  -------------------------------------------------------------------------\n");
    mat_print(tensor0, 3, 3, outfile);

  }

  for(i=0; i < params.nomega; i++) {
    zero_mat(tensor_rl0, 3, 3);
    zero_mat(tensor_rl1, 3, 3);
    zero_mat(tensor_pl0, 3, 3);
    zero_mat(tensor_pl1, 3, 3);

    sprintf(lbl1, "1/2 <<Mu;L>>_(%5.3f)", params.omega[i]);
    sprintf(lbl2, "1/2 <<P;L>>_(%5.3f)", params.omega[i]);
    if(!params.restart || ((compute_rl && !psio_tocscan(CC_INFO,lbl1)) || (compute_pl && !psio_tocscan(CC_INFO,lbl2)))) {

      /* prepare the dipole-length and/or dipole-velocity integrals */
      if(compute_rl) {
	transpert("Mu");
	sort_pert("Mu", moinfo.MUX, moinfo.MUY, moinfo.MUZ,
		  moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z);
	pertbar("Mu", moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z, 0);
      }
      if(compute_pl) {
	transpert("P");
	sort_pert("P", moinfo.PX, moinfo.PY, moinfo.PZ,
		  moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z);
	pertbar("P", moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z, 1);
      }

      /* prepare the magnetic-dipole integrals */
      transpert("L");
      sort_pert("L", moinfo.LX, moinfo.LY, moinfo.LZ,
		moinfo.irrep_Rx, moinfo.irrep_Ry, moinfo.irrep_Rz);
      pertbar("L", moinfo.irrep_Rx, moinfo.irrep_Ry, moinfo.irrep_Rz, 1);

      /* Compute the +omega magnetic-dipole and -omega electric-dipole CC wave functions */
      for(alpha=0; alpha < 3; alpha++) {
	if(compute_rl)
	  compute_X("Mu", cartcomp[alpha], moinfo.mu_irreps[alpha], -params.omega[i]);

	if(compute_pl)
	  compute_X("P", cartcomp[alpha], moinfo.mu_irreps[alpha], -params.omega[i]);

	compute_X("L", cartcomp[alpha], moinfo.l_irreps[alpha], params.omega[i]);
      }

      fprintf(outfile, "\n");
      if(compute_rl) {
        fprintf(outfile, "\tComputing %s tensor.\n", lbl1); fflush(outfile);
	linresp(tensor_rl0, -0.5, 0.0, "Mu", moinfo.mu_irreps, -params.omega[i],
		"L", moinfo.l_irreps, params.omega[i]);
	psio_write_entry(CC_INFO, lbl1, (char *) tensor_rl0[0], 9*sizeof(double));
      }
      if(compute_pl) {
        fprintf(outfile, "\tComputing %s tensor.\n", lbl2); fflush(outfile);
	linresp(tensor_pl0, -0.5, 0.0, "P", moinfo.mu_irreps, -params.omega[i],
		"L", moinfo.l_irreps, params.omega[i]);
	psio_write_entry(CC_INFO, lbl2, (char *) tensor_pl0[0], 9*sizeof(double));
      }

      /* Clean up disk space */
      if(strcmp(params.gauge,"BOTH")) { /* don't clean up if we want both gauges */
	psio_close(CC_LR, 0);
	psio_open(CC_LR, 0);
      }

      for(j=CC_TMP; j <= CC_TMP11; j++) {
	psio_close(j,0);
	psio_open(j,0);
      }
    }
    else {
      fprintf(outfile, "\n");
      if(compute_rl) {
	fprintf(outfile, "\tUsing %s tensor found on disk.\n", lbl1); fflush(outfile);
	psio_read_entry(CC_INFO, lbl1, (char *) tensor_rl0[0], 9*sizeof(double));
      }
      if(compute_pl) {
	fprintf(outfile, "\tUsing %s tensor found on disk.\n", lbl2); fflush(outfile);
	psio_read_entry(CC_INFO, lbl2, (char *) tensor_pl0[0], 9*sizeof(double));
      }

    }

    sprintf(lbl1, "1/2 <<Mu;L*>>_(%5.3f)", params.omega[i]);
    sprintf(lbl2, "1/2 <<P*;L*>>_(%5.3f)", params.omega[i]);
    sprintf(lbl3, "<<P;Mu>>_(%5.3f)", params.omega[i]);
    if(!params.restart || ((compute_rl && !psio_tocscan(CC_INFO,lbl1)) || (compute_pl && !psio_tocscan(CC_INFO,lbl2)))) {

      /* prepare the dipole-length or dipole-velocity integrals */
      if(compute_rl) {
	transpert("Mu");
	sort_pert("Mu", moinfo.MUX, moinfo.MUY, moinfo.MUZ,
		  moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z);
	pertbar("Mu", moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z, 0);
      }
      if(compute_pl) {
	transpert("P*");
	sort_pert("P*", moinfo.PX, moinfo.PY, moinfo.PZ,
		  moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z);
	pertbar("P*", moinfo.irrep_x, moinfo.irrep_y, moinfo.irrep_z, 1);
      }

      /* prepare the complex-conjugate of the magnetic-dipole integrals */
      transpert("L*");
      sort_pert("L*", moinfo.LX, moinfo.LY, moinfo.LZ,
		moinfo.irrep_Rx, moinfo.irrep_Ry, moinfo.irrep_Rz);
      pertbar("L*", moinfo.irrep_Rx, moinfo.irrep_Ry, moinfo.irrep_Rz, 1);

      /* Compute the -omega magnetic-dipole and +omega electric-dipole CC wave functions */
      for(alpha=0; alpha < 3; alpha++) {
	if(compute_rl)
	  compute_X("Mu", cartcomp[alpha], moinfo.mu_irreps[alpha], params.omega[i]);
	if(compute_pl)
	  compute_X("P*", cartcomp[alpha], moinfo.mu_irreps[alpha], params.omega[i]);

	compute_X("L*", cartcomp[alpha], moinfo.l_irreps[alpha], -params.omega[i]);
      }

      fprintf(outfile, "\n");
      if(compute_rl) {
	fprintf(outfile, "\tComputing %s tensor.\n", lbl1); fflush(outfile);
	linresp(tensor_rl1, -0.5, 0.0, "Mu", moinfo.mu_irreps, params.omega[i],
		"L*", moinfo.l_irreps, -params.omega[i]);
	psio_write_entry(CC_INFO, lbl1, (char *) tensor_rl1[0], 9*sizeof(double));
      }
      if(compute_pl) {
	fprintf(outfile, "\tComputing %s tensor.\n", lbl2); fflush(outfile);
	linresp(tensor_pl1, -0.5, 0.0, "P*", moinfo.mu_irreps, params.omega[i],
		"L*", moinfo.l_irreps, -params.omega[i]);
	psio_write_entry(CC_INFO, lbl2, (char *) tensor_pl1[0], 9*sizeof(double));
      }

      if(!strcmp(params.gauge,"BOTH")) {
	fprintf(outfile, "\tComputing %s tensor.\n", lbl3); fflush(outfile);
	linresp(tensor_rp[i], 0.5, 0.0, "P", moinfo.mu_irreps, -params.omega[i],
		"Mu", moinfo.l_irreps, params.omega[i]);
	linresp(tensor_rp[i], 0.5, 1.0, "P*", moinfo.mu_irreps, params.omega[i],
		"Mu", moinfo.l_irreps, -params.omega[i]);
	psio_write_entry(CC_INFO, lbl3, (char *) tensor_rp[i][0], 9*sizeof(double));
      }

      /* Clean up disk space */
      psio_close(CC_LR, 0);
      psio_open(CC_LR, 0);

      for(j=CC_TMP; j <= CC_TMP11; j++) {
	psio_close(j,0);
	psio_open(j,0);
      }

    }
    else {
      fprintf(outfile, "\n");
      if(compute_rl) {
	fprintf(outfile, "\tUsing %s tensor found on disk.\n", lbl1); fflush(outfile);
	psio_read_entry(CC_INFO, lbl1, (char *) tensor_rl1[0], 9*sizeof(double));
      }
      if(compute_pl) {
	fprintf(outfile, "\tUsing %s tensor found on disk.\n", lbl2); fflush(outfile);
	psio_read_entry(CC_INFO, lbl2, (char *) tensor_pl1[0], 9*sizeof(double));
      }
      if(!strcmp(params.gauge,"BOTH")) {
	fprintf(outfile, "\tUsing %s tensor found on disk.\n", lbl3); fflush(outfile);
	psio_read_entry(CC_INFO, lbl3, (char *) tensor_rp[i][0], 9*sizeof(double));
      }
    }

    /* sum the two 1/2 tensors */
    for(j=0; j < 3; j++)
      for(k=0; k < 3; k++) {
	if(compute_rl) tensor_rl[i][j][k] = tensor_rl0[j][k] + tensor_rl1[j][k];
	if(compute_pl) tensor_pl[i][j][k] = tensor_pl0[j][k] + tensor_pl1[j][k];
      }

    /* compute the specific rotation */
    for(j=0,M=0.0; j < moinfo.natom ;j++) M += an2masses[(int) moinfo.zvals[j]]; /* amu */
    nu = params.omega[i]; /* hartree */
    bohr2a4 = _bohr2angstroms * _bohr2angstroms * _bohr2angstroms * _bohr2angstroms;
    m2a = _bohr2angstroms * 1.0e-10;
    hbar = _h/(2.0 * _pi);
    prefactor = 1.0e-2 * hbar/(_c * 2.0 * _pi * _me * m2a * m2a);
    prefactor *= prefactor;
    prefactor *= 288.0e-30 * _pi * _pi * _na * bohr2a4;

    if(compute_rl) {
      if (!strcmp(params.wfn,"CC2")) 
	fprintf(outfile, "\n            CC2 Optical Rotation Tensor (Length Gauge):\n");
      else if(!strcmp(params.wfn,"CCSD"))
	fprintf(outfile, "\n           CCSD Optical Rotation Tensor (Length Gauge):\n");

      fprintf(outfile, "  -------------------------------------------------------------------------\n");
      fprintf(outfile,   "   Evaluated at omega = %8.6f E_h (%6.2f nm, %5.3f eV, %8.2f cm-1)\n", params.omega[i],
	      (_c*_h*1e9)/(_hartree2J*params.omega[i]), _hartree2ev*params.omega[i],
	      _hartree2wavenumbers*params.omega[i]);
      fprintf(outfile, "  -------------------------------------------------------------------------\n");
      mat_print(tensor_rl[i], 3, 3, outfile);

      TrG_rl = (tensor_rl[i][0][0] + tensor_rl[i][1][1] + tensor_rl[i][2][2])/(3.0 * params.omega[i]);

      rotation_rl[i] = prefactor * TrG_rl * nu * nu / M;
      fprintf(outfile, "\n   Specific rotation using length-gauge electric-dipole Rosenfeld tensor.\n");
      fprintf(outfile, "\t[alpha]_(%5.3f) = %10.5f deg/[dm (g/cm^3)]\n", params.omega[i], rotation_rl[i]);
    }

    if(compute_pl) {

      if (!strcmp(params.wfn,"CC2")) 
	fprintf(outfile, "\n          CC2 Optical Rotation Tensor (Velocity Gauge):\n");
      else if(!strcmp(params.wfn,"CCSD"))
	fprintf(outfile, "\n         CCSD Optical Rotation Tensor (Velocity Gauge):\n");

      fprintf(outfile, "  -------------------------------------------------------------------------\n");
      fprintf(outfile,   "   Evaluated at omega = %8.6f E_h (%6.2f nm, %5.3f eV, %8.2f cm-1)\n", params.omega[i],
	      (_c*_h*1e9)/(_hartree2J*params.omega[i]), _hartree2ev*params.omega[i],
	      _hartree2wavenumbers*params.omega[i]);
      fprintf(outfile, "  -------------------------------------------------------------------------\n");
      mat_print(tensor_pl[i], 3, 3, outfile);

      TrG_pl = (tensor_pl[i][0][0] + tensor_pl[i][1][1] + tensor_pl[i][2][2])/(3.0 * params.omega[i]);
      TrG_pl /= params.omega[i];

      rotation_pl[i] = prefactor * TrG_pl * nu * nu / M;
      fprintf(outfile, "\n   Specific rotation using velocity-gauge electric-dipole Rosenfeld tensor.\n");
      fprintf(outfile, "\t[alpha]_(%5.3f) = %10.5f deg/[dm (g/cm^3)]\n", params.omega[i], rotation_pl[i]);

      /* subtract the zero-frequency beta tensor */
      for(j=0; j < 3; j++)
	for(k=0; k < 3; k++)
	  tensor_pl[i][j][k] -= tensor0[j][k];

      if (!strcmp(params.wfn,"CC2"))
	fprintf(outfile, "\n        CC2 Optical Rotation Tensor (Modified Velocity Gauge):\n");
      else if(!strcmp(params.wfn,"CCSD"))
	fprintf(outfile, "\n        CCSD Optical Rotation Tensor (Modified Velocity Gauge):\n");

      fprintf(outfile, "  -------------------------------------------------------------------------\n");
      fprintf(outfile,   "   Evaluated at omega = %8.6f E_h (%6.2f nm, %5.3f eV, %8.2f cm-1)\n", params.omega[i],
	      (_c*_h*1e9)/(_hartree2J*params.omega[i]), _hartree2ev*params.omega[i],
	      _hartree2wavenumbers*params.omega[i]);
      fprintf(outfile, "  -------------------------------------------------------------------------\n");
      mat_print(tensor_pl[i], 3, 3, outfile);

      /* compute the specific rotation */
      TrG_pl = (tensor_pl[i][0][0] + tensor_pl[i][1][1] + tensor_pl[i][2][2])/(3.0 * params.omega[i]);
      TrG_pl /= params.omega[i];

      rotation_mod[i] = prefactor * TrG_pl * nu * nu / M;
      fprintf(outfile, "\n   Specific rotation using modified velocity-gauge Rosenfeld tensor.\n");
      fprintf(outfile, "\t[alpha]_(%5.3f) = %10.5f deg/[dm (g/cm^3)]\n", params.omega[i], rotation_mod[i]);
    }

    if(!strcmp(params.gauge,"BOTH")) {
      delta[i][0] = prefactor * (tensor_rp[i][1][2] - tensor_rp[i][2][1]) * nu * nu / M;
      delta[i][1] = prefactor * (tensor_rp[i][2][0] - tensor_rp[i][0][2]) * nu * nu / M;
      delta[i][2] = prefactor * (tensor_rp[i][0][1] - tensor_rp[i][1][0]) * nu * nu / M;
      delta[i][0] /= 6.0 * params.omega[i];
      delta[i][1] /= 6.0 * params.omega[i];
      delta[i][2] /= 6.0 * params.omega[i];
      fprintf(outfile, "\n   Origin-dependence vector for length-gauge rotation deg/[dm (g/cm^3)]/bohr.\n");
      fprintf(outfile, "     Delta_x = %6.2f   Delta_y = %6.2f   Delta_z = %6.2f\n",
	      delta[i][0], delta[i][1], delta[i][2]);
    }
  } /* loop i over nomega */

  if(params.nomega > 1) {  /* print a summary table for multi-wavelength calcs */

    if(compute_rl) {
      fprintf(outfile, "\n   ------------------------------------------\n");
      if (!strcmp(params.wfn,"CC2"))
	fprintf(outfile,   "       CC2 Length-Gauge Optical Rotation\n");
      else
	fprintf(outfile,   "       CCSD Length-Gauge Optical Rotation\n");
      fprintf(outfile,   "   ------------------------------------------\n");

      if(!strcmp(params.gauge,"BOTH")) {
	fprintf(outfile,   "       Omega           alpha                        Delta\n");
	fprintf(outfile,   "    E_h      nm   deg/[dm (g/cm^3)]        deg/[dm (g/cm^3)]/bohr\n");
	fprintf(outfile,   "   -----   ------ ------------------  ----------------------------------\n");
	fprintf(outfile,   "                                          x           y           z      \n");
	for(i=0; i < params.nomega; i++)
	  fprintf(outfile, "   %5.3f   %6.2f      %10.5f    %10.5f  %10.5f  %10.5f\n", params.omega[i], (_c*_h*1e9)/(_hartree2J*params.omega[i]), 
		  rotation_rl[i], delta[i][0], delta[i][1], delta[i][2]);  
      }
      else {
	fprintf(outfile,   "       Omega           alpha\n");
	fprintf(outfile,   "    E_h      nm   deg/[dm (g/cm^3)]\n");
	fprintf(outfile,   "   -----   ------ ------------------\n");
	for(i=0; i < params.nomega; i++)
	  fprintf(outfile, "   %5.3f   %6.2f      %10.5f\n", params.omega[i], (_c*_h*1e9)/(_hartree2J*params.omega[i]), 
		  rotation_rl[i]);  
      }
    }

    if(compute_pl) {
      fprintf(outfile, "\n   ------------------------------------------------------\n");
      if (!strcmp(params.wfn,"CC2"))
	fprintf(outfile,   "            CC2 Velocity-Gauge Optical Rotation\n");
      else
	fprintf(outfile,   "            CCSD Velocity-Gauge Optical Rotation\n");
      fprintf(outfile,   "   ------------------------------------------------------\n");
      fprintf(outfile,   "       Omega           alpha (deg/[dm (g/cm^3)]\n");
      fprintf(outfile, "\n    E_h      nm   Velocity-Gauge  Modified Velocity-Gauge\n");
      fprintf(outfile,   "   -----   ------ --------------  -----------------------\n");
      for(i=0; i < params.nomega; i++)
	fprintf(outfile, "   %5.3f   %6.2f   %10.5f          %10.5f\n", params.omega[i], (_c*_h*1e9)/(_hartree2J*params.omega[i]), rotation_pl[i], rotation_mod[i]);
    }
  }

  free(rotation_rl);
  free(rotation_pl);
  free(rotation_rp);
  free(rotation_mod);
  free_block(delta);

  for(i=0; i < params.nomega; i++) {
    free_block(tensor_rl[i]);
    free_block(tensor_pl[i]);
    free_block(tensor_rp[i]);
  }
  free(tensor_rl);
  free(tensor_pl);
  free(tensor_rp);
  free_block(tensor0);
  free_block(tensor_rl0);
  free_block(tensor_rl1);
  free_block(tensor_pl0);
  free_block(tensor_pl1);

  free(cartcomp[0]);
  free(cartcomp[1]);
  free(cartcomp[2]);
  free(cartcomp);
}
