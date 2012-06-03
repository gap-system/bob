#!/bin/sh
rm -rf dmgdir bob-osx.dmg
cp -a bob dmgdir
cp -a BOB.app dmgdir
hdiutil create -srcfolder dmgdir bob-osx.dmg
scp bob-osx.dmg neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/Computer/Software/Gap/bob/bob-osx.dmg
