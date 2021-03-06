#!/bin/bash
# Utility script to easily run a netCDF file.

set -e

mpirun --np 4 ./struct --tasks 2 2 --size 2 2 2 --tsteps 10 --hdf5
#mpirun --np 4 ./struct --tasks 2 2 --size 16 12 2 --tsteps 1 --hdf5
