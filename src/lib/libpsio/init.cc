/*!
 \file init.cc
 \ingroup (PSIO)
 */

#include <stdio.h>
#include <stdlib.h>
extern "C" {
#include <libpsio/psio.h>
}
#include <libpsio/psio.hpp>

using namespace psi;

/* Definition of global data */
PSIO* psi::_default_psio_lib_ = 0;
int PSIO::_error_exit_code_ = 1;
extern "C" {
  psio_address PSIO_ZERO = { 0, 0 };
}

PSIO::PSIO() {
  int i, j;
  
  psio_unit = (psio_ud *) malloc(sizeof(psio_ud)*PSIO_MAXUNIT);
#ifdef PSIO_STATS
  psio_readlen = (ULI *) malloc(sizeof(ULI) * PSIO_MAXUNIT);
  psio_writlen = (ULI *) malloc(sizeof(ULI) * PSIO_MAXUNIT);
#endif
  state_ = 1;
  
  if (psio_unit == NULL) {
    fprintf(stderr, "Error in PSIO_INIT()!\n");
    exit(_error_exit_code_);
  }
  
  for (i=0; i < PSIO_MAXUNIT; i++) {
#ifdef PSIO_STATS
    psio_readlen[i] = psio_writlen[i] = 0;
#endif      
    psio_unit[i].numvols = 0;
    for (j=0; j < PSIO_MAXVOL; j++) {
      psio_unit[i].vol[j].path = NULL;
      psio_unit[i].vol[j].stream = -1;
    }
    psio_unit[i].toclen = 0;
    psio_unit[i].toc = NULL;
  }
}

extern "C" {
  
  /*!
   ** PSIO_INIT(): Allocates global memory needed by the I/O routines.
   **
   ** No arguments.
   **
   ** \ingroup (PSIO)
   */

  int psio_init(void) {
    if (!_default_psio_lib_) {
      _default_psio_lib_ = new PSIO;
      if (_default_psio_lib_ == 0) {
        fprintf(stderr,"LIBPSIO::init() -- failed to allocate the memory");
        exit(PSIO::_error_exit_code_);
      }
    }
    
    return 1;
  }
  
  /*!
   ** PSIO_STATE(): Returns state of the library (1=initialized, 0=noninitialized).
   **
   ** No arguments.
   **
   ** \ingroup (PSIO)
   */

  int psio_state() {
    return _default_psio_lib_->state();
  }

}
