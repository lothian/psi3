#include <stdio.h>
#include <stdlib.h>
#include <math.h>
extern "C" {
#include <libciomr/libciomr.h>
#include <libchkpt/chkpt.h>
#include <libqt/qt.h>
#include <psifiles.h>
}
#include "moinfo.h"
#include "mo_overlap.h"
#include "float.h"
#include "linalg.h"

extern MOInfo_t MOInfo;
extern FILE *outfile;

extern void done(const char *);

//
// High-spin ROHF case only
//

double eval_rohf_derwfn_overlap()
{
  int nalpha = MOInfo.nalpha;
  int nbeta = MOInfo.nbeta;
  int ndocc = nbeta;
  FLOAT **CSC = eval_S_alpha();

  chkpt_init(PSIO_OPEN_OLD);
  int* clsdpi = chkpt_rd_clsdpi();
  int* openpi = chkpt_rd_openpi();
  int* orbspi = chkpt_rd_orbspi();
  int nirreps = chkpt_rd_nirreps();
  chkpt_close();

  // Extract the alpha and beta blocks
  FLOAT **CSC_alpha = create_matrix(nalpha,nalpha);
  FLOAT **CSC_beta = create_matrix(nbeta,nbeta);
  int mo_offset1 = 0;
  int docc_offset1 = 0;
  int socc_offset1 = ndocc;
  for(int irrep1=0; irrep1<nirreps; irrep1++) {

    int ndocc1 = clsdpi[irrep1];
    int nsocc1 = openpi[irrep1];

    int mo_offset2 = 0;
    int docc_offset2 = 0;
    int socc_offset2 = ndocc;
    for(int irrep2=0; irrep2<nirreps; irrep2++) {

      int ndocc2 = clsdpi[irrep2];
      int nsocc2 = openpi[irrep2];

      for(int i=0;i<ndocc1;i++)
	for(int j=0;j<ndocc2;j++) {
	  CSC_alpha[i+docc_offset1][j+docc_offset2] = CSC[i+mo_offset1][j+mo_offset2];
	  CSC_beta[i+docc_offset1][j+docc_offset2] = CSC[i+mo_offset1][j+mo_offset2];
	}

      for(int i=0;i<ndocc1;i++)
	for(int j=0;j<nsocc2;j++) {
	  CSC_alpha[i+docc_offset1][j+socc_offset2] = CSC[i+mo_offset1][j+ndocc2+mo_offset2];
	}

      for(int i=0;i<nsocc1;i++)
	for(int j=0;j<ndocc2;j++) {
	  CSC_alpha[i+socc_offset1][j+docc_offset2] = CSC[i+mo_offset1+ndocc1][j+mo_offset2];
	}

      for(int i=0;i<nsocc1;i++)
	for(int j=0;j<nsocc2;j++) {
	  CSC_alpha[i+socc_offset1][j+socc_offset2] = CSC[i+mo_offset1+ndocc1][j+mo_offset2+ndocc2];
	}

      docc_offset2 += ndocc2;
      socc_offset2 += nsocc2;
      mo_offset2 += orbspi[irrep2];
    }

    docc_offset1 += ndocc1;
    socc_offset1 += nsocc1;
    mo_offset1 += orbspi[irrep1];
  }
  delete[] clsdpi;
  delete[] openpi;
  delete[] orbspi;
  delete_matrix(CSC);

  // Compute the overlap of alpha part
  int *tmpintvec = new int[nalpha];
  FLOAT sign;
  lu_decom(CSC_alpha, nalpha, tmpintvec, &sign);
  delete[] tmpintvec;
  FLOAT deter_a = 1.0;
  for(int i=0;i<nalpha;i++)
    deter_a *= CSC_alpha[i][i];
  deter_a = FABS(deter_a);
  delete_matrix(CSC_alpha);

  // Compute the overlap of beta part
  tmpintvec = new int[nbeta];
  lu_decom(CSC_beta, nbeta, tmpintvec, &sign);
  delete[] tmpintvec;
  FLOAT deter_b = 1.0;
  for(int i=0;i<nbeta;i++)
    deter_b *= CSC_beta[i][i];
  deter_b = FABS(deter_b);
  delete_matrix(CSC_beta);

  return (double)deter_a*deter_b;
}

