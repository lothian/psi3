#include "model_space.h"
#include "moinfo.h"
#include <cstdio>

extern FILE *outfile;

namespace psi {

ModelSpace::ModelSpace(MOInfo* moinfo_obj_) : moinfo_obj(moinfo_obj_)
{
  startup();
  build();
  classify();
//  print();
}

ModelSpace::~ModelSpace()
{
  cleanup();
}

void ModelSpace::startup()
{
  wfn_sym = moinfo_obj->get_wfn_sym();
}

void ModelSpace::cleanup()
{
}

void ModelSpace::print()
{
  fprintf(outfile,"\n\n  Model space:");
  fprintf(outfile,"\n  ------------------------------------------------------------------------------");
  for(int mu = 0; mu < determinants.size(); ++mu){
    fprintf(outfile,"\n  %2d %s",mu,determinants[mu].get_label().c_str());
  }
  fprintf(outfile,"\n\n  Closed-shell to model space mapping");
  for(int mu = 0; mu < closed_to_all.size(); ++mu){
    fprintf(outfile,"\n  %d -> %d",mu,closed_to_all[mu]);
  }
  fprintf(outfile,"\n\n  Open-shell to model space mapping");
  for(int mu = 0; mu < opensh_to_all.size(); ++mu){
    fprintf(outfile,"\n  %d -> %d",mu,opensh_to_all[mu]);
  }

}

}
