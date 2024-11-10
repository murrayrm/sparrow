AC_INIT([Sparrow Library],[4.0.0],[Richard M. Murray <murray@cds.caltech.edu>],[sparrow])
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
AC_CACHE_CHECK(
	[if compiler recognizes -pthread],
	myapp_cv_gcc_pthread,
	ac_save_CFLAGS=$CFLAGS
	CFLAGS="$CFLAGS -pthread"
	AC_LANG_SAVE
	AC_LANG([C])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <pthread.h>]], [[void *p = pthread_create;]])],[myapp_cv_gcc_pthread=yes],[myapp_cv_gcc_pthread=no
	])
	AC_LANG_RESTORE
	CFLAGS=$ac_save_CFLAGS
)
if test $myapp_cv_gcc_pthread = yes
then
	myapp_threads=""
	CFLAGS="$CFLAGS -pthread"
else
	AC_CHECK_LIB(pthread, pthread_create, 
	  THREAD_LIBS = "-lpthread", 
	  AC_MSG_ERROR([can't find pthreads library])
	)
fi
AC_CONFIG_FILES([Makefile src/Makefile doc/Makefile])
AC_OUTPUT
