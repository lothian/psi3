#include <math.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <ip_libv1.h>
#include <iwl.h>
#include <libciomr.h>
#include <libint.h>
#include <libderiv.h>
#include <pthread.h>
#include "defines.h"
#define EXTERN
#include "global.h"
#include "int_fjt.h"
#include "small_fns.h"

extern void *te_deriv1_scf_thread(void *);
extern pthread_mutex_t deriv1_mutex;
extern double **grad_te;

void te_deriv1_scf()
{
  pthread_attr_t thread_attr;
  pthread_t *thread_id;
  
  int i;

  /*---------------
    Initialization
   ---------------*/
  init_fjt(BasisSet.max_am*4+DERIV_LVL);
  init_libderiv_base();
  grad_te = block_matrix(Molecule.num_atoms,3);

  thread_id = (pthread_t *) malloc(UserOptions.num_threads*sizeof(pthread_t));
  pthread_attr_init(&thread_attr);
  pthread_attr_setscope(&thread_attr,
			PTHREAD_SCOPE_SYSTEM);
  pthread_mutex_init(&deriv1_mutex,NULL);
  for(i=0;i<UserOptions.num_threads-1;i++)
    pthread_create(&(thread_id[i]),&thread_attr,
		   te_deriv1_scf_thread,(void *)i);
  te_deriv1_scf_thread( (void *) (UserOptions.num_threads - 1) );
  for(i=0;i<UserOptions.num_threads-1;i++)
    pthread_join(thread_id[i], NULL);
  free(thread_id);
  
  if (UserOptions.print_lvl >= PRINT_TEDERIV)
    print_atomvec("Two-electron contribution to the forces (a.u.)",grad_te);

  for(i=0;i<Molecule.num_atoms;i++) {
    Grad[i][0] += grad_te[i][0];
    Grad[i][1] += grad_te[i][1];
    Grad[i][2] += grad_te[i][2];
  }
  
  /*---------
    Clean-up
   ---------*/
  free_block(grad_te);
  free_fjt();

  return;
}



