struct Params {
  char *wfn;
  int ref;
  int cachelev;
  int dertype;
  int reset;          /* cmdline argument; if true, all CC-related
                         files are deleted at the beginning of the
                         run */
  int print_lvl;      /* Output level control */
  int print_tei;      /* Boolean for printing two-electron integrals */
  double tolerance;   /* Cutoff value for integrals in IWL Buffers */
  long int memory;    /* Memory available (in bytes) */
  int semicanonical;  /* Boolean for semicanonical orbitals */
};