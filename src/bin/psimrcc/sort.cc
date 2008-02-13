/***************************************************************************
 *  PSIMRCC : Copyright (C) 2007 by Francesco Evangelista and Andrew Simmonett
 *  frank@ccc.uga.edu   andysim@ccc.uga.edu
 *  A multireference coupled cluster code
 ***************************************************************************/
/**
 *  @file ccsort.cpp
 *  @ingroup PSIMRCC
*/
#include <cmath>
#include <algorithm>
#include "moinfo.h"
#include "sort.h"
#include "transform.h"
#include "utilities.h"
#include "blas.h"

#include <libciomr/libciomr.h>
#include <libpsio/psio.h>
#include <libchkpt/chkpt.h>
#include <libipv1/ip_lib.h>
#include <libiwl/iwl.h>
#include <libqt/qt.h>
#include "psifiles.h"

extern FILE* outfile;

namespace psi{ namespace psimrcc{

using namespace std;

CCSort::CCSort(SortAlgorithm algorithm):
efzc(0.0),frozen_core(0)
{
  init();
  // Two algorithms for forming the integrals
  
  // 1. Full in-core algorithm: the transformed integrals and CCMatrix object fit in core
  //   build_integrals_in_core();

  // 2. Out-of-core algorithm: the transformed integrals and the CCMatrix(s) don't fit in
  //    core or they are requested to be out-of-core
  switch (algorithm) {
    case out_of_core_sort :
      build_integrals_out_of_core();
      break;
    case mrpt2_sort :
      build_integrals_mrpt2();
      break;
  }

  moinfo->set_fzcore_energy(efzc);
  fprintf(outfile,"\n  Frozen Energy    = %20.15f",efzc);
}

CCSort::~CCSort()
{
  cleanup();
}

/**
 * Initialize the CCSort class
 */
void CCSort::init()
{
  // Find the frozen core orbitals in Pitzer ordering
  nfzc        = moinfo->get_nfocc();
  int* focc   = moinfo->get_focc();
  int* orbspi = moinfo->get_orbspi();
  frozen_core = new int[nfzc];
  int count1  = 0;
  int count2  = 0;
  for(int h=0;h<moinfo->get_nirreps();h++){
    for(int i=0;i<focc[h];i++)
      frozen_core[count1++]=count2+i;
    count2+=orbspi[h];
  }
}

/**
 * Clean the CCSort class
 */
void CCSort::cleanup()
{
  if(frozen_core!=0)
    delete[] frozen_core;
}

}} /* End Namespaces */
