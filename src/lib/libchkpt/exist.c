/*!
  \file exist.c
  \ingroup (CHKPT)
*/

#include <stdio.h>
#include <stdlib.h>
#include "chkpt.h"
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <psifiles.h>

/*!
** chkpt_exist(): Checks to see if entry already exists in chkpt file. Note
** this function should be called only by functions in the chkpt library, as
** the calling function prepends the prefix.
**
**   takes no arguments.
**  
**   returns: 1 if entry exists, 0 otherwise
**        
** \ingroup (CHKPT)
*/

int chkpt_exist(char *keyword)
{
  int exists=0;

  if (psio_tocscan(PSIF_CHKPT, keyword) != NULL)
    exists = 1;

  return exists;
}
