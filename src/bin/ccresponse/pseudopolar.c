#include <stdio.h>
#include <string.h>
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

double pseudopolar(char *pert, char *cart, int irrep, double omega)
{
  dpdfile2 X1, mubar1;
  dpdbuf4 X2, mubar2;
  char lbl[32];
  double polar1, polar2;

  sprintf(lbl, "%sBAR_%1s_IA", pert, cart);
  dpd_file2_init(&mubar1, CC_OEI, irrep, 0, 1, lbl);
  sprintf(lbl, "X_%s_%1s_IA (%5.3f)", pert, cart, omega);
  dpd_file2_init(&X1, CC_OEI, irrep, 0, 1, lbl);
  polar1 = 2.0 * dpd_file2_dot(&mubar1, &X1);
  dpd_file2_close(&mubar1);
  dpd_file2_close(&X1);

  sprintf(lbl, "%sBAR_%1s_IjAb", pert, cart);
  dpd_buf4_init(&mubar2, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  sprintf(lbl, "X_%s_%1s_(2IjAb-IjbA) (%5.3f)", pert, cart, omega);
  dpd_buf4_init(&X2, CC_LR, irrep, 0, 5, 0, 5, 0, lbl);
  polar2 = dpd_buf4_dot(&mubar2, &X2);
  dpd_buf4_close(&mubar2);
  dpd_buf4_close(&X2);

  /*
  fprintf(outfile, "polar1 = %20.12f \tpolar2 = %20.12f \tpolar = %20.12f\n", polar1, polar2, polar1+polar2);
  */

  return polar1+polar2;
}