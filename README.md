GAP source installer BOB
========================

What is BOB?
------------

This program downloads, extracts, compiles and configures GAP 4.7 on
your machine. BOB is written in C++ and comes as a precompiled binary
for your operating system and architecture (see
[table](#downloadtab) below).

It has builtin support for downloading and extracting archives and it
uses the C-compiler, development tools and libraries on your system to
compile GAP and its packages. BOB will first check whether everything it
needs from your system is installed and if not it will warn you and tell
you what is needed. In the end, a status report and some log files will
be produced such that you can easily see whether or not everything went
well.

How to use BOB?
---------------

First you might want to look at Section [Installing the
prerequisites](#install) below. 
There are installation instructions for
the prerequisites, depending on your operating system. However, you can
also ignore this for now and just move on.

Simply download the archive for your operating system and architecture
from the table below. For some combinations there is an alternative
binary available, try this if the first one fails to execute. Currently,
we support BOB for machines with Intel/AMD processors running Linux
or Mac OS X, both on 64-bit (architecture "x86\_64") and 32-bit
(architecture "i686"). If you are in doubt about your machine, try
`uname -a` or `gcc -v` and look for "Linux", "Darwin", "x86\_64", "i686"
or "i386".

On Linux, copy the executable to an empty directory on a filesystem with
at least 1,8GB of free space and run the executable. On Mac OS X click
on the Applescript application and choose such a directory. The path
name of the directory must not contain spaces.

In either case, if any important tool is missing you get an immediate
diagnosis and hints on how to fix the problems. If all necessary tools
are there, after 10-30 minutes (depending on the speed of your internet
connection, hard disk and computer), you will see a status report about
the installation process and hopefully have a fully functional GAP
4.7 in the `gap4r7` directory of the current directory. Additionally,
you have a few startup scripts to start GAP in the current directory.
You can copy them to an arbitrary place in your PATH to complete the
installation. If you want, you can remove the directory "bobdownloads"
to save disk space, it is not needed after the installation.

Motivation
----------

Compiling GAP with all its packages has become more difficult over
time. This is, first of all, because there are a lot more packages.
Secondly, the libraries and programs on your system that GAP needs have
become more numerous. Thirdly, the number of different combinations
of operating systems and C-compilers is quite high and some require
special attention. Last but not least, the question of 32-bit or 64-bit
word size poses further difficulties. This all makes a pure source
distribution of GAP rather user unfriendly.

On the other hand, providing precompiled binaries for the various
variants of Linux and Mac OS X is not really feasible. Although this
would be the best way from a user's perspective, the GAP group would
probably be struggling to realise this.

This program BOB tries to make a compromise between these two approaches
by offering convenience for the user whilst still working with a source
distribution.

###Advantages of this way to install GAP:

 * Hassle free full installation of GAP 4.7.
 * Nearly all packages compiled for which this is necessary.
 * Dynamic linking against libraries on your system.
 * Good and quick reports about missing prerequisites.
 * Hints to fix missing prerequisites.

###Disadvantages of this way to install GAP:

 * Takes some time, depending on the speed of your internet connection,
   hard disk and computer.
 * Limited choice of special compilers and compilation flags.
 * Limited choice of special compilation options for GAP.

<a name="downloadtab"/>
###Download binary:

<table>
<tr>
<th>Operating System</th>
<th>Architecture</th>
<th>Link to binary</th>
<th>Alternative binary</th>
</tr>
<tr>
<td>Linux</td>
<td>x86\_64 (64-bit)</td>
<td><a href="http://gap-system.github.io/bob/bob-linux-64bit.tar.gz">
    bob-linux-64bit.tar.gz</a></td>
<td><a href="http://gap-system.github.io/bob/bob-linux-64bit-new.tar.gz">
    bob-linux-64bit-new.tar.gz</a></td>
</tr>
<tr>
<td>Linux</td>
<td>i686 (32-bit)</td>
<td><a href="http://gap-system.github.io/bob/bob-linux-32bit.tar.gz">
    bob-linux-32bit.tar.gz</a></td>
<td><a href="http://gap-system.github.io/bob/bob-linux-32bit-new.tar.gz">
    bob-linux-32bit-new.tar.gz</a></td>
</tr>
<tr>
<td>Mac OS X</td>
<td>Intel</td>
<td><a href="http://gap-system.github.io/bob/bob-osx.dmg">
    bob-osx.dmg</a></td>
<td><a href="http://gap-system.github.com/bob/bob-osx.tar.gz">
    bob-osx.tar.gz</a></td>
</tr>
</table>

If you want to build BOB from source, just clone the repository and
have a go. However, I do not intend to give support for this. For me, it
is enough to be able to build BOB myself on Linux and Mac OS X.

<a name="install"/>
Installing the prerequisites
----------------------------

Here we present simple commands to install all required software to
compile GAP on a machine running x86\_64 Linux or Mac OS X:

###For debian-based Linux, do as root (or using sudo):

<pre>
  # apt-get install gcc make m4 libc6-dev libreadline-dev \
                       lib32readline5-dev libncurses-dev lib32ncurses5-dev 
                       mawk wget libx11-dev libxt-dev libxaw7-dev
</pre>

On a 64bit-machine you probably also want to do:

<pre>
  # apt-get install g++-multilib gcc-multilib
</pre>

###For rpm-based Linux with yum, do as root (or using sudo):

<pre>
  # yum install gcc make m4 glibc-devel readline-devel \
               libc-devel.i686 readline-devel.i686 ncurses-devel \
                n glibc-devel.i686 readline-devel.i686 ncurses-devel \
                ncurses-devel.i686 mawk wget \
                libX11-devel libXaw-devel libXmu-devel libXt-devel \
                libXext-devel libSM-devel libICE-devel
</pre>

###For Mac OS X using MacPorts:

If you haven't installed the Apple Developer Tools (aka "XCode") yet
(i.e., if your boot disk does not have a folder "Developer"), you must
install them before running BOB (see here for instructions). Note
that if you are on Mac OS X 10.7 or later, then after downloading and
installing Xcode, you need to perform one extra step: launch Xcode, then
open its Preferences dialog, and go to the "Downloads" pane. You will be
presented with an optional list of extra components. From there, install
the "Command Line tools" component. For Mac OS X 10.6 or earlier, some
(possibly already outdated) version of Xcode may be contained on your
Mac OS X Installer DVD.

Then you have to install MacPorts (see this page). Finally you can do:

<pre>
  % sudo port install readline +universal ncurses +universal gawk wget xorg-libX11 xorg-libXaw
</pre>

to install the required additional libraries and tools.

###For Mac OS X using Fink:

If you haven't installed the Apple Developer Tools (aka "XCode") yet
(i.e., if your boot disk does not have a folder "Developer"), you must
install them before running BOB (see here for instructions). Note
that if you are on Mac OS X 10.7 or later, then after downloading and
installing Xcode, you need to perform one extra step: launch Xcode, then
open its Preferences dialog, and go to the "Downloads" pane. You will be
presented with an optional list of extra components. From there, install
the "Command Line tools" component. For Mac OS X 10.6 or earlier, some
(possibly already outdated) version of Xcode may be contained on your
Mac OS X Installer DVD.

Then you have to install Fink (see this page). We recommend to do a
64bit-only install. Finally you can do:

<pre>
  % fink install readline5 libncurses5 gawk wget x11-dev 
</pre>
to install the required additional libraries and tools. Note that this
does not install 32bit-libraries and so the 32-bit compile of GAP will
not be fully functional.

Please send any questions, comments or bug reports to me:
```
max AT 9hoeffer.de
```

