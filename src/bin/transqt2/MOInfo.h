struct MOInfo {
  int nirreps;           /* no. of irreducible representations */
  int nmo;               /* no. of molecular orbitals */
  int nso;               /* no. of symmetry orbitals */
  int nao;               /* no. of atomic orbitals */
  int *sopi;             /* no. of SOs per irrep */
  int *mopi;             /* no. of MOs per irrep */
  int *sosym;            /* SO symmetry array */
  int *mosym;            /* MO symmetry array */
  int *clsdpi;           /* no. of closed-shells per irrep ex. frdocc */
  int *openpi;           /* no. of open-shells per irrep */
  int *uoccpi;           /* no. of unoccupied orbitals per irrep ex. fruocc */
  int *frdocc;           /* no. of frozen core orbitals per irrep */
  int *fruocc;           /* no. of frozen unoccupied orbitals per irrep */
  char **labels;         /* irrep labels */
  int nfzc;              /* total no. of frozen core orbitals */
  int nfzv;              /* total no. of frozen virtual orbitals */
  int nactive;           /* no. of active MOs */

  double enuc;           /* Nuclear repulsion energy */
  double efzc;           /* Frozen core energy */
  double eref;           /* The reference energy (computed here) */

  int *pitzer2qt;        /* reordering array (RHF): Pitzer MO -> QT */
  int *pitzer2qt_A;      /* reordering array (UHF): Pitzer MO -> QT (alpha) */
  int *pitzer2qt_B;      /* reordering array (UHF): Pitzer MO -> QT (beta) */

  int *act2qt;           /* reordering array (RHF): active MO -> QT */
  int *act2qt_A;         /* reordering array (UHF): active MO -> QT (alpha) */
  int *act2qt_B;         /* reordering array (UHF): active MO -> QT (beta) */
};