#!/bin/sh
export F=bob-linux-32bit-new.tar.gz
export D=../bobpages
rm -rf $F
tar czvf $F bob README
cp $F $D
cd $D
git commit -a -m "New linux 32bit-new binary"
git push
