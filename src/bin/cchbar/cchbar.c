/*
**  CCHBAR: Program to calculate the elements of the CCSD HBAR matrix.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libipv1/ip_lib.h>
#include <libpsio/psio.h>
#include <libciomr/libciomr.h>
#include <libdpd/dpd.h>
#include <psifiles.h>
#include "globals.h"

/* Function prototypes */
void init_io(int argc, char *argv[]);
void title(void);
void get_moinfo(void);
void get_params(void);
void exit_io(void);
void F_build(void);
void Wmbej_build(void);
void Wmnie_build(void);
void Wmbij_build(void);
void Wamef_build(void);
void Wabei_build(void);
void purge(void);
void cleanup(void);
int **cacheprep_rhf(int level, int *cachefiles);
int **cacheprep_uhf(int level, int *cachefiles);
void cachedone_uhf(int **cachelist);
void cachedone_rhf(int **cachelist);
void sort_amps(void);
void tau_build(void);
void taut_build(void);
void status(char *, FILE *);
void cc3_HET1(void);

int main(int argc, char *argv[])
{
  int **cachelist, *cachefiles;

  init_io(argc, argv);
  title();
  get_moinfo();
  get_params();

  cachefiles = init_int_array(PSIO_MAXUNIT);

  if(params.ref == 0 || params.ref == 1) { /** RHF or ROHF **/

  cachelist = cacheprep_rhf(params.cachelev, cachefiles);

  dpd_init(0, moinfo.nirreps, params.memory, 0, cachefiles, cachelist, NULL,
           2, moinfo.occpi, moinfo.occ_sym, moinfo.virtpi, moinfo.vir_sym);
  }
  else if(params.ref == 2) { /** UHF **/

    cachelist = cacheprep_uhf(params.cachelev, cachefiles);

    dpd_init(0, moinfo.nirreps, params.memory, 0, cachefiles, 
	     cachelist, NULL, 4, moinfo.aoccpi, moinfo.aocc_sym, moinfo.avirtpi,
	     moinfo.avir_sym, moinfo.boccpi, moinfo.bocc_sym, moinfo.bvirtpi, moinfo.bvir_sym);
  }

  sort_amps();
  tau_build();
  taut_build();

  F_build();
  if(params.print & 2) status("F elements", outfile);

  Wabei_build();
  if(params.print & 2) status("Wabei elements", outfile);
  Wmbej_build();
  if(params.print & 2) status("Wmbej elements", outfile);
  Wmnie_build();
  if(params.print & 2) status("Wmnie elements", outfile);
  Wamef_build();
  if(params.print & 2) status("Wamef elements", outfile);
  Wmbij_build();
  if(params.print & 2) status("Wmbij elements", outfile);

  if( (!strcmp(params.wfn,"CC3")) || (!strcmp(params.wfn,"EOM_CC3")) ) {
    /* switch to ROHF to generate all spin cases of He^T1 elements */
    if(params.dertype == 3 && params.ref == 0) {
      params.ref = 1;
      cc3_HET1(); /* compute remaining Wmbej [H,eT1] */
      norm_HET1();
      params.ref = 0; 
    }
    else {
      cc3_HET1(); /* compute remaining Wmbej [H,eT1] */
      norm_HET1();
    }
  }

  if(params.ref == 1) purge(); /** ROHF only **/
  dpd_close(0);

  if(params.ref == 2) cachedone_uhf(cachelist);
  else cachedone_rhf(cachelist);
  free(cachefiles);

  cleanup(); 
  exit_io();
  exit(PSI_RETURN_SUCCESS);
}

void init_io(int argc, char *argv[])
{
  int i;
  extern char *gprgid();
  char *progid;

  progid = (char *) malloc(strlen(gprgid())+2);
  sprintf(progid, ":%s",gprgid());

  psi_start(argc-1,argv+1,0);
  ip_cwk_add(progid);
  free(progid);
  tstart(outfile);

  psio_init();

  /* Open all dpd data files */
  for(i=CC_MIN; i <= CC_MAX; i++) psio_open(i,1);
}

void title(void)
{
  fprintf(outfile, "\n");
  fprintf(outfile, "\t\t\t**************************\n");
  fprintf(outfile, "\t\t\t*                        *\n");
  fprintf(outfile, "\t\t\t*         CCHBAR         *\n");
  fprintf(outfile, "\t\t\t*                        *\n");
  fprintf(outfile, "\t\t\t**************************\n");
  fprintf(outfile, "\n");
}

void exit_io(void)
{
  int i;
 
  /* Close all dpd data files here */
  for(i=CC_MIN; i < CC_TMP; i++) psio_close(i,1);
  for(i=CC_TMP; i <= CC_TMP11; i++) psio_close(i,0);  /* get rid of TMP files */
  for(i=CC_TMP11+1; i < CC_MAX; i++) psio_close(i,1);

  psio_done();
  tstop(outfile);
  psi_stop();
}

char *gprgid()
{
   char *prgid = "CCHBAR";

   return(prgid);
}
