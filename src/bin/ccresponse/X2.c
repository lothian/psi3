#include <stdio.h>
#include <string.h>
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

void denom2(char *cart, int irrep, double omega, dpdbuf4 *X2);

void X2_build(char *cart, int irrep, double omega)
{
  dpdfile2 X1, z, F, t1;
  dpdbuf4 X2, X2new, Z, Z1, Z2, W, T2, I;
  char lbl[32];

  sprintf(lbl, "MuBAR_%1s_IjAb", cart);
  dpd_buf4_init(&X2new, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  sprintf(lbl, "New X_%1s_IjAb (%5.3f)", cart, omega);
  dpd_buf4_copy(&X2new, CC_LR, lbl);
  dpd_buf4_close(&X2new);
  dpd_buf4_init(&X2new, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);

  /*** D-S ***/

  sprintf(lbl, "X_%1s_IA (%5.3f)", cart, omega);
  dpd_file2_init(&X1, CC_OEI, irrep, 0, 1, lbl);

  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&W, CC_HBAR, 0, 10, 0, 10, 0, 0, "WMbIj");
  dpd_contract244(&X1, &W, &Z, 0, 0, 1, 1, 0);
  dpd_buf4_close(&W);
  dpd_buf4_axpy(&Z, &X2new, -1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, -1);
  dpd_buf4_close(&Z);

  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&W, CC_HBAR, 0, 11, 5, 11, 5, 0, "WEiAb");
  dpd_contract244(&X1, &W, &Z, 1, 0, 0, 1, 0);
  dpd_buf4_close(&W);
  dpd_buf4_axpy(&Z, &X2new, 1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, 1);
  dpd_buf4_close(&Z);

  sprintf(lbl, "z(N,I) %1s", cart);
  dpd_file2_init(&z, CC_TMP0, irrep, 0, 0, lbl);
  dpd_buf4_init(&W, CC_HBAR, 0, 0, 11, 0, 11, 0, "2WMnIe - WnMIe");
  dpd_dot23(&X1, &W, &z, 0, 0, 1, 0);
  dpd_buf4_close(&W);
  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&T2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_contract244(&z, &T2, &Z, 0, 0, 0, 1, 0);
  dpd_buf4_close(&T2);
  dpd_file2_close(&z);
  dpd_buf4_axpy(&Z, &X2new, -1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, -1);
  dpd_buf4_close(&Z);

  sprintf(lbl, "z(A,E) %1s", cart);
  dpd_file2_init(&z, CC_TMP0, irrep, 1, 1, lbl);
  dpd_buf4_init(&W, CC_HBAR, 0, 10, 5, 10, 5, 0, "WAmEf 2(mA,Ef) - (mA,fE)");
  dpd_dot14(&X1, &W, &z, 0, 0, 1, 0);
  dpd_buf4_close(&W);
  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&T2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_contract424(&T2, &z, &Z, 3, 1, 0, 1, 0);
  dpd_buf4_close(&T2);
  dpd_file2_close(&z);
  dpd_buf4_axpy(&Z, &X2new, 1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, 1);
  dpd_buf4_close(&Z);

  dpd_file2_close(&X1);

  /*** D-D ***/

  sprintf(lbl, "X_%1s_IjAb (%5.3f)", cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);

  dpd_buf4_axpy(&X2, &X2new, -omega);

  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_file2_init(&F, CC_OEI, 0, 1, 1, "FAE");
  dpd_contract424(&X2, &F, &Z, 3, 1, 0, 1, 0);
  dpd_file2_close(&F);
  dpd_buf4_axpy(&Z, &X2new, 1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, 1);
  dpd_buf4_close(&Z);

  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_file2_init(&F, CC_OEI, 0, 0, 0, "FMI");
  dpd_contract244(&F, &X2, &Z, 0, 0, 0, 1, 0);
  dpd_file2_close(&F);
  dpd_buf4_axpy(&Z, &X2new, -1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, -1);
  dpd_buf4_close(&Z);

  dpd_buf4_init(&W, CC_HBAR, 0, 0, 0, 0, 0, 0, "WMnIj");
  dpd_contract444(&W, &X2, &X2new, 1, 1, 1, 1);
  dpd_buf4_close(&W);

  sprintf(lbl, "Z(Ab,Ij) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 5, 0, 5, 0, 0, lbl);
  dpd_buf4_init(&I, CC_BINTS, 0, 5, 5, 5, 5, 0, "B <ab|cd>");
  dpd_contract444(&I, &X2, &Z, 0, 0, 1, 0);
  dpd_buf4_close(&I);
  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, rspq, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, 1);
  dpd_buf4_close(&Z);

  sprintf(lbl, "Z(Ij,Mb) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 10, 0, 10, 0, lbl);
  dpd_buf4_init(&I, CC_FINTS, 0, 10, 5, 10, 5, 0, "F <ia|bc>");
  dpd_contract444(&X2, &I, &Z, 0, 0, 1, 0);
  dpd_buf4_close(&I);
  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z1, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_file2_init(&t1, CC_OEI, 0, 0, 1, "tIA");
  dpd_contract244(&t1, &Z, &Z1, 0, 2, 1, 1, 0);
  dpd_file2_close(&t1);
  dpd_buf4_close(&Z);
  dpd_buf4_axpy(&Z1, &X2new, -1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z1, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z1);
  dpd_buf4_init(&Z1, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z1, &X2new, -1);
  dpd_buf4_close(&Z1);

  sprintf(lbl, "Z(Ij,Mn) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 0, 0, 0, 0, lbl);
  dpd_buf4_init(&I, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  dpd_contract444(&X2, &I, &Z, 0, 0, 1, 0);
  dpd_buf4_close(&I);
  dpd_buf4_init(&T2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tauIjAb");
  dpd_contract444(&Z, &T2, &X2new, 0, 1, 1, 1);
  dpd_buf4_close(&T2);
  dpd_buf4_close(&Z);

  dpd_buf4_close(&X2);

  sprintf(lbl, "Z(Ib,jA) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 10, 10, 10, 10, 0, lbl);
  dpd_buf4_init(&W, CC_HBAR, 0, 10, 10, 10, 10, 0, "WMbeJ");
  sprintf(lbl, "X_%1s_IbjA (%5.3f)", cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 10, 10, 10, 10, 0, lbl);
  dpd_contract444(&X2, &W, &Z, 0, 1, 1, 0);
  dpd_buf4_close(&X2);
  dpd_buf4_close(&W);
  sprintf(lbl, "X(IA,jb) III %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, psrq, 10, 10, lbl);
  dpd_buf4_close(&Z);
  sprintf(lbl, "X(IA,jb) I %1s", cart);
  dpd_buf4_init(&Z1, CC_TMP0, irrep, 10, 10, 10, 10, 0, lbl);
  sprintf(lbl, "X_%1s_(2IAjb-IbjA) (%5.3f)", cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 10, 10, 10, 10, 0, lbl);
  dpd_buf4_init(&W, CC_HBAR, 0, 10, 10, 10, 10, 0, "2 W(ME,jb) + W(Me,Jb)");
  dpd_contract444(&X2, &W, &Z1, 0, 1, 0.5, 0);
  dpd_buf4_close(&W);
  dpd_buf4_close(&X2);
  sprintf(lbl, "Z(Ib,jA) %1s", cart);
  dpd_buf4_init(&Z2, CC_TMP0, irrep, 10, 10, 10, 10, 0, lbl);
  dpd_buf4_axpy(&Z2, &Z1, 0.5);
  dpd_buf4_close(&Z2);
  sprintf(lbl, "X(IA,jb) III %1s", cart);
  dpd_buf4_init(&Z2, CC_TMP0, irrep, 10, 10, 10, 10, 0, lbl);
  dpd_buf4_axpy(&Z2, &Z1, 1);
  dpd_buf4_close(&Z2);
  sprintf(lbl, "X(Ij,Ab) I+III %1s", cart);
  dpd_buf4_sort(&Z1, CC_TMP0, prqs, 0, 5, lbl);
  dpd_buf4_close(&Z1);
  dpd_buf4_init(&Z1, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z1, &X2new, 1);
  sprintf(lbl, "X(Ij,Ab) II+IV %1s", cart);
  dpd_buf4_sort(&Z1, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z1);
  dpd_buf4_init(&Z1, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z1, &X2new, 1);
  dpd_buf4_close(&Z1);

  sprintf(lbl, "z(F,A) %1s", cart);
  dpd_file2_init(&z, CC_TMP0, irrep, 1, 1, lbl);
  sprintf(lbl, "X_%1s_(2IjAb-IjbA) (%5.3f)", cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&I, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  dpd_contract442(&I, &X2, &z, 2, 2, 1, 0);
  dpd_buf4_close(&I);
  dpd_buf4_close(&X2);
  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&T2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_contract424(&T2, &z, &Z, 3, 0, 0, 1, 0);
  dpd_buf4_close(&T2);
  dpd_file2_close(&z);
  dpd_buf4_axpy(&Z, &X2new, -1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, -1);
  dpd_buf4_close(&Z);

  sprintf(lbl, "z(N,I) %1s", cart);
  dpd_file2_init(&z, CC_TMP0, irrep, 0, 0, lbl);
  sprintf(lbl, "X_%1s_(2IjAb-IjbA) (%5.3f)", cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&I, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  dpd_contract442(&I, &X2, &z, 0, 0, 1, 0);
  dpd_buf4_close(&I);
  dpd_buf4_close(&X2);
  sprintf(lbl, "Z(Ij,Ab) %1s", cart);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_init(&T2, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_contract244(&z, &T2, &Z, 0, 0, 0, 1, 0);
  dpd_buf4_close(&T2);
  dpd_file2_close(&z);
  dpd_buf4_axpy(&Z, &X2new, -1);
  sprintf(lbl, "Z(jI,bA) %1s", cart);
  dpd_buf4_sort(&Z, CC_TMP0, qpsr, 0, 5, lbl);
  dpd_buf4_close(&Z);
  dpd_buf4_init(&Z, CC_TMP0, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&Z, &X2new, -1);
  dpd_buf4_close(&Z);

  denom2(cart, irrep, omega, &X2new);
  dpd_buf4_close(&X2new);
}
