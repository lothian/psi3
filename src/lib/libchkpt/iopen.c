/*!
  \file iopen.c
  \ingroup (CHKPT)
*/

#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>

/*!
** int chkpt_rd_iopen()
** Reads in dimensionality of ALPHA and BETA vectors of two-electron 
** coupling coefficients for open shells.
**
** Note : IOPEN = MM * (MM + 1), where MM is the total number 
** of irreps containing singly occupied orbitals.
**
** returns: 
**   iopen = dimensionality of ALPHA and BETA vectors of coupling
**           coefficients for open shells. 
** \ingroup (CHKPT)
*/

int chkpt_rd_iopen(void)
{
  int iopen;

  psio_read_entry(PSIF_CHKPT, "::Iopen", (char *) &iopen, sizeof(int));
  return iopen;
}


/*!
** void chkpt_wt_iopen(int)
** Writes out the dimensionality of ALPHA and BETA vectors of two-electron 
** coupling coefficients for open shells.
**
** Note : IOPEN = MM * (MM + 1), where MM is the total number 
** of irreps containing singly occupied orbitals.
**
**  arguments: 
**   \param iopen = dimensionality of ALPHA and BETA vectors of coupling
**                  coefficients for open shells. 
** \ingroup (CHKPT)
*/

void chkpt_wt_iopen(int iopen)
{
  psio_write_entry(PSIF_CHKPT, "::Iopen", (char *) &iopen, sizeof(int));
}
