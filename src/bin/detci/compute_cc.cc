/*
** COMPUTE_CC.CC
**
** This will be a prototype implementation of arbitrary-order 
** coupled-cluster code.
** 
** C. David Sherrill
** Center for Computational Molecular Science and Technology
** Georgia Institute of Technology
** March 2005
**
** Note: I think I need onel ints as g for formation of sigma
** in non-FCI cases, but make sure any CC parts don't try to get
** h and actually get g instead...
*/

#define EXTERN

#include <math.h>

extern "C" {
   #include <stdio.h>
   #include <stdlib.h>
   #include <psifiles.h>
   #include <libipv1/ip_lib.h>
   #include <libciomr/libciomr.h>
   #include <libqt/qt.h>
   #include <libqt/slaterdset.h>
   #include <libpsio/psio.h>
   #include "structs.h"
   #include "globals.h"
}
                                                                                
int cc_reqd_sblocks[CI_BLK_MAX];

/*
** compute_cc()
**
** This is the top-level function that controls the coupled-cluster
** computation
**
*/
void compute_cc(void)
{
  printf("compute_cc: Not yet available\n");
}
