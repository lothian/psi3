/* MOCUBE - make Gaussian-compatible cube file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libipv1/ip_lib.h>
#include <libchkpt/chkpt.h>
#include <libciomr/libciomr.h>
#include <libqt/qt.h>
#include "mocube.h"
#define DEFAULT_BORDER (4.0)
#define DEFAULT_NGRID (80)
#define MAX(I,J) ((I>J) ? I : J)
#define MIN(I,J) ((I<J) ? I : J)

char *progid;
void init_io(int argc, char *argv[]);
void get_params(void);
void setup_delta(double **scf, double **u);
void compute_mos(double *movals, double x, double y, double z,
    double **scf, double **u);
void wrt_cube(void);
void exit_io(void);
FILE *infile, *outfile, *cubfile;
char *psi_file_prefix;

int main(int argc, char *argv[]) {
  int i,j, nirreps, cnt_x, cnt_y, cnt_z;
  double x, y, z, *movals, **scf, **u;
  char *filename;

  init_io(argc, argv);
  fprintf(outfile,"\n\t*****************************************************\n");
  fprintf(outfile,  "\t*  MOCUBE: Produces a Gaussian compatible .cub file *\n");
  fprintf(outfile,  "\t*****************************************************\n");

  get_params();
  strcpy(cube.title,"Title Card Provided!");
  strcpy(cube.subtitle,"Alpha MO grid values generated by PSI3's mocube");

  /* read zvals and geometry from chkpt */
  chkpt_init(PSIO_OPEN_OLD);
  cube.zvals = chkpt_rd_zvals();
  cube.geom = chkpt_rd_geom();
  cube.natom = params.natom;
  chkpt_close();

  /* print_mat(cube.geom, cube.natom, 3, stdout); */

  /* determine output filename */
  if (ip_exist("CUBFILE",0))
    ip_string("CUBFILE", &filename, 0);
  else {
    filename = (char *) malloc(20*sizeof(char *));
    sprintf(filename , "mos.cub");
  }
  ffile(&cubfile, filename, 0);
  fprintf(outfile,"Writing to cubfile %s\n", filename);

  /*** determine location of grid points ***/
    /* determine border size */
  cube.border = DEFAULT_BORDER; 
  if (ip_exist("BORDER",0)) ip_data("BORDER","%lf",&(cube.border),0);

    /* determine start/end of grid */
  for (i=0; i<3; ++i) {
    cube.grid_start[i] = 0.0;
    cube.grid_end[i] = 0.0;
  }
  for (i=0; i<cube.natom; ++i) {
    for (j=0; j<3; ++j) {
      cube.grid_start[j] = MIN(cube.grid_start[j], cube.geom[i][j]);
      cube.grid_end[j] = MAX(cube.grid_end[j], cube.geom[i][j]);
    }
  }
    /* include molecule plus border in box */
  for (i=0; i<3; ++i) {
    cube.grid_start[i] -= cube.border;
    cube.grid_end[i] += cube.border;
  }
  fprintf(outfile,"Grid starts at: %10.5lf %10.5lf %10.5lf\n",
      cube.grid_start[0], cube.grid_start[1], cube.grid_start[2]);
  fprintf(outfile,"Grid ends at  : %10.5lf %10.5lf %10.5lf\n",
      cube.grid_end[0], cube.grid_end[1], cube.grid_end[2]);

    /* determine number of grid points */
  if (ip_exist("NGRID",0)) {
    ip_data("NGRID", "%d", &j, 0);
    for (i=0;i<3;++i) cube.ngrid[i] = j;
  }
  else {
    for (i=0;i<3;++i) cube.ngrid[i] = DEFAULT_NGRID;
  }
  fprintf(outfile,"Number of grid points: %d %d %d\n",
      cube.ngrid[0], cube.ngrid[1], cube.ngrid[2]);

    /* calculate step_size */
  for (i=0;i<3;++i)
    cube.step_size[i] = (cube.grid_end[i]-cube.grid_start[i])
      /( cube.ngrid[i] - 1 );
  fprintf(outfile,"Step sizes between grid points: %10.5lf %10.5lf %10.5lf\n",
      cube.step_size[0], cube.step_size[1], cube.step_size[2]);

  fflush(outfile);

  /* prepare for numerous compute_mos() calls */
  chkpt_init(PSIO_OPEN_OLD);
  u = chkpt_rd_usotao();
  scf = block_matrix(params.nmo, params.nmo);
  chkpt_close();
  setup_delta(scf, u); /* also determines HOMO,LUMO */

  /* determine which MOS to plot */
  if (ip_exist("CUBEMOS",0)) {
    ip_count("CUBEMOS", &(cube.nmo_to_plot), 0);
    cube.mos_to_plot = (int *) malloc( cube.nmo_to_plot * sizeof(int) );

    for (i=0; i<cube.nmo_to_plot; ++i) {
      ip_data("CUBEMOS","%d", &(cube.mos_to_plot[i]), 1, i);
      cube.mos_to_plot[i] -= 1;
    }
  }
  else { /* do HOMO and LUMO */
    cube.nmo_to_plot = 2;
    cube.mos_to_plot = (int *) malloc( cube.nmo_to_plot * sizeof(int) );
    cube.mos_to_plot[0] = params.homo;
    cube.mos_to_plot[1] = params.lumo;
  }
  fprintf(outfile,"MOs to plot: ");
  for (i=0; i<cube.nmo_to_plot; ++i)
    fprintf(outfile,"%d ", cube.mos_to_plot[i] + 1);
  fprintf(outfile,"\n");

  /* malloc memory for grid */
  cube.grid = (double ****) malloc(cube.ngrid[0] * sizeof(double ***));
  for (cnt_x=0; cnt_x<cube.ngrid[0]; ++cnt_x) {
    cube.grid[cnt_x] = (double ***) malloc(cube.ngrid[1] * sizeof(double **));
    for (cnt_y=0; cnt_y<cube.ngrid[1]; ++cnt_y)
      cube.grid[cnt_x][cnt_y] = block_matrix(cube.ngrid[2], cube.nmo_to_plot);
  }

  /* calculate MO values at grid points */
  movals = init_array(cube.nmo_to_plot);
  x = cube.grid_start[0];
  for (cnt_x=0; cnt_x<cube.ngrid[0]; ++cnt_x, x += cube.step_size[0]) {
    y = cube.grid_start[1];
    for (cnt_y=0; cnt_y<cube.ngrid[1]; ++cnt_y, y += cube.step_size[1]) {
      z = cube.grid_start[2];
      for (cnt_z=0; cnt_z<cube.ngrid[2]; ++cnt_z, z += cube.step_size[2]) {
        compute_mos(movals, x, y, z, scf, u);
        for (i=0; i<cube.nmo_to_plot; ++i)
          cube.grid[cnt_x][cnt_y][cnt_z][i] = movals[i];
      }
    }
  }
  free(movals);

  wrt_cube();
  free_block(u);
  free_block(scf);
  exit_io();
  return 0 ;
}

char *gprgid() {
  char *prgid = "MOCUBE";
  return (prgid);
}

void init_io(int argc, char *argv[]) {
  extern char *gprgid();
  char *filename;
  progid = (char *) malloc(strlen(gprgid())+2);
  sprintf(progid, ":%s",gprgid());

  psi_start(argc-1,argv+1,0);
  ip_cwk_add(":INPUT");
  ip_cwk_add(progid);
  psio_init();
  tstart(outfile);
}

void exit_io(void) {
  int i;
/*  psio_close(32,1); */
  psio_done();
  tstop(outfile);
  ip_done();
}

void get_params(void) {
  int i, irrep_max, index_max;

  chkpt_init(PSIO_OPEN_OLD);
  params.nao = chkpt_rd_nao();
  params.natom = chkpt_rd_natom();
  params.nirreps = chkpt_rd_nirreps();
  params.nirreps_present = chkpt_rd_nsymhf();
  params.nprim = chkpt_rd_nprim();
  params.nso = chkpt_rd_nso();
  params.nmo = chkpt_rd_nmo();
  params.clsdpi = chkpt_rd_clsdpi(); 
  params.label = chkpt_rd_label();
  params.openpi = chkpt_rd_openpi();
  params.orbspi = chkpt_rd_orbspi();

  for (i=0; i<params.nirreps; ++i)
    params.nclsd += params.clsdpi[i];

  chkpt_close();
}

/* this function writes a cubfile in Gaussian .cub format */
void wrt_cube(void) {
  int i,j,k,cnt,x,y,z;

  fprintf(cubfile," %s\n",cube.title);
  fprintf(cubfile," %s\n",cube.subtitle);
  fprintf(cubfile,"%5d%12.6lf%12.6lf%12.6lf\n",-1*cube.natom,
      cube.grid_start[0], cube.grid_start[1], cube.grid_start[2]);
  fprintf(cubfile,"%5d%12.6lf%12.6lf%12.6lf\n", cube.ngrid[0],
      cube.step_size[0], 0.0, 0.0);
  fprintf(cubfile,"%5d%12.6lf%12.6lf%12.6lf\n", cube.ngrid[1],
      0.0, cube.step_size[1], 0.0);
  fprintf(cubfile,"%5d%12.6lf%12.6lf%12.6lf\n", cube.ngrid[2],
      0.0, 0.0, cube.step_size[2]);

  for (i=0; i<params.natom; ++i) {
    fprintf(cubfile,"%5d%12.6lf%12.6lf%12.6lf%12.6lf\n", (int) cube.zvals[i],
      cube.zvals[i], cube.geom[i][0], cube.geom[i][1], cube.geom[i][2]);
  }
  cnt = 0;
  fprintf(cubfile,"%5d",cube.nmo_to_plot);
  ++cnt;
  for (i=0; i<cube.nmo_to_plot; ++i) {
    fprintf(cubfile,"%5d", cube.mos_to_plot[i] + 1);
    ++cnt;
    if ( (cnt == 8) && (i<cube.nmo_to_plot) ) {
      fprintf(cubfile,"\n");
      cnt = 0;
    }
  }
  fprintf(cubfile,"\n");

  /* write out grid points */
  for (x=0; x<cube.ngrid[0]; ++x) {
    for (y=0; y<cube.ngrid[1]; ++y) {
      cnt = 0;
      for (z=0; z<cube.ngrid[2]; ++z) {
        for (i=0; i<cube.nmo_to_plot; ++i) {
          fprintf(cubfile," %12.5E", cube.grid[x][y][z][i]);
          ++cnt;
          if (cnt == 6) {
            fprintf(cubfile,"\n");
            cnt = 0;
          }
        }
      }
      if (cnt != 0) fprintf(cubfile,"\n");
    }
  }

  fclose(cubfile);
  return;
}

void setup_delta(double **scf, double **u)
{
  int nmo, nao, i, I, j, errcod;
  int nirreps, nfzc, nfzv;
  int *order, *clsdpi, *openpi, *orbspi, *fruocc, *frdocc;
  double **scf_pitzer, *evals_pitzer, *evals, tval;

  nmo = params.nmo;
  nao = params.nao;
  nirreps = params.nirreps;
  orbspi = params.orbspi;
  openpi = params.openpi;
  clsdpi = params.clsdpi;

  /* read SCF eigenvectors in Pitzer ordering */
  chkpt_init(PSIO_OPEN_OLD);
  scf_pitzer = chkpt_rd_scf();
  /* u = chkpt_rd_usotao(); */
  evals_pitzer = chkpt_rd_evals();
  chkpt_close();

  frdocc = init_int_array(nirreps);
  fruocc = init_int_array(nirreps);
  errcod = ip_int_array("FROZEN_DOCC", frdocc, nirreps);
  errcod = ip_int_array("FROZEN_UOCC", fruocc, nirreps);

  nfzc = nfzv = 0;
  for(i=0; i < nirreps; i++) { 
    nfzc += frdocc[i];
    nfzv += fruocc[i];
  }

  /*** Get the Pitzer -> QT reordering array ***/
  order = init_int_array(nmo);
  reorder_qt(clsdpi, openpi, frdocc, fruocc, order, orbspi, nirreps);

  /*** Arrange the SCF eigenvectors into QT ordering ***/
 /* scf = block_matrix(nmo, nmo); */
  evals = (double *) malloc(nmo * sizeof(double));
  for(i=0; i < nmo; i++) {
      I = order[i];  /* Pitzer --> QT */
      for(j=0; j < nmo; j++) scf[j][I] = scf_pitzer[j][i];
      evals[I] = evals_pitzer[i];
    }

  /* determine HOMO and LUMO */
  tval = -1.0E6;
  for (i=0; i<params.nclsd; ++i) {
    if (evals[i] > tval) {
        tval = evals[i];
        params.homo = i;
    }
  }

  tval = 1.0E6;
  for (i=params.nclsd; i<params.nmo; ++i) {
    if (evals[i] < tval) {
        tval = evals[i];
        params.lumo = i ;
    }
  }
  fprintf(outfile,"orbitals in QT ordering, HOMO: %d LUMO: %d\n",
      params.homo+1, params.lumo+1);

  free(evals);
  free(evals_pitzer);

  free(order);
  /* free(clsdpi);
  free(openpi);
  free(orbspi); */
  free(fruocc);
  free(frdocc);
  free_block(scf_pitzer);

  return;
}

