#include <stdio.h>
#include <string.h>
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

void update_X(char *pert, char *cart, int irrep, double omega)
{
  dpdfile2 X1new, X1;
  dpdbuf4 X2new, X2;
  char lbl[32];

  sprintf(lbl, "New X_%s_%1s_IA (%5.3f)", pert, cart, omega);
  dpd_file2_init(&X1new, CC_OEI, irrep, 0, 1, lbl);
  sprintf(lbl, "X_%s_%1s_IA (%5.3f)", pert, cart, omega);
  dpd_file2_init(&X1, CC_OEI, irrep, 0, 1, lbl);
  dpd_file2_axpy(&X1, &X1new, 1, 0);
  dpd_file2_close(&X1);
  dpd_file2_close(&X1new);

  sprintf(lbl, "New X_%s_%1s_IjAb (%5.3f)", pert, cart, omega);
  dpd_buf4_init(&X2new, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  sprintf(lbl, "X_%s_%1s_IjAb (%5.3f)", pert, cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  dpd_buf4_axpy(&X2, &X2new, 1);
  dpd_buf4_close(&X2);
  dpd_buf4_close(&X2new);
}