#include <stdio.h>
#include <libciomr.h>
#include <dpd.h>
#define EXTERN
#include "globals.h"

void init_amps(void)
{
  dpdfile2 tIA, tia, fIA, fia, dIA, dia;
  dpdbuf4 tIJAB, tijab, tIjAb, D, dIJAB, dijab, dIjAb;

  /* Restart from previous amplitudes if we can/should */
  /*  Still need to shift this to new I/O
  if(params.restart && psio_flen(CC_tIA) && psio_flen(CC_tia)
     && psio_flen(CC_tIJAB) && psio_flen(CC_tijab) && psio_flen(CC_tIjAb))
      return;
      */

  dpd_file2_init(&fIA, CC_OEI, 0, 0, 1, "fIA");
  dpd_file2_copy(&fIA, CC_OEI, "tIA");
  dpd_file2_close(&fIA);
  dpd_file2_init(&tIA, CC_OEI, 0, 0, 1, "tIA");
/*  dpd_oe_scm(&tIA, 0);  */
  dpd_file2_close(&tIA);
  
  dpd_file2_init(&fia, CC_OEI, 0, 0, 1, "fia");
  dpd_file2_copy(&fia, CC_OEI, "tia");
  dpd_file2_close(&fia);
  dpd_file2_init(&tia, CC_OEI, 0, 0, 1, "tia");
/*  dpd_oe_scm(&tia, 0); */
  dpd_file2_close(&tia);

  dpd_file2_init(&tIA, CC_OEI, 0, 0, 1, "tIA");
  dpd_file2_init(&dIA, CC_OEI, 0, 0, 1, "dIA");
  dpd_file2_dirprd(&dIA, &tIA);
  dpd_file2_close(&tIA);
  dpd_file2_close(&dIA);

  dpd_file2_init(&tia, CC_OEI, 0, 0, 1, "tia");
  dpd_file2_init(&dia, CC_OEI, 0, 0, 1, "dia");
  dpd_file2_dirprd(&dia, &tia);
  dpd_file2_close(&tia);
  dpd_file2_close(&dia);

  dpd_buf4_init(&D, CC_DINTS, 0, 2, 7, 2, 7, 0, "D <ij||ab> (i>j,a>b)");
  dpd_buf4_copy(&D, CC_TAMPS, "tIJAB");
  dpd_buf4_copy(&D, CC_TAMPS, "tijab");
  dpd_buf4_close(&D);

  dpd_buf4_init(&dIJAB, CC_DENOM, 0, 1, 6, 1, 6, 0, "dIJAB");
  dpd_buf4_init(&tIJAB, CC_TAMPS, 0, 2, 7, 2, 7, 0, "tIJAB");
  dpd_buf4_dirprd(&dIJAB, &tIJAB);
  dpd_buf4_close(&tIJAB);
  dpd_buf4_close(&dIJAB);

  dpd_buf4_init(&dijab, CC_DENOM, 0, 1, 6, 1, 6, 0, "dijab");
  dpd_buf4_init(&tijab, CC_TAMPS, 0, 2, 7, 2, 7, 0, "tijab");
  dpd_buf4_dirprd(&dijab, &tijab);
  dpd_buf4_close(&tijab);
  dpd_buf4_close(&dijab);

  dpd_buf4_init(&D, CC_DINTS, 0, 0, 5, 0, 5, 0, "D <ij|ab>");
  dpd_buf4_copy(&D, CC_TAMPS, "tIjAb");
  dpd_buf4_close(&D);
  
  dpd_buf4_init(&dIjAb, CC_DENOM, 0, 0, 5, 0, 5, 0, "dIjAb");
  dpd_buf4_init(&tIjAb, CC_TAMPS, 0, 0, 5, 0, 5, 0, "tIjAb");
  dpd_buf4_dirprd(&dIjAb, &tIjAb);
  dpd_buf4_close(&tIjAb);
  dpd_buf4_close(&dIjAb);
}
