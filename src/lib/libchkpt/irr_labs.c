/*!
  \file irr_labs.c
  \ingroup (CHKPT)
*/

#include <stdio.h>
#include <stdlib.h>
#include "chkpt.h"
#include <psifiles.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>

/*!
** chkpt_rd_irr_labs(): Read in the symmetry labels for all irreps in the 
**   point group in which the molecule is considered.
**
**   takes no arguments.
**
**   returns: irr_labs =  an array of labels (strings) which denote
**      the irreps for the point group	in which the molecule is considered,
**      _regardless_ of whether there exist any symmetry orbitals which 
**      transform as that irrep.  
** \ingroup (CHKPT)
*/

char **chkpt_rd_irr_labs(void)
{
  int i,nirreps;
  char **irr_labs;
  psio_address ptr;
  char *key;

  nirreps = chkpt_rd_nirreps();

  ptr = PSIO_ZERO;
  irr_labs = (char **)malloc(sizeof(char *)*nirreps);
  key = chkpt_build_keyword("Irrep labels");
  for(i=0;i<nirreps;i++) {
    irr_labs[i] = (char *) malloc(4*sizeof(char));
    psio_read(PSIF_CHKPT, key, (char *) irr_labs[i],  4*sizeof(char), ptr, &ptr);
    irr_labs[i][3] = '\0';
  }
  free(key);

  return irr_labs;
}


/*!
** chkpt_wt_irr_labs(): Write out the symmetry labels for all irreps in the 
**   point group in which the molecule is considered.
**
**  arguemnts: 
**  \param irr_labs = an array of labels (strings) which denote
**      the irreps for the point group	in which the molecule is considered,
**      _regardless_ of whether there exist any symmetry orbitals which 
**      transform as that irrep.  
** \ingroup (CHKPT)
*/

void chkpt_wt_irr_labs(char **irr_labs)
{
  int i,nirreps;
  psio_address ptr;
  char *key;

  nirreps = chkpt_rd_nirreps();

  key = chkpt_build_keyword("Irrep labels");
  ptr = PSIO_ZERO;
  for(i=0;i<nirreps;i++)
    psio_write(PSIF_CHKPT, key, (char *) irr_labs[i], 4*sizeof(char), ptr, &ptr);
  free(key);
}

