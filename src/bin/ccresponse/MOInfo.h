struct MOInfo {
  int nirreps;        /* no. of irreducible representations */
  int nmo;            /* no. of molecular orbitals */
  int nso;            /* no. of symmetry orbitals */
  int nao;            /* no. of atomic orbitals */
  int noei;           /* no. of elements in SOxSO lower triangle */
  int ntri;           /* no. of elements in MOxMO lower triangle */
  int noei_ao;        /* no. of elements in AOxAO lower triangle */
  int nactive;        /* no. of active MO's */
  int nfzc;           /* no. of frozen core orbitals */
  int *orbspi;        /* no. of MOs per irrep */
  int *clsdpi;        /* no. of closed-shells per irrep  */
  int *openpi;        /* no. of open-shells per irrep */
  int *uoccpi;        /* no. of unoccupied orbitals per irrep  */
  int *frdocc;        /* no. of frozen core orbitals per irrep */
  int *fruocc;        /* no. of frozen unoccupied orbitals per irrep */
  int *actpi;         /* no. of active orbitals per irrep */
  char **labels;      /* irrep labels */
  int *pitzer2qt;     /* Pitzer -> QT reordering array */
  int *qt2pitzer;     /* QT -> Pitzer reordering array */
  int *occpi;         /* no. of occupied orbs. (incl. open) per irrep */
  int *aoccpi;        /* no. of alpha occupied orbs. (incl. open) per irrep */
  int *boccpi;        /* no. of beta occupied orbs. (incl. open) per irrep */
  int *virtpi;        /* no. of virtual orbs. (incl. open) per irrep */
  int *avirtpi;       /* no. of alpha virtual orbs. (incl. open) per irrep */
  int *bvirtpi;       /* no. of beta virtual orbs. (incl. open) per irrep */
  int *occ_sym;       /* relative occupied index symmetry */
  int *aocc_sym;      /* relative alpha occupied index symmetry */
  int *bocc_sym;      /* relative beta occupied index symmetry */
  int *vir_sym;       /* relative virtual index symmetry */
  int *avir_sym;      /* relative alpha virtual index symmetry */
  int *bvir_sym;      /* relative beta virtual index symmetry */
  int *occ_off;       /* occupied orbital offsets within each irrep */
  int *aocc_off;      /* occupied alpha orbital offsets within each irrep */
  int *bocc_off;      /* occupied beta orbital offsets within each irrep */
  int *vir_off;       /* virtual orbital offsets within each irrep */
  int *avir_off;      /* virtual alpha orbital offsets within each irrep */
  int *bvir_off;      /* virtual beta orbital offsets within each irrep */
  int *qt_occ;        /* CC->QT active occupied reordering array */
  int *qt_aocc;       /* CC->QT alpha active occupied reordering array */
  int *qt_bocc;       /* CC->QT beta active occupied reordering array */
  int *qt_vir;        /* CC->QT active virtiual reordering array */
  int *qt_avir;       /* CC->QT alpha active virtiual reordering array */
  int *qt_bvir;       /* CC->QT beta active virtiual reordering array */
  int *cc_occ;        /* QT->CC active occupied reordering array */
  int *cc_aocc;       /* QT->CC active occupied reordering array */
  int *cc_bocc;       /* QT->CC active occupied reordering array */
  int *cc_vir;        /* QT->CC active virtual reordering array */
  int *cc_avir;       /* QT->CC active virtual reordering array */
  int *cc_bvir;       /* QT->CC active virtual reordering array */
  double **scf;       /* SCF eigenvectors (RHF/ROHF) (active only) */
  double **scf_alpha; /* Alpha SCF eigenvectors (UHF) (active only) */
  double **scf_beta;  /* Beta SCF eigenvectors (UHF) (active only) */
  double **usotao;    /* SO->AO transformation matrix */
  double **MUX;       /* MO-basis x-dipole integrals (Pitzer order) */
  double **MUY;       /* MO-basis y-dipole integrals (Pitzer order) */
  double **MUZ;       /* MO-basis z-dipole integrals (Pitzer order) */
  int irrep_x;        /* irrep of x-dipole */
  int irrep_y;        /* irrep of y-dipole */
  int irrep_z;        /* irrep of z-dipole */
  int *mu_irreps;     /* irreps of x,y,z dipole components */
  double **LX;        /* MO-basis x-angular momentum ints (Pitzer order) */
  double **LY;        /* MO-basis y-angular momentum ints (Pitzer order) */
  double **LZ;        /* MO-basis z-angular momentum ints (Pitzer order) */
  int irrep_Rx;       /* irrep of x-angular momentum */
  int irrep_Ry;       /* irrep of y-angular momentum */
  int irrep_Rz;       /* irrep of z-angular momentum */
  int *l_irreps;      /* irreps of x,y,z angular momentum components */
  int natom;          /* number of atoms */
  double *zvals;      /* atomic zvals */
  double ***C;        /* Virtual orbital transformation matrix */
};
