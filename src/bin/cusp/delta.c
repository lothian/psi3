/* 
** delta(): Compute the MO-basis delta-function for a given point.
** TDC, June 2001
*/

#include <stdio.h>
#include <stdlib.h>
#include <ip_libv1.h>
#include <libciomr.h>
#include <file30.h>
#include <qt.h>

void compute_phi(double *phi, double x, double y, double z);
void setup_delta(void);

int nmo, nao;
double **scf, **u;

void compute_delta(double **delta, double x, double y, double z)
{
  int i, j;
  double *phi_ao, *phi_so, *phi_mo;

  setup_delta();

  phi_ao = init_array(nao);  /* AO function values */
  phi_so = init_array(nmo);  /* SO function values */
  phi_mo = init_array(nmo);  /* MO function values */

  compute_phi(phi_ao, x, y, z);

  /*  for(i=0; i < nao; i++) printf("%d %20.10f\n", i, phi_ao[i]); */

  /* Transform the basis function values to the MO basis */
  C_DGEMV('n', nmo, nao, 1.0, &(u[0][0]), nao, &(phi_ao[0]), 1,
	  0.0, &(phi_so[0]), 1);

  C_DGEMV('t', nmo, nmo, 1.0, &(scf[0][0]), nmo, &(phi_so[0]), 1,
	  0.0, &(phi_mo[0]), 1);

  /* for(i=0; i < nmo; i++) printf("%d %20.10f\n", i, phi_mo[i]); */


  /* Build the MO-basis delta function */
  for(i=0; i < nmo; i++)
    for(j=0; j < nmo; j++)
      delta[i][j] = phi_mo[i] * phi_mo[j];

  free(phi_ao);
  free(phi_so);
  free(phi_mo);
}

void setup_delta(void)
{
  static int done=0;
  int i, I, j, errcod;
  int nirreps, nfzc, nfzv;
  int *order, *clsdpi, *openpi, *orbspi, *fruocc, *frdocc;
  double **scf_pitzer;

  if(done) return;

  file30_init();
  nmo = file30_rd_nmo();
  nao = file30_rd_nao();
  nirreps = file30_rd_nirreps();
  clsdpi = file30_rd_clsdpi();
  openpi = file30_rd_openpi();
  orbspi = file30_rd_orbspi();
  scf_pitzer = file30_rd_scf();
  u = file30_rd_usotao_new();
  file30_close();

  frdocc = init_int_array(nirreps);
  fruocc = init_int_array(nirreps);
  errcod = ip_int_array("FROZEN_DOCC", frdocc, nirreps);
  errcod = ip_int_array("FROZEN_UOCC", fruocc, nirreps);

  nfzc = nfzv = 0;
  for(i=0; i < nirreps; i++) { 
    nfzc += frdocc[i];
    nfzv += fruocc[i];
  }

  /*
  if(nfzc || nfzv) {
    printf("Frozen orbitals not yet coded!\n");
    exit(2);
  }
  */

  /*** Get the Pitzer -> QT reordering array ***/
  order = init_int_array(nmo);
  reorder_qt(clsdpi, openpi, frdocc, fruocc, order, orbspi, nirreps);

  /*** Arrange the SCF eigenvectors into QT ordering ***/
  scf = block_matrix(nmo, nmo);
  for(i=0; i < nmo; i++) {
      I = order[i];  /* Pitzer --> QT */
      for(j=0; j < nmo; j++) scf[j][I] = scf_pitzer[j][i];
    }

  free(order);
  free(clsdpi);
  free(openpi);
  free(orbspi);
  free(fruocc);
  free(frdocc);
  free_block(scf_pitzer);

  done = 1;

  return;
}



