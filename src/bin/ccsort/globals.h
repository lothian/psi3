#include <ccfiles.h>

/* Global variables */
#ifdef EXTERN
#undef EXTERN
#define EXTERN extern
#else
#define EXTERN
#endif

EXTERN FILE *infile, *outfile;
EXTERN int *ioff;
EXTERN struct MOInfo moinfo;
EXTERN struct Params params;
