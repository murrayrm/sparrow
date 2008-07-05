# Makefile for sparrow documentation
# RMM, 30 Dec 03

DOCS = sparrow.html pgmref/index.html
TXIFILES = sparrow.txi display.txi debug.txi intro.txi channel.txi \
	dispexmp.cxi dispexmp.ddi
WWW = $(SPARROW)/www

doc: sparrow.html pgmref/index.html

sparrow.html: $(TXIFILES)
	makeinfo --html --no-split -o $@ sparrow.txi

dispexmp.cxi: ../src/dispexmp.c
	sed -e "s/@/@@/" -e "s/{/@{/" -e "s/}/@}/" $? | expand > $@

dispexmp.ddi: ../src/dispexmp.dd
	sed -e "s/@/@@/" -e "s/{/@{/" -e "s/}/@}/" $? | expand > $@

pgmref/index.html: Doxyfile
	doxygen Doxyfile

install: doc
	mkdir -p $(WWW)/sparrow
	install -m 644 sparrow.html $(WWW)/sparrow
	ln -sf sparrow.html $(WWW)/sparrow/index.html
	install -d $(WWW)/sparrow/pgmref
	install pgmref/* $(WWW)/sparrow/pgmref

