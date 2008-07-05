# Makefile for sparrow library
# RMM, 30 Dec 03

CC = g++
CFLAGS = -g -I. -DCOLOR -Dunix

# needed to compile properly in Mac OS X
CPATH = /usr/include

# added this because some distributions didn't find cdd the old way:
CDD_CMD = $(DGC)/bin/cdd

# Installation locations
DGC = ..
WWW = $(DGC)/www

# Default version for sparrow.a
DEF_VERSION = curses

LIBS = sparrow-termcap.a sparrow-curses.a
PGMS = dispexmp-termcap dispexmp-curses chntest
INCS = display.h debug.h dbglib.h channel.h flag.h keymap.h errlog.h

default: dispexmp-$(DEF_VERSION) sparrow-$(DEF_VERSION).a
doc: $(DOCS)
all: $(PGMS) $(LIBS) TAGS doc

install: default $(DGC)/bin $(DGC)/lib
	mkdir -p $(DGC)/bin
	install -s cdd $(DGC)/bin/cdd
	install -m 644 sparrow-$(DEF_VERSION).a $(DGC)/lib/sparrow.a
	ranlib $(DGC)/lib/sparrow*.a
	mkdir -p $(DGC)/include/sparrow
	install -m 644 $(INCS) $(DGC)/include/sparrow

$(DGC)/bin:; mkdir $@
$(DGC)/lib:; mkdir $@
$(WWW)/sparrow:; mkdir $@

#
# Display compiler
#
CDD_SRC = cdd.c parse.c
CDD_OBJS = $(CDD_SRC:.c=.o)
$(DGC)/bin/cdd: $(CDD_OBJS)
	$(CC) $(CFLAGS) -o cdd $(CDD_OBJS)
	mkdir -p $(DGC)/bin
	install -s cdd $(DGC)/bin/cdd

parse.c: parse.y
	bison -o $@ parse.y

#
# Display library
#
LIB_SRC = display.c keymap.c flag.c ddtypes.c hook.c debug.c ddthread.c \
	ddsave.c capture.c channel.c chnconf.c virtual.c fcn_gen.c \
	matlab.c matrix.c loadmat.c chngettok.c devlut.c dbgdisp.c \
	servo.c sertest.c errlog.c
LIB_OBJS = $(LIB_SRC:.c=.o)
TERMCAP_OBJS = $(LIB_OBJS) tclib.o termio.o conio.o
CURSES_OBJS = $(LIB_OBJS) curslib.o

sparrow-termcap.a: $(TERMCAP_OBJS)
	ar rs $@ $?

sparrow-curses.a: $(CURSES_OBJS)
	ar rs $@ $?

fcn_gen.o: fcn_tbl.h
fcn_tbl.h: fcn_tbl.dd $(CDD_CMD);	$(CDD_CMD) -o $@ fcn_tbl.dd

#
# Example program (under unix)
#
dispexmp-termcap: dispexmp.o sparrow-termcap.a
	$(CC) $(CFLAGS) -o $@ dispexmp.o sparrow-termcap.a -ltermcap -lcurses -lncurses

dispexmp-curses: dispexmp.o sparrow-curses.a
	$(CC) $(CFLAGS) -o $@ dispexmp.o sparrow-curses.a -lcurses -lncurses

dispexmp.o: dispexmp.h
dispexmp.h: dispexmp.dd $(CDD_CMD);	$(CDD_CMD) -o $@ dispexmp.dd

#
# chntest program - test channel libraries
#
chntest: chntest.o sparrow-$(DEF_VERSION).a
	$(CC) $(CFLAGS) -o $@ chntest.o sparrow-$(DEF_VERSION).a -lpthread -ltermcap -lcurses

chntest.o: chntest.h
chntest.h: chntest.dd $(CDD_CMD);	$(CDD_CMD) -o $@ chntest.dd

# Generate a TAGS file
TAGS: *.c
	etags *.[ch]

tidy:;	/bin/rm -f *.o *~ parse.tab.* y.tab.c
clean: tidy;	
	/bin/rm -f parse.c cdd $(PGMS) $(TBLS) *.a *.html *.cxi *.ddi
	/bin/rm -rf pgmref TAGS dispexmp.h fcn_tbl.h chntest.h

depend:
	makedepend -Y -- $(CFLAGS) -- *.c

# DO NOT DELETE THIS LINE - makedep depends on it

capture.o: channel.h flag.h hook.h
cdd.o: cdd.h
cgets.o: conio.h
channel.o: channel.h hook.h
chnconf.o: channel.h display.h matlab.h matrix.h
chngettok.o: display.h
conio.o: conio.h tclib.h termio.h
curslib.o: keymap.h conio.h tclib.h termio.h
dbgdisp.o: dbglib.h display.h conio.h tclib.h
ddsave.o: display.h
ddthread.o: display.h
ddtypes.o: display.h
debug.o: dbglib.h
devlut.o: channel.h
dispexmp.o: display.h dbglib.h dispexmp.h
display.o: display.h keymap.h ddkeymap.h hook.h flag.h
fcn_gen.o: channel.h servo.h display.h fcn_gen.h fcn_tbl.h
flag.o: conio.h flag.h
hook.o: hook.h
keymap.o: keymap.h
matlab.o: matlab.h matrix.h
matrix.o: matrix.h
schntest.o: display.h keymap.h channel.h servo.h
tclib.o: tclib.h keymap.h termio.h
virtual.o: channel.h virtual.h
