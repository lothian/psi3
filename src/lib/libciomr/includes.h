
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "libciomr.h"

#if defined(DEC)
#  include <sys/types.h>
#  include <time.h>
#  include <sys/times.h>
#  include <sys/param.h>
#  include <sys/mount.h>
#  include <sys/stat.h>
#elif defined(SGI)
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <sys/times.h>
#  include <time.h>
#  include <sys/param.h>
#  include <sys/mount.h>
#elif defined(AIX)
#  include <sys/stat.h>
#  include <sys/vfs.h>
#  include <time.h>
#  include <sys/times.h>
#elif defined(Linux)
#  include <sys/stat.h>
#  include <sys/vfs.h>
#  include <time.h>
#  include <sys/times.h>
#  include <sys/param.h>
#else
#  include <sys/stat.h>
#  include <sys/vfs.h>
#  include <time.h>
#  include <sys/times.h>
#endif

#if (defined(DEC)||defined(SUN))
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

#define DSIGN(a,b) ((b) >= 0.0) ? (fabs(a)) : (-fabs(a))
#define MIN0(a,b) ((a)<(b)) ? (a) : (b)
#define MAX0(a,b) ((a)>(b)) ? (a) : (b)

#ifdef AIX
# define MALLOC malloc
#else
# define MALLOC malloc
#endif

#ifdef USE_DMALLOC
#include <dmalloc.h>
#endif
