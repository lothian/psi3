#include <stdio.h>
#include <libqt/qt.h>
#include "dpd.h"

int dpd_buf4_mat_irrep_rd_block(dpdbuf4 *Buf, int irrep, int start_pq,
				int num_pq)
{
  int method, filerow, all_buf_irrep;
  int pq, rs;  /* dpdbuf row and column indices */
  int p, q, r, s;  /* orbital indices */
  int filepq, filers, filesr;  /* Input dpdfile row and column indices */
  int rowtot, coltot;  /* dpdbuf row and column dimensions */
  int b_perm_pq, b_perm_rs, b_peq, b_res;
  int f_perm_pq, f_perm_rs, f_peq, f_res;
  int pq_permute, permute;
  double value; 

#ifdef DPD_TIMER
  timer_on("buf4_rd_bk");
#endif

  all_buf_irrep = Buf->file.my_irrep;
  rowtot = Buf->params->rowtot[irrep];
  coltot = Buf->params->coltot[irrep^all_buf_irrep];

  b_perm_pq = Buf->params->perm_pq; b_perm_rs = Buf->params->perm_rs;
  f_perm_pq = Buf->file.params->perm_pq; f_perm_rs = Buf->file.params->perm_rs;
  b_peq = Buf->params->peq; b_res = Buf->params->res;
  f_peq = Buf->file.params->peq; f_res = Buf->file.params->res;

  if((b_perm_pq == f_perm_pq) && (b_perm_rs == f_perm_rs) &&
     (b_peq == f_peq) && (b_res == f_res)) {
      if(Buf->anti) method = 11;
      else method = 12;
      }
  else if((b_perm_pq != f_perm_pq) && (b_perm_rs == f_perm_rs) &&
	  (b_res == f_res)) {
      if(f_perm_pq && !b_perm_pq) {
	  if(Buf->anti) {
	      fprintf(stderr, "\n\tUnpack pq and antisymmetrize?\n");
	      exit(PSI_RETURN_FAILURE);
	    }
	  method = 21;
	}
      else if(!f_perm_pq && b_perm_pq) {
	  if(Buf->anti) method = 22;
	  else method = 23;
	}
      else {
	  fprintf(stderr, "\n\tInvalid second-level method!\n");
	  exit(PSI_RETURN_FAILURE);
	}
    }
  else if((b_perm_pq == f_perm_pq) && (b_perm_rs != f_perm_rs) &&
	  (b_peq == f_peq)) {
      if(f_perm_rs && !b_perm_rs) {
	  if(Buf->anti) {
	      fprintf(stderr, "\n\tUnpack rs and antisymmetrize?\n");
	      exit(PSI_RETURN_FAILURE);
	    }
	  method = 31;
	}
      else if(!f_perm_rs && b_perm_rs) {
	  if(Buf->anti) method = 32;
	  else method = 33;
	}
      else {
	  fprintf(stderr, "\n\tInvalid third-level method!\n");
	  exit(PSI_RETURN_FAILURE);
	}
    }
  else if((b_perm_pq != f_perm_pq) && (b_perm_rs != f_perm_rs)) {
      if(f_perm_pq && !b_perm_pq) {
	  if(f_perm_rs && !b_perm_rs) {
	      if(Buf->anti) {
		  fprintf(stderr, "\n\tUnpack pq and rs and antisymmetrize?\n");
		  exit(PSI_RETURN_FAILURE);
		}
	      else method = 41;
	    }
	  else if(!f_perm_rs && b_perm_rs) {
	      if(Buf->anti) {
		  fprintf(stderr, "\n\tUnpack pq and antisymmetrize?\n");
		  exit(PSI_RETURN_FAILURE);
		}
	      else method = 42;
	    }
	}
      else if(!f_perm_pq && b_perm_pq) {
	  if(f_perm_rs && !b_perm_rs) {
	      if(Buf->anti) {
		  fprintf(stderr, "\n\tUnpack rs and antisymmetrize?\n");
		  exit(PSI_RETURN_FAILURE);
		}
	      else method = 43;
	    }
	  else if(!f_perm_rs && b_perm_rs) {
	      if(Buf->anti) method = 44;
	      else method = 45;
	    }
	}
      else {
	  fprintf(stderr, "\n\tInvalid fourth-level method!\n");
	  exit(PSI_RETURN_FAILURE);
	}
    }
  else {
      fprintf(stderr, "\n\tInvalid method in dpd_buf_mat_irrep_rd!\n");
      exit(PSI_RETURN_FAILURE);
    }


  switch(method) {
  case 11: /* No change in pq or rs; antisymmetrize */
      
      /* Prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* Loop over rows in the dpdbuf/dpdfile */
      for(pq=0; pq < num_pq; pq++) {

	  filerow = Buf->file.incore ? pq : 0;

	  /* Fill the buffer */
	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, pq+start_pq);

	  /* Loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];

	      /* Column indices in the dpdfile */
	      filers = rs;
	      filesr = Buf->file.params->colidx[s][r];

	      value = Buf->file.matrix[irrep][filerow][filers];

	      value -= Buf->file.matrix[irrep][filerow][filesr];

	      /* Assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  
  case 12: /* no change in pq or rs */

      if(Buf->file.incore)
	  for(pq=0; pq < num_pq; pq++)
	      for(rs=0; rs < coltot; rs++)
		  Buf->matrix[irrep][pq][rs] =
				     Buf->file.matrix[irrep][pq+start_pq][rs];
      else {
	  Buf->file.matrix[irrep] = Buf->matrix[irrep];
	  dpd_file4_mat_irrep_rd_block(&(Buf->file), irrep, start_pq, num_pq);
	}
      
      break;
  case 21: /* unpack pq; no change in rs */
      /* prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* loop over rows in the dpdbuf */
      for(pq=0; pq < num_pq; pq++) {
	  p = Buf->params->roworb[irrep][pq+start_pq][0];
	  q = Buf->params->roworb[irrep][pq+start_pq][1];
	  filepq = Buf->file.params->rowidx[p][q];

	  filerow = Buf->file.incore ? filepq : 0;

	  /* set the permutation operator's value */
	  permute = ((p < q) && (f_perm_pq < 0) ? -1 : 1);

	  /* fill the buffer */
	  if(filepq >= 0)
	      dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);
	  else
	      dpd_file4_mat_irrep_row_zero(&(Buf->file), irrep, filepq);

	  /* loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      filers = rs;

	      value = Buf->file.matrix[irrep][filerow][filers];

	      /* assign the value, keeping track of the sign */
	      Buf->matrix[irrep][pq][rs] = permute*value;
	    }
	}

      /* close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 22: /* pack pq; no change in rs; antisymmetrize */
      /* prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* loop over rows in the dpdbuf */
      for(pq=0; pq < num_pq; pq++) {
	  p = Buf->params->roworb[irrep][pq+start_pq][0];
	  q = Buf->params->roworb[irrep][pq+start_pq][1];
	  filepq = Buf->file.params->rowidx[p][q];

	  filerow = Buf->file.incore ? filepq : 0;

	  /* fill the buffer */
	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];

	      /* column indices in the dpdfile */
	      filers = rs;
	      filesr = Buf->file.params->colidx[s][r];

	      value = Buf->file.matrix[irrep][filerow][filers];

	      value -= Buf->file.matrix[irrep][filerow][filesr];

	      /* assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 23: /* pack pq; no change in rs */
      /* prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* loop over rows in the dpdbuf */
      for(pq=0; pq < num_pq; pq++) {
	  p = Buf->params->roworb[irrep][pq+start_pq][0];
	  q = Buf->params->roworb[irrep][pq+start_pq][1];
	  filepq = Buf->file.params->rowidx[p][q];

	  filerow = Buf->file.incore ? filepq : 0;

	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      filers = rs;

	      value = Buf->file.matrix[irrep][filerow][filers];

	      /* assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 31: /* no change in pq; unpack rs */
      /* prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* loop over rows in the dpdbuf/dpdfile */
      for(pq=0; pq < num_pq; pq++) {
	  filepq = pq+start_pq;

	  filerow = Buf->file.incore ? filepq : 0;

	  /* fill the buffer */
	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];
	      filers = Buf->file.params->colidx[r][s];

	      /* rs permutation operator */
	      permute = ((r < s) && (f_perm_rs < 0) ? -1 : 1);

	      /* is this fast enough? */
	      value = ((filers < 0) ? 0 :
		       Buf->file.matrix[irrep][filerow][filers]);

	      /* assign the value */
	      Buf->matrix[irrep][pq][rs] = permute*value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 32: /* No change in pq; pack rs; antisymmetrize */
      /* Prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* Loop over rows in the dpdbuf/dpdfile */
      for(pq=0; pq < num_pq; pq++) {
	  filepq = pq+start_pq;

	  filerow = Buf->file.incore ? filepq : 0;

	  /* Fill the buffer */
	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* Loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];

	      /* Column indices in the dpdfile */
	      filers = Buf->file.params->colidx[r][s];
	      filesr = Buf->file.params->colidx[s][r];

	      value = Buf->file.matrix[irrep][filerow][filers];
	      value -= Buf->file.matrix[irrep][filerow][filesr];

	      /* Assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 33: /* No change in pq; pack rs */
      /* Prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* Loop over rows in the dpdbuf/dpdfile */
      for(pq=0; pq < num_pq; pq++) {
	  filepq = pq+start_pq;

	  filerow = Buf->file.incore ? filepq : 0;

	  /* Fill the buffer */
	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* Loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];
	      filers = Buf->file.params->colidx[r][s];

	      value = Buf->file.matrix[irrep][filerow][filers];

	      /* Assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 41: /* Unpack pq and rs */
      /* Prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* Loop over rows in the dpdbuf */
      for(pq=0; pq < num_pq; pq++) {
	  p = Buf->params->roworb[irrep][pq+start_pq][0];
	  q = Buf->params->roworb[irrep][pq+start_pq][1];
	  filepq = Buf->file.params->rowidx[p][q];

	  filerow = Buf->file.incore ? filepq : 0;

	  /* Set the value of the pq permutation operator */
	  pq_permute = ((p < q) && (f_perm_pq) ? -1 : 1);

	  /* Fill the buffer */
	  if(filepq >= 0)
	      dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);
	  else
	      dpd_file4_mat_irrep_row_zero(&(Buf->file), irrep, filepq);

	  /* Loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];
	      filers = Buf->file.params->colidx[r][s];

	      /* Set the value of the pqrs permutation operator */
	      permute = ((r < s) && (f_perm_rs) ? -1 : 1)*pq_permute;

              value = 0;

	      if(filers >= 0)
		  value = Buf->file.matrix[irrep][filerow][filers];

	      /* Assign the value */
	      Buf->matrix[irrep][pq][rs] = permute*value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 42: /* Pack pq; unpack rs */
      fprintf(stderr, "\n\tHaven't programmed method 42 yet!\n");
      exit(PSI_RETURN_FAILURE);

      break;
  case 43: /* Unpack pq; pack rs */
      fprintf(stderr, "\n\tHaven't programmed method 43 yet!\n");
      exit(PSI_RETURN_FAILURE);

      break;
  case 44: /* Pack pq; pack rs; antisymmetrize */
      /* Prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* Loop over rows in the dpdbuf */
      for(pq=0; pq < num_pq; pq++) {
	  p = Buf->params->roworb[irrep][pq+start_pq][0];
	  q = Buf->params->roworb[irrep][pq+start_pq][1];
	  filepq = Buf->file.params->rowidx[p][q];

	  filerow = Buf->file.incore ? filepq : 0;

	  /* Fill the buffer */
	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* Loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];

	      /* Column indices in the dpdfile */
	      filers = Buf->file.params->colidx[r][s];
	      filesr = Buf->file.params->colidx[s][r];

	      value = Buf->file.matrix[irrep][filerow][filers];
	      value -= Buf->file.matrix[irrep][filerow][filesr];

	      /* Assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  case 45: /* Pack pq and rs */
      /* Prepare the input buffer from the input file */
      dpd_file4_mat_irrep_row_init(&(Buf->file), irrep);

      /* Loop over rows in the dpdbuf */
      for(pq=0; pq < num_pq; pq++) {
	  p = Buf->params->roworb[irrep][pq+start_pq][0];
	  q = Buf->params->roworb[irrep][pq+start_pq][1];
	  filepq = Buf->file.params->rowidx[p][q];

	  filerow = Buf->file.incore ? filepq : 0;

	  dpd_file4_mat_irrep_row_rd(&(Buf->file), irrep, filepq);

	  /* Loop over the columns in the dpdbuf */
	  for(rs=0; rs < coltot; rs++) {
	      r = Buf->params->colorb[irrep^all_buf_irrep][rs][0];
	      s = Buf->params->colorb[irrep^all_buf_irrep][rs][1];
	      filers = Buf->file.params->colidx[r][s];

	      if(filers < 0) {
		  fprintf(stderr, "\n\tNegative colidx in method 44?\n");
		  exit(PSI_RETURN_FAILURE);
		}

	      value = Buf->file.matrix[irrep][filerow][filers];

	      /* Assign the value */
	      Buf->matrix[irrep][pq][rs] = value;
	    }
	}

      /* Close the input buffer */
      dpd_file4_mat_irrep_row_close(&(Buf->file), irrep);

      break;
  default:  /* Error trapping */
      fprintf(stderr, "\n\tInvalid switch case in dpd_buf_mat_irrep_rd!\n");
      exit(PSI_RETURN_FAILURE);
      break;
    }
  
#ifdef DPD_TIMER
  timer_off("buf4_rd_bk");
#endif
  return 0;

}
