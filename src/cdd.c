/*!
 * \file cdd.c 
 * \brief compile display description files
 *
 * \author Richard M. Murray
 * \date August 18, 1988
 *
 * The cdd program is used to compile a dynamic display description into
 * a table definition file.  It is documented in the Sparrow user's manual.
 *
 * \ingroup display
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include "cdd.h"
extern int yyparse();

#define LENCHAR '$'		/* character to use to specify field lengths */

extern FILE *yylex_fp;		/* file pointer for parser */
extern int yylex_line;		/* line number for parser errors */
extern char *yylex_file;	/* input filename */

char *tblname = "display";
char *bufname = "buffer";
char *nilstr = "NULL";		/* string for empty items in entry */
char *nilmgr = "dd_nilmgr";	/* string for empty entry manager */
char *nilcbk = "dd_nilcbk";	/* string for empty callback function */
char *nillong = "(long) 0";	/* string for empty long */

#define MAXREF 1024		/* max number of reference points */
struct reference_point {
    int row, col;		/* Location of point */
    char name[32];		/* Name of reference */
    int length;			/* optional field length */
} reftbl[MAXREF];
int refoff = 0;			/* Reference table offset */

int vflg = 0;			/* turn on verbose messages */
int wflg = 0;			/* turn off warning messages */
int cflg = 0;			/* generate a separate .c file */
char *header_path = NULL;	/* path to header file */

int main(int argc, char **argv)
{
    int c, errflg = 0;
    FILE *in = stdin, *out = stdout, *code = NULL;
    char *header = NULL;

    extern int optind;
    extern char *optarg;

    /* Parse command line arguments */
    while ((c = getopt(argc, argv, "wvc:o:p:h:?")) != EOF)
	switch (c) {
	case 'v':	vflg++;		break;
	case 'w':	wflg++;		break;

	case 'c':
	    if ((code = fopen(optarg, "w")) == NULL) {
  	        perror(optarg);
	        exit(1);
	    }
	    ++cflg;
	    break;

	case 'h':
        case 'o':
            if ((out = fopen(header = optarg, "w")) == NULL) {
                perror(optarg);
                exit(1);
            }
            break;

	case 'p':		/* store path to header */
	    header_path = optarg;
	    break;

	default:
	    errflg++;		/* print usage information */
	    break;
	}

    if (errflg) {
	fprintf(stderr, 
		"usage: %s [-wv] [-o file.h] [-c file.c] [-p path] file.dd\n", 
		argv[0]);
	exit(2);
    }

    if (optind < argc) {
	if ((in = fopen(argv[optind], "r")) == NULL) {
	    perror(argv[optind]);
	    exit(1);
	}
	/* Save the filename for the parser */
	yylex_file = argv[optind];
    }

    /* Read in the display definition file */
    copy_header(in, out);
    parse_screen(in);
    parse_trailer(in);

    /* Generate the header file */
    if (code != NULL) {
      /* Include the path to the sparrow display.h file */
      fprintf(out, "#include \"");
      if (header_path != NULL) fprintf(out, "%s/", header_path);
      fprintf(out, "display.h\"\n");
    }
    dump_header(out, cflg);

    /* Genenerate the code (in header or in separate file) */
    if (code != NULL) {
      /* Include the path to the sparrow display.h file */
      fprintf(code, "#include \"");
      if (header_path != NULL) fprintf(code, "%s/", header_path);
      fprintf(code, "display.h\"\n");

      /* Next include the header file */
      fprintf(code, "#include \"%s\"\n\n", header);

      /* And finally we can include the code */
      dump_code(code, cflg);

    } else {
      /* Everything in one file */
      dump_code(out, cflg);
    }

    return 0;
}

/* Skip header (terminated by %%) */
int copy_header(FILE *in, FILE *out)
{
    char line[256];

    while (fgets(line, 256, in) != NULL) {
	++yylex_line;
	if (strncmp(line, "%%", 2) == 0) break;
	fputs(line, out);
    }
    return 0;
}

/* Read a screen dump */
int parse_screen(FILE *fp)
{
  int row, col, offset;
  int space;
  char line[256];
  
  for (row = 0; fgets(line, 256, fp) != NULL; ++row) {
    ++yylex_line;
    if (strncmp(line, "%%", 2) == 0) break;
    
    offset = col = 0;
    while (line[offset] != '\n') {
      if (line[offset] == '\t') {
	/* Move forward to next tab stop */
	space = 8 - (col % 8);
	col += space;
	++offset;
	
      } else if (line[offset] == '%') {
	/* Save the location of the marker */
	save_marker(line, &offset, row, &col);
	
      } else if (line[offset] == '#') {
	/* Skip the rest of this line */
	break;
	
      } else if(line[offset] == LENCHAR){
	/* Calculate the length for the last marker read, if
	   the last marker was on the same row and it does not
	   already have a length */
	if(refoff>0 && reftbl[refoff-1].row==row+1 && reftbl[refoff-1].length==-1)
	  reftbl[refoff-1].length = col + 2 - reftbl[refoff-1].col;
	++offset;
	++col;

      } else if (isprint(line[offset]) && line[offset] != ' ') {
	/* Generate a label */
	save_label(stdout, line, &offset, row, &col);
	
      } else {
	/* Move forward one character and column */
	++offset;
	++col;
      }
    }
  }
  return 0;
}

/* Save the location of a marker */
int save_marker(char *line, int *offset, int row, int *col)
{
    int length;
    int fieldlen = -1, lineoff;
    char buf[256];

    /* Get the name of the marker in buf (skip the meta) */
    for (lineoff=1,length=0; isalnum(line[*offset+lineoff]) || 
	 line[*offset+lineoff] == '_' || line[*offset+lineoff] == LENCHAR; lineoff++){
      if(line[*offset+lineoff] == LENCHAR)
	fieldlen = lineoff+1;	/* the user wants this to be the maxumim field length */
      else
	buf[length++] = line[*offset+lineoff];
    }
    buf[length++] = '\0';	/* null-terminate */

    /* Save the marker in the table */
    strncpy(reftbl[refoff].name, buf, length);
    reftbl[refoff].row = row+1;
    reftbl[refoff].col = *col+1;
    reftbl[refoff++].length = fieldlen;
    if (refoff >= MAXREF) {
	/* Too many entries; abort */
        fprintf(stderr, "cdd: too many references; max = %d\n", MAXREF);
	exit(1);
    }

    *offset += lineoff;
    *col += lineoff;
    return(1);
}

/* Find a marker */
int find_marker(char *name, int *row, int *col, int *len)
{
    int off;
    for (off = 0; off < refoff; ++off)
	if (strcmp(reftbl[off].name, name) == 0) {
	    *row = reftbl[off].row;
	    *col = reftbl[off].col;
	    *len = reftbl[off].length;
	    return(refoff);
	}
    return(-1);
}

#define SPACELIM	4	/* Maximum number of spaces in a label */

/* Generate a label */
int save_label(FILE *fp, char *line, int *offset, int row, int *col)
{
    int length, spaced = 0, lineoff;
    char buf[256];

    /* Find the length of the label */
    for (lineoff=0,length=0; line[*offset+lineoff] != '\0'; ++lineoff) {
	/* Only allow a single space in a label */
	if (line[*offset+lineoff] == ' ' && spaced > SPACELIM) break;

	/* Check for terminators (tabs, newlines) */
	if (iscntrl(line[*offset+lineoff])) break;
	if (line[*offset+lineoff] == '%') break;

	/* allow the length character to interrupt a label */
	if(line[*offset+lineoff] == LENCHAR){ 
	/* Calculate the length for the last marker read, if
	   the last marker was on the same row and it does not
	   already have a length */
	  if(refoff>0 && reftbl[refoff-1].row==row+1 && reftbl[refoff-1].length==-1)
	    reftbl[refoff-1].length = *col + 2 + lineoff - reftbl[refoff-1].col;
	  continue;
	}

	spaced = (line[*offset+lineoff] == ' ' ? spaced+1 : 0);
	buf[length++] = line[*offset+lineoff]; /* store label */
    }
    /* Subtract off trailing spaces */
    length -= spaced;
    buf[length++] = '\0';	/* null-terminate */

    /* Generate the label */
    make_label(row+1, *col+1, buf);

    /* Update position */
    *offset += lineoff;
    *col += lineoff;

    return 0;
}

/* Parse the action section of a dispfile */
int parse_trailer(FILE *in)
{
    yylex_fp = in;
    yyparse();

    return 0;
}

/*
 * Table generation routines
 *
 * make_entry		generate a new table entry
 * make_label		special form for static labels
 *
 */

/* Allocate space for table entries (fixed size for now) */
static struct TableEntry tbl[1024];

int tbllen = 0;			/* current offset into table */
int bufsiz = 0;			/* buffer space allocated */

int make_entry(struct TableEntry *data)
{
    struct TableEntry *p = tbl + tbllen++;

    /* Copy the data into the table */
    *p = *data;

    /* Now got through and duplicate any strings that are present */
    if (data->type) p->type = strdup(data->type);
    if (data->name) p->name = strdup(data->name);
    if (data->manager) p->manager = strdup(data->manager);
    if (data->format) p->format = strdup(data->format);
    if (data->callback) p->callback = strdup(data->callback);
    if (data->userarg) p->userarg = strdup(data->userarg);
    if (data->fgname) p->fgname = strdup(data->fgname);
    if (data->bgname) p->bgname = strdup(data->bgname);

    /* Modify lvalues to be strings or memory refs, as appropriate */
    if (data->lvalue) {
	char addrbuf[256];
	switch (data->size) {
	case 0:
	    /* Generate a string */
	    sprintf(addrbuf, "\"%s\"", data->lvalue);
	    p->lvalue = strdup(addrbuf);
	    break;

	case -1:
	    /* no processing on name (used for string data type) */
	    p->lvalue = strdup(data->lvalue);
	    if (p->varname == NULL) p->varname = p->lvalue;
	    break;

	default:
	    /* generate a memory reference */
	    sprintf(addrbuf, "&(%s)", data->lvalue);
	    p->lvalue = strdup(addrbuf);
	    if (p->varname == NULL) p->varname = strdup(data->lvalue);
	    break;
	}
    }
    /* Keep track of the storage requirements */
#   ifdef unix
    if ((bufsiz % sizeof(double)) != 0)
	/* Adjust buffer size for proper alignment */
	bufsiz += sizeof(double) - (bufsiz % sizeof(double));
#   endif
    if (data->size > 0) bufsiz += data->size;

    return tbllen;
}

int init_entry(struct TableEntry *data)
{
    /* Intialize all the attributes to something safe */
    data->type = NULL;		/* type of object */
    data->name = NULL;		/* name of the entry */
    data->x = data->y = 0;
    data->lvalue = NULL;	/* data location */
    data->manager = NULL;	/* interface manager */
    data->format = NULL;	/* format string */
    data->size = 0;		/* object size (in bytes) */
    data->callback = NULL;	/* user callback function */
    data->userarg = NULL;	/* argument to callback function */
    data->fgname = NULL;	/* foreground color string */
    data->bgname = NULL;	/* background color string */
    data->varname = NULL; 	/* name of data variable */
    data->length = -1;		/* default, don't truncate fields */
    data->rw = 1;		/* set if selectable */

    return 0;
}

int make_label(
int x, int y,			/* location of the label */
char *s			/* text for the label */
)
{
    struct TableEntry data;

    /* Initialize a table entry and fill it appropriately */
    init_entry(&data);
    data.type = "Label";
    data.x = x;
    data.y = y;
    data.lvalue = s;
    data.manager = "dd_label";
    data.rw = 0;

    return make_entry(&data);
}

/* Dump the display header information to disk */
int dump_header(FILE *fp, int cflg)
{
  register int i;

  /* Go through and make function declarations for managers and callbacks */
  /*! Not implemented !*/

  /* Go through table and make id definitions */
  fprintf(fp, "/* Object ID's (offset into table) */\n");
  for (i = 0; i < tbllen; ++i)
    if (tbl[i].name != NULL)
	    fprintf(fp, "#define %s\t%d\n", tbl[i].name, i);
  fputs("\n", fp);

  /* If we are generating separate code file, include extern declaration */
  if (cflg) {
    /* Use extern "C" so it works correctly with C++ */
    fprintf(fp, "\n#ifdef __cplusplus\n");
    fprintf(fp, "extern \"C\" {\n");
    fprintf(fp, "#endif\n\n");

    fprintf(fp, "extern DD_IDENT %s[];\n", tblname);

    fprintf(fp, "\n#ifdef __cplusplus\n");
    fprintf(fp, "}\n");
    fprintf(fp, "#endif\n\n");
  }

  return 0;
}

/* Dump the table definition to disk */
int dump_code(FILE *fp, int cflg)
{
  register int i;
  int offset;

  /* If this is a single file, use extern "C" so it works correctly with C++ */
  if (!cflg) {
    fprintf(fp, "\n#ifdef __cplusplus\n");
    fprintf(fp, "extern \"C\" {\n");
    fprintf(fp, "#endif\n\n");
  }
  
  /*
   * Generate display table
   */
  fprintf(fp, "/* Allocate space for buffer storage */\n");
  if (bufsiz == 0)
    fputs("/* (none required) */\n\n", fp);
  else {
    fprintf(fp, "static char %s[%d];\n\n", bufname, bufsiz);
  }
    
  /* Generate the header for the display table */
  if (!cflg) fprintf(fp, "static "); 
  fprintf(fp, "DD_IDENT %s[] = {\n", tblname);

  /* Go through each object and generate an entry */
  for (offset = 0, i = 0; i < tbllen; ++i) {
#	ifdef unix
    if ((offset % sizeof(double)) != 0)
	    /* Adjust buffer size for proper alignment */
	    offset += sizeof(double) - (offset % sizeof(double));
#	endif
    if (dump_entry(tbl + i, offset, fp) < 0) return -1;
    offset += tbl[i].size;
  }

  /* Finish off the list with an empty entry */
  fprintf(fp, "DD_End};\n");
  
  /* Put in extern "C" so it works correctly with C++ */
  if (!cflg) {
    fprintf(fp, "\n#ifdef __cplusplus\n");
    fprintf(fp, "}\n");
    fprintf(fp, "#endif\n\n");
  }

  return 0;
}

/* Dump a single object entry */
int dump_entry(struct TableEntry *p, int offset, FILE *fp)
{
    /* Make sure that we always have a manager */
    if (p->manager == NULL) {
	fprintf(stderr, "Warning: empty manager function\n");
    }

    /* First print out the stuff that is always the same */
    fprintf(fp, "{%d, %d, (void *)(%s), %s, \"%s\", ",
        p->x, p->y, p->lvalue ? p->lvalue : nilstr,
	p->manager ? p->manager : nilmgr,
	p->format ? p->format : nilstr);

    /* Now do some size dependent stuff */
    switch (p->size) {
    case 0:
	/* Don't allocate any buffer space */
	fprintf(fp, "(char *) NULL, ");
	break;

    case -1:
	/* Special handling for string objects */
	/* Use buffer field to store length */
	fprintf(fp, "(char *)(sizeof(%s)), ", p->lvalue);
	break;

    default:
	/* Allocate some space in the buffer for the data */
	fprintf(fp, "%s+%d, ", bufname, offset);
	break;
    }

    /* Now write out the final arguments (same for all data types) */
    #warning Magic number related to string length
    fprintf(fp, "%d, %s, (long)%s, %s, %s, %s, \"%.29s\", %d},\n",
	p->rw,
	p->callback ? p->callback : nilcbk,
	p->userarg ? p->userarg : nillong,
	p->fgname ? p->fgname : "0",
	p->bgname ? p->bgname : "0",
	p->type ? p->type : "Data",
	(p->varname == NULL) ? "" : p->varname,
	p->length);

    return 0;
}

/* Reset the buffer and display table names */
void cdd_set_bufname(char *s) { bufname = strdup(s); }
void cdd_set_tblname(char *s) { tblname = strdup(s); }

