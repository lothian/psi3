#include <stdio.h>
#include <libipv1/ip_lib.h>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libchkpt/chkpt.h>
#include <psifiles.h>
#define EXTERN
#include "globals.h"

/* get_moinfo(): Routine to obtain basic orbital information from
** CHKPT and CC_INFO.
**
** T. Daniel Crawford, October 1996
** Modified for CCRESPONSE by TDC May, 2003 
*/

void get_moinfo(void)
{
  int i, j, h, p, q, errcod, nactive, nirreps, nfzc, nfzv;
  int *actpi, offset, act_offset;
  double **scf;

  psio_read_entry(CC_INFO, "Reference Wavefunction", (char *) &(params.ref), 
		  sizeof(int));

  chkpt_init(PSIO_OPEN_OLD);
  moinfo.nirreps = chkpt_rd_nirreps();
  moinfo.nmo = chkpt_rd_nmo();
  moinfo.nso = chkpt_rd_nso();
  moinfo.nao = chkpt_rd_nao();
  moinfo.labels = chkpt_rd_irr_labs();
  moinfo.orbspi = chkpt_rd_orbspi();
  moinfo.clsdpi = chkpt_rd_clsdpi();
  moinfo.openpi = chkpt_rd_openpi();
  moinfo.usotao = chkpt_rd_usotao();
  chkpt_close();

  nirreps = moinfo.nirreps;

  moinfo.ntri = moinfo.nmo * (moinfo.nmo+1)/2;
  moinfo.noei = moinfo.nso * (moinfo.nso+1)/2;
  moinfo.noei_ao = moinfo.nao * (moinfo.nao+1)/2;

  /* Get frozen and active orbital lookups from CC_INFO */
  moinfo.frdocc = init_int_array(nirreps);
  moinfo.fruocc = init_int_array(nirreps);
  psio_read_entry(CC_INFO, "Frozen Core Orbs Per Irrep",
		  (char *) moinfo.frdocc, sizeof(int)*nirreps);
  psio_read_entry(CC_INFO, "Frozen Virt Orbs Per Irrep",
		  (char *) moinfo.fruocc, sizeof(int)*nirreps);

  psio_read_entry(CC_INFO, "No. of Active Orbitals", (char *) &(nactive),
		  sizeof(int)); 
  moinfo.nactive = nactive;

  if(params.ref == 2) { /** UHF **/

    moinfo.aoccpi = init_int_array(nirreps);
    moinfo.boccpi = init_int_array(nirreps);
    moinfo.avirtpi = init_int_array(nirreps);
    moinfo.bvirtpi = init_int_array(nirreps);

    psio_read_entry(CC_INFO, "Active Alpha Occ Orbs Per Irrep",
		    (char *) moinfo.aoccpi, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Beta Occ Orbs Per Irrep",
		    (char *) moinfo.boccpi, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Alpha Virt Orbs Per Irrep",
		    (char *) moinfo.avirtpi, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Beta Virt Orbs Per Irrep",
		    (char *) moinfo.bvirtpi, sizeof(int)*moinfo.nirreps);

    moinfo.aocc_sym = init_int_array(nactive);
    moinfo.bocc_sym = init_int_array(nactive);
    moinfo.avir_sym = init_int_array(nactive);
    moinfo.bvir_sym = init_int_array(nactive);

    psio_read_entry(CC_INFO, "Active Alpha Occ Orb Symmetry",
		    (char *) moinfo.aocc_sym, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "Active Beta Occ Orb Symmetry",
		    (char *) moinfo.bocc_sym, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "Active Alpha Virt Orb Symmetry",
		    (char *) moinfo.avir_sym, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "Active Beta Virt Orb Symmetry",
		    (char *) moinfo.bvir_sym, sizeof(int)*nactive);

    moinfo.aocc_off = init_int_array(moinfo.nirreps);
    moinfo.bocc_off = init_int_array(moinfo.nirreps);
    moinfo.avir_off = init_int_array(moinfo.nirreps);
    moinfo.bvir_off = init_int_array(moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Alpha Occ Orb Offsets",
		    (char *) moinfo.aocc_off, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Beta Occ Orb Offsets",
		    (char *) moinfo.bocc_off, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Alpha Virt Orb Offsets",
		    (char *) moinfo.avir_off, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Beta Virt Orb Offsets",
		    (char *) moinfo.bvir_off, sizeof(int)*moinfo.nirreps);

    moinfo.qt_aocc = init_int_array(nactive);
    moinfo.qt_bocc = init_int_array(nactive);
    moinfo.qt_avir = init_int_array(nactive);
    moinfo.qt_bvir = init_int_array(nactive);

    psio_read_entry(CC_INFO, "CC->QT Alpha Active Occ Order",
		    (char *) moinfo.qt_aocc, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "CC->QT Beta Active Occ Order",
		    (char *) moinfo.qt_bocc, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "CC->QT Alpha Active Virt Order",
		    (char *) moinfo.qt_avir, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "CC->QT Beta Active Virt Order",
		    (char *) moinfo.qt_bvir, sizeof(int)*nactive);


  }
  else { /** RHF or ROHF **/

    moinfo.occpi = init_int_array(nirreps);
    moinfo.virtpi = init_int_array(nirreps);
    psio_read_entry(CC_INFO, "Active Occ Orbs Per Irrep",
		    (char *) moinfo.occpi, sizeof(int)*nirreps);
    psio_read_entry(CC_INFO, "Active Virt Orbs Per Irrep",
		    (char *) moinfo.virtpi, sizeof(int)*nirreps);

    moinfo.occ_sym = init_int_array(nactive);
    moinfo.vir_sym = init_int_array(nactive);
    psio_read_entry(CC_INFO, "Active Occ Orb Symmetry",
		    (char *) moinfo.occ_sym, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "Active Virt Orb Symmetry",
		    (char *) moinfo.vir_sym, sizeof(int)*nactive);

    moinfo.occ_off = init_int_array(moinfo.nirreps);
    moinfo.vir_off = init_int_array(moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Occ Orb Offsets",
		    (char *) moinfo.occ_off, sizeof(int)*moinfo.nirreps);
    psio_read_entry(CC_INFO, "Active Virt Orb Offsets",
		    (char *) moinfo.vir_off, sizeof(int)*moinfo.nirreps);

    /* Get CC->QT and QT->CC active occupied and virtual reordering arrays */
    moinfo.qt_occ = init_int_array(nactive);
    moinfo.qt_vir = init_int_array(nactive);
    psio_read_entry(CC_INFO, "CC->QT Active Occ Order",
		    (char *) moinfo.qt_occ, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "CC->QT Active Virt Order",
		    (char *) moinfo.qt_vir, sizeof(int)*nactive);

    moinfo.cc_occ = init_int_array(nactive);
    moinfo.cc_vir = init_int_array(nactive);
    psio_read_entry(CC_INFO, "QT->CC Active Occ Order",
		    (char *) moinfo.cc_occ, sizeof(int)*nactive);
    psio_read_entry(CC_INFO, "QT->CC Active Virt Order",
		    (char *) moinfo.cc_vir, sizeof(int)*nactive);
  }

  /* Adjust clsdpi array for frozen orbitals */
  for(i=0; i < nirreps; i++)
    moinfo.clsdpi[i] -= moinfo.frdocc[i];

  moinfo.uoccpi = init_int_array(moinfo.nirreps);
  for(i=0; i < nirreps; i++)
    moinfo.uoccpi[i] = moinfo.orbspi[i] - moinfo.clsdpi[i] -
      moinfo.openpi[i] - moinfo.fruocc[i] -
      moinfo.frdocc[i];

  /* Compute spatial-orbital reordering arrays */
  moinfo.pitzer2qt = init_int_array(moinfo.nmo);
  moinfo.qt2pitzer = init_int_array(moinfo.nmo);
  reorder_qt(moinfo.clsdpi, moinfo.openpi, moinfo.frdocc, moinfo.fruocc, 
	     moinfo.pitzer2qt, moinfo.orbspi, moinfo.nirreps);
  for(i=0; i < moinfo.nmo; i++) {
    j = moinfo.pitzer2qt[i];
    moinfo.qt2pitzer[j] = i;
  }

  /*** arrange active SCF MO's ***/
  actpi = init_int_array(nirreps);
  for(h=0; h < nirreps; h++) 
    actpi[h] = moinfo.orbspi[h] - moinfo.frdocc[h] - moinfo.fruocc[h];
  moinfo.actpi = actpi;

  chkpt_init(PSIO_OPEN_OLD);

  if(params.ref == 0 || params.ref == 1) {  /* RHF/ROHF */

    scf = chkpt_rd_scf();
    moinfo.scf = block_matrix(moinfo.nso, moinfo.nactive);
    offset = 0;
    act_offset = 0;
    for(h=0; h < nirreps; h++) {
      for(p=0; p < moinfo.orbspi[h]; p++)
	for(q=0; q < moinfo.actpi[h]; q++)
	  moinfo.scf[p+offset][q+act_offset] = scf[p+offset][q+offset+moinfo.frdocc[h]];

      offset += moinfo.orbspi[h];
      act_offset += moinfo.actpi[h];
    }
    free_block(scf);
  }
  else if(params.ref == 2) {  /* UHF */

    scf = chkpt_rd_alpha_scf();
    moinfo.scf_alpha = block_matrix(moinfo.nso, moinfo.nactive);
    offset = 0;
    act_offset = 0;
    for(h=0; h < nirreps; h++) {
      for(p=0; p < moinfo.orbspi[h]; p++)
	for(q=0; q < moinfo.actpi[h]; q++)
	  moinfo.scf_alpha[p+offset][q+act_offset] = scf[p+offset][q+offset+moinfo.frdocc[h]];

      offset += moinfo.orbspi[h];
      act_offset += moinfo.actpi[h];
    }
    free_block(scf);

    moinfo.scf_beta = block_matrix(moinfo.nso, moinfo.nactive);
    scf = chkpt_rd_beta_scf();
    offset = 0;
    act_offset = 0;
    for(h=0; h < nirreps; h++) {
      for(p=0; p < moinfo.orbspi[h]; p++)
	for(q=0; q < moinfo.actpi[h]; q++)
	  moinfo.scf_beta[p+offset][q+act_offset] = scf[p+offset][q+offset+moinfo.frdocc[h]];

      offset += moinfo.orbspi[h];
      act_offset += moinfo.actpi[h];
    }
    free_block(scf);
  }

  chkpt_close();
}

/* Frees memory allocated in get_moinfo() and dumps out the energy. */
void cleanup(void)
{
  int i;

  free(moinfo.orbspi);
  free(moinfo.clsdpi);
  free(moinfo.openpi);
  free(moinfo.uoccpi);
  free(moinfo.fruocc);
  free(moinfo.frdocc);
  free(moinfo.actpi);
  for(i=0; i < moinfo.nirreps; i++)
    free(moinfo.labels[i]);
  free(moinfo.labels);

  if(params.ref == 2) { /* UHF */
    free(moinfo.aoccpi);
    free(moinfo.boccpi);
    free(moinfo.avirtpi);
    free(moinfo.bvirtpi);
    free(moinfo.aocc_sym);
    free(moinfo.bocc_sym);
    free(moinfo.avir_sym);
    free(moinfo.bvir_sym);
    free(moinfo.aocc_off);
    free(moinfo.bocc_off);
    free(moinfo.avir_off);
    free(moinfo.bvir_off);
    free(moinfo.qt_aocc);
    free(moinfo.qt_bocc);
    free(moinfo.qt_avir);
    free(moinfo.qt_bvir);
    free_block(moinfo.scf_alpha);
    free_block(moinfo.scf_beta);
  }
  else {
    free(moinfo.occpi);
    free(moinfo.virtpi);
    free(moinfo.occ_sym);
    free(moinfo.vir_sym);
    free(moinfo.occ_off);
    free(moinfo.vir_off);
    free(moinfo.qt_occ);
    free(moinfo.qt_vir);
    free_block(moinfo.scf);
    free_block(moinfo.MUX);
    free_block(moinfo.MUY);
    free_block(moinfo.MUZ);
  }

  free_block(moinfo.usotao);
  free_block(moinfo.MUX);
  free_block(moinfo.MUY);
  free_block(moinfo.MUZ);
  free(moinfo.pitzer2qt);
  free(moinfo.qt2pitzer);
}
