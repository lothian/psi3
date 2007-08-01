/*! \file fold.c
    \ingroup (CCDENSITY)
    \brief Enter brief description of file here 
*/
#define EXTERN
#include "globals.h"

/* FOLD(): Fold the Fock matrix contributions to the energy (or energy
** derivative) into the two-particle density matrix.  Here we are
** trying to convert from an energy expression of the form:
**
** E = sum_pq Dpq fpq + 1/4 sum_pqrs Gpqrs <pq||rs>
**
** to the form:
**
** E = sum_pq Dpq hpq + 1/4 sum_pqrs Gpqrs <pq||rs>
**
** We do this by shifting some one-particle density matrix components
** into appropriate two-particle density matrix components:
**
** G'pmrm = Dpr + 4 * Gpmrm
**
** One problem is that we need to make sure the resulting density,
** G'pqrs, is still antisymmetric to permutation of p and q or r and
** s.  So, for example, for the Gimkm component we compute:
**
** G'pmrm = Dpr + Gpmrm
** G'mprm = Dpr - Gmprm
** G'pmmr = Dpr - Gpmmr
** G'mpmr = Dpr + Gmpmr
** */

void fold_ROHF(struct RHO_Params rho_params);
void fold_UHF(struct RHO_Params rho_params);

void fold(struct RHO_Params rho_params)
{
  if(params.ref == 0 || params.ref == 1) fold_ROHF(rho_params);
  else if(params.ref == 2) fold_UHF(rho_params);
}
