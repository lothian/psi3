/*! \file
    \ingroup CCHBAR
    \brief Enter brief description of file here 
*/
#include <stdio.h>

namespace psi { namespace cchbar {

void status(char *s, FILE *out)
{
  fprintf(out, "     %-15s...complete\n", s);
  fflush(out);
}

}} // namespace psi::cchbar
