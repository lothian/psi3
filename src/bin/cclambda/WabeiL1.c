#include <stdio.h>
#include <dpd.h>
#define EXTERN
#include "globals.h"

void WabeiL1(void)
{
  dpdfile2 newL1;
  dpdbuf4 W, L2;

  dpd_file2_init(&newL1, CC_OEI, 0, 0, 1, "New L(I,A)");
  dpd_buf4_init(&W, CC_HBAR, 0, 11, 7, 11, 7, 0, "W(AM,EF)");
  dpd_buf4_init(&L2, CC_LAMPS, 0, 0, 7, 2, 7, 0, "L2(IM,EF)");
  dpd_contract442(&L2, &W, &newL1, 0, 0, 1.0, 1.0);
  dpd_buf4_close(&W);
  dpd_buf4_close(&L2);
  dpd_buf4_init(&W, CC_HBAR, 0, 11, 5, 11, 5, 0, "W(Am,Ef)");
  dpd_buf4_init(&L2, CC_LAMPS, 0, 0, 5, 0, 5, 0, "L2(Im,Ef)");
  dpd_contract442(&L2, &W, &newL1, 0, 0, 1.0, 1.0);
  dpd_buf4_close(&W);
  dpd_buf4_close(&L2);
  dpd_file2_close(&newL1);

  dpd_file2_init(&newL1, CC_OEI, 0, 0, 1, "New L(i,a)");
  dpd_buf4_init(&W, CC_HBAR, 0, 11, 7, 11, 7, 0, "W(am,ef)");
  dpd_buf4_init(&L2, CC_LAMPS, 0, 0, 7, 2, 7, 0, "L2(im,ef)");
  dpd_contract442(&L2, &W, &newL1, 0, 0, 1.0, 1.0);
  dpd_buf4_close(&W);
  dpd_buf4_close(&L2);
  dpd_buf4_init(&W, CC_HBAR, 0, 11, 5, 11, 5, 0, "W(aM,eF)");
  dpd_buf4_init(&L2, CC_LAMPS, 0, 0, 5, 0, 5, 0, "L2(iM,eF)");
  dpd_contract442(&L2, &W, &newL1, 0, 0, 1.0, 1.0);
  dpd_buf4_close(&W);
  dpd_buf4_close(&L2);
  dpd_file2_close(&newL1);
}
