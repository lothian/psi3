
#include <libciomr/libciomr.h>
#include <libipv1/ip_lib.h>
#include <libchkpt/chkpt.h>
#include <psifiles.h>
#include "qt.h"

#define PSIO_INIT if (!psio_state()) { \
    psio_init(); \
    need_to_init_psio = 1; \
  }

#define CHKPT_INIT(n) if (!psio_open_check(PSIF_CHKPT)) { \
    chkpt_init(n); \
    need_to_init_chkpt = 1; \
  }

#define PSIO_DONE if (need_to_init_psio) \
    psio_done();

#define CHKPT_DONE if (need_to_init_chkpt) \
    chkpt_close();

int* get_frzcpi()
{
  int errcod;
  int nirreps, nirr;
  int need_to_init_psio = 0;
  int need_to_init_chkpt = 0;
  int if_exists;
  int* frzcpi;

PSIO_INIT
CHKPT_INIT(PSIO_OPEN_OLD);
  nirreps = chkpt_rd_nirreps();
  
  frzcpi = init_int_array(nirreps);
  
  if_exists = ip_exist(":DEFAULT:FROZEN_DOCC",0);
  if (if_exists) {
    errcod = ip_int_array(":DEFAULT:FROZEN_DOCC",frzcpi,nirreps);
  }
  else {
    free(frzcpi);
    frzcpi = chkpt_rd_frzcpi();
  }

CHKPT_DONE
PSIO_DONE

  return frzcpi;
}


int* get_frzvpi()
{
  int errcod;
  int nirreps, nirr;
  int need_to_init_psio = 0;
  int need_to_init_chkpt = 0;
  int if_exists;
  int* frzvpi;

PSIO_INIT
CHKPT_INIT(PSIO_OPEN_OLD);
  nirreps = chkpt_rd_nirreps();
CHKPT_DONE
PSIO_DONE
  
  frzvpi = init_int_array(nirreps);
  
  if_exists = ip_exist(":DEFAULT:FROZEN_UOCC",0);
  if (if_exists) {
    errcod = ip_int_array(":DEFAULT:FROZEN_UOCC",frzvpi,nirreps);
  }

  return frzvpi;
}
