#!/bin/sh
make distclean
unset CFLAGS
unset CXXFLAGS
./configure
make
strip bob
