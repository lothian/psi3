/*--- Useful and not-so-useful macro definitions ---*/
#define USE_BLAS 1

#define IOFF 32641
#define MAXIOFF3 255
#define MAX_NUM_IRREPS 8
#define INDEX(i,j) ((i>j) ? (ioff[(i)]+(j)) : (ioff[(j)]+(i)))
#define MIN(i,j) (((i) >= (j)) ? (j) : (i))
