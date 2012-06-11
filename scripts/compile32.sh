#!/bin/sh
export CFLAGS=-m32
export CXXFLAGS=-m32
./configure
make bz2lib_m32
make
strip bob
