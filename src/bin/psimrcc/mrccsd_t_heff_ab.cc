#include "blas.h"
#include "index_iterator.h"
#include "mrccsd_t.h"
#include "special_matrices.h"

namespace psi{ namespace psimrcc{

double MRCCSD_T::compute_AB_ooO_contribution_to_Heff(int u_abs,int V_abs,int x_abs,int Y_abs,int i_abs,int j_abs,int k_abs,int mu,int nu,BlockMatrix* T3)
{
  double value = 0.0;
  int    i_sym  = o->get_tuple_irrep(i_abs);
  int    j_sym  = o->get_tuple_irrep(j_abs);
  int    k_sym  = o->get_tuple_irrep(k_abs);

  int  ijk_sym  = i_sym xor j_sym xor k_sym;

  size_t i_rel  = o->get_tuple_rel_index(i_abs);


  int  x_sym    = v->get_tuple_irrep(x_abs);
  int  y_sym    = v->get_tuple_irrep(Y_abs);
  int ij_sym    = oo->get_tuple_irrep(i_abs,j_abs);
  int jk_sym    = oo->get_tuple_irrep(j_abs,k_abs);
  int uv_sym    = oo->get_tuple_irrep(u_abs,V_abs);
  int xy_sym    = vv->get_tuple_irrep(x_abs,Y_abs);

  size_t  x_rel = v->get_tuple_rel_index(x_abs);
  size_t  y_rel = v->get_tuple_rel_index(Y_abs);

  size_t ij_rel = oo->get_tuple_rel_index(i_abs,j_abs);
  size_t jk_rel = oo->get_tuple_rel_index(j_abs,k_abs);
  size_t kj_rel = oo->get_tuple_rel_index(k_abs,j_abs);
  size_t uv_rel = oo->get_tuple_rel_index(u_abs,V_abs);
  size_t xy_rel = vv->get_tuple_rel_index(x_abs,Y_abs);

  if((j_abs == u_abs) and (k_abs == V_abs)){
    CCIndexIterator  e("[v]",i_sym);
    while(++e){
      int    e_sym  = v->get_tuple_irrep(e.ind_abs[0]);
      size_t e_abs  = e.ind_abs[0];
      size_t e_rel  = v->get_tuple_rel_index(e_abs);
      if(uv_sym == xy_sym){
        value += T3->get(e_sym,e_rel,xy_rel) * F_ov[mu][i_sym][i_rel][e_rel];
      }
    }
  }
  if(i_abs == u_abs){
    CCIndexIterator  e("[v]",ijk_sym xor xy_sym);
    while(++e){
      int    e_sym  = v->get_tuple_irrep(e.ind_abs[0]);
      size_t e_rel  = v->get_tuple_rel_index(e.ind_abs[0]);
      int    ve_sym = ov->get_tuple_irrep(V_abs,e.ind_abs[0]);
      size_t ve_rel = ov->get_tuple_rel_index(V_abs,e.ind_abs[0]);
      if(jk_sym == ve_sym){
        value += T3->get(e_sym,e_rel,xy_rel) * V_oOoV[jk_sym][kj_rel][ve_rel];
      }
    }
  }
  if(k_abs == V_abs){
    CCIndexIterator  e("[v]",ijk_sym xor xy_sym);
    while(++e){
      int    e_sym  = v->get_tuple_irrep(e.ind_abs[0]);
      size_t e_rel  = v->get_tuple_rel_index(e.ind_abs[0]);
      int    ue_sym = ov->get_tuple_irrep(u_abs,e.ind_abs[0]);
      size_t ue_rel = ov->get_tuple_rel_index(u_abs,e.ind_abs[0]);
      if(ij_sym == ue_sym){
        value += 0.5 * T3->get(e_sym,e_rel,xy_rel) * V_ooov[ij_sym][ij_rel][ue_rel];
      }
    }
  }
  if((j_abs == u_abs) and (k_abs == V_abs)){
    CCIndexIterator  ef("[vv]",ijk_sym xor x_sym);
    while(++ef){
      int   ief_sym  = ovv->get_tuple_irrep(i_abs,ef.ind_abs[0],ef.ind_abs[1]);
      size_t fe_rel  = vv->get_tuple_rel_index(ef.ind_abs[1],ef.ind_abs[0]);
      size_t ief_rel = ovv->get_tuple_rel_index(i_abs,ef.ind_abs[0],ef.ind_abs[1]);

      if(y_sym == ief_sym){
        value -= T3->get(x_sym,x_rel,fe_rel) * V_vOvV[y_sym][y_rel][ief_rel];
      }
    }
  }
  if((j_abs == u_abs) and (k_abs == V_abs)){
    CCIndexIterator  ef("[vv]",ijk_sym xor x_sym);
    while(++ef){
      int      e_sym =   v->get_tuple_irrep(ef.ind_abs[0]);
      int    ief_sym = ovv->get_tuple_irrep(i_abs,ef.ind_abs[0],ef.ind_abs[1]);
      size_t   e_rel =   v->get_tuple_rel_index(ef.ind_abs[0]);
      size_t  fy_rel =  vv->get_tuple_rel_index(ef.ind_abs[1],Y_abs);
      size_t ief_rel = ovv->get_tuple_rel_index(i_abs,ef.ind_abs[0],ef.ind_abs[1]);

      if(y_sym == ief_sym){
        value -= 0.5 * T3->get(e_sym,e_rel,fy_rel) * V_vovv[x_sym][x_rel][ief_rel];
      }
    }
  }

  return value;
}

double MRCCSD_T::compute_AB_oOO_contribution_to_Heff(int u_abs,int V_abs,int x_abs,int Y_abs,int i_abs,int j_abs,int k_abs,int mu,int nu,BlockMatrix* T3)
{
  double value = 0.0;
  int    i_sym  = o->get_tuple_irrep(i_abs);
  int    j_sym  = o->get_tuple_irrep(j_abs);
  int    k_sym  = o->get_tuple_irrep(k_abs);

  int  ijk_sym  = i_sym xor j_sym xor k_sym;

  size_t k_rel  = o->get_tuple_rel_index(k_abs);


  int  x_sym    = v->get_tuple_irrep(x_abs);
  int  y_sym    = v->get_tuple_irrep(Y_abs);
  int ij_sym    = oo->get_tuple_irrep(i_abs,j_abs);
  int jk_sym    = oo->get_tuple_irrep(j_abs,k_abs);
  int uv_sym    = oo->get_tuple_irrep(u_abs,V_abs);
  int xy_sym    = vv->get_tuple_irrep(x_abs,Y_abs);

  size_t  x_rel = v->get_tuple_rel_index(x_abs);
  size_t  y_rel = v->get_tuple_rel_index(Y_abs);

  size_t ij_rel = oo->get_tuple_rel_index(i_abs,j_abs);
  size_t jk_rel = oo->get_tuple_rel_index(j_abs,k_abs);
  size_t kj_rel = oo->get_tuple_rel_index(k_abs,j_abs);
  size_t uv_rel = oo->get_tuple_rel_index(u_abs,V_abs);
  size_t xy_rel = vv->get_tuple_rel_index(x_abs,Y_abs);

  if((i_abs == u_abs) and (j_abs == V_abs)){
    CCIndexIterator  e("[v]",k_sym);
    while(++e){
      size_t  e_rel  = v->get_tuple_rel_index(e.ind_abs[0]);
      size_t ye_rel  = vv->get_tuple_rel_index(Y_abs,e.ind_abs[0]);
      if(uv_sym == xy_sym){
        value += T3->get(x_sym,x_rel,ye_rel) * F_OV[mu][k_sym][k_rel][e_rel];
      }
    }
  }
  if(i_abs == u_abs){
    CCIndexIterator  e("[v]",ijk_sym xor xy_sym);
    while(++e){
      int    ve_sym = ov->get_tuple_irrep(V_abs,e.ind_abs[0]);
      size_t ve_rel = ov->get_tuple_rel_index(V_abs,e.ind_abs[0]);
      size_t ye_rel  = vv->get_tuple_rel_index(Y_abs,e.ind_abs[0]);
      if(jk_sym == ve_sym){
        value -= 0.5 * T3->get(x_sym,x_rel,ye_rel) * V_ooov[jk_sym][jk_rel][ve_rel];
      }
    }
  }
  if(k_abs == V_abs){
    CCIndexIterator  e("[v]",ijk_sym xor xy_sym);
    while(++e){
      int    ue_sym = ov->get_tuple_irrep(u_abs,e.ind_abs[0]);
      size_t ue_rel = ov->get_tuple_rel_index(u_abs,e.ind_abs[0]);
      size_t ye_rel = vv->get_tuple_rel_index(Y_abs,e.ind_abs[0]);
      if(ij_sym == ue_sym){
        value += T3->get(x_sym,x_rel,ye_rel) * V_oOoV[ij_sym][ij_rel][ue_rel];
      }
    }
  }
  if((i_abs == u_abs) and (j_abs == V_abs)){
    CCIndexIterator  ef("[vv]",ijk_sym xor x_sym);
    while(++ef){
      int   kef_sym  = ovv->get_tuple_irrep(k_abs,ef.ind_abs[0],ef.ind_abs[1]);
      size_t ef_rel  = vv->get_tuple_rel_index(ef.ind_abs[0],ef.ind_abs[1]);
      size_t kef_rel = ovv->get_tuple_rel_index(k_abs,ef.ind_abs[0],ef.ind_abs[1]);
      if(y_sym == kef_sym){
        value += 0.5 * T3->get(x_sym,x_rel,ef_rel) * V_vovv[y_sym][y_rel][kef_rel];
      }
    }
  }
  if((i_abs == u_abs) and (j_abs == V_abs)){
    CCIndexIterator  ef("[vv]",ijk_sym xor x_sym);
    while(++ef){
      int      e_sym =   v->get_tuple_irrep(ef.ind_abs[0]);
      int   kef_sym  = ovv->get_tuple_irrep(k_abs,ef.ind_abs[0],ef.ind_abs[1]);
      size_t   e_rel =   v->get_tuple_rel_index(ef.ind_abs[0]);
      size_t  yf_rel =  vv->get_tuple_rel_index(Y_abs,ef.ind_abs[1]);
      size_t ief_rel = ovv->get_tuple_rel_index(i_abs,ef.ind_abs[0],ef.ind_abs[1]);

      size_t kef_rel = ovv->get_tuple_rel_index(k_abs,ef.ind_abs[0],ef.ind_abs[1]);

      if(x_sym == kef_sym){
        value += T3->get(e_sym,e_rel,yf_rel) * V_vOvV[x_sym][x_rel][kef_rel];
      }
    }
  }

  return value;
}


}} /* End Namespaces */
