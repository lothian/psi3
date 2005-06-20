/* Input parameters for cclambda */
struct Params {
  double tolerance;
  long int memory;
  int cachelev;
  int aobasis;
  int ref;
  int onepdm; /* produce ONLY the onepdm for properties */
  int relax_opdm;
  int use_zeta;
  int calc_xi;
  int connect_xi;
  int restart;
  int ground;
  int user_transition; /* was L specified on command-line? */
  int dertype;
  double cceom_energy;
  double R0;
  double L0;
  int L_irr;
  int R_irr;
  int G_irr;
  int L_root;
  int R_root;
  char *wfn;
  double overlap1; /* <L1|R1> */
  double overlap2; /* <L2|R2> */
  double RD_overlap; /* Rmnef <mn||ef> */
  double RZ_overlap; /* <R|zeta> */
};

