/*!
  \file shells_per_am.c
  \ingroup (CHKPT)
*/

#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>
#include <libciomr/libciomr.h>

/*!
** int *chkpt_rd_shells_per_am() 
** Reads in the numbers of shells of each angular momentum.
**
** returns: shells_per_am = array of shells per angular momentum
**
** \ingroup (CHKPT)
*/

int *chkpt_rd_shells_per_am(void)
{
  int *shells_per_am;
  int max_am;

  max_am = chkpt_rd_max_am();
  shells_per_am = init_int_array(max_am+1);

  psio_read_entry(PSIF_CHKPT, "::Shells per am", (char *) shells_per_am,
		  (max_am+1)*sizeof(int));

  return shells_per_am; 
}


/*!
** void chkpt_wt_shells_per_am(int *) 
** Writes out the numbers of shells of each angular momentum.
**
** \param shells_per_am = array of shells per angular momentum
**
** returns: none
**
** \ingroup (CHKPT)
*/

void chkpt_wt_shells_per_am(int *shells_per_am)
{
  int max_am;

  max_am = chkpt_rd_max_am();

  psio_write_entry(PSIF_CHKPT, "::Shells per am", (char *) shells_per_am,
		   (max_am+1)*sizeof(int));
}
