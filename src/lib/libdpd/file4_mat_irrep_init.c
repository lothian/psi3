#include <stdio.h>
#include <libciomr/libciomr.h>
#include "dpd.h"
#define EXTERN
#include "dpd.gbl"

/* dpd_file4_mat_irrep_init(): Allocates and initializes memory for a
** matrix for a single irrep of a dpd four-index file.
**
** Arguments:
**   dpdfile4 *File: A pointer to the input dpdfile.
**   int irrep: The irrep number to be prepared.
*/

int dpd_file4_mat_irrep_init(dpdfile4 *File, int irrep)
{
  int my_irrep, rowtot, coltot;

  my_irrep = File->my_irrep;
  rowtot = File->params->rowtot[irrep];
  coltot = File->params->coltot[irrep^my_irrep];

  if(File->incore) return 0;  /* We've already got the memory */

  if(rowtot * coltot) File->matrix[irrep] = dpd_block_matrix(rowtot,coltot);

  return 0;
}
