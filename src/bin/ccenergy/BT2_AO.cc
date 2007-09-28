/*! \file 
    \ingroup (CCENERGY)
    \brief Enter brief description of file here 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libiwl/iwl.h>
#include <libdpd/dpd.h>
#include <libqt/qt.h>
#include <psifiles.h>
#include "Params.h"
#include "MOInfo.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccenergy {

void halftrans(dpdbuf4 *Buf1, int dpdnum1, dpdbuf4 *Buf2, int dpdnum2, double ***C1, double ***C2,
               int nirreps, int **mo_row, int **so_row, int *mospi_left, int *mospi_right,
               int *sospi, int type, double alpha, double beta);

int AO_contribute(struct iwlbuf *InBuf, dpdbuf4 *tau1_AO, dpdbuf4 *tau2_AO);

void BT2_AO(void)
{
  int h, nirreps, i, Gc, Gd, Ga, Gb, ij;
  double ***C, **X;
  double ***Ca, ***Cb;
  int *orbspi, *virtpi;
  int *avirtpi, *bvirtpi;
  int **T2_cd_row_start, **T2_pq_row_start, offset, cd, pq;
  int **T2_CD_row_start, **T2_Cd_row_start;
  dpdbuf4 tau, t2, tau1_AO, tau2_AO;
  dpdfile4 T;
  psio_address next;
  struct iwlbuf InBuf;
  int lastbuf;
  double tolerance=1e-14;
  double **integrals;
  int **tau1_cols, **tau2_cols, *num_ints;
  int counter=0, counterAA=0, counterBB=0, counterAB=0;

  nirreps = moinfo.nirreps;
  orbspi = moinfo.orbspi;

  T2_pq_row_start = init_int_matrix(nirreps,nirreps);
  for(h=0; h < nirreps; h++) {
    for(Gc=0,offset=0; Gc < nirreps; Gc++) {
      Gd = Gc ^ h;
      T2_pq_row_start[h][Gc] = offset;
      offset += orbspi[Gc] * orbspi[Gd];
    }
  }

  if(params.ref == 0 || params.ref == 1) { /** RHF or ROHF **/
    virtpi = moinfo.virtpi;
    C = moinfo.C;

    T2_cd_row_start = init_int_matrix(nirreps,nirreps);
    for(h=0; h < nirreps; h++) {
      for(Gc=0,offset=0; Gc < nirreps; Gc++) {
        Gd = Gc ^ h;
        T2_cd_row_start[h][Gc] = offset;
        offset += virtpi[Gc] * virtpi[Gd];
      }
    }
  }
  else if(params.ref == 2) {  /** UHF **/
    avirtpi = moinfo.avirtpi;
    bvirtpi = moinfo.bvirtpi;
    Ca = moinfo.Ca;
    Cb = moinfo.Cb;

    T2_CD_row_start = init_int_matrix(nirreps,nirreps);
    for(h=0; h < nirreps; h++) {
      for(Gc=0,offset=0; Gc < nirreps; Gc++) {
        Gd = Gc ^ h;
        T2_CD_row_start[h][Gc] = offset;
        offset += avirtpi[Gc] * avirtpi[Gd];
      }
    }
    T2_cd_row_start = init_int_matrix(nirreps,nirreps);
    for(h=0; h < nirreps; h++) {
      for(Gc=0,offset=0; Gc < nirreps; Gc++) {
        Gd = Gc ^ h;
        T2_cd_row_start[h][Gc] = offset;
        offset += bvirtpi[Gc] * bvirtpi[Gd];
      }
    }
    T2_Cd_row_start = init_int_matrix(nirreps,nirreps);
    for(h=0; h < nirreps; h++) {
      for(Gc=0,offset=0; Gc < nirreps; Gc++) {
        Gd = Gc ^ h;
        T2_Cd_row_start[h][Gc] = offset;
        offset += avirtpi[Gc] * bvirtpi[Gd];
      }
    }

  }

  if(params.ref == 0) { /** RHF **/

    if(!strcmp(params.aobasis,"DISK")) {

      dpd_set_default(1);
      dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjPq (1)");
      dpd_buf4_scm(&tau1_AO, 0.0);

      dpd_set_default(0);
      dpd_buf4_init(&tau, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");

      halftrans(&tau, 0, &tau1_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
		virtpi, virtpi, orbspi, 0, 1.0, 0.0);

      dpd_buf4_close(&tau);
      dpd_buf4_close(&tau1_AO);

      /* Transpose tau1_AO for better memory access patterns */
      dpd_set_default(1);
      dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjPq (1)");
      dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 5, 0, "tauPqIj (1)");
      dpd_buf4_close(&tau1_AO);

    
      dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 5, 0, 5, 0, 0, "tauPqIj (1)");
      dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 0, 5, 0, 0, "tauPqIj (2)");
      dpd_buf4_scm(&tau2_AO, 0.0);
        
      for(h=0; h < nirreps; h++) {
	dpd_buf4_mat_irrep_init(&tau1_AO, h);
	dpd_buf4_mat_irrep_rd(&tau1_AO, h);
	dpd_buf4_mat_irrep_init(&tau2_AO, h);
      }
        
      iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);
        
      lastbuf = InBuf.lastbuf;
        
      counter += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
        
      while(!lastbuf) {
	iwl_buf_fetch(&InBuf);
	lastbuf = InBuf.lastbuf;
        
	counter += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
      }
        
      iwl_buf_close(&InBuf, 1);
        
      if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <ab||cd> --> T2\n", counter);
        
      for(h=0; h < nirreps; h++) {
	dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
	dpd_buf4_mat_irrep_close(&tau2_AO, h);
	dpd_buf4_mat_irrep_close(&tau1_AO, h);
      }
      dpd_buf4_close(&tau1_AO);
      dpd_buf4_close(&tau2_AO);

      /* Transpose tau2_AO for the half-backtransformation */
      dpd_set_default(1);
      dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 0, 5, 0, 0, "tauPqIj (2)");
      dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 0, 5, "tauIjPq (2)");
      dpd_buf4_close(&tau2_AO);

      dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjPq (2)");
    
      dpd_set_default(0);
      dpd_buf4_init(&t2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "New tIjAb");

      halftrans(&t2, 0, &tau2_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
		virtpi, virtpi, orbspi, 1, 1.0, 1.0);

      dpd_buf4_close(&t2);
      dpd_buf4_close(&tau2_AO);

    }
    else if(!strcmp(params.aobasis,"DIRECT")) {
      dpd_file4_init(&T, CC_TAMPS, 0, 0, 5, "tauIjAb");
      dpd_file4_cache_del(&T);
      dpd_file4_close(&T);

      dpd_file4_init(&T, CC_TAMPS, 0, 0, 5, "New tIjAb");
      dpd_file4_cache_del(&T);
      dpd_file4_close(&T);

      /* close the CC_TAMPS file for cints to use it */
      psio_close(CC_TAMPS, 1);
        
      system("cints --cc_bt2");

      /* re-open CCC_TAMPS for remaining terms */
      psio_open(CC_TAMPS, PSIO_OPEN_OLD);
    }

  }
  else if(params.ref == 1) { /** ROHF **/

    /************************************* AA *****************************************/

    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 2, 5, 2, 5, 0, "tauIJPQ (1)");
    dpd_buf4_scm(&tau1_AO, 0.0);

    dpd_set_default(0);
    dpd_buf4_init(&tau, CC_TAMPS, 0, 2, 5, 2, 7, 0, "tauIJAB");

    halftrans(&tau, 0, &tau1_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
	      virtpi, virtpi, orbspi, 0, 1.0, 0.0);

    dpd_buf4_close(&tau);
    dpd_buf4_close(&tau1_AO);

    /* Transpose tau1_AO for better memory access patterns */
    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 2, 5, 2, 5, 1, "tauIJPQ (1)");
    dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 5, 2, "tauPQIJ (1)");
    dpd_buf4_close(&tau1_AO);

    dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "tauPQIJ (1)");
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "tauPQIJ (2)");
    dpd_buf4_scm(&tau2_AO, 0.0);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_init(&tau1_AO, h);
      dpd_buf4_mat_irrep_rd(&tau1_AO, h);
      dpd_buf4_mat_irrep_init(&tau2_AO, h);
    }

    iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);

    lastbuf = InBuf.lastbuf;

    counterAA += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);

    while(!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      counterAA += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
    }

    iwl_buf_close(&InBuf, 1);

    if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <AB||CD> --> T2\n", counterAA);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau1_AO, h);
    }
    dpd_buf4_close(&tau1_AO);
    dpd_buf4_close(&tau2_AO);


    /* Transpose tau2_AO for the half-backtransformation */
    dpd_set_default(1);
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "tauPQIJ (2)");
    dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 2, 5, "tauIJPQ (2)");
    dpd_buf4_close(&tau2_AO);

    dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 2, 5, 2, 5, 0, "tauIJPQ (2)");

    dpd_set_default(0);
    dpd_buf4_init(&t2, CC_TAMPS, 0, 2, 5, 2, 7, 0, "New tIJAB");

    halftrans(&t2, 0, &tau2_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
	      virtpi, virtpi, orbspi, 1, 0.5, 1.0);

    dpd_buf4_close(&t2);
    dpd_buf4_close(&tau2_AO);

    /************************************* BB *****************************************/

    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 2, 5, 2, 5, 0, "tauijpq (1)");
    dpd_buf4_scm(&tau1_AO, 0.0);

    dpd_set_default(0);
    dpd_buf4_init(&tau, CC_TAMPS, 0, 2, 5, 2, 7, 0, "tauijab");

    halftrans(&tau, 0, &tau1_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
	      virtpi, virtpi, orbspi, 0, 1.0, 0.0);

    dpd_buf4_close(&tau);
    dpd_buf4_close(&tau1_AO);

    /* Transpose tau1_AO for better memory access patterns */
    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 2, 5, 2, 5, 1, "tauijpq (1)");
    dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 5, 2, "taupqij (1)");
    dpd_buf4_close(&tau1_AO);

    dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "taupqij (1)");
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "taupqij (2)");
    dpd_buf4_scm(&tau2_AO, 0.0);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_init(&tau1_AO, h);
      dpd_buf4_mat_irrep_rd(&tau1_AO, h);
      dpd_buf4_mat_irrep_init(&tau2_AO, h);
    }

    iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);

    lastbuf = InBuf.lastbuf;

    counterBB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);

    while(!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      counterBB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
    }

    iwl_buf_close(&InBuf, 1);

    if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <ab||cd> --> T2\n", counterBB);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau1_AO, h);
    }
    dpd_buf4_close(&tau1_AO);
    dpd_buf4_close(&tau2_AO);


    /* Transpose tau2_AO for the half-backtransformation */
    dpd_set_default(1);
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "taupqij (2)");
    dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 2, 5, "tauijpq (2)");
    dpd_buf4_close(&tau2_AO);

    dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 2, 5, 2, 5, 0, "tauijpq (2)");

    dpd_set_default(0);
    dpd_buf4_init(&t2, CC_TAMPS, 0, 2, 5, 2, 7, 0, "New tijab");

    halftrans(&t2, 0, &tau2_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
	      virtpi, virtpi, orbspi, 1, 0.5, 1.0);

    dpd_buf4_close(&t2);
    dpd_buf4_close(&tau2_AO);

    /************************************* AB *****************************************/

    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjPq (1)");
    dpd_buf4_scm(&tau1_AO, 0.0);

    dpd_set_default(0);
    dpd_buf4_init(&tau, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");

    halftrans(&tau, 0, &tau1_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
	      virtpi, virtpi, orbspi, 0, 1.0, 0.0);

    dpd_buf4_close(&tau);
    dpd_buf4_close(&tau1_AO);


    /* Transpose tau1_AO for better memory access patterns */
    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjPq (1)");
    dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 5, 0, "tauPqIj (1)");
    dpd_buf4_close(&tau1_AO);

    dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 5, 0, 5, 0, 0, "tauPqIj (1)");
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 0, 5, 0, 0, "tauPqIj (2)");
    dpd_buf4_scm(&tau2_AO, 0.0);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_init(&tau1_AO, h);
      dpd_buf4_mat_irrep_rd(&tau1_AO, h);
      dpd_buf4_mat_irrep_init(&tau2_AO, h);
    }

    iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);

    lastbuf = InBuf.lastbuf;

    counterAB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);

    while(!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      counterAB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
    }

    iwl_buf_close(&InBuf, 1);

    if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <Ab|Cd> --> T2\n", counterAB);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau1_AO, h);
    }
    dpd_buf4_close(&tau1_AO);
    dpd_buf4_close(&tau2_AO);

    /* Transpose tau2_AO for the half-backtransformation */
    dpd_set_default(1);
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 0, 5, 0, 0, "tauPqIj (2)");
    dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 0, 5, "tauIjPq (2)");
    dpd_buf4_close(&tau2_AO);

    dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjPq (2)");

    dpd_set_default(0);
    dpd_buf4_init(&t2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "New tIjAb");

    halftrans(&t2, 0, &tau2_AO, 1, C, C, nirreps, T2_cd_row_start, T2_pq_row_start, 
	      virtpi, virtpi, orbspi, 1, 1.0, 1.0);

    dpd_buf4_close(&t2);
    dpd_buf4_close(&tau2_AO);

  }  /** ROHF **/
  else if(params.ref == 2) { /** UHF **/

    /************************************* AA *****************************************/

    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 2, 5, 2, 5, 0, "tauIJPQ (1)");
    dpd_buf4_scm(&tau1_AO, 0.0);

    dpd_set_default(0);
    dpd_buf4_init(&tau, CC_TAMPS, 0, 2, 5, 2, 7, 0, "tauIJAB");

    halftrans(&tau, 0, &tau1_AO, 1, Ca, Ca, nirreps, T2_CD_row_start, T2_pq_row_start,
              avirtpi, avirtpi, orbspi, 0, 1.0, 0.0);

    dpd_buf4_close(&tau);
    dpd_buf4_close(&tau1_AO);

    /* Transpose tau1_AO for better memory access patterns */
    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 2, 5, 2, 5, 1, "tauIJPQ (1)");
    dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 5, 2, "tauPQIJ (1)");
    dpd_buf4_close(&tau1_AO);

    dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "tauPQIJ (1)");
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "tauPQIJ (2)");
    dpd_buf4_scm(&tau2_AO, 0.0);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_init(&tau1_AO, h);
      dpd_buf4_mat_irrep_rd(&tau1_AO, h);
      dpd_buf4_mat_irrep_init(&tau2_AO, h);
    }

    iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);

    lastbuf = InBuf.lastbuf;

    counterAA += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);

    while(!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      counterAA += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
    }

    iwl_buf_close(&InBuf, 1);

    if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <AB||CD> --> T2\n", counterAA);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau1_AO, h);
    }
    dpd_buf4_close(&tau1_AO);
    dpd_buf4_close(&tau2_AO);


    /* Transpose tau2_AO for the half-backtransformation */
    dpd_set_default(1);
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 5, 2, 5, 2, 0, "tauPQIJ (2)");
    dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 2, 5, "tauIJPQ (2)");
    dpd_buf4_close(&tau2_AO);

    dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 2, 5, 2, 5, 0, "tauIJPQ (2)");

    dpd_set_default(0);
    dpd_buf4_init(&t2, CC_TAMPS, 0, 2, 5, 2, 7, 0, "New tIJAB");

    halftrans(&t2, 0, &tau2_AO, 1, Ca, Ca, nirreps, T2_CD_row_start, T2_pq_row_start,
              avirtpi, avirtpi, orbspi, 1, 0.5, 1.0);

    dpd_buf4_close(&t2);
    dpd_buf4_close(&tau2_AO);

    /************************************* BB *****************************************/

    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 12, 15, 12, 15, 0, "tauijpq (1)");
    dpd_buf4_scm(&tau1_AO, 0.0);

    dpd_set_default(0);
    dpd_buf4_init(&tau, CC_TAMPS, 0, 12, 15, 12, 17, 0, "tauijab");

    halftrans(&tau, 0, &tau1_AO, 1, Cb, Cb, nirreps, T2_cd_row_start, T2_pq_row_start,
              bvirtpi, bvirtpi, orbspi, 0, 1.0, 0.0);

    dpd_buf4_close(&tau);
    dpd_buf4_close(&tau1_AO);

    /* Transpose tau1_AO for better memory access patterns */
    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 12, 15, 12, 15, 1, "tauijpq (1)");
    dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 15, 12, "taupqij (1)");
    dpd_buf4_close(&tau1_AO);

    dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 15, 12, 15, 12, 0, "taupqij (1)");
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 15, 12, 15, 12, 0, "taupqij (2)");
    dpd_buf4_scm(&tau2_AO, 0.0);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_init(&tau1_AO, h);
      dpd_buf4_mat_irrep_rd(&tau1_AO, h);
      dpd_buf4_mat_irrep_init(&tau2_AO, h);
    }

    iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);

    lastbuf = InBuf.lastbuf;

    counterBB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);

    while(!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      counterBB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
    }

    iwl_buf_close(&InBuf, 1);

    if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <ab||cd> --> T2\n", counterBB);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau1_AO, h);
    }
    dpd_buf4_close(&tau1_AO);
    dpd_buf4_close(&tau2_AO);


    /* Transpose tau2_AO for the half-backtransformation */
    dpd_set_default(1);
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 15, 12, 15, 12, 0, "taupqij (2)");
    dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 12, 15, "tauijpq (2)");
    dpd_buf4_close(&tau2_AO);

    dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 12, 15, 12, 15, 0, "tauijpq (2)");

    dpd_set_default(0);
    dpd_buf4_init(&t2, CC_TAMPS, 0, 12, 15, 12, 17, 0, "New tijab");

    halftrans(&t2, 0, &tau2_AO, 1, Cb, Cb, nirreps, T2_cd_row_start, T2_pq_row_start,
              bvirtpi, bvirtpi, orbspi, 1, 0.5, 1.0);

    dpd_buf4_close(&t2);
    dpd_buf4_close(&tau2_AO);

    /************************************* AB *****************************************/

    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 22, 28, 22, 28, 0, "tauIjPq (1)");
    dpd_buf4_scm(&tau1_AO, 0.0);

    dpd_set_default(0);
    dpd_buf4_init(&tau, CC_TAMPS, 0, 22, 28, 22, 28, 0, "tauIjAb");

    halftrans(&tau, 0, &tau1_AO, 1, Ca, Cb, nirreps, T2_Cd_row_start, T2_pq_row_start,
              avirtpi, bvirtpi, orbspi, 0, 1.0, 0.0);

    dpd_buf4_close(&tau);
    dpd_buf4_close(&tau1_AO);


    /* Transpose tau1_AO for better memory access patterns */
    dpd_set_default(1);
    dpd_buf4_init(&tau1_AO, CC_TAMPS, 0, 22, 28, 22, 28, 0, "tauIjPq (1)");
    dpd_buf4_sort(&tau1_AO, CC_TMP0, rspq, 28, 22, "tauPqIj (1)");
    dpd_buf4_close(&tau1_AO);

    dpd_buf4_init(&tau1_AO, CC_TMP0, 0, 28, 22, 28, 22, 0, "tauPqIj (1)");
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 28, 22, 28, 22, 0, "tauPqIj (2)");
    dpd_buf4_scm(&tau2_AO, 0.0);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_init(&tau1_AO, h);
      dpd_buf4_mat_irrep_rd(&tau1_AO, h);
      dpd_buf4_mat_irrep_init(&tau2_AO, h);
    }

    iwl_buf_init(&InBuf, PSIF_SO_TEI, tolerance, 1, 1);

    lastbuf = InBuf.lastbuf;

    counterAB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);

    while(!lastbuf) {
      iwl_buf_fetch(&InBuf);
      lastbuf = InBuf.lastbuf;

      counterAB += AO_contribute(&InBuf, &tau1_AO, &tau2_AO);
    }

    iwl_buf_close(&InBuf, 1);

    if(params.print & 2) fprintf(outfile, "     *** Processed %d SO integrals for <Ab|Cd> --> T2\n", counterAB);

    for(h=0; h < nirreps; h++) {
      dpd_buf4_mat_irrep_wrt(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau2_AO, h);
      dpd_buf4_mat_irrep_close(&tau1_AO, h);
    }
    dpd_buf4_close(&tau1_AO);
    dpd_buf4_close(&tau2_AO);

    /* Transpose tau2_AO for the half-backtransformation */
    dpd_set_default(1);
    dpd_buf4_init(&tau2_AO, CC_TMP0, 0, 28, 22, 28, 22, 0, "tauPqIj (2)");
    dpd_buf4_sort(&tau2_AO, CC_TAMPS, rspq, 22, 28, "tauIjPq (2)");
    dpd_buf4_close(&tau2_AO);

    dpd_buf4_init(&tau2_AO, CC_TAMPS, 0, 22, 28, 22, 28, 0, "tauIjPq (2)");

    dpd_set_default(0);
    dpd_buf4_init(&t2, CC_TAMPS, 0, 22, 28, 22, 28, 0, "New tIjAb");

    halftrans(&t2, 0, &tau2_AO, 1, Ca, Cb, nirreps, T2_Cd_row_start, T2_pq_row_start,
              avirtpi, bvirtpi, orbspi, 1, 1.0, 1.0);

    dpd_buf4_close(&t2);
    dpd_buf4_close(&tau2_AO);

  }  /** UHF **/

  if(params.ref == 0 || params.ref == 1)
    free_int_matrix(T2_cd_row_start);
  else if(params.ref ==2) {
    free_int_matrix(T2_CD_row_start);
    free_int_matrix(T2_cd_row_start);
    free_int_matrix(T2_Cd_row_start);
  }

  free_int_matrix(T2_pq_row_start);

  /* Reset the default dpd back to 0 --- this stuff gets really ugly */
  dpd_set_default(0);

}

}} // namespace psi::ccenergy