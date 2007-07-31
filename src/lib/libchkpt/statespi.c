/*!
  \file statespi.c
  \ingroup (CHKPT)
*/

#include <stdio.h>
#include <stdlib.h>
#include "chkpt.h"
#include <psifiles.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>

/*!
** chkpt_rd_statespi():  Reads in the number of excited-states for each
**   irrep.
**
**   takes no arguments.
**
**   returns:
**     int *statespi  an array which has an element for each irrep of the
**                 point group of the molecule (n.b. not just the ones
**                 with a non-zero number of basis functions). each 
**                 element contains the number of excited states of that
**                 irrep to be studied.
** \ingroup (CHKPT)
*/

int *chkpt_rd_statespi(void)
{
  int nirreps;
  int *statespi;
  char *keyword;
  keyword = chkpt_build_keyword("States per irrep");

  nirreps = chkpt_rd_nirreps();
  statespi = init_int_array(nirreps);

  psio_read_entry(PSIF_CHKPT, keyword, (char *) statespi, 
    nirreps*sizeof(int));

  free(keyword);
  return statespi;
}


/*!
** chkpt_wt_statespi():  Writes the number of excited states in each irrep.
**
** \param statespi = an array which has an element for each irrep of the
**                 point group of the molecule (n.b. not just the ones
**                 with a non-zero number of basis functions). each 
**                 element contains the number of excited states of that
**                 irrep to be studied.
**
** returns: none
** \ingroup (CHKPT)
*/

void chkpt_wt_statespi(int *statespi)
{
  int nirreps;
  char *keyword;
  keyword = chkpt_build_keyword("States per irrep");

  nirreps = chkpt_rd_nirreps();

  psio_write_entry(PSIF_CHKPT, keyword, (char *) statespi, 
    nirreps*sizeof(int));

  free(keyword);
}