#!/bin/sh
make distclean
unset CFLAGS
unset CXXFLAGS
./configure
make
strip bob
scp bob neunhoef@schur.mcs.st-and.ac.uk/mywebpage.pub/for/BOB/bob-linux-64bit
