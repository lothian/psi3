#include <stdio.h>
#include <stdlib.h>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>
#include <libiwl/iwl.h>
#define EXTERN
#include <psifiles.h>
#include "globals.h"

void transp(double sign)
{
  int stat, nao, nso, nmo, noei_ao;
  int i, j, ij;
  double *scratch, **TMP, **X;
  double **MUX, **MUY, **MUZ;  /* MO-basis momentum integrals */

  nao = moinfo.nao;
  nso = moinfo.nso;
  nmo = moinfo.nmo;
  noei_ao = moinfo.noei_ao;

  /**** Transform the momentum integrals to the MO basis ****/

  MUX = block_matrix(nmo,nmo);
  MUY = block_matrix(nmo,nmo);
  MUZ = block_matrix(nmo,nmo);

  TMP = block_matrix(nao, nao);
  X = block_matrix(nao, nao);
  scratch = init_array(noei_ao);

  stat = iwl_rdone(PSIF_OEI, PSIF_AO_NablaX, scratch, noei_ao, 0, 0, outfile);
  for(i=0,ij=0; i < nao; i++)
    for(j=0; j <= i; j++,ij++) {
      TMP[i][j] = -sign * scratch[ij];
      TMP[j][i] = +sign * scratch[ij];
    }

  C_DGEMM('n','t',nao,nso,nao,1,&(TMP[0][0]),nao,&(moinfo.usotao[0][0]),nao,
	  0,&(X[0][0]),nao);
  C_DGEMM('n','n',nso,nso,nao,1,&(moinfo.usotao[0][0]),nao,&(X[0][0]),nao,
	  0,&(TMP[0][0]),nao);

  C_DGEMM('n','n',nso,nmo,nso,1,&(TMP[0][0]),nao,&(moinfo.scf[0][0]),nmo,
	  0,&(X[0][0]),nao);
  C_DGEMM('t','n',nmo,nmo,nso,1,&(moinfo.scf[0][0]),nmo,&(X[0][0]),nao,
	  0,&(MUX[0][0]),nmo);

  zero_arr(scratch,noei_ao);

  stat = iwl_rdone(PSIF_OEI, PSIF_AO_NablaY, scratch, noei_ao, 0, 0, outfile);
  for(i=0,ij=0; i < nao; i++)
    for(j=0; j <= i; j++,ij++) {
      TMP[i][j] = -sign * scratch[ij];
      TMP[j][i] = +sign * scratch[ij];
    }

  C_DGEMM('n','t',nao,nso,nao,1,&(TMP[0][0]),nao,&(moinfo.usotao[0][0]),nao,
	  0,&(X[0][0]),nao);
  C_DGEMM('n','n',nso,nso,nao,1,&(moinfo.usotao[0][0]),nao,&(X[0][0]),nao,
	  0,&(TMP[0][0]),nao);

  C_DGEMM('n','n',nso,nmo,nso,1,&(TMP[0][0]),nao,&(moinfo.scf[0][0]),nmo,
	  0,&(X[0][0]),nao);
  C_DGEMM('t','n',nmo,nmo,nso,1,&(moinfo.scf[0][0]),nmo,&(X[0][0]),nao,
	  0,&(MUY[0][0]),nmo);

  zero_arr(scratch,noei_ao);

  stat = iwl_rdone(PSIF_OEI, PSIF_AO_NablaZ, scratch, noei_ao, 0, 0, outfile); 
  for(i=0,ij=0; i < nao; i++)
    for(j=0; j <= i; j++,ij++) {
      TMP[i][j] = -sign * scratch[ij];
      TMP[j][i] = +sign * scratch[ij];
    }

  C_DGEMM('n','t',nao,nso,nao,1,&(TMP[0][0]),nao,&(moinfo.usotao[0][0]),nao,
	  0,&(X[0][0]),nao);
  C_DGEMM('n','n',nso,nso,nao,1,&(moinfo.usotao[0][0]),nao,&(X[0][0]),nao,
	  0,&(TMP[0][0]),nao);

  C_DGEMM('n','n',nso,nmo,nso,1,&(TMP[0][0]),nao,&(moinfo.scf[0][0]),nmo,
	  0,&(X[0][0]),nao);
  C_DGEMM('t','n',nmo,nmo,nso,1,&(moinfo.scf[0][0]),nmo,&(X[0][0]),nao,
	  0,&(MUZ[0][0]),nmo);

  free_block(TMP);
  free_block(X);

  /*
  fprintf(outfile, "MO-Basis pX Integrals:\n");
  mat_print(MUX, nmo, nmo, outfile);
  fprintf(outfile, "MO-Basis pY Integrals:\n");
  mat_print(MUY, nmo, nmo, outfile);
  fprintf(outfile, "MO-Basis pZ Integrals:\n");
  mat_print(MUZ, nmo, nmo, outfile);
  */

  moinfo.MUX = MUX;
  moinfo.MUY = MUY;
  moinfo.MUZ = MUZ;
}