#include <stdio.h>
#include "params.h"

#define PRINT_INTRO 1
#define PRINT_PARAMS 1

extern FILE *outfile;
extern Params_t Params;

void print_intro()
{
  if (Params.print_lvl >= PrintLevels::print_intro) {
     fprintf(outfile,"                  ----------------------------------------------\n");
     fprintf(outfile,"                    DBOC: diagonal Born-Oppenheimer correction\n");
     fprintf(outfile,"                      evaluation by numerical differentiation\n");
     fprintf(outfile,"                  ----------------------------------------------\n\n");
  }
  fflush(outfile);
}

void print_params()
{
  if (Params.print_lvl >= PrintLevels::print_params) {
    fprintf(outfile,"\n  -OPTIONS:\n");
    fprintf(outfile,"    Print level                 = %d\n",Params.print_lvl);
    fprintf(outfile,"    Displacement size           = %lf a.u.\n",Params.delta);
    if (strcmp(Params.wfn,"SCF"))
      fprintf(outfile,"    Wave function               = %s\n",Params.wfn);
    else if (Params.reftype == Params_t::rhf)
      fprintf(outfile,"    Wave function               = RHF SCF\n");
    else if (Params.reftype == Params_t::rohf)
      fprintf(outfile,"    Wave function               = ROHF SCF\n");
    else if (Params.reftype == Params_t::uhf)
      fprintf(outfile,"    Wave function               = UHF SCF\n");
    fprintf(outfile,"\n");
  }

  fflush(outfile);
}
  
