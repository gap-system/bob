#!/bin/sh
rm -rf GAP_Installer_BOB bob-osx.dmg
mkdir GAP_Installer_BOB
cp -a README GAP_Installer_BOB
cp -a bob GAP_Installer_BOB
cp -a BOB.app GAP_Installer_BOB
hdiutil create -srcfolder GAP_Installer_BOB bob-osx.dmg
scp bob-osx.dmg neunhoef@schur.mcs.st-and.ac.uk:/scratch/neunhoef/mywebpage.pub/Computer/Software/Gap/bob/bob-osx.dmg
