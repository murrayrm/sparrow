/*
 * matrix.c - copy of matrix routes from falcon
 * RMM, 6 Aug 08
 *
 * These files are needed for loading filters in the channel library.
 * Unfortunately they do not work with new MATLAB files.  Eventually
 * need to remove this file and replace calls in chnconf.c with calls
 * to libmatio.
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

#ifndef _have_matrix
#define _have_matrix

typedef struct matlab_entry {
    int type;			/* data type */
    int nrows, ncols;		/* size of matrix */
    int imagf;			/* flag indicating imag part */
    int namelen;		/* name length (including NULL) */
    char name[20];		/* name of matrix */
    double *real;		/* real data (row, column format) */
    double *imag;		/* imaginary data (if imagf set) */
    struct matlab_entry *prev;  /* previous element in list */
    struct matlab_entry *next;  /* next element in list */
} MATRIX;

/* matrix Inits and removal stuff */
MATRIX *mat_load(char *file);

MATRIX *mat_find(MATRIX *list, char *name);
void mat_list(MATRIX *list);

MATRIX *mat_init(int nrows, int ncols); /* Create & Initialize an nxm matrix */
MATRIX *mat_create(void);               /* Create a matrix */

int mat_set_name(MATRIX *a, char *name);
char *mat_get_name(MATRIX *a);
void mat_print(MATRIX *a);
int mat_list_free(MATRIX *list);
void mat_free(MATRIX *a);
void mat_reset(MATRIX *a);
int mat_resize(MATRIX *a, int nrows, int ncols);

/* Matrix element access and manipulation */
double mat_element_get(MATRIX *mx, int row, int col);
double mat_element_get_f(MATRIX *mx, int row, int col);
int mat_element_set(MATRIX *mx, int row, int col, double value);
void mat_element_set_f(MATRIX *mx, int row, int col, double value);
int mat_copy(MATRIX *dst, MATRIX *src);
void mat_copy_f(MATRIX *dst, MATRIX *src);
double *mat_get_real(MATRIX *mat);

int mat_get_rows(MATRIX *mat);
int mat_get_cols(MATRIX *mat);

/* Matrix operations */

/* dst = transpose(a) */
int mat_transpose(MATRIX *dst, MATRIX *a);
void mat_transpose_f(MATRIX *dst, MATRIX *a);

/* det = determinant(mat) */
int mat_det(double *det, MATRIX *mat);
double mat_det_f(MATRIX *mat);

/* ainv = inverse(a) */
int mat_inverse(MATRIX *ainv, MATRIX *a);
void mat_inverse_f(MATRIX *ainv, MATRIX *a);

/* matrix multiplication: dst = ab */
int mat_mult(MATRIX *dst, MATRIX *a, MATRIX *b);
void mat_mult_f(MATRIX *dst, MATRIX *a, MATRIX *b);

/* matrix mult, element by element:  dst = a.*b */
int mat_dotmult(MATRIX *dst, MATRIX *a, MATRIX *b);
void mat_dotmult_f(MATRIX *dst, MATRIX *a, MATRIX *b);

/* matrix addition:  dst = a + b */
int mat_add(MATRIX *dst, MATRIX *a, MATRIX *b);
void mat_add_f(MATRIX *dst, MATRIX *a, MATRIX *b);

/* matrix subtraction:  dst = a - b */
int mat_subtract(MATRIX *dst, MATRIX *a, MATRIX *b);
void mat_subtract_f(MATRIX *dst, MATRIX *a, MATRIX *b);

/* matrix scaling:  dst = scale*A, scale is a scalar */
int mat_scale(MATRIX *dst, MATRIX *a, double scale);
void mat_scale_f(MATRIX *dst, MATRIX *a, double scale);

/* matrix offset: dst = offset + A, offset is added to each element of A */
int mat_offset(MATRIX *dst, MATRIX *a, double offset);
void mat_offset_f(MATRIX *dst, MATRIX *a, double offset);

/* Still to be done */
int mat_lstsqr();
int mat_lstsqr_f();

int loadmat(FILE *fp, int *type, int *mrows, int *ncols, int *imagf,
	    char *pname, double **preal, double **pimag);

#endif
