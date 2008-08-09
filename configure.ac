AC_INIT([Sparrow Library], [4.0.0],
        [Richard M. Murray <murray@cds.caltech.edu>],
        [sparrow])
AM_INIT_AUTOMAKE		dnl initialize automake
AC_PROG_CC			dnl configure C compiler
AC_PROG_YACC			dnl configure Yacc
AC_PROG_RANLIB			dnl configure ar, ranlib
AC_PROG_INSTALL			dnl configuration install
AC_SEARCH_LIBS([Mat_VarReadNext], [matio], [
    AC_DEFINE([HAVE_MATIO], [1], [Define if matio library is installed])
    AC_SUBST(LIBMATIO, [-lmatio])
  ], [
    echo "WARNING: matio library not found; MATLAB channel filters not enabled"
  ], [-lz -lm])
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile])
AC_OUTPUT
