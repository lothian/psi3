#include <stdio.h>
#include <stdlib.h>
#include "dpd.h"

/* dpd_file4_close(): Closes a four-index dpd file.
**
** Arguments:
**   dpdfile4 *File: A pointer to the file to be closed.
*/

int dpd_file4_close(dpdfile4 *File)
{

  free(File->lfiles);

  if(!File->incore) free(File->matrix);
  else File->matrix = NULL;
  
  return 0;
}
