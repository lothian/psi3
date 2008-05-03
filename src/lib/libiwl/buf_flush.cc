/*!
  \file
  \ingroup IWL
*/
#include <cstdio>
#include <libciomr/libciomr.h>
#include "iwl.h"
#include "iwl.hpp"

using namespace psi;

void IWL::flush(int lastbuf)
{
    int idx;
    Label *lblptr;
    Value *valptr;

    inbuf_ = idx_;
    lblptr = labels_;
    valptr = values_;

    idx = 4 * idx_;

    while (idx_ < ints_per_buf_) {
        lblptr[idx++] = 0;
        lblptr[idx++] = 0;
        lblptr[idx++] = 0;
        lblptr[idx++] = 0;
        valptr[idx_] = 0.0;
        idx_++;
    }

    if (lastbuf) lastbuf_ = 1;
    else lastbuf_ = 0;

    put();
    idx_ = 0;
}

extern "C" {
	
/*!
** iwl_buf_flush()
**
**	\param Buf     To be flushed buffer
**	\param lastbuf Flag for the last buffer
**
** Flush an Integrals With Labels Buffer
** All flushing should be done through this routine!
** David Sherrill, March 1995
** \ingroup IWL
*/
void iwl_buf_flush(struct iwlbuf *Buf, int lastbuf)
{
  int idx;
  Label *lblptr;
  Value *valptr;
  
  Buf->inbuf = Buf->idx;
  lblptr = Buf->labels;
  valptr = Buf->values;
  
  idx = 4 * Buf->idx;

  while (Buf->idx < Buf->ints_per_buf) {
    lblptr[idx++] = 0;
    lblptr[idx++] = 0;
    lblptr[idx++] = 0;
    lblptr[idx++] = 0;
    valptr[Buf->idx] = 0.0;
    Buf->idx++;
  }
  
  if (lastbuf) Buf->lastbuf = 1;
  else Buf->lastbuf = 0;

  iwl_buf_put(Buf);
  Buf->idx = 0;
}

} /* extern "C" */