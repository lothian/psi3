/*! \file buf4_close.c
    \ingroup (DPD)
    \brief Enter brief description of file here 
*/
#include <stdio.h>
#include <stdlib.h>
#include <libciomr/libciomr.h>
#include "dpd.h"

/* dpd_buf4_close(): Closes a dpd four-index buffer.
**
** Arguments:
**   dpdbuf4 *Buf: A pointer to the dpdbuf4 to be closed.
*/

int dpd_buf4_close(dpdbuf4 *Buf)
{
  int nirreps;

  nirreps = Buf->params->nirreps;
  
  dpd_file4_close(&(Buf->file));

  free(Buf->matrix);

  free_int_matrix(Buf->shift.rowtot,nirreps);
  free_int_matrix(Buf->shift.coltot,nirreps);

  free_int_matrix(Buf->row_offset, nirreps);
  free_int_matrix(Buf->col_offset, nirreps);

  free(Buf->shift.matrix);

  return 0;
}
