/*! \file status.c
    \ingroup (CCHBAR)
    \brief Enter brief description of file here 
*/
#include <stdio.h>

void status(char *s, FILE *out)
{
  fprintf(out, "     %-15s...complete\n", s);
  fflush(out);
}
