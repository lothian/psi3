#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libchkpt/chkpt.h>
#include <libipv1/ip_lib.h>
#include <liboptions/liboptions.h> 
#include <libutil/libutil.h>

#include "moinfo_base.h"

extern FILE *outfile;

using namespace std;

MOInfoBase::MOInfoBase()
{
  startup();
  charge       = options_get_int("CHARGE");
  multiplicity = options_get_int("MULTIPLICITY");
}

MOInfoBase::~MOInfoBase()
{
  cleanup();
}

void MOInfoBase::startup()
{
  PSI_NULL(sopi);
  PSI_NULL(docc);
  PSI_NULL(actv);
  PSI_NULL(ioff);
  
  nael = 0;
  nbel = 0;
  compute_ioff();
}

void MOInfoBase::cleanup()
{
  for(int h = 0;h < nirreps;h++)
     free(irr_labs[h]);
  free(irr_labs);
  
  PSI_FREE(sopi);
  PSI_DELETE_ARRAY(docc);
  PSI_DELETE_ARRAY(actv);
  PSI_DELETE_ARRAY(ioff);
}

void MOInfoBase::read_chkpt_data()
{
  nirreps  = chkpt_rd_nirreps();
  nso      = chkpt_rd_nso();

  sopi     = chkpt_rd_sopi();

  irr_labs = chkpt_rd_irr_labs();
  
  nuclear_energy = chkpt_rd_enuc();
}

void MOInfoBase::compute_number_of_electrons()
{
  int     nel   = 0;
  int     natom = chkpt_rd_natom();
  double* zvals = chkpt_rd_zvals();

  for(int i=0; i < natom;i++){
    nel += static_cast<int>(zvals[i]);
  }
  nel -= charge;

  // Check if the multiplicity makes sense
  if( ((nel+1-multiplicity) % 2) != 0)
    print_error("\n\n  MOInfoBase: Wrong multiplicity.\n\n",__FILE__,__LINE__);
  
  nael = (nel + multiplicity -1)/2;
  nbel =  nel - nael;
  
  PSI_FREE(zvals);
}

void MOInfoBase::compute_ioff()
{
  ioff    = new int[IOFF];
  ioff[0] = 0;
  for(unsigned long int i=1; i < IOFF; i++)
    ioff[i] = ioff[i-1] + i;
}

void MOInfoBase::read_mo_space(int nirreps_ref,int& n, int* mo, const char* label)
{
  // TODO Convert to liboptions
  int size;
  int stat = ip_count(const_cast<char *>(label),&size,0); // Cast to avoid warnings
  n=0;
  if(stat == IPE_OK){
    if(size==nirreps_ref){
      for(int i=0;i<size;i++){
        stat=ip_data(const_cast<char *>(label),const_cast<char *>("%d"),(&(mo)[i]),1,i); // Cast to avoid warnings
        n += mo[i];
      }
    }else{
      fprintf(outfile,"\n\n  The size of the %s array (%d) does not match the number of irreps (%d), please fix the input file",label,size,nirreps_ref);
      fflush(outfile);
      exit(1);
    }
  }else{ // The keyword label was not found
    for(int i=0;i<nirreps_ref;i++)
      mo[i]=0;
    n = 0;
  }
}

void MOInfoBase::print_mo_space(int& n, int* mo, const char* label)
{
  fprintf(outfile,"\n  %s",label);

  for(int i=nirreps;i<8;i++)
    fprintf(outfile,"     ");
  for(int i=0;i<nirreps;i++)
    fprintf(outfile," %3d ",mo[i]);
  fprintf(outfile,"  %3d",n);
}

void MOInfoBase::write_chkpt_mos()
{
  chkpt_wt_scf(scf);
}

void MOInfoBase::correlate(char *ptgrp, int irrep, int& nirreps_old, int& nirreps_new,int*& arr)
{ /* This is a hack from input!!! (ACS) */
  int  i;

  if (strcmp(ptgrp,"C1 ") == 0)
    nirreps_old = 1;
  else if (strcmp(ptgrp,"Cs ") == 0)
    nirreps_old = 2;
  else if (strcmp(ptgrp,"Ci ") == 0)
    nirreps_old = 2;
  else if (strcmp(ptgrp,"C2 ") == 0)
    nirreps_old = 2;
  else if (strcmp(ptgrp,"C2v") == 0)
    nirreps_old = 4;
  else if (strcmp(ptgrp,"D2 ") == 0)
    nirreps_old = 4;
  else if (strcmp(ptgrp,"C2h") == 0)
    nirreps_old = 4;
  else if (strcmp(ptgrp,"D2h") == 0)
    nirreps_old = 8;
  else {
    fprintf(outfile,"point group %s unknown.\n",ptgrp);
    exit(1);
  }

  arr = new int[nirreps_old];

  if (irrep == 0) { /* return identity */
    nirreps_new = nirreps_old;
    for (i=0; i<nirreps_old; ++i)
      arr[i] = i;
    return;
  }

  nirreps_new = nirreps_old / 2;
  if ((strcmp(ptgrp,"C1 ") == 0) || (strcmp(ptgrp,"Cs ") == 0) ||
      (strcmp(ptgrp,"Ci ") == 0) || (strcmp(ptgrp,"C2 ") == 0) ) {
        arr[0] = 0; arr[1] = 0;
  }
  else if ( (strcmp(ptgrp,"C2v") == 0) || (strcmp(ptgrp,"D2 ") == 0) ||
            (strcmp(ptgrp,"C2h") == 0) ) {
    if (irrep == 1) {
      arr[0] = 0;  arr[1] = 0; arr[2] = 1;  arr[3] = 1;
    }
    else if (irrep == 2) {
      arr[0] = 0;  arr[1] = 1; arr[2] = 0;  arr[3] = 1;
    }
    else if (irrep == 3) {
      arr[0] = 0;  arr[1] = 1; arr[2] = 1;  arr[3] = 0;
    }
  }
  else if (strcmp(ptgrp,"D2h") == 0) {
    /* 1,2,3 give C2h displaced geometries */
    if (irrep == 1) {
      arr[0] = 0;  arr[1] = 0; arr[2] = 1;  arr[3] = 1;
      arr[4] = 2;  arr[5] = 2; arr[6] = 3;  arr[7] = 3;
    }
    else if (irrep == 2) {
      arr[0] = 0;  arr[1] = 1; arr[2] = 0;  arr[3] = 1;
      arr[4] = 2;  arr[5] = 3; arr[6] = 2;  arr[7] = 3;
    }
    else if (irrep == 3) {
      arr[0] = 0;  arr[1] = 1; arr[2] = 1;  arr[3] = 0;
      arr[4] = 2;  arr[5] = 3; arr[6] = 3;  arr[7] = 2;
    }
    /* 4 gives D2 displaced geometries */
    else if (irrep == 4) { /* D2 */
      arr[0] = 0;  arr[1] = 1; arr[2] = 2;  arr[3] = 3;
      arr[4] = 0;  arr[5] = 1; arr[6] = 2;  arr[7] = 3;
    }
    /* displacements along irreps 5,6,7 make C2v structures */
    /* care is taken to make sure definition of b1 and b2 will
       match those that input will generate - the following seems to work:
       b1u disp: has C2(z), b2 irrep symmetric wrt sigma(yz)
       b2u disp: has C2(y), b2 irrep symmetric wrt sigma(xy)
       b3u disp: has C2(x), b2 irrep symmetric wrt sigma(xz) */
    else if (irrep == 5) { /* b1u */
      arr[0] = 0;  arr[1] = 1; arr[2] = 2;  arr[3] = 3;
      arr[4] = 1;  arr[5] = 0; arr[6] = 3;  arr[7] = 2;
    }
    else if (irrep == 6) { /* b2u */
      arr[0] = 0;  arr[1] = 3; arr[2] = 1;  arr[3] = 2;
      arr[4] = 1;  arr[5] = 2; arr[6] = 0;  arr[7] = 3;
    }
    else if (irrep == 7) { /* b3u */
      arr[0] = 0;  arr[1] = 2; arr[2] = 3;  arr[3] = 1;
      arr[4] = 1;  arr[5] = 3; arr[6] = 2;  arr[7] = 0;
    }
  }
  else {
    fprintf(outfile,"Point group unknown for correlation table.\n");
  }
  return;
}