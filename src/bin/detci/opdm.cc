#define EXTERN
#define TOL 1E-14
/* #define DEBUG */

#include <math.h>

extern "C" {
   #include <stdio.h>
   #include <stdlib.h>
   /* may no longer need #include <libc.h> */
   #include <libciomr/libciomr.h>
   #include <libqt/qt.h>
#if USE_LIBCHKPT
   #include <libchkpt/chkpt.h>
#else
   #include <libfile30/file30.h>
#endif
   #include <libiwl/iwl.h>
   #include <psifiles.h>
   #include "structs.h"
   #include "globals.h"
}


#ifndef CIVECT_H
#include "civect.h"
#endif

#define MIN0(a,b) (((a)<(b)) ? (a) : (b))
#define MAX0(a,b) (((a)>(b)) ? (a) : (b))

void orbsfile_rd_blk(int targetfile, int root, int irrep, double **orbs_vector);
void orbsfile_wt_blk(int targetfile, int root, int irrep, double **orbs_vector);
void ave(int targetfile);
void opdm_block(struct stringwr **alplist, struct stringwr **betlist,
		double **onepdm, double **CJ, double **CI, int Ja_list, 
		int Jb_list, int Jnas, int Jnbs, int Ia_list, int Ib_list, 
		int Inas, int Inbs);
void opdm_ke(double **onepdm);

void opdm(struct stringwr **alplist, struct stringwr **betlist, 
          int Inroots, int Iroot, int Inunits, int Ifirstunit, 
	  int Jnroots, int Jroot, int Jnunits, int Jfirstunit, 
	  int targetfile, int writeflag, int printflag)
{

  CIvect Ivec, Jvec;
  int i, j, k, l, klast, roots;
  int maxrows, maxcols;
  unsigned long bufsz;
  int max_orb_per_irrep;
  double **transp_tmp = NULL;
  double **transp_tmp2 = NULL;
  double *buffer1, *buffer2, **onepdm;
  int i_ci, j_ci, irrep, mo_offset, so_offset, orb_length=0, opdm_length=0;
  double *opdm_eigval, **opdm_eigvec, **opdm_blk, **scfvec;
  int Iblock, Iblock2, Ibuf, Iac, Ibc, Inas, Inbs, Iairr;
  int Jblock, Jblock2, Jbuf, Jac, Jbc, Jnas, Jnbs, Jairr;
  int do_Jblock, do_Jblock2;
  int populated_orbs;
  double **tmp_mat, **opdmso;
  char opdm_key[80]; /* libpsio TOC entry name for OPDM for each root */

  Ivec.set(CIblks.vectlen, CIblks.num_blocks, Parameters.icore, Parameters.Ms0,
           CIblks.Ia_code, CIblks.Ib_code, CIblks.Ia_size, CIblks.Ib_size,
           CIblks.offset, CIblks.num_alp_codes, CIblks.num_bet_codes,
           CalcInfo.nirreps, AlphaG->subgr_per_irrep, Inroots, Inunits,
           Ifirstunit, CIblks.first_iablk, CIblks.last_iablk, CIblks.decode);

  Jvec.set(CIblks.vectlen, CIblks.num_blocks, Parameters.icore, Parameters.Ms0,
           CIblks.Ia_code, CIblks.Ib_code, CIblks.Ia_size, CIblks.Ib_size,
           CIblks.offset, CIblks.num_alp_codes, CIblks.num_bet_codes,
           CalcInfo.nirreps, AlphaG->subgr_per_irrep, Jnroots, Jnunits,
           Jfirstunit, CIblks.first_iablk, CIblks.last_iablk, CIblks.decode);


  populated_orbs = CalcInfo.num_ci_orbs + CalcInfo.num_fzc_orbs;
  for (irrep=0; irrep<CalcInfo.nirreps; irrep++) {
     opdm_length += (CalcInfo.orbs_per_irr[irrep] - CalcInfo.frozen_uocc[irrep])
                 * (CalcInfo.orbs_per_irr[irrep] - CalcInfo.frozen_uocc[irrep]);
     orb_length += (CalcInfo.so_per_irr[irrep]*CalcInfo.orbs_per_irr[irrep]);
     }

  /* find biggest blocksize */
  for (irrep=0,max_orb_per_irrep=0; irrep<CalcInfo.nirreps; irrep++) {
    if (CalcInfo.orbs_per_irr[irrep] > max_orb_per_irrep)
      max_orb_per_irrep = CalcInfo.so_per_irr[irrep];
  }

  opdm_eigvec = block_matrix(max_orb_per_irrep, max_orb_per_irrep);
  opdm_eigval = init_array(max_orb_per_irrep);
  opdm_blk = block_matrix(max_orb_per_irrep, max_orb_per_irrep);
  opdmso = block_matrix(CalcInfo.nso, CalcInfo.nso);
  tmp_mat = block_matrix(max_orb_per_irrep, max_orb_per_irrep);

  /* this index stuff is probably irrelevant now ... CDS 6/03 */
  Parameters.opdm_idxmat =
    init_int_matrix(Parameters.num_roots+2, CalcInfo.nirreps);
  Parameters.orbs_idxmat = 
    init_int_matrix(Parameters.num_roots+2, CalcInfo.nirreps);
  for (l=0; l<=(Parameters.num_roots+1); l++) {
     Parameters.opdm_idxmat[l][0] = l * opdm_length * sizeof(double); 
     Parameters.orbs_idxmat[l][0] = l * orb_length * sizeof(double);
     for (irrep=1; irrep<CalcInfo.nirreps; irrep++) {
        Parameters.orbs_idxmat[l][irrep] =
          Parameters.orbs_idxmat[l][irrep-1]+
          CalcInfo.so_per_irr[irrep-1]*CalcInfo.orbs_per_irr[irrep-1]
          *sizeof(double);  
        Parameters.opdm_idxmat[l][irrep] =
          Parameters.opdm_idxmat[l][irrep-1] +
          (CalcInfo.orbs_per_irr[irrep-1]-CalcInfo.frozen_uocc[irrep]) *
          (CalcInfo.orbs_per_irr[irrep-1]-CalcInfo.frozen_uocc[irrep]) *
          sizeof(double);
        } 
     } 

  buffer1 = Ivec.buf_malloc();
  buffer2 = Jvec.buf_malloc();
  Ivec.buf_lock(buffer1);
  Jvec.buf_lock(buffer2);
  onepdm = block_matrix(populated_orbs, populated_orbs); 

  if ((Ivec.icore==2 && Ivec.Ms0 && CalcInfo.ref_sym != 0) || 
      (Ivec.icore==0 && Ivec.Ms0)) {
    for (i=0, maxrows=0, maxcols=0; i<Ivec.num_blocks; i++) {
      if (Ivec.Ia_size[i] > maxrows) maxrows = Ivec.Ia_size[i];
      if (Ivec.Ib_size[i] > maxcols) maxcols = Ivec.Ib_size[i];
    }
    if (maxcols > maxrows) maxrows = maxcols;
    transp_tmp = (double **) malloc (maxrows * sizeof(double *));
    transp_tmp2 = (double **) malloc (maxrows * sizeof(double *));
    if (transp_tmp == NULL || transp_tmp2 == NULL) {
      printf("(opdm): Trouble with malloc'ing transp_tmp\n");
    }
    bufsz = Ivec.get_max_blk_size();
    transp_tmp[0] = init_array(bufsz);
    transp_tmp2[0] = init_array(bufsz);
    if (transp_tmp[0] == NULL || transp_tmp2[0] == NULL) {
      printf("(opdm): Trouble with malloc'ing transp_tmp[0]\n");
    }
  }
 
  if (writeflag) 
    /* rfile(targetfile); */
    psio_open(targetfile, PSIO_OPEN_OLD);

  for (k=0; k<Parameters.num_roots; k++) {
   
    if (k != 0) zero_mat(onepdm, populated_orbs, populated_orbs); 

    for (i=0; i<CalcInfo.num_fzc_orbs; i++)
      onepdm[i][i] = 2.0;

    if (Parameters.icore == 0) {
 
      for (Ibuf=0; Ibuf<Ivec.buf_per_vect; Ibuf++) {
        Ivec.read(Iroot, Ibuf);
        Iblock = Ivec.buf2blk[Ibuf];
        Iac = Ivec.Ia_code[Iblock];
        Ibc = Ivec.Ib_code[Iblock];
        Inas = Ivec.Ia_size[Iblock];
        Inbs = Ivec.Ib_size[Iblock];
       
        for (Jbuf=0; Jbuf<Jvec.buf_per_vect; Jbuf++) {
          do_Jblock=0; do_Jblock2=0;
          Jblock = Jvec.buf2blk[Jbuf];
          Jblock2 = -1;
          Jac = Jvec.Ia_code[Jblock];
          Jbc = Jvec.Ib_code[Jblock];
          if (Jvec.Ms0) Jblock2 = Jvec.decode[Jbc][Jac];
          Jnas = Jvec.Ia_size[Jblock];
          Jnbs = Jvec.Ib_size[Jblock];
          if (s1_contrib[Iblock][Jblock] || s2_contrib[Iblock][Jblock]) 
            do_Jblock = 1;
          if (Jvec.buf_offdiag[Jbuf] && (s1_contrib[Iblock][Jblock2] ||
                                         s2_contrib[Iblock][Jblock2]))
            do_Jblock2 = 1;
          if (!do_Jblock && !do_Jblock2) continue;
	 
          Jvec.read(Jroot, Jbuf);
	 
          if (do_Jblock) {
            opdm_block(alplist, betlist, onepdm, Jvec.blocks[Jblock], 
                       Ivec.blocks[Iblock], Jac, Jbc, Jnas,
                       Jnbs, Iac, Ibc, Inas, Inbs);
            }
	 
          if (do_Jblock2) {
            Jvec.transp_block(Jblock, transp_tmp);
            opdm_block(alplist, betlist, onepdm, transp_tmp,
                       Ivec.blocks[Iblock], Jbc, Jac, Jnbs,
                       Jnas, Iac, Ibc, Inas, Inbs);
          }
	 
        } /* end loop over Jbuf */
       
        if (Ivec.buf_offdiag[Ibuf]) { /* need to get contrib of transpose */
          Iblock2 = Ivec.decode[Ibc][Iac];
          Iac = Ivec.Ia_code[Iblock2];
          Ibc = Ivec.Ib_code[Iblock2];
          Inas = Ivec.Ia_size[Iblock2];
          Inbs = Ivec.Ib_size[Iblock2];
       
          Ivec.transp_block(Iblock, transp_tmp2);

          for (Jbuf=0; Jbuf<Jvec.buf_per_vect; Jbuf++) {
            do_Jblock=0; do_Jblock2=0;
            Jblock = Jvec.buf2blk[Jbuf];
            Jblock2 = -1;
            Jac = Jvec.Ia_code[Jblock];
            Jbc = Jvec.Ib_code[Jblock];
            if (Jvec.Ms0) Jblock2 = Jvec.decode[Jbc][Jac];
            Jnas = Jvec.Ia_size[Jblock];
            Jnbs = Jvec.Ib_size[Jblock];
            if (s1_contrib[Iblock2][Jblock] || s2_contrib[Iblock2][Jblock]) 
              do_Jblock = 1;
            if (Jvec.buf_offdiag[Jbuf] && (s1_contrib[Iblock2][Jblock2] ||
                                           s2_contrib[Iblock2][Jblock2]))
              do_Jblock2 = 1;
            if (!do_Jblock && !do_Jblock2) continue;
	   
            Jvec.read(Jroot, Jbuf);
	 
            if (do_Jblock) {
              opdm_block(alplist, betlist, onepdm, Jvec.blocks[Jblock], 
                         transp_tmp2, Jac, Jbc, Jnas,
                         Jnbs, Iac, Ibc, Inas, Inbs);
            }
	   
            if (do_Jblock2) {
              Jvec.transp_block(Jblock, transp_tmp);
              opdm_block(alplist, betlist, onepdm, transp_tmp,
                         transp_tmp2, Jbc, Jac, Jnbs,
                         Jnas, Iac, Ibc, Inas, Inbs);
            }
          } /* end loop over Jbuf */
        } /* end loop over Ibuf transpose */
      } /* end loop over Ibuf */
    } /* end icore==0 */

    else if (Parameters.icore==1) { /* whole vectors in-core */
      Ivec.read(Iroot, 0);
      Jvec.read(Jroot, 0);
      for (Iblock=0; Iblock<Ivec.num_blocks; Iblock++) {
        Iac = Ivec.Ia_code[Iblock];
        Ibc = Ivec.Ib_code[Iblock];
        Inas = Ivec.Ia_size[Iblock];
        Inbs = Ivec.Ib_size[Iblock];
        if (Inas==0 || Inbs==0) continue;
        for (Jblock=0; Jblock<Jvec.num_blocks; Jblock++) {
          Jac = Jvec.Ia_code[Jblock];
          Jbc = Jvec.Ib_code[Jblock];
          Jnas = Jvec.Ia_size[Jblock];
          Jnbs = Jvec.Ib_size[Jblock];
          if (s1_contrib[Iblock][Jblock] || s2_contrib[Iblock][Jblock])
            opdm_block(alplist, betlist, onepdm, Jvec.blocks[Jblock],
                       Ivec.blocks[Iblock], Jac, Jbc, Jnas,
                       Jnbs, Iac, Ibc, Inas, Inbs);
        }
      } /* end loop over Iblock */
    } /* end icore==1 */

    else if (Parameters.icore==2) { /* icore==2 */
      for (Ibuf=0; Ibuf<Ivec.buf_per_vect; Ibuf++) {
        Ivec.read(Iroot, Ibuf);
        Iairr = Ivec.buf2blk[Ibuf];

        for (Jbuf=0; Jbuf<Jvec.buf_per_vect; Jbuf++) {
          Jvec.read(Jroot, Jbuf);
          Jairr = Jvec.buf2blk[Jbuf];
	
        for (Iblock=Ivec.first_ablk[Iairr]; Iblock<=Ivec.last_ablk[Iairr];
             Iblock++) {
          Iac = Ivec.Ia_code[Iblock];
          Ibc = Ivec.Ib_code[Iblock];
          Inas = Ivec.Ia_size[Iblock];
          Inbs = Ivec.Ib_size[Iblock];
	   
          for (Jblock=Jvec.first_ablk[Jairr]; Jblock<=Jvec.last_ablk[Jairr];
               Jblock++) {
            Jac = Jvec.Ia_code[Jblock];
            Jbc = Jvec.Ib_code[Jblock];
            Jnas = Jvec.Ia_size[Jblock];
            Jnbs = Jvec.Ib_size[Jblock];
	   
            if (s1_contrib[Iblock][Jblock] || s2_contrib[Iblock][Jblock])
              opdm_block(alplist, betlist, onepdm, Jvec.blocks[Jblock],
                         Ivec.blocks[Iblock], Jac, Jbc, Jnas,
                         Jnbs, Iac, Ibc, Inas, Inbs);

            if (Jvec.buf_offdiag[Jbuf]) {
              Jblock2 = Jvec.decode[Jbc][Jac];
              if (s1_contrib[Iblock][Jblock2] ||
                  s2_contrib[Iblock][Jblock2]) {
              Jvec.transp_block(Jblock, transp_tmp);
                opdm_block(alplist, betlist, onepdm, transp_tmp,
                  Ivec.blocks[Iblock], Jbc, Jac,
                  Jnbs, Jnas, Iac, Ibc, Inas, Inbs);
	      }
	    }

          } /* end loop over Jblock */

          if (Ivec.buf_offdiag[Ibuf]) {
            Iblock2 = Ivec.decode[Ibc][Iac];
            Ivec.transp_block(Iblock, transp_tmp2);
            Iac = Ivec.Ia_code[Iblock2];
            Ibc = Ivec.Ib_code[Iblock2];
            Inas = Ivec.Ia_size[Iblock2];
            Inbs = Ivec.Ib_size[Iblock2];
	   
            for (Jblock=Jvec.first_ablk[Jairr]; Jblock<=Jvec.last_ablk[Jairr];
              Jblock++) {
              Jac = Jvec.Ia_code[Jblock];
              Jbc = Jvec.Ib_code[Jblock];
              Jnas = Jvec.Ia_size[Jblock];
              Jnbs = Jvec.Ib_size[Jblock];
	   
              if (s1_contrib[Iblock2][Jblock] || s2_contrib[Iblock2][Jblock])
                opdm_block(alplist, betlist, onepdm, Jvec.blocks[Jblock],
                           transp_tmp2, Jac, Jbc, Jnas, Jnbs, Iac, Ibc, 
                           Inas, Inbs);

                if (Jvec.buf_offdiag[Jbuf]) {
                  Jblock2 = Jvec.decode[Jbc][Jac];
                  if (s1_contrib[Iblock][Jblock2] || 
                    s2_contrib[Iblock][Jblock2]) {
                    Jvec.transp_block(Jblock, transp_tmp);
                    opdm_block(alplist, betlist, onepdm, transp_tmp,
                      transp_tmp2, Jbc, Jac, Jnbs, Jnas, Iac, Ibc, Inas, Inbs);
                  }
	        }

	      } /* end loop over Jblock */
            } /* end Ivec offdiag */

          } /* end loop over Iblock */
        } /* end loop over Jbuf */
      } /* end loop over Ibuf */
    } /* end icore==2 */

    else {
      printf("opdm: unrecognized core option!\n");
      return;
    }

    /* write and/or print the opdm */
    if (printflag) {
      fprintf(outfile, 
              "\n\nOne-particle density matrix MO basis for root %d\n",Iroot+1);
      print_mat(onepdm, populated_orbs, populated_orbs, outfile);
      fprintf(outfile, "\n");
    }


    if (writeflag) {
      sprintf(opdm_key,"MO-basis OPDM Root %d",k);
      psio_write_entry(targetfile, opdm_key, (char *) onepdm[0], 
        populated_orbs * populated_orbs * sizeof(double));

      /* write it without the "Root n" part if it's the desired root      */
      /* plain old "MO-basis OPDM" is what is searched by the rest of PSI */
      if (k==Parameters.root) {
        psio_write_entry(targetfile, "MO-basis OPDM", (char *) onepdm[0],
          populated_orbs * populated_orbs * sizeof(double));
      }
    }

    /* Get the kinetic energy if requested */
    if (Parameters.opdm_ke) {
      opdm_ke(onepdm);
    }

    fflush(outfile);
    Iroot++; Jroot++;
  } /* end loop over num_roots k */  

  if (writeflag) psio_close(targetfile, 1);

  if (transp_tmp != NULL) free_block(transp_tmp);
  if (transp_tmp2 != NULL) free_block(transp_tmp2);
  Ivec.buf_unlock();
  Jvec.buf_unlock();
  free(buffer1);
  free(buffer2);

  /* Average the opdm's */
  /* if (Parameters.opdm_diag) rfile(targetfile); */
  if (Parameters.opdm_ave) {
    psio_open(targetfile, PSIO_OPEN_OLD); 
    ave(targetfile); 
    psio_close(targetfile, 1);
  }

  /* get CI Natural Orbitals */
  if (Parameters.opdm_diag) {

    psio_open(targetfile, PSIO_OPEN_OLD);
    chkpt_init(PSIO_OPEN_OLD);

    /* reorder opdm from ci to pitzer and diagonalize each 
    ** symmetry blk
    */
    if (Parameters.opdm_ave) {
      klast = 1;
    }
    else {
      klast = Parameters.num_roots;
    }

    /* loop over roots or averaged opdm */
    for(k=0; k<klast; k++) {
      if (Parameters.opdm_ave && Parameters.print_lvl > 1) {
        fprintf(outfile,"\n\n\t\t\tCI Natural Orbitals for the Averaged\n");
        fprintf(outfile,"\t\t\tOPDM of %d Roots in terms of Molecular"
                 " Orbitals\n\n",k); 
      }
      else if (Parameters.print_lvl > 1) {
        fprintf(outfile,
             "\n\t\t\tCI Natural Orbitals in terms of Molecular Orbitals\n\n");
        fprintf(outfile,"\t\t\t Root %d\n\n",k+1);
        fflush(outfile);
      }

      mo_offset = 0;

      if (!Parameters.opdm_ave)
        sprintf(opdm_key, "MO-basis OPDM Root %d", k);
      else 
        sprintf(opdm_key, "MO-basis OPDM Ave");

      psio_read_entry(targetfile, opdm_key, (char *) onepdm[0],
        populated_orbs * populated_orbs * sizeof(double));

      for (irrep=0; irrep<CalcInfo.nirreps; irrep++) {
        if (CalcInfo.orbs_per_irr[irrep] == 0) continue; 
        for (i=0; i<CalcInfo.orbs_per_irr[irrep]-
                    CalcInfo.frozen_uocc[irrep]; i++) {
          for (j=0; j<CalcInfo.orbs_per_irr[irrep]-
                    CalcInfo.frozen_uocc[irrep]; j++) {
            i_ci = CalcInfo.reorder[i+mo_offset];
            j_ci = CalcInfo.reorder[j+mo_offset]; 
            opdm_blk[i][j] = onepdm[i_ci][j_ci];
          } 
        }
 
        /* Writing SCF vector to OPDM file for safe keeping 
         * because you might overwrite it with NO's for earlier root 
         * and we're only storing one irrep in core.  Only need to do once.
         */
        if (k==0) {

          scfvec = chkpt_rd_scf_irrep(irrep);

            #ifdef DEBUG
            fprintf(outfile,"Cvec for k==0, read in from chkpt original\n");
            fprintf(outfile," %s Block \n", CalcInfo.labels[irrep]);
            print_mat(scfvec, CalcInfo.orbs_per_irr[irrep],
                      CalcInfo.orbs_per_irr[irrep], outfile);
            #endif

          sprintf(opdm_key, "Old SCF Matrix Irrep %d", irrep);
          psio_write_entry(targetfile, opdm_key, (char *) scfvec[0],
            CalcInfo.orbs_per_irr[irrep] * CalcInfo.orbs_per_irr[irrep] *
            sizeof(double));
	  free_block(scfvec);
        }

        zero_mat(opdm_eigvec, max_orb_per_irrep, max_orb_per_irrep);

        if (CalcInfo.orbs_per_irr[irrep]-CalcInfo.frozen_uocc[irrep] > 0) {
          sq_rsp(CalcInfo.orbs_per_irr[irrep]-CalcInfo.frozen_uocc[irrep],
                 CalcInfo.orbs_per_irr[irrep]-CalcInfo.frozen_uocc[irrep],
                 opdm_blk, opdm_eigval, 1, opdm_eigvec, TOL); 
          }
        for (i=CalcInfo.orbs_per_irr[irrep]-CalcInfo.frozen_uocc[irrep]; 
             i<CalcInfo.orbs_per_irr[irrep]; i++) {
           opdm_eigvec[i][i] = 1.0;
           opdm_eigval[i] = 0.0;
           }
        eigsort(opdm_eigval, opdm_eigvec, -(CalcInfo.orbs_per_irr[irrep]));
        if (Parameters.print_lvl > 0) {
          if (irrep==0) {
            if (Parameters.opdm_ave) { 
              fprintf(outfile, "\n Averaged CI Natural Orbitals in terms "
                "of Molecular Orbitals\n\n");
              }
            else fprintf(outfile, "\n CI Natural Orbitals in terms of "
                   "Molecular Orbitals: Root %d\n\n", k+1);
          }
          fprintf(outfile,"\n %s Block (MO basis)\n", CalcInfo.labels[irrep]);
          eivout(opdm_eigvec, opdm_eigval, CalcInfo.orbs_per_irr[irrep],
                 CalcInfo.orbs_per_irr[irrep], outfile);
        }

        /* Write them if we need to */
        if (Parameters.opdm_wrtnos && (k==Parameters.opdm_orbs_root)) {
          if (irrep==0) {
            if (!Parameters.opdm_ave) {
              fprintf(outfile,"\n Writing CI Natural Orbitals for root %d"
                      " to checkpoint in terms of Symmetry Orbitals\n\n",k+1);
            }
          }
	  
	  scfvec = block_matrix(CalcInfo.orbs_per_irr[irrep], 
                       CalcInfo.orbs_per_irr[irrep]);
          sprintf(opdm_key, "Old SCF Matrix Irrep %d", irrep);
          psio_read_entry(targetfile, opdm_key, (char *) scfvec[0],
            CalcInfo.orbs_per_irr[irrep] * CalcInfo.orbs_per_irr[irrep] *
            sizeof(double));

          #ifdef DEBUG
          fprintf(outfile,"\nCvec read for MO to SO trans\n\n");
          fprintf(outfile," %s Block \n", CalcInfo.labels[irrep]);
          print_mat(scfvec, CalcInfo.orbs_per_irr[irrep],
                    CalcInfo.orbs_per_irr[irrep], outfile);
          fprintf(outfile,"\nOpdm_eigvec before MO to SO trans\n\n");
          fprintf(outfile," %s Block \n", CalcInfo.labels[irrep]);
          print_mat(opdm_eigvec, CalcInfo.orbs_per_irr[irrep],
                    CalcInfo.orbs_per_irr[irrep], outfile); 
          #endif
          mmult(scfvec, 0, opdm_eigvec, 0, opdm_blk, 0,
                CalcInfo.so_per_irr[irrep], CalcInfo.orbs_per_irr[irrep],
                CalcInfo.orbs_per_irr[irrep], 0); 
          free_block(scfvec);

          fprintf(outfile," %s Block (SO basis)\n", CalcInfo.labels[irrep]);
          print_mat(opdm_blk, CalcInfo.so_per_irr[irrep],
                    CalcInfo.orbs_per_irr[irrep], outfile);
          chkpt_wt_scf_irrep(opdm_blk, irrep);
          fprintf(outfile, "\n Warning: Natural Orbitals for the ");
	  if (Parameters.opdm_ave)
            fprintf(outfile, "Averaged OPDM ");
          else
            fprintf(outfile, "Root %d OPDM ", k);
          fprintf(outfile, "have been written to the checkpoint file!\n\n"); 
        } /* end code to write the NO's to disk */
        mo_offset += CalcInfo.orbs_per_irr[irrep];
      } /* end loop over irreps */
    } /* end loop over roots */

    free_block(onepdm);
    free_block(opdm_eigvec);
    free(opdm_eigval); 
    chkpt_close();
    psio_close(targetfile, 1);
  } /* CINOS completed */


  fflush(outfile);
  free_block(opdm_blk);

}

void opdm_block(struct stringwr **alplist, struct stringwr **betlist,
		double **onepdm, double **CJ, double **CI, int Ja_list, 
		int Jb_list, int Jnas, int Jnbs, int Ia_list, int Ib_list, 
		int Inas, int Inbs)
{
  int Ia_idx, Ib_idx, Ja_idx, Jb_idx, Ja_ex, Jb_ex, Jbcnt, Jacnt; 
  struct stringwr *Jb, *Ja;
  signed char *Jbsgn, *Jasgn;
  unsigned int *Jbridx, *Jaridx;
  double C1, C2, Ib_sgn, Ia_sgn;
  int i, j, oij, nfzc, *Jboij, *Jaoij;
 
  nfzc = CalcInfo.num_fzc_orbs;

  /* loop over Ia in Ia_list */
  if (Ia_list == Ja_list) {
    for (Ia_idx=0; Ia_idx<Inas; Ia_idx++) {
      for (Jb=betlist[Jb_list], Jb_idx=0; Jb_idx<Jnbs; Jb_idx++, Jb++) {
	C1 = CJ[Ia_idx][Jb_idx];

	/* loop over excitations E^b_{ij} from |B(J_b)> */
	Jbcnt = Jb->cnt[Ib_list];
	Jbridx = Jb->ridx[Ib_list];
	Jbsgn = Jb->sgn[Ib_list];
	Jboij = Jb->oij[Ib_list];
	for (Jb_ex=0; Jb_ex < Jbcnt; Jb_ex++) {
	  oij = *Jboij++;
	  Ib_idx = *Jbridx++;
	  Ib_sgn = (double) *Jbsgn++;
	  C2 = CI[Ia_idx][Ib_idx];
          i = oij/CalcInfo.num_ci_orbs + nfzc;
          j = oij%CalcInfo.num_ci_orbs + nfzc;
	  onepdm[i][j] += C1 * C2 * Ib_sgn;
	}
      }
    }
  }

  /* loop over Ib in Ib_list */
  if (Ib_list == Jb_list) {
    for (Ib_idx=0; Ib_idx<Inbs; Ib_idx++) {
      for (Ja=alplist[Ja_list], Ja_idx=0; Ja_idx<Jnas; Ja_idx++, Ja++) {
	C1 = CJ[Ja_idx][Ib_idx];
	
	/* loop over excitations */
	Jacnt = Ja->cnt[Ia_list];
	Jaridx = Ja->ridx[Ia_list];
	Jasgn = Ja->sgn[Ia_list];
	Jaoij = Ja->oij[Ia_list];
	for (Ja_ex=0; Ja_ex < Jacnt; Ja_ex++) {
	  oij = *Jaoij++;
	  Ia_idx = *Jaridx++;
	  Ia_sgn = (double) *Jasgn++;
	  C2 = CI[Ia_idx][Ib_idx];
          i = oij/CalcInfo.num_ci_orbs + nfzc;
          j = oij%CalcInfo.num_ci_orbs + nfzc;
	  onepdm[i][j] += C1 * C2 * Ia_sgn;
	}
      }
    }
  }
}


/*
** ave()
** 
** Parameters:
**  targetfile = file number to obtain matrices from 
**
*/
void ave(int targetfile)
{
  int root, i, j, populated_orbs;
  double **tmp_mat1, **tmp_mat2;
  char opdm_key[80];

  populated_orbs = CalcInfo.nmo-CalcInfo.num_fzv_orbs;
  tmp_mat1 = block_matrix(populated_orbs, populated_orbs);  
  tmp_mat2 = block_matrix(populated_orbs, populated_orbs);  
  zero_mat(tmp_mat1, populated_orbs, populated_orbs);

  for(root=0; root<Parameters.num_roots; root++) {

    sprintf(opdm_key, "MO-basis OPDM Root %d", root);

    if (root==0) {
      psio_read_entry(targetfile, opdm_key, (char *) tmp_mat1[0],
        populated_orbs * populated_orbs * sizeof(double));

      if (Parameters.opdm_print) {
        fprintf(outfile,"\n\n\t\tOPDM for Root 1");
        print_mat(tmp_mat1, populated_orbs, populated_orbs, outfile);
      }
    }

    else {
      psio_read_entry(targetfile, opdm_key, (char *) tmp_mat2[0],
        populated_orbs * populated_orbs * sizeof(double));

      for(i=0; i<populated_orbs; i++)
         for(j=0; j<populated_orbs; j++) 
            tmp_mat1[i][j] += tmp_mat2[i][j];    
      if (Parameters.opdm_print) {
        fprintf(outfile,"\n\n\t\tOPDM for Root %d",root+1);
        print_mat(tmp_mat2, populated_orbs, populated_orbs, outfile);
      }
    }

    zero_mat(tmp_mat2, populated_orbs, populated_orbs);
  }


  for (i=0; i<populated_orbs; i++)
    for (j=0; j<populated_orbs; j++) 
       tmp_mat1[i][j] *= (1.0/((double)Parameters.num_roots));

  psio_write_entry(targetfile, "MO-basis OPDM Ave", (char *) tmp_mat1[0],
    populated_orbs*populated_orbs*sizeof(double));

  if (Parameters.print_lvl > 0 || Parameters.opdm_print) {
    fprintf(outfile,
      "\n\t\t\t Averaged OPDM's for %d Roots written to opdm_file \n\n",
      Parameters.num_roots);
  }
  if (Parameters.opdm_print) {
    print_mat(tmp_mat1, populated_orbs, populated_orbs, outfile);
  }

  free_block(tmp_mat1);
  free_block(tmp_mat2);

}


/*
** opdm_ke
**
** Compute the kinetic energy contribution from the correlated part of the
** one-particle density matrix.  For Daniel Crawford
*/
void opdm_ke(double **onepdm)
{
  int errcod;
  int src_T_file, mo_offset, so_offset, irrep, i, j, i_ci, j_ci, i2, j2, ij;
  int maxorbs;
  int noeints;
  double *T, **scfmat, **opdm_blk, **tmp_mat, ke, kei;

  ke = kei = 0.0;

  /* read in the kinetic energy integrals */
  noeints = CalcInfo.nso*(CalcInfo.nso+1)/2;
  src_T_file = PSIF_OEI;

  T = init_array(noeints);
  if (Parameters.print_lvl>2) 
    fprintf(outfile, "Kinetic energy integrals (SO basis):\n");
  errcod = iwl_rdone(src_T_file,PSIF_SO_T,T,noeints,0,
                     (Parameters.print_lvl>2),outfile);
  if (!errcod) {
    printf("(detci): Error reading kinetic energy ints\n");
    exit(1);
  }

  /* find biggest blocksize */
  for (irrep=0,maxorbs=0; irrep<CalcInfo.nirreps; irrep++) {
    if (CalcInfo.orbs_per_irr[irrep] > maxorbs)
      maxorbs = CalcInfo.so_per_irr[irrep];
  }
  opdm_blk = block_matrix(maxorbs, maxorbs);
  tmp_mat = block_matrix(maxorbs, maxorbs);

  /* transform the onepdm into SO form, one irrep at a time */
  so_offset = mo_offset = 0;
  fprintf(outfile,"Correlation Piece of OPDM in SO basis\n");
  for (irrep=0; irrep<CalcInfo.nirreps; irrep++) {
    if (CalcInfo.orbs_per_irr[irrep] == 0) continue;
    for (i=0; i<CalcInfo.orbs_per_irr[irrep]; i++) {
      for (j=0; j<CalcInfo.orbs_per_irr[irrep]; j++) {
        i_ci = CalcInfo.reorder[i+mo_offset];
        j_ci = CalcInfo.reorder[j+mo_offset];
        opdm_blk[i][j] = onepdm[i_ci][j_ci];
      }
    }
    /* need to subtract out reference piece, assume single det ref */
    for (i=0; i<CalcInfo.docc[irrep]; i++) {
      opdm_blk[i][i] -= 2.0;
    }
    /* keep counting from i to take out socc part */
    for (j=0; j<CalcInfo.socc[irrep]; j++,i++) {
      opdm_blk[i][i] -= 1.0;
    }

    
    fprintf(outfile, "Irrep %d\n", irrep);
    fprintf(outfile, "MO basis, Pitzer order\n");
    print_mat(opdm_blk,CalcInfo.so_per_irr[irrep],
              CalcInfo.so_per_irr[irrep],outfile);

    /* transform back to SO basis */
#if USE_LIBCHKPT
    scfmat = chkpt_rd_scf_irrep(irrep);
#else
    scfmat = file30_rd_blk_scf(irrep);    
#endif
    mmult(opdm_blk,0,scfmat,1,tmp_mat,0,CalcInfo.orbs_per_irr[irrep],
          CalcInfo.orbs_per_irr[irrep],CalcInfo.so_per_irr[irrep],0);
    mmult(scfmat,0,tmp_mat,0,opdm_blk,0,CalcInfo.so_per_irr[irrep],
          CalcInfo.orbs_per_irr[irrep],CalcInfo.so_per_irr[irrep],0); 
    
    fprintf(outfile, "SO basis, Pitzer order\n");
    print_mat(opdm_blk,CalcInfo.so_per_irr[irrep],
              CalcInfo.so_per_irr[irrep],outfile);

    /* get kinetic energy contribution */
    kei = 0.0;
    for (i=0,i2=so_offset; i<CalcInfo.so_per_irr[irrep]; i++,i2++) {
      for (j=0,j2=so_offset; j<CalcInfo.so_per_irr[irrep]; j++,j2++) {
        ij = ioff[MAX0(i2,j2)] + MIN0(i2,j2);
        kei += opdm_blk[i][j] * T[ij];
      }
    }

    fprintf(outfile,"Contribution to correlation kinetic energy = %15.10lf\n", 
            kei);

    ke += kei;
    mo_offset += CalcInfo.orbs_per_irr[irrep];
    so_offset += CalcInfo.so_per_irr[irrep];

  } /* end loop over irreps */

  fprintf(outfile, "\nTotal correlation kinetic energy = %15.10lf\n", ke);

}

