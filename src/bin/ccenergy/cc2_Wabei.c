#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

/* cc2_Wabei(): Compute the Wabei matrix from CC2 theory, which is
** given in spin orbitals as:
**
** Wabei = <ab||ei> - P(ab) t_m^a <mb||ei> + t_i^f <ab||ef> 
**         - P(ab) t_i^f t_m^b <am||ef> + t_m^a t_n^b <mn||ei>
**         + t_m^a t_i^f t_n^b <mn||ef>
**
** The basic strategy for this code is to generate two intermediate
** quantities, Z1(Ab,EI) and Z2(Ei,Ab), which are summed in the final
** step to give the complete W(Ei,Ab) intermediate.  This is sorted
** to W(iE,bA) storage for use in the triples equations.
**
** TDC, Feb 2004
*/

void purge_cc2_Wabei(void);

void cc2_Wabei_build(void)
{
  int omit = 0;
  dpdfile2 t1, tIA, tia;
  dpdbuf4 Z, Z1, Z2, Z3;
  dpdbuf4 B, C, D, E, F, W;

  timer_on("F->Wabei");
  if(params.ref == 0) { /** RHF **/
    dpd_buf4_init(&F, CC_FINTS, 0, 11, 5, 11, 5, 0, "F <ai|bc>");
    dpd_buf4_copy(&F, CC2_HET1, "CC2 WAbEi (Ei,Ab)");
    dpd_buf4_close(&F);
  }

  else if (params.ref == 1) { /* ROHF */

    dpd_file2_init(&tIA, CC_OEI, 0, 0, 1, "tIA");
    dpd_file2_init(&tia, CC_OEI, 0, 0, 1, "tia");

    /* W(A>B,EI) <--- <AB||EI> */
    /* W(a>b,ei) <--- <ab||ei> */
    dpd_buf4_init(&F, CC_FINTS, 0, 11, 7, 11, 5, 1, "F <ai|bc>");
    dpd_buf4_sort(&F, CC_TMP2, rspq, 7, 11, "CC2 WABEI (A>B,EI)");
    dpd_buf4_sort(&F, CC_TMP2, rspq, 7, 11, "CC2 Wabei (a>b,ei)");
    dpd_buf4_close(&F);

    /* W(Ab,Ei) <--- <Ab|Ei> */
    /* W(aB,eI) <--- <aB|eI> */
    dpd_buf4_init(&F, CC_FINTS, 0, 11, 5, 11, 5, 0, "F <ai|bc>");
    dpd_buf4_sort(&F, CC_TMP2, rspq, 5, 11, "CC2 WAbEi (Ab,Ei)");
    dpd_buf4_sort(&F, CC_TMP2, rspq, 5, 11, "CC2 WaBeI (aB,eI)");
    dpd_buf4_close(&F);
  }
  else if (params.ref == 2) { /* UHF */
    /* W(A>B,EI) <--- <AB||EI> */
    dpd_buf4_init(&F, CC_FINTS, 0, 21, 7, 21, 5, 1, "F <AI|BC>");
    dpd_buf4_sort(&F, CC_TMP0, rspq, 7, 21, "CC2 WABEI (A>B,EI)");
    dpd_buf4_close(&F);

    /* W(a>b,ei) <--- <ab||ei> */
    dpd_buf4_init(&F, CC_FINTS, 0, 31, 17, 31,15, 1, "F <ai|bc>");
    dpd_buf4_sort(&F, CC_TMP0, rspq, 17, 31, "CC2 Wabei (a>b,ei)");
    dpd_buf4_close(&F);

    /* W(Ab,Ei) <--- <Ab|Ei> */
    dpd_buf4_init(&F, CC_FINTS, 0, 28, 26, 28, 26, 0, "F <Ab|Ci>");
    dpd_buf4_copy(&F, CC_TMP0, "CC2 WAbEi (Ab,Ei)");
    dpd_buf4_close(&F);

    /* W(aB,eI) <--- <aB|eI> */
    dpd_buf4_init(&F, CC_FINTS, 0, 25, 29, 25, 29, 0, "F <aI|bC>");
    dpd_buf4_sort(&F, CC_TMP0, psrq, 29, 25, "CC2 WaBeI (aB,eI)");
    dpd_buf4_close(&F);
  }
  timer_off("F->Wabei");

  timer_on("B->Wabei");
  if(params.ref == 0) { /** RHF **/

    dpd_file2_init(&t1, CC_OEI, 0, 0, 1, "tIA");

    /* WEbEi <-- <Ab|Ef> * t(i,f) */
    dpd_buf4_init(&W, CC2_HET1, 0, 11, 5, 11, 5, 0, "CC2 WAbEi (Ei,Ab)");
    dpd_buf4_init(&B, CC_BINTS, 0, 5, 5, 5, 5, 0, "B <ab|cd>");
    dpd_contract424(&B, &t1, &W, 3, 1, 1, 0.5, 1);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    dpd_file2_close(&t1);
  }
  else if (params.ref == 1) { /* ROHF */

    /** W(A>B,EI) <--- <AB||EF> * t1[I][F] **/
    /** W(a>b,ei) <--- <ab||ef> * t1[i][f] **/
    dpd_buf4_init(&W, CC_TMP2, 0, 7, 11, 7, 11, 0, "CC2 WABEI (A>B,EI)");
    dpd_buf4_init(&B, CC_BINTS, 0, 7, 5, 5, 5, 1, "B <ab|cd>");
    dpd_contract424(&B, &tIA, &W, 3, 1, 0, 0.5, 1);
    dpd_buf4_close(&W);

    dpd_buf4_init(&W, CC_TMP2, 0, 7, 11, 7, 11, 0, "CC2 Wabei (a>b,ei)");
    dpd_contract424(&B, &tia, &W, 3, 1, 0, 0.5, 1);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    /** W(Ab,Ei) <--- <Ab|Ef> * t1[i][f] **/
    dpd_buf4_init(&W, CC_TMP2, 0, 5, 11, 5, 11, 0, "CC2 WAbEi (Ab,Ei)");
    dpd_buf4_init(&B, CC_BINTS, 0, 5, 5, 5, 5, 0, "B <ab|cd>");
    dpd_contract424(&B, &tia, &W, 3, 1, 0, 0.5, 1);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    /** W(aB,eI) <--- t1[I][F] * <aB|eF>  **/
    dpd_buf4_init(&W, CC_TMP2, 0, 5, 11, 5, 11, 0, "CC2 WaBeI (aB,eI)");
    dpd_buf4_init(&B, CC_BINTS, 0, 5, 5, 5, 5, 0, "B <ab|cd>");
    dpd_contract424(&B, &tIA, &W, 3, 1, 0, 0.5, 1.0);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    dpd_file2_close(&tIA);
    dpd_file2_close(&tia);
  }
  else if (params.ref == 2) { /* UHF */

    dpd_file2_init(&tIA, CC_OEI, 0, 0, 1, "tIA");
    dpd_file2_init(&tia, CC_OEI, 0, 2, 3, "tia");

    /** W(A>B,EI) <--- <AB||EF> * t1[I][F] **/
    dpd_buf4_init(&W, CC_TMP0, 0, 7, 21, 7, 21, 0, "CC2 WABEI (A>B,EI)");
    dpd_buf4_init(&B, CC_BINTS, 0, 7, 5, 5, 5, 1, "B <AB|CD>");
    dpd_contract424(&B, &tIA, &W, 3, 1, 0, 0.5, 1);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    /** W(a>b,ei) <--- <ab||ef> * t1[i][f] **/
    dpd_buf4_init(&W, CC_TMP0, 0, 17, 31, 17, 31, 0, "CC2 Wabei (a>b,ei)");
    dpd_buf4_init(&B, CC_BINTS, 0, 17, 15, 15, 15, 1, "B <ab|cd>");
    dpd_contract424(&B, &tia, &W, 3, 1, 0, 0.5, 1);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    /** W(Ab,Ei) <--- <Ab|Ef> * t1[i][f] **/
    dpd_buf4_init(&W, CC_TMP0, 0, 28, 26, 28, 26, 0, "CC2 WAbEi (Ab,Ei)");
    dpd_buf4_init(&B, CC_BINTS, 0, 28, 28, 28, 28, 0, "B <Ab|Cd>");
    dpd_contract424(&B, &tia, &W, 3, 1, 0, 0.5, 1);
    dpd_buf4_close(&B);
    dpd_buf4_close(&W);

    /** W(aB,eI) <--- t1[I][F] * <aB|eF>  **/
    dpd_buf4_init(&Z, CC_TMP0, 0, 28, 24, 28, 24, 0, "CC2 ZBaIe (Ba,Ie)");
    dpd_buf4_init(&B, CC_BINTS, 0, 28, 28, 28, 28, 0, "B <Ab|Cd>");
    dpd_contract244(&tIA, &B, &Z, 1, 2, 1, 0.5, 0);
    dpd_buf4_close(&B);
    dpd_buf4_sort_axpy(&Z, CC_TMP0, qpsr, 29, 25, "CC2 WaBeI (aB,eI)", 1);
    dpd_buf4_close(&Z);

    dpd_file2_close(&tIA);
    dpd_file2_close(&tia);
  }
  timer_off("B->Wabei");

  timer_on("Wabei_sort");
  if (params.ref == 1) { /* ROHF */

    /* sort to Wabei (ei,ab) */
    dpd_buf4_init(&W, CC_TMP2, 0, 7, 11, 7, 11, 0, "CC2 WABEI (A>B,EI)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 11, 7, "CC2 WABEI (EI,A>B)");
    dpd_buf4_close(&W);
    dpd_buf4_init(&W, CC_TMP2, 0, 7, 11, 7, 11, 0, "CC2 Wabei (a>b,ei)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 11, 7, "CC2 Wabei (ei,a>b)");
    dpd_buf4_close(&W);
    dpd_buf4_init(&W, CC_TMP2, 0, 5, 11, 5, 11, 0, "CC2 WAbEi (Ab,Ei)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 11, 5, "CC2 WAbEi (Ei,Ab)");
    dpd_buf4_close(&W);
    dpd_buf4_init(&W, CC_TMP2, 0, 5, 11, 5, 11, 0, "CC2 WaBeI (aB,eI)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 11, 5, "CC2 WaBeI (eI,aB)");
    dpd_buf4_close(&W);

    /* purge before final sort */

/*     purge_cc2_Wabei(); */
  }
  else if (params.ref == 2) { /* UHF */

    /* sort to Wabei (ei,ab) */
    dpd_buf4_init(&W, CC_TMP0, 0, 7, 21, 7, 21, 0, "CC2 WABEI (A>B,EI)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 21, 7, "CC2 WABEI (EI,A>B)");
    dpd_buf4_close(&W);
    dpd_buf4_init(&W, CC_TMP0, 0, 17, 31, 17, 31, 0, "CC2 Wabei (a>b,ei)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 31, 17, "CC2 Wabei (ei,a>b)");
    dpd_buf4_close(&W);
    dpd_buf4_init(&W, CC_TMP0, 0, 28, 26, 28, 26, 0, "CC2 WAbEi (Ab,Ei)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 26, 28, "CC2 WAbEi (Ei,Ab)");
    dpd_buf4_close(&W);
    dpd_buf4_init(&W, CC_TMP0, 0, 29, 25, 29, 25, 0, "CC2 WaBeI (aB,eI)");
    dpd_buf4_sort(&W, CC2_HET1, rspq, 25, 29, "CC2 WaBeI (eI,aB)");
    dpd_buf4_close(&W);
  }
  timer_off("Wabei_sort");

}

void purge_cc2_Wabei(void) {
  dpdfile4 W;
  int *occpi, *virtpi;
  int h, a, b, e, f, i, j, m, n;
  int    A, B, E, F, I, J, M, N;
  int mn, ei, ma, ef, me, jb, mb, ij, ab;
  int asym, bsym, esym, fsym, isym, jsym, msym, nsym;
  int *occ_off, *vir_off;
  int *occ_sym, *vir_sym;
  int *openpi, nirreps;

  nirreps = moinfo.nirreps;
  occpi = moinfo.occpi; virtpi = moinfo.virtpi;
  occ_off = moinfo.occ_off; vir_off = moinfo.vir_off;
  occ_sym = moinfo.occ_sym; vir_sym = moinfo.vir_sym;
  openpi = moinfo.openpi;

  /* Purge Wabei matrix elements */
  dpd_file4_init(&W, CC_TMP2, 0, 11, 7,"CC2 WABEI (EI,A>B)");
  for(h=0; h < nirreps; h++) {
    dpd_file4_mat_irrep_init(&W, h);
    dpd_file4_mat_irrep_rd(&W, h);
    for(ei=0; ei<W.params->rowtot[h]; ei++) {
      e = W.params->roworb[h][ei][0];
      esym = W.params->psym[e];
      E = e - vir_off[esym]; 
      for(ab=0; ab<W.params->coltot[h]; ab++) {
        a = W.params->colorb[h][ab][0];
        b = W.params->colorb[h][ab][1];
        asym = W.params->rsym[a];
        bsym = W.params->ssym[b]; 
        A = a - vir_off[asym];
        B = b - vir_off[bsym];
        if ((E >= (virtpi[esym] - openpi[esym])) ||
            (A >= (virtpi[asym] - openpi[asym])) ||
            (B >= (virtpi[bsym] - openpi[bsym])) )
          W.matrix[h][ei][ab] = 0.0;
      }
    }
    dpd_file4_mat_irrep_wrt(&W, h);
    dpd_file4_mat_irrep_close(&W, h);
  }
  dpd_file4_close(&W);

  dpd_file4_init(&W, CC_TMP2, 0, 11, 7,"CC2 Wabei (ei,a>b)");
  for(h=0; h < nirreps; h++) {
    dpd_file4_mat_irrep_init(&W, h);
    dpd_file4_mat_irrep_rd(&W, h);
    for(ei=0; ei<W.params->rowtot[h]; ei++) {
      i = W.params->roworb[h][ei][1];
      isym = W.params->qsym[i]; 
      I = i - occ_off[isym];
      for(ab=0; ab<W.params->coltot[h]; ab++) {
        if (I >= (occpi[isym] - openpi[isym]))
          W.matrix[h][ei][ab] = 0.0;
      }
    }
    dpd_file4_mat_irrep_wrt(&W, h);
    dpd_file4_mat_irrep_close(&W, h);
  }
  dpd_file4_close(&W);

  dpd_file4_init(&W, CC_TMP2, 0, 11, 5,"CC2 WAbEi (Ei,Ab)");
  for(h=0; h < nirreps; h++) {
    dpd_file4_mat_irrep_init(&W, h);
    dpd_file4_mat_irrep_rd(&W, h);
    for(ei=0; ei<W.params->rowtot[h]; ei++) {
      e = W.params->roworb[h][ei][0];
      i = W.params->roworb[h][ei][1];
      esym = W.params->psym[e];
      isym = W.params->qsym[i];
      E = e - vir_off[esym];
      I = i - occ_off[isym];
      for(ab=0; ab<W.params->coltot[h]; ab++) {
        a = W.params->colorb[h][ab][0];
        asym = W.params->rsym[a];
        bsym = W.params->ssym[b];
        A = a - vir_off[asym];
        if ((E >= (virtpi[esym] - openpi[esym])) ||
            (I >= (occpi[isym] - openpi[isym])) ||
            (A >= (virtpi[asym] - openpi[asym])) )
          W.matrix[h][ei][ab] = 0.0;
      }
    }
    dpd_file4_mat_irrep_wrt(&W, h);
    dpd_file4_mat_irrep_close(&W, h);
  }
  dpd_file4_close(&W);

  dpd_file4_init(&W, CC_TMP2, 0, 11, 5,"CC2 WaBeI (eI,aB)");
  for(h=0; h < nirreps; h++) {
    dpd_file4_mat_irrep_init(&W, h);
    dpd_file4_mat_irrep_rd(&W, h);
    for(ei=0; ei<W.params->rowtot[h]; ei++) {
      for(ab=0; ab<W.params->coltot[h]; ab++) {
        b = W.params->colorb[h][ab][1];
        bsym = W.params->ssym[b];
        B = b - vir_off[bsym];
        if (B >= (virtpi[bsym] - openpi[bsym]))
          W.matrix[h][ei][ab] = 0.0;
      }
    }
    dpd_file4_mat_irrep_wrt(&W, h);
    dpd_file4_mat_irrep_close(&W, h);
  }
  dpd_file4_close(&W);

}

