AC_PREREQ(2.68)
AC_INIT([bob], 0.1, neunhoef@mcs.st-and.ac.uk)
AC_CONFIG_SRCDIR([bob.cc])
AC_CONFIG_AUX_DIR([cnf])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_CANONICAL_HOST
case "$host_os" in
   *cygwin*)
      AC_SUBST([SYSTEMINFO],
       ["-DSYS_IS_WINDOWS=1 -DSYS_IS_OSX=0 -DSYS_IS_LINUX=0 -DSYS_IS_BSD=0"])
        ;;
    *darwin*)
      AC_SUBST([SYSTEMINFO],
       ["-DSYS_IS_WINDOWS=0 -DSYS_IS_OSX=1 -DSYS_IS_LINUX=0 -DSYS_IS_BSD=0"])
        ;;
    *linux*)
      AC_SUBST([SYSTEMINFO],
       ["-DSYS_IS_WINDOWS=0 -DSYS_IS_OSX=0 -DSYS_IS_LINUX=1 -DSYS_IS_BSD=0"])
        ;;
    *bsd*)
      AC_SUBST([SYSTEMINFO],
       ["-DSYS_IS_WINDOWS=0 -DSYS_IS_OSX=0 -DSYS_IS_LINUX=0 -DSYS_IS_BSD=1"])
        ;;
    *)
      AC_SUBST([SYSTEMINFO],
       ["-DSYS_IS_WINDOWS=0 -DSYS_IS_OSX=0 -DSYS_IS_LINUX=0 -DSYS_IS_BSD=0"])
        ;;
esac
AC_SUBST([HOSTOS],["-DHOSTOS=$host_os"])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
