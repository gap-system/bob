AC_PREREQ(2.68)
AC_INIT([bob], 0.1, neunhoef@mcs.st-and.ac.uk)
AC_CONFIG_SRCDIR([bob.cc])
AC_CONFIG_AUX_DIR([cnf])
AC_CONFIG_HEADER(config.h:config.h.in)
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE
AC_PROG_CXX
AC_CANONICAL_HOST
case "$host_os" in
   *cygwin*)
      AC_DEFINE([SYS_IS_WINDOWS],[1],[Set to 1 if on Windows])
      FORLDADD="-lrt"
      FORLDFLAGS=""
      ;;
    *darwin*)
      AC_DEFINE([SYS_IS_OSX],[1],[Set to 1 if on OSX])
      FORLDADD="-framework CoreServices"
      FORLDFLAGS=""
      ;;
    *linux*)
      AC_DEFINE([SYS_IS_LINUX],[1],[Set to 1 if on Linux])
      FORLDADD="-lrt"
      FORLDFLAGS=""
      ;;
    *bsd*)
      AC_DEFINE([SYS_IS_BSD],[1],[Set to 1 if on BSD])
      FORLDADD="-lrt"
      FORLDFLAGS=""
      ;;
esac
AC_SUBST([FORLDADD],[${FORLDADD}])
AC_SUBST([FORLDFLAGS],[${FORLDFLAGS}])
AC_DEFINE([HOSTOS],[${host_os}],[Set this to the name of the OS])
AC_CHECK_SIZEOF([void *])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
