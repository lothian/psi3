/*!
  \file am2canon_shell_order.c
*/

#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>
#include <libciomr/libciomr.h>

/*!
** int *chkpt_rd_am2canon_shell_order() 
** Reads in the mapping array from the am-ordered
** to the canonical (in the order of appearance)
** list of shells.
**
** returns: int *am2can_shell_order
*/


int *chkpt_rd_am2canon_shell_order(void)
{
  int *am2can_sh_ord, nshell;

  nshell = chkpt_rd_nshell();
  am2can_sh_ord = init_int_array(nshell);

  psio_read_entry(PSIF_CHKPT, "::AM -> canonical shell map", (char *) am2can_sh_ord, 
		  nshell*sizeof(int));

  return am2can_sh_ord;
}

/*!
** void chkpt_wt_am2canon_shell_order(int *) 
** Writes out the mapping array from the am-ordered
** to the canonical (in the order of appearance)
** list of shells.
**
** arguments: 
**  \param int *am2can_shell_order
**
** returns: none
*/


void chkpt_wt_am2canon_shell_order(int *am2can_sh_ord)
{
  int nshell;

  nshell = chkpt_rd_nshell();

  psio_write_entry(PSIF_CHKPT, "::AM -> canonical shell map", (char *) am2can_sh_ord, 
		   nshell*sizeof(int));
}
