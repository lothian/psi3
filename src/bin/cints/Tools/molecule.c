#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<libciomr/libciomr.h>
#if USE_LIBCHKPT
#include<libchkpt/chkpt.h>
#else
#include<libfile30/file30.h>
#endif

#include<libint/libint.h>
#include"defines.h"
#define EXTERN
#include"global.h"

/*-------------------------------
  Explicit function declarations
 -------------------------------*/
static void get_geometry(void);
static void compute_enuc(void);

void init_molecule()
{
#if USE_LIBCHKPT
  Molecule.label = chkpt_rd_label();
  Molecule.num_atoms = chkpt_rd_natom();
  Molecule.Rref = chkpt_rd_rref();
#else
  Molecule.label = file30_rd_label();
  Molecule.num_atoms = file30_rd_natom();
  Molecule.Rref = file30_rd_rref();
#endif
/* Molecule.centers = */ get_geometry();

  return;
}


void cleanup_molecule()
{
  free(Molecule.centers);
  free(Molecule.Rref);
  
  return;
}

void get_geometry()
{
   int i;
   double *Z;  /* nuclear charges */
   double **g; /* cartesian geometry */
   
   Molecule.centers = (struct coordinates *)malloc(sizeof(struct coordinates)*Molecule.num_atoms);

   g = chkpt_rd_geom();
   Z = chkpt_rd_zvals();

   /*--- move it into the appropriate struct form ---*/
   for (i=0; i<Molecule.num_atoms; i++){
      Molecule.centers[i].x = g[i][0];
      Molecule.centers[i].y = g[i][1];
      Molecule.centers[i].z = g[i][2];
      Molecule.centers[i].Z_nuc = Z[i];
   }
   free_block(g);
   free(Z);

   return;
}


/*---------------------------------------
  enuc computes nuclear repulsion energy
 ---------------------------------------*/

void compute_enuc()
{  
  int i, j;
  double r = 0.0;
  double E = 0.0;

  if(Molecule.num_atoms > 1)
    for(i=1; i<Molecule.num_atoms; i++)
      for(j=0; j<i; j++){
	r = 0.0;
	r += (Molecule.centers[i].x-Molecule.centers[j].x)*
	     (Molecule.centers[i].x-Molecule.centers[j].x);
	r += (Molecule.centers[i].y-Molecule.centers[j].y)*
	     (Molecule.centers[i].y-Molecule.centers[j].y);
	r += (Molecule.centers[i].z-Molecule.centers[j].z)*
	     (Molecule.centers[i].z-Molecule.centers[j].z);
	r = 1.0/sqrt(r);
	E += (Molecule.centers[i].Z_nuc*Molecule.centers[j].Z_nuc)*r;
      }
  Molecule.Enuc = E;

  return;
}

