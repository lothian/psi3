/*!
  \file shell_transm.c
*/

#include <stdio.h>
#include <libciomr/libciomr.h>
#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>

/*!
** chkpt_rd_shell_transm():	Read in a matrix of nshell*nirreps integers 
**			        that contains symmetry information.
**
**  takes no arguments.
**
**  returns: int **shell_transm	
*/


int **chkpt_rd_shell_transm(void)
{
  int i, nshell, nirreps;
  int **shell_transm;
  psio_address ptr;

  nshell = chkpt_rd_nshell();
  nirreps = chkpt_rd_nirreps();

  shell_transm = init_int_matrix(nshell,nirreps);
  ptr = PSIO_ZERO;
  for(i=0; i < nshell; i++)
    psio_read(PSIF_CHKPT, "::Shell transmat", (char *) shell_transm[i], 
	      nirreps*sizeof(int), ptr, &ptr);

  return shell_transm;
}

/*!
** chkpt_wt_shell_transm():	Write out a matrix of nshell*nirreps integers 
**			        that contains symmetry information.
**
**  arguments: 
**   \param int **shell_transm
**
** returns: none
*/

void chkpt_wt_shell_transm(int **shell_transm)
{
  int i, nshell, nirreps;
  psio_address ptr;

  nshell = chkpt_rd_nshell();
  nirreps = chkpt_rd_nirreps();

  ptr = PSIO_ZERO;
  for(i=0; i < nshell; i++) {
    psio_write(PSIF_CHKPT, "::Shell transmat", (char *) shell_transm[i], 
		nirreps*sizeof(int), ptr, &ptr);
  }
}
