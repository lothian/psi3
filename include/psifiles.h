/*
** PSIFILES.H
**
** This header file contains the definitions of the numbers assigned
**  to various binary files in PSI.  This was created primarily to 
**  help avoid conflicts in the numbering of new PSI files in developmental
**  programs but will grow to encompass some older binary files.
**
** This additional level of abstraction will aid in the maintenance of
**  code.  You are strongly encouraged to refer to files using these
**  definitions rather than the actual numbers; the numbers may change 
**  in the future but the names will not.
**
** Created by C. David Sherrill on 29 April 1998
*/

#define PSI_DEFAULT_FILE_PREFIX "psi"

#define PSIF_CHKPT          32 /* new libpsio checkpoint file number */

#define PSIF_OPTKING        1
#define PSIF_DSCF           31
#define PSIF_SO_TEI         33
#define PSIF_OEI            35
#define PSIF_SO_R12         38
#define PSIF_SO_R12T1       39
#define PSIF_DERINFO        40
#define PSIF_SO_PRESORT     41
#define PSIF_OLD_CHKPT      42   /* Until we have flexible PSIF_CHKPT this will store previous calculation info */
#define PSIF_CIVECT         43   /* CI vector from DETCI along with string and determinant info */
#define PSIF_MO_R12         79
#define PSIF_MO_R12T1       80
#define PSIF_SO_PKSUPER1    92
#define PSIF_SO_PKSUPER2    93

#define PSIF_MO_TEI         72
#define PSIF_MO_OPDM        73
#define PSIF_MO_TPDM        74
#define PSIF_MO_LAG         75
#define PSIF_AO_OPDM        76   /* PSIF_AO_OPDM also contains AO Lagrangian */
#define PSIF_AO_TPDM        77

/*
** MO Hessian File (also contains specialized integral and Fock lists.
** See programs STABLE and CPHF for more info.
** -TDC, 7/00
*/
#define PSIF_MO_HESS        78
#define PSIF_CPHF           78

/*
** Additions for UHF-based transformations.
** -TDC, 6/01
*/
#define PSIF_MO_AA_TEI      81
#define PSIF_MO_BB_TEI      82
#define PSIF_MO_AB_TEI      83
#define PSIF_MO_AA_TPDM     84
#define PSIF_MO_BB_TPDM     85
#define PSIF_MO_AB_TPDM     86
#define PSIF_AA_PRESORT     87   /* AA UHF twopdm presort file */
#define PSIF_BB_PRESORT     88   /* BB UHF twopdm presort file */
#define PSIF_AB_PRESORT     89   /* AB UHF twopdm presort file */

/* All of these one-electron quantities have been moved into PSIF_OEI */
/* These macros give libpsio TOC strings for easy identification.     */
#define PSIF_SO_S           "SO-basis Overlap Ints"
#define PSIF_SO_T           "SO-basis Kinetic Energy Ints"
#define PSIF_SO_V           "SO-basis Potential Energy Ints"
#define PSIF_AO_S           "AO-basis Overlap Ints"
#define PSIF_AO_MX          "AO-basis Mu-X Ints"
#define PSIF_AO_MY          "AO-basis Mu-Y Ints"
#define PSIF_AO_MZ          "AO-basis Mu-Z Ints"
#define PSIF_MO_MX          "MO-basis Mu-X Ints"
#define PSIF_MO_MY          "MO-basis Mu-Y Ints"
#define PSIF_MO_MZ          "MO-basis Mu-Z Ints"
#define PSIF_AO_NablaX      "AO-basis Nabla-X Ints"
#define PSIF_AO_NablaY      "AO-basis Nabla-Y Ints"
#define PSIF_AO_NablaZ      "AO-basis Nabla-Z Ints"
#define PSIF_AO_LX          "AO-basis LX Ints"
#define PSIF_AO_LY          "AO-basis LY Ints"
#define PSIF_AO_LZ          "AO-basis LZ Ints"
#define PSIF_MO_OEI         "MO-basis One-electron Ints"
#define PSIF_MO_A_OEI       "MO-basis Alpha One-electron Ints"
#define PSIF_MO_B_OEI       "MO-basis Beta One-electron Ints"
#define PSIF_MO_FZC         "MO-basis Frozen-Core Operator"
#define PSIF_MO_A_FZC       "MO-basis Alpha Frozen-Core Oper"
#define PSIF_MO_B_FZC       "MO-basis Beta Frozen-Core Oper"

/* More macros */
#define PSIF_AO_OPDM_TRIANG "AO-basis OPDM triang"
#define PSIF_AO_LAG_TRIANG  "AO-basis Lagrangian triang"
#define PSIF_AO_OPDM_SQUARE "AO-basis OPDM square"
#define PSIF_SO_OPDM        "SO-basis OPDM"
#define PSIF_SO_OPDM_TRIANG "SO-basis triang"

/* PSI return codes --- for new PSI driver           */
#define PSI_RETURN_SUCCESS      0
#define PSI_RETURN_FAILURE      1
#define PSI_RETURN_ENDLOOP      2

