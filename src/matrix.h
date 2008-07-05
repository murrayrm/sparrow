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
