/*
 * cdd.h - declarations for display compiler
 *
 * \author Richard M. Murray
 * \date 27 Jul 92
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

struct TableEntry {
    char *type;			/* data type */
    char *name;			/* entry name (for error messages) */
    int x, y;			/* object location */
    char *lvalue;		/* data location */
    char *manager;		/* interface manager */
    char *format;		/* format string for interface */
    int size;			/* size of data object */
    char *callback;		/* callback function */
    char *fgname, *bgname;	/* color names for fg and bg */
    char *userarg;		/* user argument */
    char *varname;		/* name of variable for display dump */
    int length;			/* maximum length of field */

    unsigned rw: 1;		/* read/write flag */
};

int copy_header(FILE *, FILE *);
int parse_screen(FILE *);
int save_marker(char *, int *, int, int *);
int find_marker(char *, int *, int *, int *);
int save_label(FILE *, char *, int *, int, int *);
int parse_trailer(FILE *);
int dump_header(FILE *, int), dump_code(FILE *, int);
int make_label(int, int, char *s);
int dump_entry(struct TableEntry *, int, FILE *);


