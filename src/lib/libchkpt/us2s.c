/*!
  \file us2c.c
*/

#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>
#include <libciomr/libciomr.h>

/*!
** int *chkpt_rd_us2s()
** Read in a mapping array betwen unique shell and 
** full shell lists
**
**  returns: int *us2s   Read in an array num_unique_shell
*/


int *chkpt_rd_us2s(void)
{
  int *us2s;
  int num_unique_shells;

  num_unique_shells = chkpt_rd_num_unique_shell();
  us2s = init_int_array(num_unique_shells);

  psio_read_entry(PSIF_CHKPT, "::Unique shell -> full shell map", (char *) us2s, 
		  num_unique_shells*sizeof(int));

  return us2s;
}

/*!
** void chkpt_wt_us2s(int *)
** Writes out a mapping array betwen unique shell and 
** full shell lists.
**
**  arguments: 
**    \param int *us2s   An array num_unique_shell
**
**  returns: none
*/


void chkpt_wt_us2s(int *us2s)
{
  int num_unique_shells;

  num_unique_shells = chkpt_rd_num_unique_shell();

  psio_write_entry(PSIF_CHKPT, "::Unique shell -> full shell map", (char *) us2s, 
		   num_unique_shells*sizeof(int));
}
