#!/bin/sh
make distclean
unset CFLAGS
unset CXXFLAGS
./configure
make
strip bob
scp bob neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/for/BOB/bob-osx-64bit
