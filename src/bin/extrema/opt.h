#ifdef EXTERN
# undef EXTERN
# define EXTERN extern
extern int MAX_LINELENGTH;
#else 
# define EXTERN
 int MAX_LINELENGTH = 133;
#endif



EXTERN FILE *infile, *outfile;

EXTERN int num_atoms;
EXTERN int num_coords;
EXTERN double **cart_geom;                         /*cartesian geometry matrix*/
EXTERN double **cart_grad;                         /*cartesian gradients in matrix from*/
EXTERN double **H;                                 /*inverse hessian*/
EXTERN double **H_old;                             /*last iteration inverse hessian*/
EXTERN double *grad_vec;                           /*gradient vector in coordinates used for optimization*/
EXTERN double *grad_old;                           /*last iteration gradient vector in coordinates used for optimizatoin*/
EXTERN double *coord_vec;                          /*coordinate vector that is being optimized*/
EXTERN double *coord_old;                          /*coordinate vector that is being optimized from last iteration*/
EXTERN double *atomic_nums;                        /*atomic numbers*/
 
EXTERN void punt(char *mess);
EXTERN void read_file11();
EXTERN int read_opt();
EXTERN void global_allocate();

/*this needs to be in C*/
extern "C" char *gprgid(); 



