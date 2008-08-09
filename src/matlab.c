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
 *
 * Copyright (c) 2008 by California Institute of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name of the California Institute of Technology nor
 *    the names of its contributors may be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CALTECH
 * OR THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
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

