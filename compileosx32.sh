#!/bin/sh
make distclean
export CFLAGS=-m32
export CXXFLAGS=-m32
./configure
make bz2lib_m32
make
strip bob
scp bob neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/Computer/Software/Gap/bob/bob-osx-32bit
