#include<stdio.h>
#include<libint/libint.h>

#include"defines.h"
#define EXTERN
#include"global.h"

#include"ccinfo.h"

extern void cc_bt2();

void direct_cc()
{
  
  init_ccinfo();

  if (UserOptions.make_cc_bt2)
    cc_bt2();

  cleanup_ccinfo();

  return;
}