#ifndef _have_matlab
#define _have_matlab

#include "matrix.h"

/* Old Matlab Routine Function declarations */
int matlab_open(char *fname);
MATRIX *matlab_find(char *name);
int matlab_close(void);
void matlab_list(void);

#endif
