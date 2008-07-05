/*
 * cdd.h - declarations for display compiler
 *
 * \author Richard M. Murray
 * \date 27 Jul 92
 *
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


