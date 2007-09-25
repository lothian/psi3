/*!
  \file buf_wrt_mp2.c
  \ingroup (IWL)
*/
#include <stdio.h>
#include <math.h>
#include <libciomr/libciomr.h>
#include "iwl.h"

extern "C" {
	
/*!
** iwl_buf_wrt_mp2()
**
** Write to an Integrals With Labels formatted PSI buffer.
** The buffer must have been initialized with iwl_buf_init().  Don't
** forget to call iwl_buf_flush() when finished with all writes to the
** buffer to ensure that all contents are written to disk.
** David Sherrill, March, 1995
**
** This routine is a modified form of iwl_buf_wrt() specific to mp2-type
** restricted transforms.  It's not general, but it should work.
** Daniel, 9/25/95
** \ingroup (IWL)
*/
void iwl_buf_wrt_mp2(struct iwlbuf *Buf, int p, int q, int pq, int pqsym,
   double **arr, int rsym, int *firstr, int *lastr, int *firsts, int *lasts,
   int *occ, int *vir, int *ioff, int printflag, FILE *outfile)
{
   int idx, r, s, rs, ssym;
   int R,S,rnew,snew;
   double value;
   Label *lblptr;
   Value *valptr;

   lblptr = Buf->labels;
   valptr = Buf->values;

   ssym = pqsym ^ rsym;
   for (r=firstr[rsym],R=0; r <= lastr[rsym]; r++,R++) {
     rnew = occ[r];
     for (s=firsts[ssym],S=0; s <=lasts[ssym]; s++,S++) {
       snew = vir[s];
       rs = ioff[rnew] + snew;
       /*------------------------------------------
	 We do not need integrals with rs > pq
	 rs can only increase, hence if rs > pq -
	 it is time to leave
	------------------------------------------*/
       if (rs > pq)
	 return;
       value = arr[R][S];
       
       if (fabs(value) > Buf->cutoff) {
	 idx = 4 * Buf->idx;
	 lblptr[idx++] = (Label) p;
	 lblptr[idx++] = (Label) q;
	 lblptr[idx++] = (Label) rnew;
	 lblptr[idx++] = (Label) snew;
	 valptr[Buf->idx] = (Value) value;
	 
	 Buf->idx++;
	 
	 if (Buf->idx == Buf->ints_per_buf) {
	   Buf->lastbuf = 0;
	   Buf->inbuf = Buf->idx;
	   iwl_buf_put(Buf);
	   Buf->idx = 0;
	 }
	 
	 if(printflag)
	   fprintf(outfile, "<%d %d %d %d [%d] [%d] = %20.10lf\n",
		   p, q, rnew, snew, pq, rs, value);
	 
       } /* end if (fabs(value) > Buf->cutoff) ... */
     } /* end loop over s */
   } /* end loop over r */
   
}

} /* extern "C"*/