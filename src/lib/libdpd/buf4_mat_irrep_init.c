#include <stdio.h>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>
#include "dpd.h"
#define EXTERN
#include "dpd.gbl"

/* dpd_buf4_mat_irrep_init(): Allocates and initializes memory for a
** matrix for a single irrep of a dpd four-index buffer.
**
** Arguments:
**   dpdbuf4 *Buf: A pointer to the input dpdbuf.
**   int irrep: The irrep number to be prepared.
*/

int dpd_buf4_mat_irrep_init(dpdbuf4 *Buf, int irrep)
{
  int rowtot, coltot, all_buf_irrep;

  all_buf_irrep = Buf->file.my_irrep;
  rowtot = Buf->params->rowtot[irrep];
  coltot = Buf->params->coltot[irrep^all_buf_irrep];

#ifdef DPD_TIMER
  timer_on("buf4_init");
#endif

  if(rowtot*coltot) {

      /* If the file member is already in cache and its ordering is the 
         same as the parent buffer, don't malloc() memory, just assign 
         the pointer */
      if(Buf->file.incore && !(Buf->anti) && 
          (Buf->params->pqnum == Buf->file.params->pqnum) &&
          (Buf->params->rsnum == Buf->file.params->rsnum))
          Buf->matrix[irrep] = Buf->file.matrix[irrep];
      else {
          Buf->matrix[irrep] = dpd_block_matrix(rowtot,coltot);
	}

    }

#ifdef DPD_TIMER
  timer_off("buf4_init");
#endif

  return 0;

}
