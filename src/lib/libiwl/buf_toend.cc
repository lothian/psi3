/*!
  \file buf_toend.c
  \ingroup (IWL)
*/
#include <stdio.h>
#include <stdlib.h>
#include <libpsio/psio.h>
#include "iwl.h"

extern "C" {
	
extern FILE *outfile;

/*!
** iwl_buf_toend()
**
** Set IWL Buffer's pointer to the end. Useful when want to append to
** an already existing file
**
** Edward Valeev, January 2001
** \ingroup (IWL)
*/
void iwl_buf_toend(struct iwlbuf *Buf)
{
  psio_tocentry *this_entry;
  ULI entry_length;

  this_entry = psio_tocscan(Buf->itap, IWL_KEY_BUF);
  if (this_entry == NULL) {
    fprintf(outfile,
        "iwl_buf_toend: Can't find IWL buffer entry in file %d\n", Buf->itap);
    iwl_buf_close(Buf,1);
    return;
  } 

  /* set up buffer pointer */
  entry_length = psio_get_length(this_entry->sadd,this_entry->eadd);
  Buf->bufpos = psio_get_address(PSIO_ZERO,entry_length);
  
  return;
}

} /* extern "C" */