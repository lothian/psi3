#define EXTERN
#include <stdio.h>
#include <stdlib.h>
#include <libciomr/libciomr.h>
#include <libipv1/ip_lib.h>
#include "input.h"
#include "global.h"
#include "defines.h"

void init_globals()
{
  int i, j, errcod;

  disp_num = 0;
  
  /*Make ioff and stuff*/
  setup(MAXIOFF);

  errcod = ip_string("WFN", &wfn, 0);
     
  /*Create the elem_name array to hold all element names*/
  elem_name = (char **) malloc(sizeof(char *)*NUM_ELEMENTS);
  init_elem_names();

  /*No rotation has been performed yet -
    hence canonical and reference frames are equivalent */
  Rref = block_matrix(3,3);
  canon_eq_ref_frame();

  /*GTO data has not been initialized yes */
  GTOs.max_angmom = 0;

  return;
}


