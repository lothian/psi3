/*!
  \file buf_wrt_mat.c
  \ingroup (IWL)
*/
#include <stdio.h>
#include <math.h>
#include <libciomr/libciomr.h>
#include "iwl.h"

extern "C" {
	
#define MAX0(a,b) (((a)>(b)) ? (a) : (b))
#define MIN0(a,b) (((a)<(b)) ? (a) : (b))
#define INDEX(i,j) ((i>j) ? (ioff[(i)]+(j)) : (ioff[(j)]+(i)))

/*!
** iwl_buf_wrt_mat()
**
** Write to an Integrals With Labels formatted buffer. The buffer 
** must have been initialized with iwl_buf_init().  Don't forget to 
** call iwl_buf_flush() when finished with all writes to the buffer 
** to ensure that all contents are written to disk.
**
** This version takes as input a matrix, as might be handy for a
** matrix formulation of an integral transformation.  It assumes that
** all rs are available for a given pq. r and s are allowed to range 
** from rfirst/sfirst to rlast/slast (with s<=r), and this maps to a 
** matrix addressing of 0 to (rlast-rfirst) and 0 to (slast-sfirst).
**
** This routine is also compatible with a reordering of the orbitals
** before output.  We assume that p and q are already reordered,
** but r and s are not (yet).  The reordered address of an orbital
** is computed according to rtr = reorder[r] - reorder_offset.
**
** I have further modified the routine to spit out integrals whose
** reordered indices are canonical ij >= kl.  The routine does not
** care whether ptr >= qtr, but it insists that the transformed pq
** and kl indices (called ij and kl in the routine) satisfy ij >= kl.
** David Sherrill, October 1995
**
** Revised 6/27/96 by CDS for new format
** \ingroup (IWL)
*/
void iwl_buf_wrt_mat(struct iwlbuf *Buf, int ptr, int qtr, 
     double **mat, int rfirst, int rlast, int sfirst, int slast,
     int *reorder, int reorder_offset, int printflag, int *ioff,
      FILE *outfile)
{
   int idx, r, s, R, S, rtr, str;
   int ij, kl;
   double value;
   Label *lblptr;
   Value *valptr;

   lblptr = Buf->labels;
   valptr = Buf->values;

   ij = INDEX(ptr,qtr);
   
   for (r=rfirst,R=0; r <= rlast; r++,R++) {
     rtr = reorder[r] - reorder_offset;

     for (s=sfirst,S=0; s <= slast && s <= r; s++,S++) {
       str = reorder[s] - reorder_offset;

       kl = INDEX(rtr,str);
       
       value = mat[R][S];

       if (ij >= kl && fabs(value) > Buf->cutoff) {
	 idx = 4 * Buf->idx;
	 lblptr[idx++] = (Label) MAX0(ptr,qtr);
	 lblptr[idx++] = (Label) MIN0(ptr,qtr);
	 lblptr[idx++] = (Label) MAX0(rtr,str);
	 lblptr[idx++] = (Label) MIN0(rtr,str);
	 valptr[Buf->idx] = (Value) value;
	 
	 Buf->idx++;
	 
	 if (Buf->idx == Buf->ints_per_buf) {
	   Buf->lastbuf = 0;
	   Buf->inbuf = Buf->idx;
	   iwl_buf_put(Buf);
	   Buf->idx = 0;
	 }
	 
	 if (printflag)
	   fprintf(outfile, ">%d %d %d %d [%d] [%d] = %20.10lf\n",
		   ptr, qtr, rtr, str, ij, kl, value);
	 
       } /* end if (fabs(value) > Buf->cutoff) ... */
     } /* end loop over s */
   } /* end loop over r */
   
}

/*!
** IWL_BUF_WRT_MAT2(): This routine is exactly like iwl_buf_wrt_mat()
** except that the requirement that ij >= kl has been lifted.  This is
** necessary for the UHF alpha-beta transformation of the two-electron
** integrals.
**
** TDC, 6/01
** \ingroup (IWL)
*/
void iwl_buf_wrt_mat2(struct iwlbuf *Buf, int ptr, int qtr, 
     double **mat, int rfirst, int rlast, int sfirst, int slast,
     int *reorder, int reorder_offset, int printflag, int *ioff,
      FILE *outfile)
{
   int idx, r, s, R, S, rtr, str;
   int ij, kl;
   double value;
   Label *lblptr;
   Value *valptr;

   lblptr = Buf->labels;
   valptr = Buf->values;

   ij = INDEX(ptr,qtr);
   
   for (r=rfirst,R=0; r <= rlast; r++,R++) {
     rtr = reorder[r] - reorder_offset;

     for (s=sfirst,S=0; s <= slast && s <= r; s++,S++) {
       str = reorder[s] - reorder_offset;

       kl = INDEX(rtr,str);
       
       value = mat[R][S];

       if (fabs(value) > Buf->cutoff) {
	 idx = 4 * Buf->idx;
	 lblptr[idx++] = (Label) MAX0(ptr,qtr);
	 lblptr[idx++] = (Label) MIN0(ptr,qtr);
	 lblptr[idx++] = (Label) MAX0(rtr,str);
	 lblptr[idx++] = (Label) MIN0(rtr,str);
	 valptr[Buf->idx] = (Value) value;
	 
	 Buf->idx++;
	 
	 if (Buf->idx == Buf->ints_per_buf) {
	   Buf->lastbuf = 0;
	   Buf->inbuf = Buf->idx;
	   iwl_buf_put(Buf);
	   Buf->idx = 0;
	 }
	 
	 if (printflag)
	   fprintf(outfile, ">%d %d %d %d [%d] [%d] = %20.10lf\n",
		   ptr, qtr, rtr, str, ij, kl, value);
	 
       } /* end if (fabs(value) > Buf->cutoff) ... */
     } /* end loop over s */
   } /* end loop over r */
   
}

} /* extern "C" */