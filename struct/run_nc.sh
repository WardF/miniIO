#!/bin/bash
# Utility script to easily run a netCDF file.

set -e

#mpirun --np 4 ./struct --tasks 2 2 --size 2 2 2 --tsteps 1 --nc
mpirun --np 4 ./struct --tasks 2 2 --size 2 2 2 --tsteps 10 --nc
