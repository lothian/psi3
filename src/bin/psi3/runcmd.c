#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

void runcmd(int *errcod, char *cmd)
{

  /*
  i = system(cmd);
  *errcod = WEXITSTATUS(i);
  */

  /* if the string is empty - do nothing */
  if (strlen(cmd) == 0) {
    *errcod = 0;
    return;
  }

  /* this version has the advantage that you can ctrl-C it */
  if (fork() == 0) {
    execl("/bin/sh","/bin/sh","-c",cmd,NULL);
    fprintf(stderr,"psi: execl returned?? Is the /bin/sh command missing?\n");
    abort();
  }

  wait(errcod);

  /*
  fprintf(stderr, "errcod in runcmd before filter is %d\n", i);
  *errcod = WEXITSTATUS(i);
  */
}

