# Makefile.am - automake configuration file for sparrow library
# RMM, 6 Jul 08

# Set up flags to enable the use of color and to compile for unix
# (Note: compilation for MSDOS is no longer supported => unix required.)
AM_CPPFLAGS = -DCOLOR -Dunix

# Programs and libraries built in this directory
bin_PROGRAMS = sparrow-cdd sparrow-chntest
lib_LIBRARIES = libsparrow.a
check_PROGRAMS = dispexmp
pkginclude_HEADERS = \
  display.h debug.h dbglib.h channel.h flag.h keymap.h errlog.h hook.h \
  servo.h
pkgdata_DATA = config.dev fcn_tbl.dd dispexmp.dd chntest.dd

# Sources that are compiled from within
BUILT_SOURCES = fcn_tbl.h dispexmp.h chntest.h

# Rules for building display compiler cdd
sparrow_cdd_SOURCES = cdd.c cdd.h parse.y

# Rules for building the main sparrow library
libsparrow_a_SOURCES = \
  display.c keymap.c flag.c ddtypes.c hook.c debug.c ddthread.c \
  ddsave.c capture.c channel.c chnconf.c virtual.c fcn_gen.c \
  chngettok.c devlut.c dbgdisp.c \
  servo.c sertest.c errlog.c curslib.c fcn_tbl.dd \
  tclib.h conio.h ddkeymap.h virtual.h fcn_gen.h termio.h 

# Rules for building channel test program chntest
sparrow_chntest_SOURCES = chntest.c chntest.dd config.dev
sparrow_chntest_LDADD = libsparrow.a -lcurses @LIBMATIO@

# Rules for building check programs
dispexmp_SOURCES = dispexmp.c dispexmp.dd
dispexmp_LDADD = libsparrow.a -lcurses @LIBMATIO@

# Define rules for creating display tables
%.h: %.dd sparrow-cdd;	sparrow-cdd -o $@ $<
