# Makefile for sparrow package
# RMM, 8 Feb 94
#
# This makefile is used to make distribution files, etc for sparrow.
#

SPARROW = ..
VERSION = 4_0a
MAKE = make

# Compile the entire distribution
all: 
	(cd src; $(MAKE) SPARROW=$(SPARROW) VERSION=$(VERSION))
	(cd doc; $(MAKE) SPARROW=$(SPARROW))

# Installation
install: $(SPARROW)/bin $(SPARROW)/lib $(SPARROW)/include $(SPARROW)/www
	(cd src; $(MAKE) SPARROW=$(SPARROW) install)
	(cd doc; $(MAKE) SPARROW=$(SPARROW) VERSION=$(VERSION) install)

# Top level directories in the sparrow path
$(SPARROW):
	@-mkdir $(SPARROW)
$(SPARROW)/bin: $(SPARROW)
	@-mkdir $(SPARROW)/bin
$(SPARROW)/lib: $(SPARROW)
	@-mkdir $(SPARROW)/lib
$(SPARROW)/include: $(SPARROW)
	@-mkdir $(SPARROW)/include
$(SPARROW)/www: $(SPARROW)
	@-mkdir $(SPARROW)/www
