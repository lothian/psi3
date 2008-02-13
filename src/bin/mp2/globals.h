/*! \file
    \ingroup MP2
    \brief Enter brief description of file here 
*/

#ifndef _psi_src_bin_mp2_globals_h_
#define _psi_src_bin_mp2_globals_h_

#include <ccfiles.h>
#include "moinfo.h"
#include "params.h"

#ifdef EXTERN
#undef EXTERN
#define EXTERN extern
#else
#define EXTERN
#endif

#define MAXIOFF 32641
#define INDEX(i,j) ((i>j) ? (ioff[(i)]+(j)) : (ioff[(j)]+(i)))

extern "C" {
  EXTERN FILE *infile, *outfile;
  EXTERN char *psi_file_prefix;
}
EXTERN struct moinfo mo;
EXTERN struct params params;
EXTERN int* ioff;

#endif /* Header guard */
