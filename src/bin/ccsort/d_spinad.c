#include <stdio.h>
#include <stdlib.h>
#include <libciomr.h>
#include <dpd.h>
#include "MOInfo.h"
#include "Params.h"
#define EXTERN
#include "globals.h"

void d_spinad(void)
{
  dpdbuf4 D, D1;

  if(params.ref != 2) { /*** RHF/ROHF ***/
    dpd_buf4_init(&D, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
    dpd_buf4_copy(&D, CC_DINTS, "D 2<ij|ab> - <ji|ab>");
    dpd_buf4_sort(&D, CC_TMP0, qprs, 0, 5, "D <ji|ab>");
    dpd_buf4_close(&D);

    dpd_buf4_init(&D, CC_DINTS, 0, 0, 5, 0, 5, 0, "D 2<ij|ab> - <ji|ab>");
    dpd_buf4_scm(&D, 2.0);
    dpd_buf4_init(&D1, CC_TMP0, 0, 0, 5, 0, 5, 0, "D <ji|ab>");
    dpd_buf4_axpy(&D1, &D, -1);
    dpd_buf4_close(&D1);
    dpd_buf4_close(&D);
  }
}
