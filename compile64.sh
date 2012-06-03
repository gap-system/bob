#!/bin/sh
unset CFLAGS
unset CXXFLAGS
./configure
make
strip bob
