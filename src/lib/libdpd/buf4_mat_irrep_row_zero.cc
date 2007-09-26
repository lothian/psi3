/*! \file 
    \ingroup (DPD)
    \brief Enter brief description of file here 
*/
#include <stdio.h>
#include <libciomr/libciomr.h>
#include "dpd.h"

extern "C" {

int dpd_buf4_mat_irrep_row_zero(dpdbuf4 *Buf, int irrep, int row)
{
  int coltot, all_buf_irrep;
  
  all_buf_irrep = Buf->file.my_irrep;
  coltot = Buf->params->coltot[irrep^all_buf_irrep];

  if(coltot)
      zero_arr(Buf->matrix[irrep][0], coltot);

  return 0;

}

} /* extern "C" */
