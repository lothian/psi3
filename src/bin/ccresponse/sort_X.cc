/*! \file 
    \ingroup (CCRESPONSE)
    \brief Enter brief description of file here 
*/
#include <stdio.h>
#include <string.h>
#include <libdpd/dpd.h>
#include "MOInfo.h"
#include "Params.h"
#include "Local.h"
#define EXTERN
#include "globals.h"

namespace psi { namespace ccresponse {

void sort_X(char *pert, char *cart, int irrep, double omega)
{
  dpdbuf4 X;
  char lbl[32];

  sprintf(lbl, "X_%s_%1s_IjAb (%5.3f)", pert, cart, omega);
  dpd_buf4_init(&X, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  sprintf(lbl, "X_%s_%1s_IAjb (%5.3f)", pert, cart, omega);
  dpd_buf4_sort(&X, CC_LR, prqs, 10, 10, lbl);
  sprintf(lbl, "X_%s_%1s_IbjA (%5.3f)", pert, cart, omega);
  dpd_buf4_sort(&X, CC_LR, psqr, 10, 10, lbl);
  sprintf(lbl, "X_%s_%1s_(2IjAb-IjbA) (%5.3f)", pert, cart, omega);
  dpd_buf4_scmcopy(&X, CC_LR, lbl, 2);
  dpd_buf4_sort_axpy(&X, CC_LR, pqsr, 0, 5, lbl, -1);
  dpd_buf4_close(&X);

  sprintf(lbl, "X_%s_%1s_IAjb (%5.3f)", pert, cart, omega);
  dpd_buf4_init(&X, CC_LR, irrep, 10, 10, 10, 10, 0, lbl);
  sprintf(lbl, "X_%s_%1s_(2IAjb-IbjA) (%5.3f)", pert, cart, omega);
  dpd_buf4_scmcopy(&X, CC_LR, lbl, 2);
  dpd_buf4_sort_axpy(&X, CC_LR, psrq, 10, 10, lbl, -1);
  dpd_buf4_close(&X);

  sprintf(lbl, "X_%s_%1s_IAjb (%5.3f)", pert, cart, omega);
  dpd_buf4_init(&X, CC_LR, irrep, 10, 10, 10, 10, 0, lbl);
  sprintf(lbl, "X_%s_%1s_(2IAjb-jAIb) (%5.3f)", pert, cart, omega);
  dpd_buf4_scmcopy(&X, CC_LR, lbl, 2);
  dpd_buf4_sort_axpy(&X, CC_LR, rqps, 10, 10, lbl, -1);
  dpd_buf4_close(&X);

  if(params.ref == 0 && !strcmp(params.abcd,"NEW")) {
    /* X(-)(ij,ab) (i>j, a>b) = X(ij,ab) - X(ij,ba) */
    sprintf(lbl, "X_%s_%1s_IjAb (%5.3f)", pert, cart, omega);
    dpd_buf4_init(&X, CC_LR, irrep, 4, 9, 0, 5, 1, lbl);
    sprintf(lbl, "X_%s_%1s_(-)(ij,ab) (%5.3f)", pert, cart, omega);
    dpd_buf4_copy(&X, CC_LR, lbl);
    dpd_buf4_close(&X);

    /* X(+)(ij,ab) (i>=j, a>=b) = X(ij,ab) + X(ij,ba) */
    sprintf(lbl, "X_%s_%1s_IjAb (%5.3f)", pert, cart, omega);
    dpd_buf4_init(&X, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
    sprintf(lbl, "X_%s_%1s_(+)(ij,ab) (%5.3f)", pert, cart, omega);
    dpd_buf4_copy(&X, CC_TMP0, lbl);
    dpd_buf4_sort_axpy(&X, CC_TMP0, pqsr, 0, 5, lbl, 1);
    dpd_buf4_close(&X);
    dpd_buf4_init(&X, CC_TMP0, irrep, 3, 8, 0, 5, 0, lbl);
    dpd_buf4_copy(&X, CC_LR, lbl);
    dpd_buf4_close(&X);
  }
}

}} // namespace psi::ccresponse