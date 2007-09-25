/*!
** \file tstart.cc
** \brief Controls starting and stopping of timers
** \ingroup (CIOMR)
*/

#include "includes.h"
#define ALLOC_GLOBALS
#include "pointers.h"
#undef ALLOC_GLOBALS

#include <unistd.h>
#include <string.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

extern "C" {

/*
** tstart: Starts a timer.
**
** \param outfile = output file pointer
**
** \ingroup (CIOMR)
*/
void tstart(FILE *outfile)
{
  int i,error;
  char *name;
  name = (char *) malloc(40 * sizeof(char));
  error = gethostname(name, 40);
  if(error != 0) strncpy(name,"nohostname", 11);

  time_start = time(NULL);

  for (i=0; i < 78 ; i++) {
    fprintf(outfile,"*");
  }
  fprintf(outfile,"\n");

  fprintf(outfile,"tstart called on %s\n", name);
  fprintf(outfile,"%s\n",ctime(&time_start));

  free(name);
}

/*!
** tstop: Stop timer.
**
** \param outfile = output file pointer.
**
** \ingroup (CIOMR)
*/ 
void tstop(FILE *outfile)
{
  int i;
  int error;
  time_t total_time;
  struct tms total_tmstime;
  char *name;
  double user_s, sys_s;

  name = (char *) malloc(40 * sizeof(char));
  error = gethostname(name, 40);
  if(error != 0) strncpy(name,"nohostname", 11);

  time_end = time(NULL);
  total_time = time_end - time_start;

  times(&total_tmstime);
  const long clk_tck = sysconf(_SC_CLK_TCK);
  user_s = ((double) total_tmstime.tms_utime)/clk_tck;
  sys_s = ((double) total_tmstime.tms_stime)/clk_tck;

  for (i=0; i < 78 ; i++) {
    fprintf(outfile,"*");
  }
  fprintf(outfile,"\n");
  fprintf(outfile,"tstop called on %s\n", name);
  fprintf(outfile,"%s\n",ctime(&time_end));
  fprintf(outfile,"user time   = %10.2f seconds = %10.2f minutes\n",
	  user_s, user_s/60.0);
  fprintf(outfile,"system time = %10.2f seconds = %10.2f minutes\n",
	  sys_s, sys_s/60.0);
  fprintf(outfile,"total time  = %10d seconds = %10.2f minutes\n",
	  total_time, ((double) total_time)/60.0);

  free(name);

}

} /* extern "C" */
