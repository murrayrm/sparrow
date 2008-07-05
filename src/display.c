/*!
 * \file display.c 
 * \brief core dynamic display package for sparrow
 *
 * Richard M. Murray
 * 04 Nov 87
 *
 * \ingroup display
 */

/*
 * Revisions:
 * 21 Jul 92 (RMM)
 *	* rewrote as a toolkit for building displays
 *	* incorporated callbacks and key binding table
 *
 * 28 Jul 92 (RMM)
 *	* add user arguments on callbacks
 *	* multiple displays are allowed 
 *	* added extended key processing (so arrows will work)
 *
 * 3 Aug 92 (MVN + RMM)
 *	* support for screen wraparound (define WRAP)
 *
 * 25 May 93 (RMM)
 * 	* added functions to install user key bindings
 *	* modified input functions to allow basic format checking
 *	* fixed bug which made left selected item blanked out on startup
 *
 * 24 Jan 94 (RMM)
 *	* moved key binding functions to keymap.c
 *
 * 13 Feb 94 (RMM)
 *	* added dd_loop_hook to dd_loop() for use by async dumping
 *	* to exit dd_loop, call dd_exit_loop (instead of ret val)
 *
 * Summer, 1994 (ELW, RMM)
 *	* multiple changes, see master changes file from here on
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>		/* file control header */
#include <math.h>		/* for abs() prototype + _exception */
#include <string.h>
#ifdef MSDOS
#include <alloc.h>
#include <dir.h>
#include <dos.h>
#include <conio.h>		/* console I/O interface */
#endif
#ifdef unix
#include "tclib.h"		/* termcap display library */
#include "conio.h"		/* console I/O emulation */
#endif
#include "display.h"		/* display structure descriptions */
#include "keymap.h"		/* include keymap header */
#include "ddkeymap.h"		/* default key binding tables */
#include "hook.h"
#include "flag.h"
#include <unistd.h>

extern int c86_sprintf(char *fmt, ...);
extern int dbg_ddflg;		/* tell debug routines when we are running */
extern int (*dbg_pre_hook)(); extern void (*dbg_post_hook)();

// #define WRAP			/* allow cursor to wrap around screen */
#define VER1_COMPAT		/* include v1.x functions */
#define A_SUCCESS	1	/* return code for success */
#define A_FAILURE	0	/* failure (non-fatal => don't exit) */
#define A_ABORT		-1	/* force exit */
#define NUMHOOKS 10		/* number of display hooks we allow */

/* Global variables */
KM_BINDING *dd_keymap = NULL;	/* global key binding table */
KM_BINDING *dd_local = NULL;	/* local key binding table */
int dd_keycode = 0;		/* last keypress (not including input) */
int dd_debug = 0;		/* debug flag (set from outside) */
long long dd_dummyvar = 0;	/* dummy variable for references */

DD_IDENT *ddtbl = NULL;		/* current display description table */
DD_IDENT *ddprv = NULL;		/* previous display description table */

int dd_cur = -1;		/* currently selected entry (offset) */
int dd_rows;			/* number of rows, text and graphics */
int dd_cols;			/* number of available cols, text & graphics */
int dd_modef=-1;		/* flag indicating graphics or text mode */
int dd_delay = 100000;		/* default delay at end of loop (100 msec) */
int dd_kbdelay = 10000;		/* Delay between checking for keypress */
int dd_quietf = 0;		/* Don't make any noise */

char dd_save_string[80];	/* communication buffer for Save action */
char dd_errlog[80];		/* error log buffer */
int dd_errprint = 1;		/* print stderr messages to screen */
int dd_errlog_new = 0;		/* new message in error log */

/* Function pointers and hooks */
void (*dd_cls_fcn)(long arg) = NULL;
void (*dd_prompt_fcn)(char *) = NULL;
int (*dd_scanf_fcn)(char *, char *, void *) = NULL;

DECL_HOOKLIST(dd_loop_hooks, NUMHOOKS);

/* Static variables used in this file */
static int abort_loop = 0;	/* abort dd_loop */
static int errpipe[2];		/* file descriptors for stderr pipe */
static FILE *errfp = NULL;	/* receiving end of stderr pipe */

/*
 * Library functions - called to setup and communicated with dispay manager
 *
 * dd_open		initialize screen and display library
 * dd_usetbl		select a display description table and initialize it
 * dd_prvtbl		switch back to the previous table
 * dd_gettbl
 * dd_update		update screen; redraw changed items
 * dd_redraw		redraw entire screen
 * dd_refresh           redraw a single item
 * dd_bindkey           binds a key to a function
 * dd_setcolor          reset foreground and background colors
 * dd_setlabel          changes a label
 * dd_loop		process keyboard events (display manager)
 * dd_save              save the display variables for the current table
 * dd_tbl_save          save a specific table
 * dd_load              load saved display vars into current table
 * dd_tbl_load        load saved display vars into a specific table
 *
 * For most of these functions, a '_tbl' version is available that allows
 * the table to be specified (in case it is not the current table) and a 
 * '_cb' version can be give as a callback function.
 */

/*!
 * \fn int dd_open(void)
 * \brief initialize screen and display library
 * \ingroup display
 *
 * The dd_open() function is used to initialize the screen and the
 * internal structures required by Sparrow for displaying information.
 * When it is called, the screen will be cleared and put into raw
 * mode, with the cursor off.  The effect of this function is reversed
 * by using dd_close().
 *
 */
int dd_open()
{
    struct text_info ti;

    /* Key map initialization */
    if ((dd_keymap = km_createmap()) == NULL) return -1;
    (void) km_copymap(dd_keymap, dd_defkeymap);
    
    /* Terminal initialization */
#   ifdef unix
    if (tc_init(1) < 0) return -1;
    tc_open();
#   endif

    /* Figure out the length and width of the screen */
    co_gettextinfo(&ti);
    dd_rows = ti.screenheight;
    dd_cols = ti.screenwidth;

    /* set up hooks; use text versions of functions */
    dd_cls_fcn = dd_text_cls;
    dd_prompt_fcn = dd_text_prompt;
    dd_scanf_fcn = dd_text_scanf;
    hook_clear(dd_loop_hooks);

    /* Redirect stderr to a pipe for later processing */
    if (pipe(errpipe) < 0) { return -1; }
    dup2(errpipe[1], fileno(stderr));
    fcntl(errpipe[0], F_SETFL, O_NONBLOCK);
    if ((errfp = fdopen(errpipe[0], "r")) == NULL) { return -1; }
    dd_errlog[0] = '\0';

    DD_CLS((long) 0);
    co_setcursortype(_NOCURSOR);	/* turn off cursor */

    dd_modef=0;			/* graphics mode = false */

    /* Set up debug hooks to manage screen properly */
#   ifdef UNUSED
    dbg_ddflg = 1;		/* tell debug that we are running */
#   endif
    dbg_pre_hook = dd_dbgout_setup;
    dbg_post_hook = dd_dbgout_cleanup;
    return 0;
}

/*!
 * \fn int dd_usetbl(DD_IDENT *tbl)
 * \brief select a display description table and initialize it
 * 
 * The dd_usetbl() function sets the table that will be used to update
 * the display.
 *
 * \fn int dd_usetbl_cb(long tbl)
 * \brief Callback version of dd_usetbl()
 *
 * This version of the dd_usetbl() function can be used as a callback
 * function in a display entry.  The argument to the callback should
 * be the desired table.
 *
 * \ingroup display
 */

int dd_usetbl_cb(long tbl) { return dd_usetbl((DD_IDENT *) tbl); }
int dd_usetbl(DD_IDENT *tbl)
{
    int entry, value;

    /* Initialize the local key map */
    if (dd_local == NULL && (dd_local = km_createmap()) == NULL) return -1;
    (void) km_initmap(dd_local);

   /* Save the old display table and mark this one as the new one */
   ddprv = ddtbl;
   ddtbl = tbl;

    /* Intialize the display table */    
    for (entry = 0; tbl[entry].value != NULL; ++entry) {
	tbl[entry].initialized = 0;
	tbl[entry].reverse = Normal;
    
	/* Initialize colors if not specified */
	if (tbl[entry].foreground == 0) tbl[entry].foreground = DD_DEFFG;
	if (tbl[entry].background == 0) tbl[entry].background = DD_DEFBG;

	/* Set up pointers to up,down,left,right items for selectable items */
	if (tbl[entry].selectable){
	    tbl[entry].up = 
		(dd_find_up(entry, &value) == A_SUCCESS) ? value : entry;

	    tbl[entry].down = 
		(dd_find_down(entry, &value) == A_SUCCESS) ? value : entry;

	    tbl[entry].left = 
		(dd_find_left(entry, &value) == A_SUCCESS) ? value : entry;

	    tbl[entry].right = 
		(dd_find_right(entry, &value) == A_SUCCESS) ? value : entry;
	}
    }
    
    /* Reset the selected entry */
    dd_cur = -1; 
    if(dd_select(0) != -1)
	 /* hack so the current entry gets initialized */
	tbl[dd_cur].initialized = 0;
    
    /* Clear the screen to make sure redraw occurs correctly */
    DD_CLS((long) 0);
    
    return 0;
}

/*!
 * \fn int dd_prvtbl(void)
 * \brief switch back to the previous table
 *
 * The dd_prvtb() function switches from the current table to the
 * previous table that was being displayed.  This function is
 * typically used to switch back to a display table called from
 * another table (eg, as a button item).
 *
 * \fn int dd_usetbl_cb(long tbl)
 * \brief Callback version of dd_usetbl()
 *
 * This version of the dd_prvtbl() function can be used as a callback
 * function in a display entry.
 *
 * \ingroup display
 */

int dd_prvtbl() { return dd_usetbl(ddprv); }
int dd_prvtbl_cb(long arg) { return dd_usetbl(ddprv); }

/* Get a pointer to the current table */
DD_IDENT *dd_gettbl() { return ddtbl; }
    
/*!
 * \fn int dd_update(void)
 * \brief update screen; redraw changed items
 */
int dd_update()
{
    int entry;
    
    /* Go through and update data items (only) */
    for (entry = 0; ddtbl[entry].value != NULL; ++entry)
	if (ddtbl[entry].type == Data || !ddtbl[entry].initialized)
	    (*ddtbl[entry].function)(Update, entry);
    	
    return 0;
}

/*!
 * \fn int dd_redraw(long)
 * \brief redraw entire screen
 */
int dd_redraw(long arg)
{
    int entry;

    /* Clear the screen */
    DD_CLS((long) 0);

    /* Re-initialize the display table */
    for (entry = 0; ddtbl[entry].value != NULL; ++entry) {
        ddtbl[entry].initialized = 0;
	ddtbl[entry].reverse = Normal;
    }

    /* Redraw everything */
    dd_update();
    if (dd_cur != -1) dd_select(dd_cur);

    return 0;
}

/*!
 * \fn void dd_refresh(int offset)
 * \brief redraw a single item
 */
void dd_refresh_tbl(int offste, DD_IDENT *tbl) {}
void dd_refresh(int offset)
{
    DD_IDENT *dd = ddtbl + offset;

    /* Call the display manager to refresh this object */
    (*dd->function)(Refresh, offset);
}

/*!
 * \fn int dd_bindkey(int key, int (*fcn)(long))
 * \brief binds a key to a function
 */
int dd_bindkey(int key, int (*fcn)(long))
{
    /* Make a global key binding */
    return km_bindkey(dd_keymap, key, fcn);
}

/*!
 * \fn int dd_rebind_tbl(char *name, void *addr, DD_IDENT *tbl)
 * \brief rebind a variable to a entry with the given name
 * \ingroup display
 *
 * The dd_rebind_tbl() function allows a table entry to be rebound to
 * a new variable.  This function is often used with the reference
 * feature in cdd, which sets up a table entry that has not variable
 * bound to it.
 */
int dd_rebind_tbl(char *name, void *addr, DD_IDENT *tbl)
{
  int entry;

  /* Search for the variable in the table */
  for (entry = 0; tbl[entry].value != NULL; ++entry) {
    if (strcmp(name, tbl[entry].varname) == 0) {
      tbl[entry].value = addr;
      return 0;
    }
  }

  /* If we got here, we didn't find the variable */
  return -1;
}

/*!
 * \fn int dd_rebind(char *name, void *addr)
 * \brief rebind a variable in the current table
 * \ingroup display
 *
 * This is a short version of the dd_rebind_tbl() function that uses
 * the current display table.
 */
int dd_rebind(char *name, void *addr) { 
  return dd_rebind_tbl(name, addr, ddtbl);
}

/*!
 * \fn int dd_setcolor(int offset, int bg, int fg)
 * \brief reset foreground and background colors
 */
int dd_setcolor(int off, int bg, int fg) {
  return dd_setcolor_tbl(off, bg, fg, ddtbl);
}
int dd_setcolor_tbl(int offset, int bg, int fg, DD_IDENT *tbl)
{
    DD_IDENT *dd = tbl + offset;

    /* Reset foreground and background colors */
    dd->background = bg;
    dd->foreground = fg;

    /* Refresh the color of the object on the next update */
    /* Don't actually call refresh because we might be in a servo loop */
    dd->initialized = 0;

    return 0;
}

/*!
 * \fn int dd_setlabel(int offset, char *s)
 * \brief changes a label
 */
int dd_setlabel(int offset, char *s) {
  return dd_setlabel_tbl(offset, s, ddtbl);
}
int dd_setlabel_tbl(int offset, char *s, DD_IDENT *tbl)
{
    DD_IDENT *dd = tbl + offset;
    int oldfg = dd->foreground;

    /* Clear out the old label */
    dd->foreground = dd->background;
    (*dd->function)(Refresh, offset);
    dd->foreground = oldfg;

    /* Change the label that gets displayed */
    dd->value = s;

    /* Refresh the label */
    (*dd->function)(Refresh, offset);

    return 0;
}

/*!
 * \fn int dd_loop(void)
 * \brief process keyboard events (display manager)
 */
int dd_loop()
{
#ifdef SPARROW
    extern int matherr_flag;
    extern struct exception matherr_exception;
#endif

    /* If no element is selected, select the first element in the display */
    if (dd_cur == -1) dd_select(0);
    DD_CLS((long) 0);  dd_redraw((long) 0);
  
    /* Reset the abort flag */
    abort_loop = 0;
  
    while (!abort_loop) {
	int count;

	flag_update();			/* update the flag line */

	/* Update the display */
	if (dd_debug) flag(DISPLAY_FLAG, 'U', GREEN);
	dd_update();			/* process all refreshes */
	if (dd_debug) flag(DISPLAY_FLAG, 'y', GREEN);

	/* Excecute any hooks that have been installed */
	if (dd_debug) flag(DISPLAY_FLAG, 'H', GREEN);
	hook_execute(dd_loop_hooks); 	/* execute loop hooks */
	if (dd_debug) flag(DISPLAY_FLAG, 'h', GREEN);

#ifdef SPARROW
	/* Process pending math errors */
	if (matherr_flag) {
	    char msg[80];
	    c86_sprintf(msg, "matherr: %s (%d)",
	        matherr_exception.name, matherr_exception.type);
	    DD_PROMPT(msg);
	    matherr_flag = 0;
	}
#endif
	/* Process any data sent to stderr (via redirect pipe) */
	int errcount = 0;
	while (errfp != NULL && fgets(dd_errlog, 80, errfp) != NULL) {
	  /* See if we should actually print the line */
	  if (dd_errprint) {
	    /* Got a line of input; beep and display on prompt line */
	    if (!errcount++) dd_beep(0);
	    DD_PROMPT(dd_errlog);
	    dd_errlog_new = 1;		/* flag new error log message */
	  }
	}
    
	/* See if any keys have been hit */
	if (dd_debug) flag(DISPLAY_FLAG, 'K', GREEN);
	if (co_kbhit()) {
	    if (dd_debug) flag(DISPLAY_FLAG, 'k', GREEN);

	    /* Process a keyboard interrupt */
	    dd_keycode = dd_getc();

	    /* Clear prompt line */
	    DD_PROMPT("");
      
	    /* 
	     * Look through the bindings table to see what we should do 
	     * Process global keymap first, then local keymap
	     */
	    if (dd_debug) flag(DISPLAY_FLAG, 'E', GREEN);
	    if (dd_keymap[dd_keycode] == dd_unbound &&
		dd_local[dd_keycode] != km_unbound) 
		(void) km_execkey(dd_local, dd_keycode);
	    else
		(void) km_execkey(dd_keymap, dd_keycode);
	    if (dd_debug) flag(DISPLAY_FLAG, 'e', GREEN);
	}
        flag_update();			/* update the flag line */

	/* Wait for a while, but check for keyboard presses */
	for (count = 0; count < (dd_delay/dd_kbdelay); ++count) {
	  if (co_kbhit()) break;
	  usleep(dd_kbdelay);
	}
    }
    if (dd_debug) flag(DISPLAY_FLAG, ' ', GREEN);

    /* Reset abort loop in case this is a recursive call */
    abort_loop = 0;
    return 0;
}

/*
 * Actions - these can be bound to keys or called from outside
 *
 * dd_exit_loop		exit from dd_loop
 * dd_unbound		beep on the terminal and print a message
 * dd_exec_callback	execute the callback associated with current entry
 * dd_input		input new data into this object
 *
 */

/* for dd_loop to return control to the user */
int dd_exit_loop(long arg) {return ++abort_loop; }

/* unbound key - beep and display a message */
int dd_unbound(long arg)
{
  char msg[32];
  dd_beep((long) 0);
  sprintf(msg, "key is not bound (%d)", dd_keycode);
  DD_PROMPT(msg);
  return 0;
}

/* Execute the callback associated with an object */
int dd_exec_callback(long arg)
{
    int revflag = -1, status = 0;
    DD_IDENT *ip = ddtbl + dd_cur;
    
    if(dd_cur==-1)		/* invalid */
      return -1;
    /* If this is an option, flash the option label */
    if (ip->function == dd_label) {
	revflag = ip->reverse;
	ip->reverse = !revflag;
	dd_select(dd_cur);
    }

    /* Call the function associated with this item */
    if (ip->callback != NULL) status = (ip->callback)(ip->userarg);

    /* Put the option tag back the way that it was */
    if (revflag != -1) {
	ip->reverse = revflag;
	dd_select(dd_cur);
    }

    return status;
}

/* Modify a data object */
int dd_input(long arg)
{
    DD_IDENT *ip = ddtbl + dd_cur;
    if (dd_cur != -1) {
      if ((*ip->function)(Input, dd_cur) != -1) {
	/* On success, execute callback function */
	if (ip->callback != NULL) 
	  (*ip->callback)(ip->userarg == NULL ?
			  (long) ip->value : ip->userarg);
      }
    }
    return 0;
}


/*
 * Item selection
 *
 * dd_select			Select any item
 * dd_next			Select the next item
 * dd_prev			Select the previous item
 * dd_find_up                   Find the nearest entry on previous line
 * dd_find_down                 Find the nearest entry on next line
 * dd_find_right                Find the nearest entry to the right
 * dd_find_left                 Find the nearest entry to the left
 * dd_up			Move to the nearest entry on previous line
 * dd_down			Move to the nearest entry on next line
 * dd_right			move cursor to the right
 * dd_left			move cursor to the left
 *
 */

int dd_select(int offset)
{
    register DD_IDENT *curp = ddtbl + dd_cur;

    /* Redisplay the old cursor position */
    if (dd_cur != -1) {
	curp->reverse = Normal;
	(*curp->function)(Refresh, dd_cur);
    }

    /* Set up the new cursor position - find first selectable option */
    for (dd_cur = offset; ddtbl[dd_cur].value != NULL; ++dd_cur)
        if (ddtbl[dd_cur].selectable) break;
    curp = ddtbl + dd_cur;	/* save new pointer to entry */

    /* Highlight the entry and mark it as new current item */
    if (curp->value != NULL) {
        curp->reverse = Reverse;
        (*curp->function)(Refresh, dd_cur);
        return A_SUCCESS;
    }

    /* Couldn't find a selectable entry; return with failure */
    return (dd_cur = -1);
}

/* Move to the next entry in the display table */
int dd_next(long arg)
{
    int id = dd_cur + 1;
    if (dd_cur < 0) return A_FAILURE;

    /* Look for the next selectable entry in the display */
    while (ddtbl[id].value != NULL) {
        if (ddtbl[id].selectable) {
            dd_select(id);
            return A_SUCCESS;
        }
        ++id;
    }
    return A_FAILURE;
}

int dd_prev(long arg)
{
    int id = dd_cur - 1;
    if (id < 0) return A_FAILURE;

    /* Look for the previous selectable entry in the display */
    while (1) {
        if (ddtbl[id].selectable) {
            dd_select(id);
            return A_SUCCESS;
        }
        
        /* Termination check */
        if (--id < 0) break;
    }
    return A_FAILURE;
}

/* Return the offset to next selectable entry up */
int dd_find_up(int entry, int *up)
{
  int id, best_id = -1;
  int row, col, best;
  if (entry < 0) return A_FAILURE;

  /* Look for something on the previous row */
# ifdef WRAP
  for (row = ddtbl[entry].row-1, col = ddtbl[entry].col, best = dd_cols; 
       row !=ddtbl[entry].row; --row) {
    
    /* Wrap around if we go over the edge */
    if (row<=0) row = dd_rows - 1;
    
# else
  for (row = ddtbl[entry].row-1, col = ddtbl[entry].col, best = dd_cols; 
       row >= 0; --row) {
# endif

    /* Search the entire table */
    for (id = 0; ddtbl[id].value != NULL; ++id) {
      if (ddtbl[id].row == row && ddtbl[id].selectable &&
	  abs(ddtbl[id].col - col) < best) {
	best = abs(ddtbl[id].col - col);
	best_id = id;
      }
    }
    if (best != dd_cols) break;
  }
    
  if (best_id < 0 || row < 0) return A_FAILURE;
  *up = best_id;
  return A_SUCCESS;
}

/* Return the offset to next selectable entry right */
int dd_find_right(int entry, int *right){
  int id, best_id;
  int row, col, best;
  if (entry < 0) return A_FAILURE;

  /* Look for something on the this row */
  row = ddtbl[entry].row;
  col = ddtbl[entry].col;
  best = dd_cols;

  /* Search the entire table */
  for (id = 0; ddtbl[id].value != NULL; ++id) {
    if (ddtbl[id].row == row && ddtbl[id].selectable &&
	ddtbl[id].col > col && ddtbl[id].col - col < best) {
      best = ddtbl[id].col - col;
      best_id = id;
    }
  }

#   ifdef WRAP
  /* Wrap around past the end of the column and start over */
  if (best==dd_cols) {
    col=0;
    /* Search the entire table */
    for (id = 0; ddtbl[id].value != NULL; ++id) {
      if (ddtbl[id].row == row && ddtbl[id].selectable &&
	  ddtbl[id].col > col && ddtbl[id].col - col < best) {
	best = ddtbl[id].col - col;
	best_id = id;
      }
    }
  }
#   endif
  
  if (best != dd_cols) {
    *right = best_id;
    return A_SUCCESS;
  }
  return A_FAILURE;
}

/* Find next selectable entry down */
int dd_find_down(int entry, int *down){
  int id, best_id = -1;
  int row, col, best;
  if (entry < 0) return A_FAILURE;
  
#   ifdef WRAP
  /* Look for something on the next row; wrap across bottom */
  for (row = ddtbl[entry].row+1, col = ddtbl[entry].col, best = dd_cols;
       row != ddtbl[entry].row; ++row) {
    if (row == dd_rows) row=1;

#   else
  /* Look for something on the next row */
  for (row = ddtbl[entry].row+1, col = ddtbl[entry].col, best = dd_cols;
       row < dd_rows; ++row) {
#   endif
    /* Search the entire table */
    for (id = 0; ddtbl[id].value != NULL; ++id) {
      if (ddtbl[id].row == row && ddtbl[id].selectable &&
	  abs(ddtbl[id].col - col) < best) {
	best = abs(ddtbl[id].col - col);
	best_id = id;
      }
    }
    if (best != dd_cols) break;
  }
  if (best_id < 0 || row >= dd_rows) return A_FAILURE;
  *down = best_id;
  return A_SUCCESS;
}


/* Find next selectable entry left */
int dd_find_left(int entry, int *left){
  int id, best_id;
  int row, col, best;
  if (entry < 0) return A_FAILURE;
  
  /* Look for something on the this row */
  row = ddtbl[entry].row;
  col = ddtbl[entry].col;
  best = dd_cols;

  /* Search the entire table */
  for (id = 0; ddtbl[id].value != NULL; ++id) {
    if (ddtbl[id].row == row && ddtbl[id].selectable &&
	ddtbl[id].col < col && col - ddtbl[id].col < best) {
      best = col - ddtbl[id].col;
      best_id = id;
    }
  }
  
#   ifdef WRAP
  /* Wrap around the left side of the screen */
  if (best==dd_cols) {
	col=dd_cols;
	/* Search the entire table */
	for (id = 0; ddtbl[id].value != NULL; ++id) {
	  if (ddtbl[id].row == row && ddtbl[id].selectable &&
	      ddtbl[id].col < col && col - ddtbl[id].col < best) {
	    best = col - ddtbl[id].col;
	    best_id = id;
	  }
	}
      }
#   endif
  
  if (best != dd_cols) {
    *left = best_id;
    return A_SUCCESS;
  }
  return A_FAILURE;
}

/* Move cursor up */  
int dd_up(long arg){
  if (dd_cur < 0) return A_FAILURE;
  dd_select(ddtbl[dd_cur].up);
  return A_SUCCESS;
}

/* Move cursor right */
int dd_right(long arg)
{
  if (dd_cur < 0) return A_FAILURE;
  dd_select(ddtbl[dd_cur].right);
  return A_SUCCESS;
}

/* Move cursor down */
int dd_down(long arg)
{
  if (dd_cur < 0) return A_FAILURE;
  dd_select(ddtbl[dd_cur].down);
  return A_SUCCESS;
}

/* Move cursor left */
int dd_left(long arg)
{
  if (dd_cur < 0) return A_FAILURE;
  dd_select(ddtbl[dd_cur].left);
  return A_SUCCESS;
}

/*
 * Predefined display interface functions
 *
 * These have been moved to separate files so that you only have to
 * link in the ones you actually need (RMM, 23 Oct 94)
 *
 */
 
/* 
 * Terminal interface
 *
 * dd_beep		make a beep
 * dd_quiet, dd_noisy	turn beeps on and off
 * dd_text_scanf        text version of read formatted input
 * dd_read		read a string from the console
 * dd_text_prompt       text version of prompt routine
 * dd_puts		Output a string at the item location
 * dd_cls		Clear the screen
 * dd_text_cls          text version of clearscreen routine
 * dd_close		Close the terminal
 * dd_getc		read a key from the keyboard
 * dd_cgets             input a string on prompt line
 *
 */

/* Make a noise */
int dd_beep(long arg) { 
  if (dd_quietf) return 0;
  co_beep(); return 0; 
}

/* Turn beeping on or off */
void dd_quiet() { dd_quietf = 1; }
void dd_noisy() { dd_quietf = 0; }
int dd_toggle_beep_cb(long arg) { dd_quietf = !dd_quietf; return 0; }

/* Text version of dd_scanf */
int dd_text_scanf(char *prompt, char *fmt, void *ptr)
{
    int status;
    char buffer[80];

    DD_PROMPT(prompt);			/* Print the prompt */
    co_setcursortype(_NORMALCURSOR); 	/* Turn cursur on so we can see it */

    /* Get the input string and store it away */
    buffer[0] = 78;			/* save max size of the string */
    dd_cgets(buffer);			/* get an input string */
    status = sscanf(buffer + 2, fmt, ptr);

    co_setcursortype(_NOCURSOR);		/* Turn the cursor back off */
    co_gotoxy(1, dd_rows);  co_clreol(); 	/* Erase the prompt line */

    /* Make sure that we got valid input */
    if (status <= 0) DD_PROMPT("Error: invalid input (ignored)");
    return status;
}

/* Read a string into a buffer (with prompt) */
int dd_read(char *prompt, char *address, int length)
{
    char buffer[80];

    DD_PROMPT(prompt);			/* Print the prompt */
    co_setcursortype(_NORMALCURSOR); 	/* Turn cursur on so we can see it */

    /* Get the input string and store it away */
    buffer[0] = length;			/* save max size of the string */
    dd_cgets(buffer);			/* get an input string */
    strcpy(address, buffer+2);

    co_setcursortype(_NOCURSOR);		/* Turn the cursor back off */
    co_gotoxy(1, dd_rows);  co_clreol(); 	/* Erase the prompt line */

    return 0;
}

/* Print a string on the prompt line */
void dd_text_prompt(char *s)
{
    /* Move the the bottom line of the display and clear it */
    co_gotoxy(1, dd_rows);  co_clreol();
    while (*s) co_putch(*(s++));
}

/* Output a string */
void dd_puts(DD_IDENT *dd, char *s)
{
  co_gotoxy(dd->col, dd->row);
  if (dd->reverse) {
    /* Reverse video */
    co_textbackground(dd->foreground);
    co_textcolor(dd->background);
  }
  else {
    co_textcolor(dd->foreground);
    co_textbackground(dd->background);
  }

  /* truncate string if it is too long (this needs to be a temporary
     string, so it is okay to write into it */
  if(dd->length!=-1 && strlen(s)>dd->length){
    *(s+(dd->length)-1) = '>';
    *(s+dd->length) = '\0';
  }

  while (*s)
    co_putch(*(s++));

  if (dd->reverse) {
    /* Switch back to normal video */
    co_textbackground(dd->background);
    co_textcolor(dd->foreground);
  }
}

/* Clear the entire screen */
void dd_text_cls(long arg)
{
    co_gotoxy(1, 1);			/* for backward compatibility */
    co_clrscr();
}

/* Null callback function */
int dd_nilcbk(long arg) { return 0; }

/*!
 * \fn void dd_close(void)
 * \brief Close the terminal
 *
 * The dd_close() function is used to close sparrow and return the
 * display to its original state.  It frees up all resources allocated
 * by sparrow.
 */
void dd_close()
{
    DD_PROMPT("");
    co_setcursortype(_NORMALCURSOR);

    /* Return debugging module to normal */
    dbg_pre_hook = NULL;
    dbg_post_hook = NULL;

    /* Close off any open files */
    if (errfp != NULL) fclose(errfp);

#   ifdef unix
    tc_close();
#   endif
    dd_modef=-1;
}

/* Read a key from the keyboard (includes extended key mapping) */
int dd_getc()
{
#   ifdef unix
    return tc_getc();
#   else
    extern int co_cget(void);
    return co_cget();		/* extended DOS key reader */
#   endif
}

/* Read a string from the the keyboard */
char *dd_cgets(char *s)
{
  int c, len, maxlen;
  char *p = s + 2;
  
  len = 0;
  maxlen = s[0] & 0xff;	/* get length of buffer */
  
  while(1) {
    /* Check to see if we should call any auxilliary functions */
    (void) hook_execute(dd_loop_hooks);

    if(co_kbhit()){
      switch(c = co_getch()) {
      case 0:			/* extended keycode */
	if( co_getch() != 75 ) break;      
	/* drop through on keypad left arrow */
	
      case '\b':		/* backspace key */
	if (len) {
	  /* Remove the character from the screen */
	  co_putch('\b'); co_putch(' '); co_putch('\b');
	  
	  /* Remove the character from the buffer */
	  --len; --p;
	}
	break;
	
      case '\r':		/* return: terminate string and exit */
	*p = '\0';
	s[ 1 ] = len;   
	return s+2;
	
      default:
	/* Default action: print character and store */
	if (len < maxlen - 1) {
	  co_putch(c);
	  *p++ = c;
	  ++len;
	}
      }
    }
  }
}

