/*!
** \file print_array.c
** \ingroup (CIOMR)
*/

/* $Log$
 * Revision 1.4  2003/11/25 21:45:18  sherrill
 * Clarified comment
 *
/* Revision 1.3  2002/06/01 18:23:54  sherrill
/* Upgrade doxygen documentation
/*
/* Revision 1.2  2002/04/19 21:48:06  sherrill
/* Remove some unused functions and do doxygen markup of libciomr.
/*
/* Revision 1.1.1.1  2000/02/04 22:53:21  evaleev
/* Started PSI 3 repository
/*
/* Revision 2.1  1991/06/15 18:29:42  seidl
/* *** empty log message ***
/* */

static char *rcsid = "$Id$";

#include "includes.h"

/*!
** print_array: Prints a lower-triangle of a symmetric matrix packed as
**  an array of doubles.
**
** \ingroup (CIOMR)
*/
void print_array(double *a, int m, FILE *out)
   {
      int ii,jj,kk,mm,nn,ll;
      int i,j,k,i1,i2;

      ii=0;jj=0;
L200:
      ii++;
      jj++;
      kk=10*jj;
      nn = kk + kk*(kk-1)/2;
      mm=m;
      if (m > kk) mm=kk;
      ll = 2*(mm-ii+1)+1;
      fprintf (out,"\n");
      for (i=ii; i <= mm; i++) fprintf(out,"       %5d",i);
      fprintf (out,"\n");
      for (i=ii; i <= m; i++) {
         i1=i*(i-1)/2+ii;
         i2=i+i*(i-1)/2;
         if (i2 > nn) i2 = i1+9;
         fprintf (out,"\n%5d",i);
         for (j=i1; j <= i2; j++) {
            fprintf (out,"%12.7f",a[j-1]);
            }
         }
      if (m <= kk) {
         fprintf(out,"\n");
         fflush(out);
         return;
         }
      ii=kk; goto L200;
      }
