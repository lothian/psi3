/*!
  \file buf_rd.c
  \ingroup (IWL)
*/
#include <stdio.h>
#include <math.h>
#include <libciomr/libciomr.h>
#include "iwl.h"

extern "C" {
	
#define MIN0(a,b) (((a)<(b)) ? (a) : (b))
#define MAX0(a,b) (((a)>(b)) ? (a) : (b))

/*!
** iwl_buf_rd(struct iwlbuf *Buf, int target_pq, double *ints,
**	       int *ioff_lt, int *ioff_rt, int mp2, int printflg, 
**	       FILE *outfile)
**
**
** Read from an Integrals With Labels formatted PSI buffer.
** The buffer must have been initialized with iwl_buf_init().
** David Sherrill, March 1995
**
** Returns: 0 if end of file, otherwise 1
**
** Altered such that if the current pq value does not equal the target_pq
** then routine returns.  This may be dangerous in that if you don't know
** the order of pq's in the iwl_buf, you may skip integrals!
** -Daniel, November 9, 1995
**
** Revised 6/26/96 by CDS for new format
** \ingroup (IWL)
*/
int iwl_buf_rd(struct iwlbuf *Buf, int target_pq, double *ints,
	       int *ioff_lt, int *ioff_rt, int mp2, int printflg, 
	       FILE *outfile)
{
  int lastbuf;
  Value *valptr;
  Label *lblptr;
  int idx, p, q, r, s, pq, rs;
  
  lblptr = Buf->labels;
  valptr = Buf->values;
  
  lastbuf = Buf->lastbuf;
  
  for (idx=4*Buf->idx; Buf->idx<Buf->inbuf; Buf->idx++) {
    p = (int) lblptr[idx++];
    q = (int) lblptr[idx++];
    r = (int) lblptr[idx++];
    s = (int) lblptr[idx++];
    
    if(mp2) { /*! I _think_ this will work */
      pq = ioff_lt[p] + q;
      rs = ioff_rt[r] + s;
    }
    else {
      pq = ioff_lt[MAX0(p,q)] + MIN0(p,q);
      rs = ioff_rt[MAX0(r,s)] + MIN0(r,s);
    }
    
    /*!      if (pq < target_pq) continue; */
    if (pq != target_pq) return(1);
    
    ints[rs] = (double) valptr[Buf->idx];
    
    if (printflg) 
      fprintf(outfile, "<%d %d %d %d [%d][%d] = %20.10lf\n",
	      p, q, r, s, pq, rs, ints[rs]) ;
    
  } /*! end loop through current buffer */
  
  /*! read new buffers */
  while (!lastbuf) {
    iwl_buf_fetch(Buf);
    lastbuf = Buf->lastbuf;
    
    for (idx=4*Buf->idx; Buf->idx<Buf->inbuf; Buf->idx++) {
      p = (int) lblptr[idx++];
      q = (int) lblptr[idx++];
      r = (int) lblptr[idx++];
      s = (int) lblptr[idx++];
      
      if(mp2) { /*! I _think_ this will work */
	pq = ioff_lt[p] + q;
	rs = ioff_rt[r] + s;
      }
      else {
	pq = ioff_lt[MAX0(p,q)] + MIN0(p,q);
	rs = ioff_rt[MAX0(r,s)] + MIN0(r,s);
      }
      
      if (pq < target_pq) continue;
      if (pq > target_pq) return(1);
      
      ints[rs] = (double) valptr[Buf->idx];
      
      if (printflg) 
	fprintf(outfile, "<%d %d %d %d [%d][%d] = %20.10lf\n",
		p, q, r, s, pq, rs, ints[rs]) ;
      
    } /*! end loop through current buffer */
    
  } /*! end loop over reading buffers */
  
  return(0); /*! we must have reached the last buffer at this point */
}

} /* extern "C" */