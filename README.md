# High-Performance Thermo-Hydro-Mechanical Code (CB-Geo thm)
> [CB-Geo Computational Geomechanics Research Group](https://www.cb-geo.com)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/cb-geo/thm/develop/license.md)
[![Developer docs](https://img.shields.io/badge/developer-docs-blue.svg)](http://cb-geo.github.io/thm)
[![User docs](https://img.shields.io/badge/user-docs-blue.svg)](https://thm.cb-geo.com/)
[![CircleCI](https://circleci.com/gh/cb-geo/thm.svg?style=svg)](https://circleci.com/gh/cb-geo/thm)
[![codecov](https://codecov.io/gh/cb-geo/thm/branch/develop/graph/badge.svg)](https://codecov.io/gh/cb-geo/thm)
[![](https://img.shields.io/github/issues-raw/cb-geo/thm.svg)](https://github.com/cb-geo/thm/issues)
[![Project management](https://img.shields.io/badge/projects-view-ff69b4.svg)](https://github.com/cb-geo/thm/projects/)

## Documentation

Please refer to [CB-Geo thm Documentation](https://cb-geo.github.io/thm-doc) for information on compiling, and running the code. The documentation also include the thm theory.

## Install dependencies

* Docker image for CB-Geo thm code [https://hub.docker.com/r/cbgeo/thm](https://hub.docker.com/r/cbgeo/thm)

* Instructions for running thm docker container: [https://github.com/cb-geo/docker-thm/blob/master/README.md](https://github.com/cb-geo/thm-container/blob/master/README.md).

### Prerequisite packages
> The following prerequisite packages can be found in the docker image:

* [Boost](http://www.boost.org/)
* [Dealii](https://dealii.org)

## Compile
> See https://thm-doc.cb-geo.com/ for more detailed instructions. 

0. Run `mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++  -DDEAL_II_DIR=/path/to/dealii/ ..`

1. Run `make clean && make -jN` (where N is the number of cores).

## Compile on TACC
```
export CC=icc
export CXX=icpc

# Compile PETSc 3.13
wget http://ftp.mcs.anl.gov/pub/petsc/release-snapshots/petsc-3.13.3.tar.gz 
tar -xvzf petsc-3.13.3.tar.gz
cd petsc-3.13.3/ 
export PETSC_ARCH=clx
export PETSC_DIR=$HOME/petsc-3.13.3
./config/configure.py --with-shared=1 --with-x=0 --with-mpi=1 --with-debugging=0
make PETSC_DIR=$HOME/petsc-3.13.3 PETSC_ARCH=clx all -j4
make PETSC_DIR=$HOME/petsc-3.13.3 PETSC_ARCH=clx check -j4

# METIS
cd $HOME
wget http://glaros.dtc.umn.edu/gkhome/fetch/sw/metis/metis-5.1.0.tar.gz && \
    tar -xf metis-5.1.0.tar.gz && \
    cd metis-5.1.0/ && \
    make config shared=1 cc=icc cxx=icpc prefix=$HOME/metis && \
    make install -j4 && cd ..

# Load module boost
module load boost

# Clone and compile dealii
cd $HOME
git clone https //github.com/dealii/dealii --depth=1 dealii-src
cd dealii-src/ && mkdir build &&  cd build
cmake -DCMAKE_INSTALL_PREFIX=$HOME/dealii -DPETSC_DIR=$PETSC_DIR -DPETSC_ARCH=clx -DDEAL_II_WITH_PETSC=On  -DDEAL_II_WITH_METIS=On -DMETIS_DIR=$HOME/metis/ -DDEAL_II_WITH_MPI=On ..
make install -j4


# Clone THM
cds
git clone https://github.com/cb-geo/thm
cd thm
git checkout thm_seg_parallel
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DDEAL_II_DIR=$HOME/dealii/ ..
make -j
```