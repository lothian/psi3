/*!
  \file prefix.c
  \ingroup (CHKPT)
*/

#include <string.h>
#include "chkpt.h"
#include <psifiles.h>
#include <libpsio/psio.h>

/*!
**  char *chkpt_rd_prefix() 
**  Reads the global default chkpt prefix keyword stored in the CHKPT file.
**
**  returns: the prefix string
** \ingroup (CHKPT)
*/

char *chkpt_rd_prefix(void)
{  
  char *prefix;

  prefix = (char *) malloc(CHKPT_PREFIX_LEN*sizeof(char));

  psio_read_entry(PSIF_CHKPT, "Default prefix", prefix, CHKPT_PREFIX_LEN*sizeof(char));

  return prefix;
}

/*!
**  void chkpt_wt_prefix() 
**  Writes the global default chkpt prefix keyword.
**
**  \param prefix = the prefix string (must be CHKPT_PREFIX_LEN long)
**
**  returns: none
** \ingroup (CHKPT)
*/
void chkpt_wt_prefix(char *prefix)
{  
  psio_write_entry(PSIF_CHKPT, "Default prefix", prefix, CHKPT_PREFIX_LEN*sizeof(char));
}


/*!
**  void chkpt_set_prefix() 
**  Sets the default chkpt prefix in global memory.  After this is set,
**  it is intended that all chkpt_rd_() and chkpt_wt_() calls will use
**  this prefix for psio keyword strings.
**
**  \param prefix = the prefix string
**
**  returns: none
** \ingroup (CHKPT)
*/
void chkpt_set_prefix(char *prefix)
{
  strcpy(chkpt_prefix, prefix);
}

/*!
**  void chkpt_commit_prefix() 
**  Writes the default chkpt prefix from global memory into the chkpt file.
**
**  arguments: none
**
**  returns: none
** \ingroup (CHKPT)
*/
void chkpt_commit_prefix(void)
{
  chkpt_wt_prefix(chkpt_prefix);
}

/*!
**  void chkpt_reset_prefix() 
**  Sets the chkpt prefix in global memory back to its default.  At 
**  present this is a null string.
**
**  arguments: none
**
**  returns: none
** \ingroup (CHKPT)
*/
void chkpt_reset_prefix(void)
{
  chkpt_prefix[0] = '\0';
}

/*!
**  char * chkpt_get_prefix() 
**  Returns a copy of the current chkpt prefix default stored 
**  in global memory.
**
**  arguments: none
**
**  returns: prefix = the current global prefix
** \ingroup (CHKPT)
*/
char *chkpt_get_prefix(void)
{
  char *prefix;

  prefix = (char *) malloc(CHKPT_PREFIX_LEN*sizeof(char));

  strcpy(prefix,chkpt_prefix);

  return prefix;
}
