/*! \file internals.h
    \ingroup (OPTKING)
    \brief Enter brief description of file here 
*/
/*** INTERNALS.H class declaration for internals ***/

#include "stretch.h"
#include "bend.h"
#include "torsion.h"
#include "out_of_plane.h"
#include "lin_bend.h"

class internals {

   int num;

  public:
   
   stretch_set stre;
   bend_set bend;
   torsion_set tors;
   out_set out;
   lin_bend_set lin_bend;
  
    int stretch_count;
    void set_stretch_count(int i) { stretch_count = i;}
    int get_stretch_count() { return stretch_count; }
    void compute_internals(int natom, double *geom);
    void compute_s(int natom, double *geom);
    void print_s();
    int get_num() { return num; }
    void set_num(int new_num) { num = new_num; }
    ~internals();

   // user_intcos = 1; read in internals
   //               0; generate internals
    internals(cartesians& carts, int user_intcos);
    internals(int *num_arr);

   // print_flag: 0   print to a file in intco.dat format
   //             1   print intcos and their values
    void print(FILE *outfile, int print_flag);

    void locate_id(int id, int *intco_type, int *sub_index);
    int index_to_id(int index);
    int id_to_index(int id);
    void fix_near_lin(void) {
      tors.fix_near_lin();
    }
};

