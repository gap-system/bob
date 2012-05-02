#!/bin/sh
make distclean
unset CFLAGS
unset CXXFLAGS
./configure
make
strip bob
scp bob neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/Computer/Software/Gap/bob/bob-linux-64bit
