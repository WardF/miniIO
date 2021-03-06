/*
 * Copyright (c) DoD HPCMP PETTT.  All rights reserved.
 * See LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <mpi.h>

#include "netcdf.h"
#include "netcdf_par.h"
#include "netcdfstruct.h"
#include "hdf5.h"

#define NC_MAX_NAME_LENGTH 4095
#define NDIMS 3

#define ERR {if(err != NC_NOERR) {printf("(rank %d) Error at line %d: %s\n",rank,__LINE__,nc_strerror(err)); fflush(stdout); MPI_Abort(comm,1);}}

/*! Write structured grid via netCDF4 API.
 *
 * Writing a structured grid for the struct mini app.
 *
 * @param[in] num_varnames Number of variable names.
 * @param[in] varnames Variable names.
 * @param[in] comm MPI_Comm variable.
 * @param[in] rank MPI_Comm_rank variable.
 * @param[in] nprocs MPI_Comm_size variable.
 * @param[in] tstep
 * @param[in] is global index starting point.
 * @param[in] js global index starting point.
 * @param[in] ks global index starting point.
 * @param[in] ni global grid size.
 * @param[in] nj global grid size.
 * @param[in] nk global grid size.
 * @param[in] cni points in this task.
 * @param[in] cnj points in this task.
 * @param[in] cnk points in this task.
 * @param[in] data data to be written.
 * @param[in] height data to be writen.
 * @param[in] ola_mask data to be written.
 * @param[in] ol_mask data to be written.
 */
void writenc(const int num_varnames, char **varnames, MPI_Comm comm, int rank,
             int nprocs, int tstep,
             int is, int js, int ks,
             int ni, int nj, int nk, int cni, int cnj, int cnk,
             float *data, float *height, int *ola_mask, int *ol_mask) {


  char fname[NC_MAX_NAME_LENGTH +1];
  int i = 0;
  int loc_numvars = 0;
  int timedigits = 4;
  MPI_Info info = MPI_INFO_NULL;
  int err=0;
  size_t start[3] = {0,0,0};
  size_t count[3] = {0,0,0};
  size_t dimsf[3];
  size_t dimsm[3];
  int j;
  int ncid;
  int dimids[NDIMS];
  int varids[4];
  snprintf(fname, NC_MAX_NAME_LENGTH, "struct_t%0*d.nc", timedigits, tstep);

  dimsf[0] = (hsize_t)ni;
  dimsf[1] = (hsize_t)nj;
  dimsf[2] = (hsize_t)nk;
  dimsm[0] = (hsize_t)cni;
  dimsm[1] = (hsize_t)cnj;
  dimsm[2] = (hsize_t)cnk;


  err = nc_create_par(fname,NC_NETCDF4|NC_MPIIO,comm,info,&ncid);
  /*create dimensions. */
  err = nc_def_dim(ncid,"k", nk, &dimids[0]); ERR;
  err = nc_def_dim(ncid,"j", nj, &dimids[1]); ERR;
  err = nc_def_dim(ncid,"i", ni, &dimids[2]); ERR;

  for(i = 0; i < num_varnames; i++) {
    if(strcmp(varnames[i],"data") == 0 || strcmp(varnames[i],"height") == 0) {
      nc_def_var(ncid,varnames[i], NC_FLOAT, 3, &dimids[0], &varids[i]);
    } else {
      nc_def_var(ncid,varnames[i], NC_INT, 3, &dimids[0], &varids[i]);
    }
  }
  /* End definition mode & close file.*/
  err = nc_enddef(ncid); ERR;

  /* Wait for everybody to get to this point,
   * reopen file and do that thing. */
  MPI_Barrier(comm);

  MPI_Info_create(&info);

  err = nc_inq_varids(ncid,&loc_numvars,&varids[0]); ERR;

  for(i = 0; i < loc_numvars; i++) {
    err = nc_var_par_access(ncid,varids[i],NC_COLLECTIVE); ERR;
  }

  start[0] = (hsize_t)ks;
  start[1] = (hsize_t)js;
  start[2] = (hsize_t)is;
  count[0] = (hsize_t)cnk;
  count[1] = (hsize_t)cnj;
  count[2] = (hsize_t)cni;
  /* Write out the data for each variable. */
  for(i = 0; i < num_varnames; i++) {
    if(strcmp(varnames[i],"data") == 0) {
      err = nc_put_vara_float(ncid,varids[i],start,count,data); ERR;
    } else if(strcmp(varnames[i],"height") == 0) {
      err = nc_put_vara_float(ncid,varids[i],start,count,height); ERR;
    } else if(strcmp(varnames[i],"ola_mask") == 0) {
      err = nc_put_vara_int(ncid,varids[i],start,count,ola_mask); ERR;
    } else if (strcmp(varnames[i],"ol_mask") == 0) {
      err = nc_put_vara_int(ncid,varids[i],start,count,ol_mask); ERR;
    } else {
      printf("writenc error: Unknown variable %s\n",varnames[i]);
      MPI_Abort(comm,1);
    }
    MPI_Barrier(comm);
  } /* End loop to write variables. */

  MPI_Barrier(comm);
  /* Close out, lets go home. */
  err = nc_close(ncid); ERR;

}
