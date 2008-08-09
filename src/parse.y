/*
 * parse.y - grammer for ddisp compiler (cdd)
 *
 * Richard M. Murray
 * August 18, 1988
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

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cdd.h"

/* Declare functions used by parser */
int yylex();
int yyerror(char *);

int start = 0;			/* Offset into yylex buffer */
int aflg = 0;			/* read next token as argument */
int nflg = 0;			/* read next token as number */
FILE *yylex_fp;			/* file to be parsed */
char *yylex_file = "";		/* input file name (for errors) */
int yylex_line = 1;		/* line number for error messages */
struct TableEntry data;		/* object data structure */

#define REFVAR "dd_dummyvar"	/* Dummy variable used for references */

int read_expression(char *, int *);
int make_entry(struct TableEntry *);
int init_entry(struct TableEntry *);
void cdd_set_bufname(char *);
void cdd_set_tblname(char *);
%}

%union { 
    char *string;		/* String of characters */
    struct point {
	int x, y, len;
    } point;			/* Reference point */
    int number;			/* Integer */
}

%token	LABEL BUTTON SHORT DOUBLE FLOAT CUSTOM STR KEY TYPE BYTE INT
%token	READONLY DATA FORMAT MANAGER CALLBACK SIZE BADOPTION
%token	BUFNAME TBLNAME IDNAME USERARG FGROUND BGROUND LONG MESSAGE

%token	<string>	STRING
%token	<point>		POINT		/* %marker */
%token	<number>	NUMBER
%token	<string>	ADDRESS	
%token  <string>	NAME		/* variable name */
%token  		END_SOURCE      /* End of input source */
%token 			ERRTOK		/* Preprocessor error */

%type	<point>		point
%type   <number>	signed
%type   <string>	argument

%%

actions	:	/* empty */
        |	actions action 		{ start = 0; }
    	|	actions END_SOURCE 	{ YYACCEPT; }
	;

action
    	:	object ';'		{ make_entry(&data); }
	|	command ';'		/* Commands to control execution */
	|	error ';'		/* Try to recover from errors */
    	|	error END_SOURCE
	;

command	:	TBLNAME ':' NAME	{ cdd_set_tblname($3); }
	|	BUFNAME ':' NAME	{ cdd_set_bufname($3); }
	;

object		/* display object */
	:	LABEL ':' { init_entry(&data); }
		location label {
		    data.type = "Label";
		    data.manager = "dd_label";
		    data.rw = 0;
		    data.size = 0;	/* treat this as a string/label */
		} options

	|	BUTTON ':' { init_entry(&data); }
		location label callback {
		    data.type = "Button";
	    	    data.manager = "dd_label";
		} options

	|	SHORT ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_short";
		    data.size = sizeof(int);	/* for SPARC compatibility */
		} options 

	|	INT ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_short";
		    data.size = sizeof(int);	/* for SPARC compatibility */
		} options 

	|	BYTE ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_byte";
		    data.size = sizeof(int);	/* for SPARC compatibility */
		} options

	|	LONG ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_long";
		    data.size = sizeof(long);	/* for SPARC compatibility */
		} options

	|	STR ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "String";
	    	    data.manager = "dd_string";
		    data.size = -1;		/* special handling required */
		} options

	|	MESSAGE ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_message";
		    data.size = sizeof(char *);	/* special handling required */
		} options

	|	DOUBLE ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_double";
		    data.size = sizeof(double);
		} options

	|	FLOAT ':' { init_entry(&data); }
		location lvalue format {
		    data.type = "Data";
	    	    data.manager = "dd_float";
		    data.size = sizeof(float);
		} options

	|	KEY ':' { init_entry(&data); }
		NAME callback {
		    data.type = "Key";
		    data.format = $4;
		    data.manager = "NULL";
		} options
		

	|	CUSTOM ':' { init_entry(&data); } location options {
	    	    /* all options are set by the user */
		}
	;

/* tokens which update the object data structure */
lvalue	:	NAME			{ data.lvalue = $1; }
	|	'@' {aflg=1;} STRING	{
		  data.lvalue = REFVAR;
		  data.varname = strdup($3);
		}
	;
location:	point			{ data.x = $1.x; data.y = $1.y; data.length = $1.len; };
callback:	NAME			{ data.callback = $1; };
format	:	STRING			{ data.format = $1; };
label	:	STRING			{ data.lvalue = $1; };
signed  :       {nflg = 1;} NUMBER	{ $$ = $2; };

/* List of options (possibly empty) */
options	:	/* empty */
    	|	options option
	;

option	:	READONLY		{ data.rw = 0; }
	| 	TYPE '=' argument	{ data.type = $3; }
	|	DATA '=' argument 	{ data.lvalue = $3; }
	|	FORMAT '=' argument	{ data.format = $3; }
	|	SIZE '=' signed		{ data.size = $3; }
	|	MANAGER '=' argument	{ data.manager = $3; }
	|	CALLBACK '=' NAME	{ data.callback = $3; }
	|	IDNAME '=' NAME		{ data.name = $3; }
	|	FGROUND '=' argument	{ data.fgname = $3; }
	|	BGROUND '=' argument	{ data.bgname = $3; }
	|	USERARG '=' argument 	{ data.userarg = $3; }
	;

argument:	{aflg = 1;} STRING 	{$$ = $2; }
	;

point	:	POINT				{ $$ = $1; }
	|	'[' NUMBER ',' NUMBER ']'	{ $$.x = $2;  $$.y = $4; }
	;
%%

/* Table of command/token pairs */
struct command_token { char *name; int token; } cmdtbl[] = {
    {"label", LABEL}, {"short", SHORT}, {"button", BUTTON}, {"key", KEY},
    {"byte", BYTE}, {"int", INT},
    {"double", DOUBLE}, {"custom", CUSTOM}, {"string", STR}, {"type", TYPE},
    {"-size", SIZE}, {"long", LONG}, {"message", MESSAGE},
    {"-ro", READONLY}, {"-format", FORMAT}, {"-callback", CALLBACK},
    {"-manager", MANAGER}, {"-data", DATA}, {"bufname", BUFNAME},
    {"tblname", TBLNAME}, {"-idname", IDNAME}, {"-userarg", USERARG},
    {"float", FLOAT}, {"-fg", FGROUND}, {"-bg", BGROUND},
    {NULL, 0}
};

/* Look for a command in the table */
int find_command(char *s)
{
    int index;
    for (index = 0; cmdtbl[index].name != NULL; ++index) {
	if (strcmp(cmdtbl[index].name, s) == 0)
	    return(cmdtbl[index].token);
    }
    return(0);
}

/* Lexical scanner */
int yylex()
{
    int ch, token;
    static char buffer[1024];
    char *p = buffer + start;

 newline:
    while (isspace(ch = fgetc(yylex_fp)))
	if (ch == '\n') ++yylex_line;

    /* Simple tokens */
    switch (ch) {
    case '(':
    case ')':
    case ',':
    case ';':
	return(ch);

    case EOF:
	return(END_SOURCE);
    }

    /* Skip comments in the source file */
    if (ch == '#') {
	/* Skip past everything until a linefeed */
	while ((ch = fgetc(yylex_fp)) != EOF && ch != '\n');
	yylex_line++;
	goto newline;
    }

    /* First check for context specific parsing */
    if (aflg) {
	int quoted = (ch == '"');
	aflg = 0;

	/* Return a string containing the argument */
	if (!quoted) buffer[start++] = ch;
	while ((ch=fgetc(yylex_fp)) >= 0) {
	    if (!quoted && (isspace(ch) || ch == ';')) break;
	    if (ch == '\n') ++yylex_line;
	    if (ch == '"') break;
	    buffer[start++] = ch;
	}
	buffer[start++] = 0;

	/* Do some post-processing and error checking */
	if (!quoted) ungetc(ch, yylex_fp);
	else if (ch != '"') return(ERRTOK);

	/* All done */
	yylval.string = p;
	return(STRING);

    } else if (nflg) {
	/* Always reset flag as soon as we detect it */
	nflg = 0;

	/* Parse this token as a number */
	if (!isdigit(ch) && ch != '-') {
	    /* This doesn't look like a number */
	    fprintf(stderr, "%s:%d: invalid number\n", yylex_file, yylex_line);
	    return(BADOPTION);
	}
	    
	for (buffer[start++] = ch; isdigit(ch=fgetc(yylex_fp)); 
	     buffer[start++] = ch);
	buffer[start++] = 0;
	ungetc(ch, yylex_fp);
	yylval.number = atoi(p);
	return(NUMBER);
    }

    if (ch == '%') {
	int row, col, len;

	/* Reference point name */
	buffer[start++] = ch;
	while (isalnum(ch = fgetc(yylex_fp)) || ch == '_') buffer[start++] = ch;
	buffer[start++] = 0;

	/* Leave the terminator on the input stack */
	ungetc(ch, yylex_fp);

	if (find_marker(p+1, &row, &col, &len) != -1) {
	    yylval.point.x = row;  yylval.point.y = col; yylval.point.len = len;
	} else {
	    fprintf(stderr, "Warning: can't find marker %s\n", p+1);
	    yylval.point.x = yylval.point.y = 0; yylval.point.len = -1;
	}
	return(POINT);

    } else if (ch == '-') {
	/* Option name */
	buffer[start++] = ch;
	while (isalnum(ch = fgetc(yylex_fp)) || ch == '_')
	    buffer[start++] = ch;
	buffer[start++] = 0;

	/* Leave the terminator on the input stack */
	ungetc(ch, yylex_fp);

	/* Check for a name in the command list */
	if ((token = find_command(p))) return(token);
	fprintf(stderr, "Error: unknown options \"%s\"\n", p);
	return(BADOPTION);

    } else if (ch == '&') {
	/* Address */
	buffer[start++] = ch;
	read_expression(buffer, &start);
	yylval.string = p;
	return(ADDRESS);

    } else if (!aflg && isdigit(ch)) {
	/* Number */
	for (buffer[start++] = ch; isdigit(ch=fgetc(yylex_fp)); 
	     buffer[start++] = ch);
	buffer[start++] = 0;
	ungetc(ch, yylex_fp);
	yylval.number = atoi(p);
	return(NUMBER);

    } else if (ch == '"') {
	/* String */
	while ((ch=fgetc(yylex_fp)) >= 0) {
	    if (ch == '\n') ++yylex_line;
	    if (ch == '"') break;
	    buffer[start++] = ch;
	}
	buffer[start++] = 0;
	if (ch != '"') return(ERRTOK);
	yylval.string = p;
	return(STRING);

    } else if (isalpha(ch) || ch == '_') {
	/* Read in a string of characters */
	buffer[start++] = ch;
	read_expression(buffer, &start);

	/* Check for a name in the command list */
	if ((token = find_command(p))) return(token);
	yylval.string = p;
	return(NAME);
    }

    /* As a last resort return the character itself */
    return(ch);
}

/* Print error messages */
int yyerror(char *s)
{
    fprintf(stderr, "%s:%d: syntax error\n", yylex_file, yylex_line);
    return 0;
}

/* Read an expression */
int read_expression(char *buffer, int *start)
{
    int ch;
    int plev = 0, blev = 0;

    while ((ch = fgetc(yylex_fp)) >= 0) {
	if (ch == '\n') ++yylex_line;
	switch (ch) {
	case '(':	++plev;				break;
	case '[':	++blev;				break;
	case ')': 	if (--plev < 0) goto done; 	break;
	case ']':	if (--blev < 0) goto done;	break;
	case ';':	goto done;
	case ':':	if (plev == 0) goto done; 	break;
	default:
	    if (plev == 0 && isspace(ch)) goto done;
	    break;
	}
	buffer[(*start)++] = ch;
    }

done:
    ungetc(ch, yylex_fp);
    buffer[(*start)++] = 0;

    return 0;
}

