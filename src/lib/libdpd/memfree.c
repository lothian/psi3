/*
** Function to return number of double words available for allocation.
*/

#include <stdio.h>
#include "dpd.h"
#define EXTERN
#include "dpd.gbl"

long int dpd_memfree(void)
{
  return dpd_main.memory - (dpd_main.memused - 
			    dpd_main.memcache + 
			    dpd_main.memlocked);
}
