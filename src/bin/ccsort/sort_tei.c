#include <stdio.h>
#include <dpd.h>
#include <qt.h>
#include "MOInfo.h"
#include "Params.h"
#define EXTERN
#include "globals.h"

void distribute(void);

int file_build(dpdfile4 *File, int inputfile, double tolerance,
	       int perm_pr, int perm_qs, int perm_prqs, int keep);

void sort_tei(void)
{
  double tolerance;
  dpdfile4 A, B, C, D, E, F;

  tolerance = params.tolerance;

  distribute();

  dpd_file4_init_nocache(&A, CC_AINTS, 0, 0, 0, "A <ij|kl>");
  file_build(&A, 90, tolerance, 1, 1, 1, 0);
  dpd_file4_close(&A);

  timer_on("build B");
  dpd_file4_init_nocache(&B, CC_BINTS, 0, 5, 5, "B <ab|cd>");
  file_build(&B, 91, tolerance, 1, 1, 1, 0);
  dpd_file4_close(&B);
  timer_off("build B");

  dpd_file4_init_nocache(&C, CC_CINTS, 0, 10, 10, "C <ia|jb>");
  file_build(&C, 92, tolerance, 1, 1, 0, 0);
  dpd_file4_close(&C);

  dpd_file4_init_nocache(&D, CC_DINTS, 0, 0, 5, "D <ij|ab>");
  file_build(&D, 93, tolerance, 0, 0, 1, 0);
  dpd_file4_close(&D);

  dpd_file4_init_nocache(&E, CC_EINTS, 0, 11, 0, "E <ai|jk>");
  file_build(&E, 94, tolerance, 0, 1, 0, 0);
  dpd_file4_close(&E);

  dpd_file4_init_nocache(&F, CC_FINTS, 0, 10, 5, "F <ia|bc>");
  file_build(&F, 95, tolerance, 0, 1, 0, 0);
  dpd_file4_close(&F);

  fflush(outfile);

}
