#include <stdio.h>
#include <libciomr/libciomr.h>
#include "dpd.h"
#define EXTERN
#include "dpd.gbl"

/* dpd_file4_mat_irrep_close(): Releases memory for a matrix for a
** single irrep of a dpd four-index file.
**
** Arguments:
**   dpdfile4 *File: A pointer to the input dpdfile.
**   int irrep: The irrep number to be freed.
*/

int dpd_file4_mat_irrep_close(dpdfile4 *File, int irrep)
{
  int my_irrep, rowtot, coltot;

  my_irrep = File->my_irrep;

  rowtot = File->params->rowtot[irrep];
  coltot = File->params->coltot[irrep^my_irrep];

  if(File->incore) return 0;  /* We need to keep the memory */

  if(rowtot*coltot) dpd_free_block(File->matrix[irrep], rowtot, coltot);

  return 0;
}
