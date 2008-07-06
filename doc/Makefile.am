# Makefile.am - automake configuration for sparrow documentation
# RMM, 6 Jul 08

BUILT_SOURCES = dispexmp.cxi dispexmp.ddi
info_TEXINFOS = sparrow.txi

dispexmp.cxi: ../src/dispexmp.c
	sed -e "s/@/@@/" -e "s/{/@{/" -e "s/}/@}/" $? | expand > $@

dispexmp.ddi: ../src/dispexmp.dd
	sed -e "s/@/@@/" -e "s/{/@{/" -e "s/}/@}/" $? | expand > $@

