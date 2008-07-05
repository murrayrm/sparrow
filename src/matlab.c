/*
 * matlab.c - old matrix interface
 *
 * MLK, Feb 1995
 *
 * These routines support the old matlab interface by calling the
 * new version of these routines, which are much better.
 *
 * These routines make use of a single list of matrices.  In order to
 * preserve compatibility with the old routines, this list is overwritten
 * every time you read in a new matlab file (stupid, but backwards
 * compatible).  This also creates memory leaks, but that is what you get
 * for using the old version.
 */

#include <stdio.h>
#include "matlab.h"

static MATRIX *matlab_first = NULL;

int matlab_open(char *file)
{
    MATRIX *last;
    int cnt = 0;

    /* Load the matrices from the file and count them */
    matlab_first = mat_load(file);
    for (last = matlab_first; last != NULL; last = last->next) cnt++;

    return cnt;
}

MATRIX *matlab_find(char *name){
  return mat_find(matlab_first,name);
}

int matlab_close(){
  
  mat_list_free(matlab_first);
  matlab_first = NULL;

  return 0;
}

void matlab_list(){
  mat_list(matlab_first);
}

