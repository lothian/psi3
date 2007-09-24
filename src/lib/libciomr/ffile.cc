/*!
** \file ffile.cc
** \ingroup (CIOMR)
*/ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psifiles.h>

extern "C" {

extern char *psi_file_prefix;
/*!
** ffile(): Open a PSI3 ASCII file for reading/writing.  Returns a
** pointer to the new file.
**
** \param suffix = name of the file, not including automatic prefix
** \param code = 0 (write), 1 (write/append), 2 (read)
** \ingroup (CIOMR)
*/
void ffile(FILE **fptr, char *suffix, int code)
{
  char name[100];

  /* build the standard file name */
  sprintf(name, "%s.%s", psi_file_prefix, suffix);

  switch (code) {
  case 0:
    *fptr = fopen(name,"w+");
    break;
  case 1:
    *fptr = fopen(name,"a+");
    break;
  case 2:
    *fptr = fopen(name,"r+");
    break;
  default:
    fprintf(stderr,"error in ffile: invalid code %d\n",code);
  }
  if (*fptr == NULL) { 
    fprintf(stderr,"error in ffile: cannot open file %s\n", suffix);
    exit(PSI_RETURN_FAILURE);
  }
}

/*!
** ffile_noexit(): Open a PSI3 ASCII file for reading/writing.
** Returns a pointer to the new file via an argument.  This function
** is the same as ffile(), but will not exit if fopen() fails.
**
** \param suffix = name of the file, not including automatic prefix
** \param code = 0 (write), 1 (write/append), 2 (read)
** \ingroup (CIOMR)
*/
void ffile_noexit(FILE **fptr, char *suffix, int code)
{
  char name[100];

  /* build the standard file name */
  sprintf(name, "%s.%s", psi_file_prefix, suffix);

  switch (code) {
  case 0:
    *fptr = fopen(name,"w+");
    break;
  case 1:
    *fptr = fopen(name,"a+");
    break;
  case 2:
    *fptr = fopen(name,"r+");
    break;
  default:
    fprintf(stderr,"error in ffile_noexit: invalid code %d\n",code);
  }
}

/*!
** ffileb(): Open a PSI3 binary file for reading/writing.  Returns a
** pointer to the new file.
**
** \param suffix = name of the file, not including automatic prefix
** \param code = 0 (write), 1 (write/append), 2 (read)
** \ingroup (CIOMR)
*/
void ffileb(FILE **fptr, char *suffix, int code)
{
  char* name = (char*) malloc( (strlen(psi_file_prefix) + strlen(suffix) + 2)*sizeof(char) );

  /* build the standard file name */
  sprintf(name, "%s.%s", psi_file_prefix, suffix);

  switch (code) {
  case 0:
    *fptr = fopen(name,"wb");
    break;
  case 1:
    *fptr = fopen(name,"ab");
    break;
  case 2:
    *fptr = fopen(name,"rb");
    break;
  default:
    fprintf(stderr,"error in ffileb: invalid code %d\n",code);
  }
  free(name);

  if (*fptr == NULL) {
    fprintf(stderr,"error in ffileb: cannot open file %s\n", suffix);
    exit(PSI_RETURN_FAILURE);
  }
}

/*!
** ffileb_noexit(): Open a PSI3 binary file for reading/writing.
** Returns a pointer to the new file via an argument.  This function
** is the same as ffileb(), but will not exit if fopen() fails.
**
** \param suffix = name of the file, not including automatic prefix
** \param code = 0 (write), 1 (write/append), 2 (read)
** \ingroup (CIOMR)
*/
void ffileb_noexit(FILE **fptr, char *suffix, int code)
{
  char* name = (char*) malloc( (strlen(psi_file_prefix) + strlen(suffix) + 2)*sizeof(char) );

  /* build the standard file name */
  sprintf(name, "%s.%s", psi_file_prefix, suffix);

  switch (code) {
  case 0:
    *fptr = fopen(name,"wb");
    break;
  case 1:
    *fptr = fopen(name,"ab");
    break;
  case 2:
    *fptr = fopen(name,"rb");
    break;
  default:
    fprintf(stderr,"error in ffileb_noexit: invalid code %d\n",code);
  }
  free(name);
}

} /* extern "C" */
