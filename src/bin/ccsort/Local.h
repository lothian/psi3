/*! \file Local.h
    \ingroup (CCSORT)
    \brief Enter brief description of file here 
*/

/*! \defgroup CCSORT Add a description of the group CCSORT */

struct Local {
  int natom;
  int nso;
  int nocc;
  int nvir;
  int domain_polar;
  int domain_mag;
  int domain_sep;
  int *aostart;
  int *aostop;
  int **domain;
  int **pairdomain;
  int *domain_len;
  int *pairdom_len;
  int *pairdom_nrlen;
  int *weak_pairs;
  double ***V;
  double ***W;
  double *eps_occ;
  double **eps_vir;
  double cutoff;
  char *method;
  char *weakp;
  int filter_singles;
  double weak_pair_energy;
  double cphf_cutoff;
  char *freeze_core;
  char *pairdef;
};
