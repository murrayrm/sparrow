/*!
 *
 * \file	display.h
 * \brief	Display structures and definitions 
 *
 * \author	Richard M. Murray
 * \date	November 4, 1987
 *
 * This file contains the structures and functions needed to access the
 * Sparrow display library (dynamic display).
 *
 * \ingroup	display
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

#ifndef __DISPLAY_INCLUDED__
#define __DISPLAY_INCLUDED__

#ifdef __cplusplus
extern "C"
{
#endif

/*!
 * \typedef DD_ACTION
 * \brief Display actions
 * 
 * The DD_ACTION type provides the actions that should be handled by
 * the display manager for a table entry.
 *
 * \ingroup	display
 */
typedef enum {
  Update, 				//!< Update value of object
  Refresh, 				//!< Refresh value on display
  Input, 				//!< Parse input for object
  Save, 				//!< Save object to a file
  Load					//!< Load object from a file
} DD_ACTION;

/*!
 * \enum display_type
 * \brief Display entry type 
 * 
 * The display_type enum gives the different types of table entries
 * that are interpreted by the sparrow display manager.  The
 * KeyBinding enum is not used in this versin of sparrow.
 *
 * \ingroup display
 */
enum display_type {
  Data=0, Label, Button, KeyBinding, String
};

/*!
 * \struct display_entry
 * \brief Display table entry
 *
 * The display_entry struct gives the basic data type that is used to
 * make up all entries on the sparrow display screen.  A display table
 * is a list of display entries.
 *
 * \ingroup display
 */
typedef struct display_entry {
  int row;			//!< location of the entry on the screen
  int col;			//!< location of the entry on the screen
  void *value;			//!< pointer to the data to display
  int (*function)(DD_ACTION, int);	//!< display manager function
  char *format;			//!< format to use for displaying data 
  char *current;		//!< current value of data (for buffering)
  int selectable;		//!< Allow entry to be selected
  int (*callback)(long);	//!< fcn called after selection
  long userarg;		        //!< user argument passed to callback

  /* Optional parameters */
  int foreground;		//!< foreground color 
  int background;		//!< background color 
  enum display_type type;	//!< type of entry
  char varname[32];		//!< variable name (for load/save) 
  int length;			//!< maximum allowed length of field

  /* Parameters not normally initialized */
  int lastlen;			//!< length of last value displayed
  int up, down, left, right;    //!< indices of entries in each direction
  unsigned initialized: 1;	//!< flag to keep track of initialization
  unsigned reverse: 1;		//!< flag to reverse display colors

  /* Offsets to adjacent (selectable) entries */
  /* Initialization done in dd_usetbl() */

} DD_IDENT;

/* Some definitions for manually generated tables (not often used) */
#define DD_Label(r, c, s)  	    {r, c, s, dd_label, "NULL", NULL, 0, NULL, (long)NULL, "", -1}
#define DD_Button(r, c, s, fcn)     {r, c, s, dd_label, NULL, NULL, 1, fcn, (long)NULL, "", -1}

/* Define the end of the table (used by cdd) */
#define DD_End			    \
  {0, 0, NULL, NULL, NULL, NULL, 0, NULL, (long) 0, 0, 0,  Data, "", -1}

/* Macros for callback functions without the normal arguments */
#define DD_EXIT_LOOP		    (dd_exit_loop((long) 0))
#define DD_BEEP()		    (dd_beep((long) 0))

/* Macros for calling redefineable functions */
#define DD_CLS	(*dd_cls_fcn)	     //!< clear screen function macro
#define DD_PROMPT (*dd_prompt_fcn)   //!< prompt function macro
#define DD_SCANF (*dd_scanf_fcn)     //!< scan function macro

/* Global variables */
extern DD_IDENT *ddtbl;	        //!< current display description table */
extern DD_IDENT *ddprv;	        //!< previous display description table */
extern int dd_rows;		//!< number of available rows on screen */
extern int dd_cols;		//!< number of available columns */
extern int dd_modef;	        //!< text/graphics flag */
extern int dd_cur;		//!< current entry */
extern int dd_keycode;		//!< last keypress code */
extern int dd_debug;		//!< turn debugging info on */
extern char dd_save_string[];	//!< string for saving and loading variables */
extern long long dd_dummyvar;	//!< placeholder for reference entries */

/* Declare variables which are used to hold function pointers */
extern void (*dd_cls_fcn)(long);
extern void (*dd_prompt_fcn)(char *);
extern int (*dd_scanf_fcn)(char *, char *, void *); 

/*
 * Display functions 
 *
 * The sparrow display functions are used to display the different
 * type of standard elements defined by Sparrow.
 */
extern int dd_short(DD_ACTION, int), dd_byte(DD_ACTION, int);
extern int dd_float(DD_ACTION, int), dd_label(DD_ACTION, int);
extern int dd_double(DD_ACTION, int), dd_ubyte(DD_ACTION, int);
extern int dd_long(DD_ACTION, int), dd_string(DD_ACTION, int);
extern int dd_message(DD_ACTION, int), dd_ubyte(DD_ACTION, int);
extern int dd_nilmgr(DD_ACTION, int), dd_nilcbk(long);

/* Screen attributes */
#define Normal  0x0
#define Reverse 0x1

/* Colors (ANSI) */
#define BLACK	     0
#define RED	     1
#define GREEN	     2
#define YELLOW	     3
#define BLUE	     4
#define MAGENTA	     5
#define CYAN	     6
#define LIGHTGREY    7

#define DD_DEFFG     LIGHTGREY
#define DD_DEFBG     BLACK

/* 
 * Library functions 
 *
 * The display library functions are the primary mechanism for
 * interfacing with the sparrow dynamic display library.
 */

extern int dd_open(void);
extern void dd_close(void);
extern int dd_usetbl(DD_IDENT *tbl);
extern int dd_usetbl_cb(long);
extern int dd_prvtbl(void);
extern int dd_prvtbl_cb(long);
extern DD_IDENT *dd_gettbl(void);
extern int dd_update(void);
extern int dd_redraw(long);
extern void dd_refresh(int offset);
extern void dd_refresh_tbl(int, DD_IDENT *);
extern int dd_bindkey(int key, int (*fcn)(long));
extern int dd_rebind_tbl(char *name, void *addr, DD_IDENT *tbl);
extern int dd_rebind(char *, void *);

extern int dd_setcolor(int offset, int bg, int fg);
extern int dd_setcolor_tbl(int offset, int bg, int fg, DD_IDENT *);
extern int dd_setlabel(int offset, char *s);
extern int dd_setlabel_tbl(int offset, char *s, DD_IDENT *);

extern int dd_loop(void);
extern void *dd_loop_thread(void *);

extern int dd_save(char *filename);
#define dd_tbl_save(f, t)	dd_save_tbl(f, t)
extern int dd_save_tbl(char *filename, DD_IDENT *tbl);
extern int dd_load(char *filename);
#define dd_tbl_load(f, t)	dd_load_tbl(f, t)
extern int dd_load_tbl(char *filename, DD_IDENT *tbl);

extern int dd_select(int);
extern int dd_beep(long);
extern void dd_quiet(), dd_noisy();
extern int dd_toggle_beep_cb(long);

extern int dd_find_up(int entry, int *up);
extern int dd_find_right(int entry, int *right);
extern int dd_find_down(int entry, int *down);
extern int dd_find_left(int entry, int *left);

/* Terminal interface */
extern void dd_prompt(char *);
extern int dd_getc(void);
extern void dd_text_prompt(char *s);
extern void dd_text_cls(long);
extern int dd_text_scanf(char *prompt, char *fmt, void *ptr);
#define DD_PUTS dd_puts
extern void dd_puts(DD_IDENT *dd, char *s);
extern char *dd_cgets(char *buffer);
extern int dd_read(char *prompt, char *address, int length);

/* Display actions (usually bound to keys) */
extern int dd_exit_loop(long);
extern int dd_unbound(long);
extern int dd_exec_callback(long);
extern int dd_input(long);

/* Item selection (can also be bound to keys) */
extern int dd_next(long);
extern int dd_prev(long);
extern int dd_left(long);
extern int dd_right(long);
extern int dd_up(long);
extern int dd_down(long);

/* ddprintf */
#include <stdarg.h>
int dd_sprintf(char *, char *, int, ...);
int dd_vsprintf(char *, char *, int, va_list); 

/* chn_gettok */
#include <stdio.h>
int chn_gettok(FILE * fp, char *string, int length, char *delimiters, int *line);

/* Display hooks for debugging output */
extern int dd_dbgout_setup();
extern void dd_dbgout_cleanup();
  
#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_INCLUDED__ */
