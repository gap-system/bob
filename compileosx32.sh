#!/bin/sh
make distclean
export CFLAGS=-m32
export CXXFLAGS=-m32
./configure
make bz2lib_m32
make
strip bob
scp bob neunhoef@schur.mcs.st-and.ac.uk/mywebpage.pub/for/BOB/bob-osx-32bit
