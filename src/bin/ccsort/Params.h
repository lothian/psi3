struct Params {
  int print_lvl;         /* Output level control */
  int keep_TEIFile;      /* Should we keep the input two-elec. integrals? */
  int keep_OEIFile;      /* Should we keep the input one-elec. integrals? */
  double tolerance;      /* Cutoff value for integrals in IWL Buffers */
  long int memory;       /* Memory available (in bytes) */
  int cachelev;
  int ref;
  char *wfn;
  int make_abcd;
  int dertype;
  char *aobasis;
  int reset;            /* cmdline argument; if true, all CC-related files */
                        /* are deleted at the beginning of the run */
  int semicanonical;    /* semicanonical orbitals for perturbation theory */
};
