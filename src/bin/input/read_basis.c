#define EXTERN
#include <stdio.h>
#include <ip_libv1.h>
#include <libciomr.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <file30_params.h>
#include "input.h"
#include <physconst.h>
#include "global.h"
#include "defines.h"

/*
   Main routine to read the basis set info.  It calls recur, and all of the
   normalization routines. */

void read_basis()
{
   int i = 0;
   int j = 0;
   int k = 0;
   int l = 0;
   int m = 0;
   int flag = 0;			/*flag for hierarchy*/
   int input_flag = 0;			/*Should tell me if I'm using input.dat
                                          or not*/
   int cnt1 = 0;
   int kstart, kfinish;
   int global_shell_index;
   int unique_shell_index;
   int canon_ord_shell, am_ord_shell;   /*shell indices in 2 canonical and angmom orders */
   int ao_index;
   int basisfn_index;
   int *first_prim_unique_shell;	/*"points" to the first primitive in each unique shell*/
   int *last_prim_unique_shell;         /*"points" to the last primitive in each unique shell*/
   int first_prim_unique_atom;
   int first_shell_unique_atom;
   int last_shell_unique_atom = 0;			/*total number of contractions*/
   int last_prim_unique_atom = 0;         /*total number of primitives*/
   int errcod = 0;
   int num_levels=0;			/*levels under each keyword - 
                                          used in recur()
                                        */
   int num_exponents = 0;		/*Number of primitive exponents*/
   int num_coefficients = 0;		/*Number of primitive coefficients*/
   char **ip_token1;			/*Contains base basis set id's*/
   char **ip_token2;			/*used to create other keywords*/
   int depth;
   char *basis_type, *elem_label;
   double Z;
   int *ao_off;
   int *ang_mom;
   int *num_shells_per_unique;
   double **basis_set;                  /*Contains basis set for unique atoms*/
   
   
   /*-----------------------------------------
     Initializing global variables and arrays
    -----------------------------------------*/

     max_angmom = 0;
     num_shells = 0;
     num_ao = 0;
     num_so = 0;
     num_prims = 0;
     first_shell_on_atom = init_int_array(num_atoms);
     nshells_per_atom = init_int_array(num_atoms);
     num_unique_shells = 0;
     

   /*-----------------------
     Parsing basis set data
    -----------------------*/
   
     depth = 0;
     errcod = ip_count("BASIS",&depth,0);
     if (depth == 0) { /* the same basis for all atoms */
       errcod = ip_string("BASIS",&basis_type,0);
       if (errcod != IPE_OK)
	 punt("There is a problem with the BASIS keyword!");
       for(i=0;i<num_atoms;i++)
	 atom_basis[i] = basis_type;
     }
     else {
       errcod = ip_count("BASIS",&j,1,0);
       if (errcod == IPE_NOT_AN_ARRAY)
         /*----------
	   Basis sets for each atom is specified, e.g.
	   basis = (dzp dzp dz dz)
	  ----------*/
         if (depth == num_atoms)
	   for(i=0;i<num_atoms;i++) {
	     errcod = ip_string("BASIS",&basis_type,1,i);
	     if (errcod != IPE_OK)
	       punt("There is a problem with the BASIS array!");
	     atom_basis[i] = basis_type;
	   }
	 else
	   punt("Number of entries in the BASIS array not the same as num_atoms!");
       else {
	   /*----------
	     Basis sets for each element type is specified, e.g.
	     basis = (
	       (h dz)
	       (c dzp)
	     )
	    ----------*/
	 for(i=0;i<depth;i++) {
	   errcod = ip_count("BASIS",&j,1,i);
	   if (errcod != IPE_OK || j != 2) {
	     fprintf(outfile,"  There is a problem with line %d of the BASIS array!\n",i+1);
	     punt("Invalid basis set file");;
	   }
	   errcod = ip_string("BASIS",&elem_label,2,i,0);
	   atom_num(elem_label,&Z);
	   free(elem_label);
	   errcod = ip_string("BASIS",&basis_type,2,i,1);
	   for(k=0;k<num_atoms;k++)
	     if (!strcmp(elem_name[(int)Z],element[k]))
	        atom_basis[k] = basis_type;
	 }
	 for(i=0;i<num_atoms;i++)
	   if (atom_basis[i] == NULL) {
	     fprintf(outfile,"  Missing basis set for %s\n",element[i]);
	     punt("Missing basis set");;
	   }
       }
     }
   
   ip_token1 = init_char_matrix(MAXATOM,50);
   ip_token2 = init_char_matrix(MAXATOM,50);   

   /*Create basis "keyword"*/
   for(i=0;i<num_uniques;i++){
      sprintf(ip_token1[i],":BASIS:%s",elem_name[(int)nuclear_charges[u2a[i]]]);
      sprintf(ip_token2[i],"%s:%s", ip_token1[i],atom_basis[u2a[i]]);
     }

   /*holds basis set for just the symmetry unique atoms*/
   basis_set = init_matrix(MAXCONTRACTION,MAXBASISCOLUMNS);
   last_shell_unique_atom = 0;
   last_prim_unique_atom = 0;
   first_shell_unique_atom = 1; 
   first_prim_unique_shell = init_int_array(MAXCONTRACTION);
   last_prim_unique_shell = init_int_array(MAXCONTRACTION);

   global_shell_index = 0;
   ang_mom = init_int_array(MAXCONTRACTION);
   num_shells_per_unique = init_int_array(num_uniques);

   fprintf(outfile,"\n  -BASIS SETS:\n");
   /*Read and Print out basis set from input deck*/
   for(i=0;i<num_uniques;i++){

      /*Count number of levels,*/
      errcod = ip_exist(ip_token2[i], 0);
      if(errcod == 0){
        punt("I can't find your basis set anywhere, check BASIS array and/or basis set.");
       }

      first_prim_unique_atom = last_prim_unique_atom+1;
      first_shell_unique_atom = last_shell_unique_atom+1;
      errcod = ip_count(ip_token2[i], &num_levels, 0);
      fprintf(outfile,"\n   -Basis set on unique center %d:\n",i+1);
      fprintf(outfile,"      ("); 

      /*Call the function to read the basis set information*/
      recur(ip_token1, ip_token2, num_levels, i, basis_set,
           &last_shell_unique_atom, &last_prim_unique_atom, num_exponents, first_prim_unique_shell,
	    last_prim_unique_shell, ang_mom);
      fprintf(outfile,")\n"); 

      for(k=first_shell_unique_atom; k<=last_shell_unique_atom; k++)
	max_angmom = (max_angmom > ang_mom[k]) ? max_angmom : ang_mom[k];
      
      num_shells_per_unique[i] = last_shell_unique_atom - first_shell_unique_atom + 1;
      num_unique_shells += num_shells_per_unique[i];
      for(j=0; j<unique_degen[i]; j++) {
	nshells_per_atom[red_unique_orbit[i][j]] = num_shells_per_unique[i];
        first_shell_on_atom[red_unique_orbit[i][j]] = global_shell_index;
	global_shell_index += num_shells_per_unique[i];
      }
      first_shell_unique_atom = last_shell_unique_atom;
   }
   fprintf(outfile,"\n\n");

   /*------------------------------------------------
     If only s- and p-functions are present - puream
     is effectively off anyway. Turn it off because
     CINTS and others to determine puream just check
     num_ao and num_so
    ------------------------------------------------*/
   if (max_angmom < 2)
     puream = 0;
   
   if (max_angmom+1 > MAXANGMOM) {
     fprintf(outfile,"  Angular momentum limit of %d exceeded\n",MAXANGMOM-1);
     punt("Angular momentum too high");
   }

   num_prims = last_prim_unique_atom;

   /*Count number of shells*/
   cnt1=0;
   for(i=0;i<num_uniques;i++){
      num_shells += (num_shells_per_unique[i])*unique_degen[i];
      for(j=0;j<num_shells_per_unique[i];j++) {
	  num_ao += unique_degen[i] * ioff[ang_mom[cnt1]+1];
          num_so += unique_degen[i] *((puream) ? 2*ang_mom[cnt1]+1 : ioff[ang_mom[cnt1]+1]);
          cnt1++;
      } 
   }

   /*Initialize and compute a number of global basis info arrays*/
   shell_nucleus = init_int_array(num_shells);
   shell_ang_mom = init_int_array(num_shells);
   nprim_in_shell = init_int_array(num_shells);
   first_prim_shell = init_int_array(num_shells);
   first_ao_shell = init_int_array(num_shells);
   first_basisfn_shell = init_int_array(num_shells);
   first_ao_type_shell = init_int_array(num_shells);
   last_ao_type_shell = init_int_array(num_shells);
   global_shell_index = 0;
   unique_shell_index = 0;
   ao_index = 0;
   basisfn_index = 0;
   shells_per_am = init_int_array(max_angmom+1);
   ao_off = init_int_array(max_angmom+1);
   ao_off[0] = 0;
   for(i=1;i<=max_angmom;i++)
     ao_off[i] = ao_off[i-1] + ioff[i];
   for(i=0;i<num_uniques;i++) {
     for(j=0; j<unique_degen[i]; j++){
       kstart = unique_shell_index;
       kfinish = kstart + nshells_per_atom[u2a[i]];
       for(k=kstart;k<kfinish; k++){
         shell_nucleus[global_shell_index] = red_unique_orbit[i][j];
	 l = ang_mom[k];
	 shell_ang_mom[global_shell_index] = l;
	 shells_per_am[l] += 1;
	 nprim_in_shell[global_shell_index] = last_prim_unique_shell[k] - first_prim_unique_shell[k] + 1;
	 first_prim_shell[global_shell_index] = first_prim_unique_shell[k];
	 first_ao_shell[global_shell_index] = ao_index;
	 first_basisfn_shell[global_shell_index] = basisfn_index;
	 first_ao_type_shell[global_shell_index] = ao_off[ang_mom[k]];
	 last_ao_type_shell[global_shell_index] = ao_off[ang_mom[k]] + ioff[ang_mom[k]+1] - 1;
	 ao_index += ioff[ang_mom[k]+1];
	 basisfn_index += (puream) ? 2*ang_mom[k] + 1 : ioff[ang_mom[k]+1];
	 global_shell_index++;
       }
     }
     unique_shell_index += nshells_per_atom[u2a[i]];
   }

   exponents = init_array(num_prims);
   contr_coeff = init_array(num_prims);
   for(i=0;i<num_prims;i++) {
     exponents[i] = basis_set[i][0];
     contr_coeff[i] = basis_set[i][1];
   }

   /*---------------------------------------------------------
     Sort shells according to their angular momentum and form
      the corresponding mapping array
    ---------------------------------------------------------*/
   am2canon_shell_order = init_int_array(num_shells);
   am_ord_shell = 0;
   for(l=0;l<=max_angmom;l++)
     for(canon_ord_shell=0;canon_ord_shell<num_shells;canon_ord_shell++)
       if (shell_ang_mom[canon_ord_shell] == l) {
	 am2canon_shell_order[am_ord_shell] = canon_ord_shell;
	 am_ord_shell++;
       }
   /* this shouldn't happen, but */
   if (canon_ord_shell != num_shells)
     punt("Miscounted shells when reordering according to angmom");
   
   free(ang_mom);
   free(num_shells_per_unique);
   free(ip_token1);
   free(ip_token2);
   free_matrix(basis_set,MAXCONTRACTION);

}

int parse_am(s)
  char *s;
{
  if(!strcmp(s,"S")) return 0;
  else if(!strcmp(s,"P")) return 1;
  else if(!strcmp(s,"D")) return 2;
  else if(!strcmp(s,"F")) return 3;
  else if(!strcmp(s,"G")) return 4;
  else if(!strcmp(s,"H")) return 5;
  else if(!strcmp(s,"I")) return 6;
  else if(!strcmp(s,"K")) return 7;
  else if(!strcmp(s,"L")) return 8;
  else if(!strcmp(s,"M")) return 9;
  else if(!strcmp(s,"N")) return 10;
  else {
    fprintf(outfile,"  Angular momentum %s cannot be handled yet.\n",s);
    punt("Angular momentum too high");
  }
}


/*
   This function calls itself recursively while reading the basis set info. in
   pbasis.dat (or in input deck) until it has gotten past all of the "GET" levels
*/
void recur(char **ip_token1, char **ip_token2, int num_levels, 
int atom_number, double **basis_set, 
int *unique_shell_cnt, int *unique_prim_cnt, int num_exponents, int *first_prim, int *last_prim,
int *ang_mom)
{
   int i = 0;
   int j = 0;
   int k = 0;
   int flag = 0;
   int errcod = 0;
   int cnt = 0;
   int num_levels2 = 0;
   char **temp;
   char *grep;
   char *next_key;
 
   next_key = init_char_array(50);
   temp = init_char_matrix(MAXATOM,50);
 

   for(i=0;i<num_levels;i++){
      errcod = ip_string(ip_token2[atom_number], &grep,2,i,0);
      if(!strcmp(grep,"GET")){

        /*Create new keyword from base and string after "GET"*/
        errcod = ip_string(ip_token2[atom_number], &next_key,2,i,1);
        sprintf(temp[atom_number],"%s:%s",ip_token1[atom_number],next_key);
        errcod = ip_count(temp[atom_number], &num_levels2,0);
        flag = 1;

        /*Call function again to get down through all "GET" levels*/
        recur(ip_token1, temp, num_levels2, atom_number, basis_set,
        unique_shell_cnt, unique_prim_cnt, num_exponents, first_prim, last_prim, ang_mom);
       }

      /*if the flag == 1, that means that we have descended a GET level,
	so now we have to skip to the next level*/
      if(flag==1){
        flag = 0;
        continue;
       }

      errcod = ip_count(ip_token2[atom_number],&num_exponents,1,i); 
      if(errcod != IPE_OK){
        punt("I still can't find the basis set, something is wrong. Check BASIS array.");
       }
      ang_mom[*unique_shell_cnt] = parse_am(grep);
      first_prim[*unique_shell_cnt] = *unique_prim_cnt;

/*
      Start of a primitive set eg:
        (s ( 6665.00000000      0.00069200 )
           ( 1000.00000000      0.00532900 )
           (  228.00000000      0.02707700 )
           (   64.71000000      0.10171800 )
           (   21.06000000      0.27474000 )
           (    7.49500000      0.44856400 )
           (    2.79700000      0.28507400 )
           (    0.52150000      0.01520400 ))
*/
      /*Start print and read scheme for formatted basis set output*/
      fprintf(outfile," (%s",grep);

      /*exponent loop*/
      for(j=0;j<(num_exponents-1);j++){
         fprintf(outfile," (");

         /*alternate between exponent and coefficient*/
         for(k=0;k<2;k++){

            errcod = ip_data(ip_token2[atom_number],"%lf",
                             &basis_set[*unique_prim_cnt][k],3,i,j+1,k);

            fprintf(outfile,"%15.8lf",basis_set[*unique_prim_cnt][k]);
         
            if(basis_set[*unique_prim_cnt][k] == 0.0){
              punt("Zeros in the basis set!");
             } 
           }
         if(j != (num_exponents-1) - 1){
           fprintf(outfile,")\n          ");
          }
         else if(j == (num_exponents-1) - 1){
           fprintf(outfile,") )\n       ");
          }
         (*unique_prim_cnt)++;
         fflush(outfile);
        }
      last_prim[*unique_shell_cnt] = *unique_prim_cnt - 1;

      /* End of contracted function loop.  Normalize here. */
      normalize(basis_set, first_prim[*unique_shell_cnt], last_prim[*unique_shell_cnt], ang_mom[*unique_shell_cnt]);
      (*unique_shell_cnt)++;

     }

   free(temp);
   free(next_key);

}    


void normalize(bs, fp, lp, am)
  double **bs;
  int fp;
  int lp;
  int am;
{
  int i, j;
  double norm;
  double e_sum = 0.0;
  double tmp1, tmp2, tmp3, tmp4;
  double t, g, z;
  double rPI3 = _pi*sqrt(_pi);

/* normalize each primitive function */
  tmp1 = am+1.5;
  for(i=fp; i<=lp; i++){
    g = 2.0*bs[i][0];
    z = pow(g,tmp1);
    norm = sqrt(int_pow(2.0,am)*z/(rPI3*df[2*am]));
    bs[i][1] = bs[i][1]*norm;
  }
/* normalize the contracted function */
  for(i=fp; i<=lp; i++)
    for(j=fp; j<=lp; j++){
      g = bs[i][0]+bs[j][0];
      z = pow(g,tmp1);
      e_sum += bs[i][1]*bs[j][1]/z;
    }

/* pi^(3/2) */
  tmp1 = ((2.0*_pi/M_2_SQRTPI)*df[2*am])/(int_pow(2.0,am));
  norm = sqrt(1.0/(tmp1*e_sum));
  
  if(shownorm){
    if(fabs(norm-1.0)>ZERO) printf("Needed normalization: %lf\n", norm);
   }
  for(i=fp; i<=lp; i++){
     bs[i][1] = bs[i][1]*norm;
     if(shownorm){
       printf("%15.9lf\t%15.9lf\n", bs[i][0], bs[i][1]);
     }
   }

/* end normalization of contracted function */

/* check normalization */
  tmp1 = am+1.5;
  e_sum = 0.0;
  for(i=fp; i<lp; i++){
    for(j=fp; j<lp; j++){
      tmp2 = bs[i][0]+bs[j][0];
      tmp3 = pow(tmp2,tmp1);
      tmp4 = bs[i][1]*bs[j][1];
      e_sum += tmp4/tmp3;
      }
    }

/* pi^(3/2) */
  tmp1 = 2.0*_pi/M_2_SQRTPI;
  tmp2 = df[2*am];
  tmp3 = int_pow(2.0,am);
  norm = e_sum*tmp1*tmp2/tmp3;
  if(shownorm){
    printf("Normalized to: %lf\n", norm);
   }

}




double ovlp(a1, a2, am)
 double a1, a2;
 int am;
{
  double t, z;
  double g;

  g = a1 + a2;
  t = _pi/g;

  return t*sqrt(t)*df[2*am]/int_pow(g,am);
}

