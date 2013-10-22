#!/bin/sh
# this is no longer used
cd .. 
tar czvf bob.tar.gz bob --exclude .git  --exclude autom4te.cache --exclude TODO --exclude .deps
scp bob.tar.gz neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/Computer/Software/Gap/bob/bob.tar.gz
