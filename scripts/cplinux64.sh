#!/bin/sh
export F=bob-linux-64bit.tar.gz
export D=../bobpages
rm -rf $F
tar czvf $F bob README.md
cp $F $D
cd $D
git commit -a -m "New linux 64bit binary"
git push
