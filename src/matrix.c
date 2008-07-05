#include <stdio.h>
#include <string.h>

#ifdef MSDOS
#include <alloc.h>
#else
#include <stdlib.h>
//#include <malloc.h>
#endif

#include "matrix.h"

int mat_vflag = 0;		/* flag to allow verbose messages */

MATRIX  *mat_init(int nrows, int ncols) {
  MATRIX *handle;

  handle = mat_create();

  if (mat_resize(handle,nrows,ncols)<0){
    fprintf(stderr, "  Matrix initial sizing error!\n");
    mat_free(handle);
    return NULL;
  }

  mat_reset(handle);

  return handle;
}

MATRIX *mat_create(){
  MATRIX *handle;

  handle = (MATRIX *)malloc(sizeof(MATRIX));

  if (handle==NULL){
    fprintf(stderr, " mat_create error!\n");
    return NULL;
  }

  handle->type = 0;
  handle->nrows = 0;
  handle->ncols = 0;
  handle->imagf = 0;
  
  handle->real = NULL;
  handle->imag = NULL;
  handle->prev = NULL;
  handle->next = NULL;

  handle->namelen = 7;
  strcpy(handle->name,"NONAME");

  return handle;
}


int mat_set_name(MATRIX *a, char *name){

  if (a==NULL) return -1;
  
  strcpy(a->name,name);
  a->namelen=strlen(name)+1;
  return 0;
}

char * mat_get_name(MATRIX *a){
  return a->name;
}

void mat_reset(MATRIX *a) {
  int i;

  /* Initialize to all elements to 0 */
  for (i=0; i < a->nrows*a->ncols; i++) *(a->real+i)=0.0;
}

int mat_resize(MATRIX *mx, int nrows, int ncols) {

  double *r;

  if (mx == NULL) return -1;

  free(mx->real);
  r = (double *)malloc(nrows*ncols*sizeof(double));
  if (r == NULL) return -1;

  mx->real = r;
  mx->nrows = nrows;
  mx->ncols = ncols;
  return 0;
}
  

double mat_element_get(MATRIX *mx, int row, int col) {

  double entry;

  if (mx != NULL) 
    if ( (row < mx->nrows) && (col < mx->ncols))
      entry = mat_element_get_f(mx,row,col);

  return entry;
}

double mat_element_get_f(MATRIX *mx, int row, int col) {
  return *(mx->real + col*mx->nrows + row);
}

double *mat_get_real(MATRIX *mat) {

  if (mat==NULL) return NULL;
  return mat->real;
}

int mat_element_set(MATRIX *mx, int row, int col, double value) {

  if (mx == NULL) return -1;
  if ( (row >= mx->nrows) || (col >= mx->ncols) ) return -1;

  mat_element_set_f(mx,row,col,value);

  return 0;
}

void mat_element_set_f(MATRIX *mx, int row, int col, double value) {
  *(mx->real + col*mx->nrows + row) = value;
}


void mat_print(MATRIX *a) {
  if (a == NULL) 
    printf("Matrix is NULL\n");
  else {
    int i, j;
    
    printf("Matrix %s: %d rows, %d cols\n", a->name, a->nrows, a->ncols);
    for (i = 0; i < a->nrows; i++) {
      for (j = 0; j < (a->ncols); j++)
	printf("%8.3f ", *(a->real + i +j*a->nrows));
      printf("\n");
    }
  }
}


int mat_transpose(MATRIX *at, MATRIX *a) { /* at = transpose(a) */
  MATRIX *a1;
  int freea1=0;

  if ((a == NULL) || (at == NULL)) return -1;

  if (at == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1=0;
  }
  else
    a1=a;
 
  mat_resize(at,a1->ncols,a1->nrows);

  mat_transpose_f(at,a1);

  if (freea1) mat_free(a1);

  return 0;
}

void mat_transpose_f(MATRIX *at, MATRIX *a) { /* at = transpose(a) */

  register int i,j;

  for (i = 0; i < a->nrows; i++)
    for (j = 0; j < a->ncols; j++)
      *(at->real + j + i*at->nrows) = *(a->real + i + j*a->nrows);
}

int mat_list_free(MATRIX *a) {

  MATRIX *next;
  int i = 0;

  while(a != NULL){
    i++;
    next = a->next;
    mat_free(a);
    a = next;
  }

  return i;
}

void mat_free(MATRIX *a) {

  if (a==NULL) return;

  /* Clear a from any list */
  if (a->prev != NULL) a->prev->next = a->next;
  if (a->next != NULL) a->next->prev = a->prev;

  free(a->real);
  free(a->imag);
  free(a);

}


int mat_copy(MATRIX *dst, MATRIX *src) { /* dst = src */
  if ((src == NULL) || (dst == NULL)) return -1;
  if (src==dst) return 1;

  mat_resize(dst, src->nrows, src->ncols);

  mat_copy_f(dst,src);

  return 0;
}

void mat_copy_f(MATRIX *dst, MATRIX *src){ /* dst = src */

  register int i;

  for (i = 0; i < src->nrows*src->ncols; i++)
      *(dst->real + i) = *(src->real + i);
}

int mat_mult(MATRIX *dst, MATRIX *a, MATRIX *b) { /* dst = a.b */
  MATRIX *a1, *b1;
  int freea1=0;
  int freeb1=0;

  if ((dst == NULL) || (a == NULL) || (b == NULL)) return -1;
  if (a->ncols != b->nrows) return -1;

  if (dst == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1 = 1;
  }
  else
    a1 = a;

  if (dst == b){
    b1 = mat_create();
    mat_copy(b1,b);
    freeb1 = 1;
  }
  else
    b1 = b;

  mat_resize(dst,a1->nrows,b1->ncols);

  mat_mult_f(dst,a1,b1);

  if (freea1) mat_free(a1);
  if (freeb1) mat_free(b1);
  
  return 0;
}

void mat_mult_f(MATRIX *dst, MATRIX *a, MATRIX *b) { /* dst = a.b */

  register int i,j,k;

  mat_reset(dst);

  for (i = 0; i < dst->nrows; i++)
    for (j = 0; j < dst->ncols; j++)
      for (k = 0; k < a->ncols; k++)
	*(dst->real + i + j*dst->nrows) += 
	  *(a->real + i + k*a->nrows ) * *(b->real + k + j*b->nrows);

}


int mat_dotmult(MATRIX *dst, MATRIX *a, MATRIX *b) { /* dst = a .* b */
  MATRIX *a1, *b1;
  int freea1=0;
  int freeb1=0;


  if ((dst == NULL) || (a == NULL) || (b == NULL)) return -1;
  if ((b->nrows != a->nrows) || (b->ncols != a->ncols)) return -1;

  if (dst == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1 = 1;
  }
  else
    a1 = a;

  if (dst == b){
    b1 = mat_create();
    mat_copy(b1,b);
    freeb1 = 1;
  }
  else
    b1 = b;

  mat_resize(dst,a1->nrows,a1->ncols);

  mat_dotmult_f(dst,a1,b1);

  if (freea1) mat_free(a1);
  if (freeb1) mat_free(b1);
  
  return 0;
}

void mat_dotmult_f(MATRIX *dst, MATRIX *a, MATRIX *b){ /* dst = a .* b */
  
  register int i;

  for (i = 0; i < dst->nrows*dst->ncols; i++)
    *(dst->real + i) = *(a->real + i) * *(b->real + i);
}


int mat_add(MATRIX *dst, MATRIX *a, MATRIX *b) { /* dst = a + b */
  MATRIX *a1, *b1;
  int freea1=0;
  int freeb1=0;


  if ((dst == NULL) || (a == NULL) || (b == NULL)) return -1;
  if ((b->nrows != a->nrows) || (b->ncols != a->ncols)) return -1;

  if (dst == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1 = 1;
  }
  else
    a1 = a;

  if (dst == b){
    b1 = mat_create();
    mat_copy(b1,b);
    freeb1 = 1;
  }
  else
    b1 = b;

  mat_resize(dst,a1->nrows,a1->ncols);

  mat_add_f(dst,a1,b1);

  if (freea1) mat_free(a1);
  if (freeb1) mat_free(b1);
  
  return 0;
}

void mat_add_f(MATRIX *dst, MATRIX *a, MATRIX *b){ /* dst = a + b */
  
  register int i;

  for (i = 0; i < dst->nrows*dst->ncols; i++)
    *(dst->real + i) = *(a->real + i) + *(b->real + i);
}


int mat_subtract(MATRIX *dst, MATRIX *a, MATRIX *b) { /* dst = a - b  */ 

  MATRIX *a1, *b1; int freea1=0; int freeb1=0;


  if ((dst == NULL) || (a == NULL) || (b == NULL)) return -1;
  if ((b->nrows != a->nrows) || (b->ncols != a->ncols)) return -1;

  if (dst == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1 = 1;
  }
  else
    a1 = a;

  if (dst == b){
    b1 = mat_create();
    mat_copy(b1,b);
    freeb1 = 1;
  }
  else
    b1 = b;

  mat_resize(dst,a1->nrows,a1->ncols);

  mat_subtract_f(dst,a1,b1);

  if (freea1) mat_free(a1);
  if (freeb1) mat_free(b1);
  
  return 0;
}

void mat_subtract_f(MATRIX *dst, MATRIX *a, MATRIX *b){ /* dst = a - b */
  
  register int i;

  for (i = 0; i < dst->nrows*dst->ncols; i++)
    *(dst->real + i) = *(a->real + i) - *(b->real + i);
}

int mat_scale(MATRIX *dst, MATRIX *a, double scale) { /* dst = scale*a */
  MATRIX *a1;
  int freea1=0;

  if ((dst == NULL) || (a == NULL)) return -1;

  if (dst == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1 = 1;
  }
  else
    a1 = a;

  mat_resize(dst,a1->nrows,a1->ncols);

  mat_scale_f(dst,a1,scale);

  if (freea1) mat_free(a1);
  
  return 0;
}

void mat_scale_f(MATRIX *dst, MATRIX *a, double scale){ /* dst = scale*a*/
  
  register int i;

  for (i = 0; i < dst->nrows*dst->ncols; i++)
    *(dst->real + i) = *(a->real + i)*scale;
}

int mat_offset(MATRIX *dst, MATRIX *a, double offset) { /* dst = offset+a */
  MATRIX *a1;
  int freea1=0;

  if ((dst == NULL) || (a == NULL)) return -1;

  if (dst == a){
    a1 = mat_create();
    mat_copy(a1,a);
    freea1 = 1;
  }
  else
    a1 = a;

  mat_resize(dst,a1->nrows,a1->ncols);

  mat_offset_f(dst,a1,offset);

  if (freea1) mat_free(a1);
  
  return 0;
}

void mat_offset_f(MATRIX *dst, MATRIX *a, double offset){ /* dst = offset+a*/
  
  register int i;

  for (i = 0; i < dst->nrows*dst->ncols; i++)
    *(dst->real + i) = *(a->real + i)+offset;
}

int mat_get_rows(MATRIX *a){

  if (a==NULL) return -1;
  return a->nrows;
}

int mat_get_cols(MATRIX *a){

  if (a==NULL) return -1;
  return a->ncols;
}

/* Find a matrix and return a pointer to the matrix data structure */
MATRIX *mat_find(MATRIX *list, char *name)
{
    /* Do a bit of error checking */
    if (name == NULL) return NULL;

    while (list != NULL){
      if (list->name == NULL) break;
      if (strcmp(list->name, name) == 0) return list;
      list = list->next;
    }

    /* Couldn't find the matrix */
    return NULL;
}


MATRIX *mat_load(char *file)
{
    FILE *fp;
    int i=0;
    MATRIX *first, *prev, *cur;

    /* Open the file */
    if ((fp = fopen(file, "rb")) == NULL) return NULL;

    first = mat_create(); cur = first; prev = cur;
    /* For the first pass, prev=cur so that if the first loadmat fails,
     * the assignment prev->next = NULL will not seg fault.  If loadmat
     * is successful, things are fine. */

    /* Read in a matrix */
    while (loadmat(fp, &cur->type, &cur->nrows, &cur->ncols,
		  &cur->imagf, cur->name, &cur->real, &cur->imag) == 0){

      if (mat_vflag) printf("Loaded matrix %i, name %s\n",++i,cur->name);
      prev = cur;
      cur = mat_create();
      prev->next = cur;
      cur->prev = prev;
      
    }

    /* In case the load_mat fails on the first one */
    prev->next = NULL;
    mat_free(cur);

    fclose(fp);

    return first;
}

void mat_list(MATRIX *list){

  if (list == NULL){
    printf("The list contains no elements (NULL list).\n");
    return;
  }
  
  printf("The following matrices are in the list:\n");

  while (list != NULL) {
     printf("  %s\n",list->name);
     list = list->next;
  }
}


int mat_det(double *det, MATRIX *mat){

  if (mat == NULL) return -1;
  if (mat->ncols != mat->nrows) return -1;

  *det=mat_det_f(mat);

  return 0;
}

double mat_det_f(MATRIX *mat){

  register int i,j,k;
  register int rows = mat_get_rows(mat);
  register int sign = 1;
  register double det = 0;
  MATRIX *minor;

  if (rows==1) 
    return mat->real[0];
  if (rows==2)
    return (mat->real[0]*mat->real[3] - mat->real[1]*mat->real[2]);

  minor = mat_init(rows-1,rows-1);
  
  for (i=0; i<rows; i++){
    int cnt=0;

    for (j=1; j<rows; j++)      /* We skip the first column always */
      for (k=0; k<rows; k++){   /* Skip the appropriate row of stuff */
	if (k==i) continue;
	minor->real[cnt] = mat->real[j*rows+k];
	cnt++;
      }
    det = det + sign*mat->real[i]*mat_det_f(minor);
    sign *= -1;
  }

  mat_free(minor);
  return det;
}

int mat_inverse(MATRIX *dst, MATRIX *src){

  MATRIX *a1;
  int freea1=0;

  if ((dst == NULL) || (src == NULL)) return -1;

  if (dst == src){
    a1 = mat_create();
    mat_copy(a1,src);
    freea1=1;
  }
  else
    a1=src;
 
  mat_resize(dst,a1->ncols,a1->nrows);

  mat_inverse_f(dst,a1);

  if (freea1) mat_free(a1);

  return 0;

}

void mat_inverse_f(MATRIX *dst, MATRIX *src){

  register int i,j,k,l;
  int rows = mat_get_rows(src);
  double det = mat_det_f(src);
  double signr = 1;
  double signc = 1;
  MATRIX *temp, *minor;

  temp = mat_init(rows,rows);
  minor = mat_init(rows-1,rows-1);

  mat_transpose_f(temp,src);

  for(i=0; i<rows; i++){
    signr = 1.0;
    for(j=0; j<rows; j++){
      
      int cnt =0;
      
      for (k=0; k<rows; k++)      /* column always */
	for (l=0; l<rows; l++){   /* Skip the appropriate row of stuff */
	  if (k==i) continue;
	  if (l==j) continue;
	  minor->real[cnt] = temp->real[k*rows+l];
	  cnt++;
	}
      dst->real[i*rows+j] = signr * signc * mat_det_f(minor)/det;
      signr = -1.0 * signr;
    }
    signc=-1.0 * signc;
  }
  
  mat_free(temp);
  mat_free(minor);
}

