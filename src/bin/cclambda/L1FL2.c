#include <stdio.h>
#include <dpd.h>
#define EXTERN
#include "globals.h"

/* RHS += P(ij)P(ab)Lia*Fjb */

void L1FL2(void)
{
  int h, nirreps;
  int row,col;
  int i,j,a,b,I,J,A,B,Isym,Jsym,Asym,Bsym;
  dpdfile2 LIA, Lia, FJB, Fjb;
  dpdbuf4 newL2;

  nirreps = moinfo.nirreps;

  dpd_file2_init(&LIA, CC_OEI, 0, 0, 1, "LIA");
  dpd_file2_mat_init(&LIA);
  dpd_file2_mat_rd(&LIA);
  dpd_file2_init(&Lia, CC_OEI, 0, 0, 1, "Lia");
  dpd_file2_mat_init(&Lia);
  dpd_file2_mat_rd(&Lia);
  dpd_file2_init(&FJB, CC_OEI, 0, 0, 1, "FME");
  dpd_file2_mat_init(&FJB);
  dpd_file2_mat_rd(&FJB);
  dpd_file2_init(&Fjb, CC_OEI, 0, 0, 1, "Fme");
  dpd_file2_mat_init(&Fjb);
  dpd_file2_mat_rd(&Fjb);
  
  dpd_buf4_init(&newL2, CC_LAMPS, 0, 2, 7, 2, 7, 0, "New LIJAB");
  for(h=0; h < nirreps; h++) {

  dpd_buf4_mat_irrep_init(&newL2, h); 0,
      dpd_buf4_mat_irrep_rd(&newL2, h);

      for(row=0; row < newL2.params->rowtot[h]; row++) {
	  i = newL2.params->roworb[h][row][0];
	  j = newL2.params->roworb[h][row][1];
	  
	  for(col=0; col < newL2.params->coltot[h]; col++) {
	      a = newL2.params->colorb[h][col][0];
	      b = newL2.params->colorb[h][col][1];

   	      I = LIA.params->rowidx[i]; Isym = LIA.params->psym[i];
	      J = FJB.params->rowidx[j]; Jsym = FJB.params->psym[j];
	      A = LIA.params->colidx[a]; Asym = LIA.params->qsym[a];
	      B = FJB.params->colidx[b]; Bsym = FJB.params->qsym[b];

	      if((Isym == Asym) && (Jsym == Bsym))
		  newL2.matrix[h][row][col] += (LIA.matrix[Isym][I][A] *
						FJB.matrix[Jsym][J][B]);

   	      J = LIA.params->rowidx[j]; Jsym = LIA.params->psym[j];
	      I = FJB.params->rowidx[i]; Isym = FJB.params->psym[i];

	      if((Isym == Asym) && (Jsym == Bsym))
		  newL2.matrix[h][row][col] += (LIA.matrix[Jsym][J][B] *
						FJB.matrix[Isym][I][A]);

   	      I = LIA.params->rowidx[i]; Isym = LIA.params->psym[i];
	      J = FJB.params->rowidx[j]; Jsym = FJB.params->psym[j];
	      B = LIA.params->colidx[b]; Bsym = LIA.params->qsym[b];
	      A = FJB.params->colidx[a]; Asym = FJB.params->qsym[a];

	      if((Jsym == Asym) && (Isym == Bsym))
		  newL2.matrix[h][row][col] -= (LIA.matrix[Jsym][J][A] *
						FJB.matrix[Isym][I][B]);

   	      J = LIA.params->rowidx[j]; Jsym = LIA.params->psym[j];
	      I = FJB.params->rowidx[i]; Isym = FJB.params->psym[i];

	      if((Jsym == Asym) && (Isym == Bsym))
		  newL2.matrix[h][row][col] -= (LIA.matrix[Isym][I][B] *
						FJB.matrix[Jsym][J][A]);
	    }
	}

      dpd_buf4_mat_irrep_wrt(&newL2, h);
      dpd_buf4_mat_irrep_close(&newL2, h);
      
    }
  dpd_buf4_close(&newL2);

  dpd_buf4_init(&newL2, CC_LAMPS, 0, 2, 7, 2, 7, 0, "New Lijab");
  for(h=0; h < nirreps; h++) {

  dpd_buf4_mat_irrep_init(&newL2, h); 0,
      dpd_buf4_mat_irrep_rd(&newL2, h);

      for(row=0; row < newL2.params->rowtot[h]; row++) {
	  i = newL2.params->roworb[h][row][0];
   	  j = newL2.params->roworb[h][row][1];
	  
	  for(col=0; col < newL2.params->coltot[h]; col++) {
	      a = newL2.params->colorb[h][col][0];
	      b = newL2.params->colorb[h][col][1];

              I = Lia.params->rowidx[i]; Isym = Lia.params->psym[i];
              J = Fjb.params->rowidx[j]; Jsym = Fjb.params->psym[j];
              A = Lia.params->colidx[a]; Asym = Lia.params->qsym[a];
              B = Fjb.params->colidx[b]; Bsym = Fjb.params->qsym[b];

              if((Isym == Asym) && (Jsym == Bsym))
                  newL2.matrix[h][row][col] += (Lia.matrix[Isym][I][A] *
                                                Fjb.matrix[Jsym][J][B]);

              J = Lia.params->rowidx[j]; Jsym = Lia.params->psym[j];
              I = Fjb.params->rowidx[i]; Isym = Fjb.params->psym[i];

              if((Isym == Asym) && (Jsym == Bsym))
                  newL2.matrix[h][row][col] += (Lia.matrix[Jsym][J][B] *
                                                Fjb.matrix[Isym][I][A]);

              I = Lia.params->rowidx[i]; Isym = Lia.params->psym[i];
              J = Fjb.params->rowidx[j]; Jsym = Fjb.params->psym[j];
              B = Lia.params->colidx[b]; Bsym = Lia.params->qsym[b];
              A = Fjb.params->colidx[a]; Asym = Fjb.params->qsym[a];

              if((Jsym == Asym) && (Isym == Bsym))
                  newL2.matrix[h][row][col] -= (Lia.matrix[Jsym][J][A] *
                                                Fjb.matrix[Isym][I][B]);

              J = Lia.params->rowidx[j]; Jsym = Lia.params->psym[j];
              I = Fjb.params->rowidx[i]; Isym = Fjb.params->psym[i];

              if((Jsym == Asym) && (Isym == Bsym))
                  newL2.matrix[h][row][col] -= (Lia.matrix[Isym][I][B] *
                                                Fjb.matrix[Jsym][J][A]);
	    }
	}

      dpd_buf4_mat_irrep_wrt(&newL2, h);
      dpd_buf4_mat_irrep_close(&newL2, h);
      
    }
  dpd_buf4_close(&newL2);

  dpd_buf4_init(&newL2, CC_LAMPS, 0, 0, 5, 0, 5, 0, "New LIjAb");
  for(h=0; h < nirreps; h++) {

  dpd_buf4_mat_irrep_init(&newL2, h); 0,
      dpd_buf4_mat_irrep_rd(&newL2, h);

      for(row=0; row < newL2.params->rowtot[h]; row++) {
	  i = newL2.params->roworb[h][row][0];
	  j = newL2.params->roworb[h][row][1];
	  
	  for(col=0; col < newL2.params->coltot[h]; col++) {
	      a = newL2.params->colorb[h][col][0];
	      b = newL2.params->colorb[h][col][1];

	      I = LIA.params->rowidx[i]; Isym = LIA.params->psym[i];
	      J = Fjb.params->rowidx[j]; Jsym = Fjb.params->psym[j];
	      A = LIA.params->colidx[a]; Asym = LIA.params->qsym[a];
	      B = Fjb.params->colidx[b]; Bsym = Fjb.params->qsym[b];

	      if((Isym == Asym) && (Jsym == Bsym))
		  newL2.matrix[h][row][col] += (LIA.matrix[Isym][I][A] *
						Fjb.matrix[Jsym][J][B]);

	      J = Lia.params->rowidx[j]; Jsym = Lia.params->psym[j];
	      I = FJB.params->rowidx[i]; Isym = FJB.params->psym[i];
	      B = Lia.params->colidx[b]; Bsym = Lia.params->qsym[b];
	      A = FJB.params->colidx[a]; Asym = FJB.params->qsym[a];

	      if((Isym == Asym) && (Jsym == Bsym))
		  newL2.matrix[h][row][col] += (Lia.matrix[Jsym][J][B] *
						FJB.matrix[Isym][I][A]);
	    }
	}

      dpd_buf4_mat_irrep_wrt(&newL2, h);
      dpd_buf4_mat_irrep_close(&newL2, h);
      
    }
  dpd_buf4_close(&newL2);

  dpd_file2_mat_close(&FJB);
  dpd_file2_close(&FJB);
  dpd_file2_mat_close(&Fjb);
  dpd_file2_close(&Fjb);
  dpd_file2_mat_close(&LIA);
  dpd_file2_close(&LIA);
  dpd_file2_mat_close(&Lia);
  dpd_file2_close(&Lia);

}
