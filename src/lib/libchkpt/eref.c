/*!
  \file eref.c
*/

#include <stdio.h>
#include "chkpt.h"
#include <psifiles.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>

/*!
** chkpt_rd_eref(): Reads in the reference energy.
**
**   takes no arguments.
**
**   returns: double eref  the reference energy.
*/
double chkpt_rd_eref(void)
{
  double eref;

  psio_read_entry(PSIF_CHKPT, "::Reference energy", (char *) &eref, sizeof(double));

  return eref;
}

/*!
** chkpt_wt_eref(): Writes out the reference energy.
**
** arguments: 
**   \param double eref  the reference energy.
**
** returns: none
*/
void chkpt_wt_eref(double eref)
{
  psio_write_entry(PSIF_CHKPT, "::Reference energy", (char *) &eref, sizeof(double));
}

