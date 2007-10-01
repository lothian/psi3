/*! \file 
    \ingroup (OPTKING)
    \brief freq_energy_cart(): computes frequencies from energies generated by 
    cartesian displacements - formulas are in disp_freq_energy_cart.cc
*/

#include <math.h>
#include <stdio.h>
#include <libchkpt/chkpt.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>
#include <libipv1/ip_lib.h>
#include <physconst.h>
#include <libpsio/psio.h>
#include <psifiles.h>

#define EXTERN
#include "opt.h"
#undef EXTERN
#include "cartesians.h"
#include "internals.h"
#include "salc.h"
#include "bond_lengths.h"
#define MAX_LINE 132

namespace psi { namespace optking {

void sort_evals_all(int nsalc_all, double *evals_all, int *evals_all_irrep);
FILE *fp_energy_dat;
int iE(int *ndisp, int *nsalc, int irr, int ii, int jj, int disp_i, int disp_j);

void freq_energy_cart(cartesians &carts) {
  int i, j, k, l, dim, natom, cnt_eval = -1, *evals_all_irrep;
  int h, nirreps, *nsalc, *ndisp, ndisp_all, nsalc_all, **ict, *start_irr;
  double **B, **force_constants, energy_ref, *energies, cm_convert, k_convert;
  double *f, tval, **evects, *evals, tmp, disp_size;
  double *disp_E, *evals_all, **cartrep, ***salc, ***disp, **normal;
  int print;
  char *line1;
  print = optinfo.print_cartesians;

  nirreps = syminfo.nirreps;
  natom = carts.get_natom();
  disp_size = optinfo.disp_size;

  chkpt_init(PSIO_OPEN_OLD);
  cartrep = chkpt_rd_cartrep();
  ict = chkpt_rd_ict();
  chkpt_close();

  ndisp = init_int_array(nirreps);
  nsalc = init_int_array(nirreps);

  /* Read in data from PSIF_OPTKING */
  open_PSIF();
  psio_read_entry(PSIF_OPTKING, "OPT: Num. of disp.",
      (char *) &(ndisp_all), sizeof(int));
  psio_read_entry(PSIF_OPTKING, "OPT: Num. of coord.",
      (char *) &(nsalc_all), sizeof(int));
  psio_read_entry(PSIF_OPTKING, "OPT: Disp. per irrep",
      (char *) &(ndisp[0]), nirreps*sizeof(int));
  psio_read_entry(PSIF_OPTKING, "OPT: Coord. per irrep",
      (char *) &(nsalc[0]), nirreps*sizeof(int));
  psio_read_entry(PSIF_OPTKING, "OPT: Reference energy",
      (char *) &(energy_ref), sizeof(double));

  fprintf(outfile,"total nsalc: %d, total ndisp: %d\n", nsalc_all, ndisp_all);
  fprintf(outfile,"nsalc per irrep: "); for (h=0; h<nirreps; ++h) fprintf(outfile,"%d ",nsalc[h]);
  fprintf(outfile,"\n");
  fprintf(outfile,"ndisp per irrep: "); for (h=0; h<nirreps; ++h) fprintf(outfile,"%d ",ndisp[h]);
  fprintf(outfile,"\n\n");

  disp_E = init_array(ndisp_all);

  if (optinfo.energy_dat) { /* read energy.dat text file */
    fprintf(outfile,"Reading displaced energies from energy.dat.\n");
    fp_energy_dat = fopen("energy.dat", "r");
    if (fp_energy_dat == NULL) {
      fprintf(outfile,"energy.dat file not found.\n");
      exit(PSI_RETURN_FAILURE);
    }
    rewind (fp_energy_dat);
    line1 = new char[MAX_LINE+1];
    for (i=0; i<ndisp_all; ++i) {
      fgets(line1, MAX_LINE, fp_energy_dat);
      sscanf(line1, "%lf", &(disp_E[i]));
    }
    fclose(fp_energy_dat);
    delete [] line1;
  }
  else {
    fprintf(outfile,"Reading displaced energies from PSIF_OPTKING.\n");
    psio_read_entry(PSIF_OPTKING, "OPT: Displaced energies",
        (char *) &(disp_E[0]), ndisp_all*sizeof(double));
  }


  B = block_matrix(nsalc_all,3*natom);
  psio_read_entry(PSIF_OPTKING, "OPT: Adapted cartesians",
    (char *) &(B[0][0]), nsalc_all*3*natom*sizeof(double));
  if (print) {
    fprintf(outfile,"Reference energy: %15.10lf\n",energy_ref);
    fprintf(outfile,"Energies of displaced geometries\n");
    for (i=0; i<ndisp_all; ++i) fprintf(outfile, "%15.10lf ", disp_E[i]);
    fprintf(outfile,"\n\n");
    fprintf(outfile,"B matrix (adapted cartesians)\n");
    print_mat(B,nsalc_all,3*natom,outfile);
  }

  /* just for printing out force constants */
  start_irr = init_int_array(nirreps);
  for (h=0; h<nirreps; ++h) {
    for (i=0;i<h;++i)
      start_irr[h] += nsalc[i];
  }

  evals_all = init_array(nsalc_all);
  evals_all_irrep = init_int_array(nsalc_all);

  for (h=0; h<nirreps; ++h) {
    if (!nsalc[h]) continue;

    force_constants = block_matrix(nsalc[h],nsalc[h]);

    /*** compute diagonal force constants ***/
    if (h == 0) {
      for (i=0; i<nsalc[h]; ++i) {
        if (optinfo.points == 3) {
          force_constants[i][i] = (
             + disp_E[iE(ndisp,nsalc,h,i,0,+1,0)]
             + disp_E[iE(ndisp,nsalc,h,i,0,-1,0)]
             - 2.0 * energy_ref) / SQR(disp_size);
        }
        else if (optinfo.points == 5) {
          force_constants[i][i] = (
             -  1.0 * disp_E[iE(ndisp,nsalc,h,i,0,-2,0)]
             + 16.0 * disp_E[iE(ndisp,nsalc,h,i,0,-1,0)]
             + 16.0 * disp_E[iE(ndisp,nsalc,h,i,0, 1,0)]
             -  1.0 * disp_E[iE(ndisp,nsalc,h,i,0, 2,0)]
             - 30.0 * energy_ref ) / (12.0*SQR(disp_size));
        }
      }
    }
    else { /* asymmetric diagonal force constants */
      for (i=0; i<nsalc[h]; ++i) {
        if (optinfo.points == 3) {
          force_constants[i][i] = 
           2.0 * (disp_E[iE(ndisp,nsalc,h,i,0,+1,0)] - energy_ref) / SQR(disp_size);
        }
        else if (optinfo.points == 5) {
          force_constants[i][i] = (
             -  2.0 * disp_E[iE(ndisp,nsalc,h,i,0,-2,0)]
             + 32.0 * disp_E[iE(ndisp,nsalc,h,i,0,-1,0)]
             - 30.0 * energy_ref ) / (12.0*SQR(disp_size));
        }
      }
    }

    /*** compute off-diagonal force constants ***/
    for (i=0; i<nsalc[h]; ++i) {
     // for (j=i+1; j<nsalc[h]; ++j) {
      for (j=0; j<i; ++j) {
        if (optinfo.points == 3) {
          force_constants[i][j] = force_constants[j][i] =
            ( + disp_E[iE(ndisp,nsalc,h,i,j,+1,+1)]
              + disp_E[iE(ndisp,nsalc,h,i,j,-1,-1)]
              + 2.0 * energy_ref
              - disp_E[iE(ndisp,nsalc,h,i,0,+1,0)]
              - disp_E[iE(ndisp,nsalc,h,i,0,-1,0)]
              - disp_E[iE(ndisp,nsalc,h,j,0,+1,0)]
              - disp_E[iE(ndisp,nsalc,h,j,0,-1,0)]
             ) / (2.0*SQR(disp_size));
        }
        else if (optinfo.points == 5) {
          force_constants[i][j] = force_constants[j][i] = (
              - 1.0 * disp_E[iE(ndisp,nsalc,h,i,j,-1,-2)]
              - 1.0 * disp_E[iE(ndisp,nsalc,h,i,j,-2,-1)]
              + 9.0 * disp_E[iE(ndisp,nsalc,h,i,j,-1,-1)]
              - 1.0 * disp_E[iE(ndisp,nsalc,h,i,j,+1,-1)]
              - 1.0 * disp_E[iE(ndisp,nsalc,h,i,j,-1,1)]
              + 9.0 * disp_E[iE(ndisp,nsalc,h,i,j,+1,+1)]
              - 1.0 * disp_E[iE(ndisp,nsalc,h,i,j,+2,+1)]
              - 1.0 * disp_E[iE(ndisp,nsalc,h,i,j,+1,+2)]
              + 1.0 * disp_E[iE(ndisp,nsalc,h,i,0,-2,0)]
              - 7.0 * disp_E[iE(ndisp,nsalc,h,i,0,-1,0)]
              - 7.0 * disp_E[iE(ndisp,nsalc,h,i,0,+1,0)]
              + 1.0 * disp_E[iE(ndisp,nsalc,h,i,0,+2,0)]
              + 1.0 * disp_E[iE(ndisp,nsalc,h,j,0,-2,0)]
              - 7.0 * disp_E[iE(ndisp,nsalc,h,j,0,-1,0)]
              - 7.0 * disp_E[iE(ndisp,nsalc,h,j,0,+1,0)]
              + 1.0 * disp_E[iE(ndisp,nsalc,h,j,0,+2,0)]
              + 12.0 * energy_ref
             ) / (12.0*SQR(disp_size));
        }
      }
    }

    fprintf(outfile,"\n\t ** Force Constants for irrep %s in symmetry-adapted cartesian coordinates **\n",
      syminfo.clean_irrep_lbls[h]);
    print_mat(force_constants, nsalc[h], nsalc[h], outfile);
    fflush(outfile);

    dim = nsalc[h];

    /** Find eigenvalues of force constant matrix **/
    evals  = init_array(dim);
    evects = block_matrix(dim, dim);
    dgeev_optking(dim, force_constants, evals, evects);
    free_block(force_constants);
    sort(evals, evects, dim);

    fprintf(outfile,"\n\tNormal coordinates for irrep %s\n",syminfo.clean_irrep_lbls[h]);
    normal = block_matrix(3*natom, dim);
    mmult(&(B[start_irr[h]]),1,evects,0,normal,0,3*natom,dim,dim,0);
    print_evects(normal, evals, 3*natom, dim, outfile);
    free_block(normal);
    free_block(evects);

    for (i=0; i<dim; ++i) {
      ++cnt_eval;
      evals_all[cnt_eval] = evals[i];
      evals_all_irrep[cnt_eval] = h;
    } 
    free(evals);
  }

  sort_evals_all(nsalc_all,evals_all, evals_all_irrep);

  /* convert evals from H/(kg bohr^2) to J/(kg m^2) = 1/s^2 */
  /* v = 1/(2 pi c) sqrt( eval ) */
  fprintf(outfile, "\n\t Harmonic Vibrational Frequencies in cm^(-1) \n");
  fprintf(outfile,   "\t---------------------------------------------\n");
  k_convert = _hartree2J/(_bohr2m * _bohr2m * _amu2kg);
  cm_convert = 1.0/(2.0 * _pi * _c * 100.0);
  for(i=nsalc_all-1; i>-1; --i) {
    if(evals_all[i] < 0.0)
      fprintf(outfile, "\t %5s %10.3fi\n", syminfo.irrep_lbls[evals_all_irrep[i]],
          cm_convert * sqrt(-k_convert * evals_all[i]));
    else
      fprintf(outfile, "\t %5s %10.3f\n", syminfo.irrep_lbls[evals_all_irrep[i]],
          cm_convert * sqrt(k_convert * evals_all[i]));
    }
  fprintf(outfile,   "\t---------------------------------------------\n");
  fflush(outfile);
  free(start_irr);
  free(evals_all);
  free(evals_all_irrep);

  optinfo.disp_num = 0;
  psio_write_entry(PSIF_OPTKING, "OPT: Current disp_num",
      (char *) &(optinfo.disp_num),sizeof(int));
  close_PSIF();
  return;
}

/* iE() returns index in array disp_E for the energy of displacements
this lookup must match the ordering of displacements generated in disp_freq_energy_cart()
i and j are coordinates in irrep irr, displaced by steps disp_i and disp_j
disp_i,disp_j are {-1,0,+1} for a three-point formula
disp_j,disp_j are {-2,-1,0,+1,+2} for a five-point formula
for diagonal displacements disp_j=0 and jj is meaningless
i >= j */

int iE(int *ndisp, int *nsalc, int irr, int ii, int jj, int disp_i, int disp_j) {
  int *irr_start, nirreps, ndiag, ij_pair, h;

  nirreps = syminfo.nirreps;
  irr_start = init_int_array(nirreps);
  irr_start[0] = 0;
  for (h=1; h<nirreps; ++h)
    irr_start[h] = irr_start[h-1] + ndisp[h-1];

  if (optinfo.points == 3) {
    if (disp_j==0) {
      if (irr==0) {                  // diagonal, symmetric irrep
        if (disp_i==-1)
          return (2*ii);             // f(-1,0)
        else if (disp_i==1)
          return (2*ii+1);           // f(+1,0)
      }
      else if ((disp_i==1)||(disp_i==-1)) // diagonal, asymmetric irrep
        return (irr_start[irr]+ii);  // f(-1,0) = f(+1,0)
    }
    else {                           // off-diagonal
      if (irr==0)
        ndiag = 2 * nsalc[0];
      else 
        ndiag = nsalc[irr];

      ij_pair = irr_start[irr] + ndiag + 2*((ii*(ii-1))/2 + jj);

      if ((disp_i==+1) && (disp_j==+1)) // f(+1,+1)
        return(ij_pair + 0);
      if ((disp_i==-1) && (disp_j==-1)) // f(-1,-1)
        return(ij_pair + 1);
    }
  }

  else if (optinfo.points == 5) {
    if (disp_j==0) {                
      if (irr==0) {                        // diagonal, symmetric irrep
        if (disp_i==-2)
          return (irr_start[irr]+4*ii);    // f(-2,0)
        else if (disp_i==-1)
          return (irr_start[irr]+4*ii+1);  // f(-1,0)
        else if (disp_i==+1)
          return (irr_start[irr]+4*ii+2);  // f(+1,0)
        else if (disp_i==+2)
          return (irr_start[irr]+4*ii+3);  // f(+2,0)
      }
      else {                               // diagonal, asymmetric irrep
        if      ((disp_i==+2) || (disp_i==-2))
          return (irr_start[irr]+2*ii);    // f(-2,0) = f(+2,0)
        else if ((disp_i==+1) || (disp_i==-1))
          return (irr_start[irr]+2*ii+1);  // f(-1,0) = f(+1,0)
      }
    }
    else {                                 // off-diagonal
      if (irr==0)
        ndiag = 4 * nsalc[0];
      else 
        ndiag = 2 * nsalc[irr];
  
      ij_pair = irr_start[irr] + ndiag + 8*((ii*(ii-1))/2 + jj);
  
      if ((disp_i==-1) && (disp_j==-2)) // f(-1,-2) 
        return(ij_pair + 0);
      if ((disp_i==-2) && (disp_j==-1)) // f(-2,-1)
        return(ij_pair + 1);
      if ((disp_i==-1) && (disp_j==-1)) // f(-1,-1)
        return(ij_pair + 2);
      if ((disp_i==+1) && (disp_j==-1)) // f(+1,-1)
        return(ij_pair + 3);
      if ((disp_i==-1) && (disp_j==+1)) // f(-1,+1)
        return(ij_pair + 4);
      if ((disp_i==+1) && (disp_j==+1)) // f(+1,+1)
        return(ij_pair + 5);
      if ((disp_i==+2) && (disp_j==+1)) // f(+2,+1)
        return(ij_pair + 6);
      if ((disp_i==+1) && (disp_j==+2)) // f(+1,+2)
        return(ij_pair + 7);
    }
  }
  fprintf(outfile, "Problem finding displaced energy\n");
  exit(2);
}

}} /* namespace psi::optking */

