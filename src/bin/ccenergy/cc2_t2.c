#include <stdio.h>
#include <stdlib.h>
#include <libdpd/dpd.h>
#define EXTERN
#include "globals.h"

void dijabT2(void);
void cc2_faeT2(void);
void cc2_fmiT2(void);
void cc2_WmbijT2(void);
void cc2_WabeiT2(void);

void cc2_t2_build(void)
{

  DT2();

  if((params.ref == 0) || params.t2_coupled) { /** RHF or ROHF with coupled T2's **/ 
    timer_on("fT2", outfile);
    cc2_faeT2(); cc2_fmiT2();
    if(params.print & 2) status("f -> T2", outfile);
    timer_off("fT2", outfile);
  }

  timer_on("WmbijT2", outfile);
  cc2_WmbijT2();
  if(params.print & 2) status("Wmbij -> T2", outfile);
  timer_off("WmbijT2", outfile);

  timer_on("WabeiT2", outfile);
  cc2_WabeiT2();
  if(params.print & 2) status("Wabei -> T2", outfile);
  timer_off("WabeiT2", outfile);

  dijabT2();

}
