/*! \file
    \ingroup MVO
    \brief Enter brief description of file here 
*/
#ifndef _psi_src_bin_mvo_globals_h
#define _psi_src_bin_mvo_globals_h

/* First definitions of globals */
extern "C" {
  extern FILE *infile, *outfile;
  extern char *psi_file_prefix;
}

namespace psi { namespace mvo {

extern int *ioff;
extern struct MOInfo moinfo;
extern struct Params params;

#define PRINTOPDMLEVEL 3

}} // end namespace psi::mvo

#endif // header guard

