/*! \file 
    \ingroup (DETCASMAN)
    \brief Enter brief description of file here 
*/

/*! \defgroup DETCASMAN Add a description of the group DETCASMAN */

/*
**
** DETCASMAN
**
** Program to manage the iteration of (transqt, detci, clag, detcas)
** required for orbital optimization using the DETCAS program.
**
** This program does not really do much...it simply iterates until
** convergence or until iterations are exhausted.  It would not be
** necessary if it were possible to rewrite the PSI driver to be more
** general and allow non-crashing exits out of loops; somebody who
** knows how to do this should do it.
**
** C. David Sherrill
** University of California, Berkeley
** April 1998
**
**
** Modification History:
**
** - Modified 10 February 1999 by C. David Sherrill -
** Added the ability to parse the orbital optimization log file (file14)
** so this information can be used to allow looser convergence on the 
** CI during early iterations.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libipv1/ip_lib.h>
#include <libciomr/libciomr.h>
#include "setup_io.h"

extern "C" {
  FILE *infile, *outfile;
  char *psi_file_prefix;
}

namespace psi { namespace detcasman {

#define MAX_COMMENT 10

void title(void);
void quote(void);
double calc_ci_conv(double scale, double *energy);

}} // end namespace psi::detcasman


using namespace psi::detcasman;

int main(int argc, char *argv[])
{
  int converged = 0;
  int i, errcod = 0;
  char *wfn;                   /* wavefunction type                        */
  int ncasiter = 0;            /* max cas iterations */
  char detci_string[80];       /* string containing system call for DETCI  */
  char rmstring[100];          /* remove command for diis.dat, etc         */
  double ci_conv;              /* desired CI convergence 
                                  (changes dynamically during CAS opt)     */
  double scale_conv;           /* CI convergence threshold = 
                                  orbital gradient * scale_conv            */

  double energy_last;          /* last CI energy                           */
  init_io(argc,argv);          /* open input and output files              */
  title();                     /* print program identification             */

  ncasiter = 30;
  errcod = ip_data("NCASITER","%d",&ncasiter,0);
  scale_conv = 0.01;
  errcod = ip_data("SCALE_CONV","%lf",&scale_conv,0);
  errcod = ip_string("WFN", &wfn,0);
  if (errcod == IPE_KEY_NOT_FOUND) {
    wfn = (char *) malloc(sizeof(char)*7);
    strcpy(wfn, "DETCAS");
  }

  /* First iteration prints DETCI information */
  ci_conv = calc_ci_conv(scale_conv, &energy_last);

  if (ci_conv > 1.0E-7) {
    sprintf(detci_string, "detci -c %12.9lf\n", ci_conv);
  }
  else 
    sprintf(detci_string, "detci \n");

  check(!system("transqt2 --quiet"), "TRANSQT2 failed");
  check(!system(detci_string), "DETCI failed");
  check(!system("clag --quiet"), "CLAG failed");
  converged = system("detcas --quiet");

  for (i=1; i<ncasiter && !converged; i++) {
    ci_conv = calc_ci_conv(scale_conv, &energy_last);

    if (ci_conv > 1.0E-7) {
      sprintf(detci_string, "detci --quiet -c %12.9lf\n", ci_conv);
    }
    else 
      sprintf(detci_string, "detci --quiet\n");

    check(!system("transqt2 --quiet"), "TRANSQT2 failed");
    check(!system(detci_string), "DETCI failed");
    check(!system("clag --quiet"), "CLAG failed");
    converged = system("detcas --quiet");
  }

  fprintf(outfile,"\n");
  fprintf(outfile,"*******************************************************\n");

  if (converged) {
    fprintf(outfile,"                  ORBITALS CONVERGED\n");
    fprintf(outfile,"\n  Final %s Energy = %17.12lf\n", wfn, energy_last);
  }
  else
    fprintf(outfile,"               ORBITALS DID NOT CONVERGE\n");

  if (converged) {
    sprintf(rmstring, "rm -f %s.%s %s.%s %s.%s", psi_file_prefix, "diis.dat",
      psi_file_prefix, "orbs.dat", psi_file_prefix, "thetas.dat");
    system(rmstring);
  }

  quote();
  close_io();
  return(!converged);
}

namespace psi { namespace detcasman {

/*
** title(): Function prints a program identification
*/
void title(void)
{
  fprintf(outfile,"\n");
  fprintf(outfile,"*******************************************************\n");
  fprintf(outfile,"                   D E T C A S M A N\n");
  fprintf(outfile,"\n");
  fprintf(outfile,"                   C. David Sherrill\n") ;
  fprintf(outfile,"                    October 7 1998\n") ;
  fprintf(outfile,"*******************************************************\n");
  fprintf(outfile,"\n\n\n");
  fflush(outfile);
}


void quote(void)
{
  fprintf(outfile,"\n");
  fprintf(outfile,"                DETCAS MANAGER EXITING\n");
  fprintf(outfile,"*******************************************************\n");
  fprintf(outfile,"\n\n\n");
  fflush(outfile);
}


/*
** Read the current orbital convergence from file14
*/
double calc_ci_conv(double scale_conv, double *energy_last)
{
  FILE *sumfile;
  char sumfile_name[] = "file14.dat";
  char comment[MAX_COMMENT];
  int i, entries, iter, nind;
  double scaled_rmsgrad, rmsgrad;
  double tval;

  ffile_noexit(&sumfile,sumfile_name,2);

  if (sumfile == NULL) {
    return(scale_conv * 0.1);
  }

  if (fscanf(sumfile, "%d", &entries) != 1) {
    fprintf(outfile,"detcasman: Trouble reading num entries in file %s\n",
            sumfile_name);
    fclose(sumfile);
    return(scale_conv * 0.1);
  }

  for (i=0; i<entries; i++) {
    fscanf(sumfile, "%d %d %lf %lf %lf %s", &iter, &nind, &scaled_rmsgrad,
           &rmsgrad, energy_last, comment);
  }
  fclose(sumfile);

  tval = (scaled_rmsgrad < rmsgrad) ? scaled_rmsgrad : rmsgrad;
  tval *= scale_conv;

  return(tval);
}

}} // end namespace psi::detcasman 

