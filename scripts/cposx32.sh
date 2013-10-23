#!/bin/sh
export D=../bobpages
rm -rf GAP_Installer_BOB bob-osx.dmg
mkdir GAP_Installer_BOB
cp -a README.md GAP_Installer_BOB
cp -a bob GAP_Installer_BOB
cp -a BOB.app GAP_Installer_BOB
hdiutil create -srcfolder GAP_Installer_BOB bob-osx.dmg
cp bob-osx.dmg ../bobpages
cd GAP_Installer_BOB
tar czvf ../bob-osx.tar.gz bob README.md BOB.app
cp ../bob-osx.tar.gz ../$D
cd ..
cd $D
git commit -a -m "New osx archives."
git push
