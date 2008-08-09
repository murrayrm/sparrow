/*
 * tclib.c - termcap interface
 *
 * \author Richard M. Murray
 * \date 27 Jul 92
 *
 * This file provides an interface to the unix termcap library.  All
 * display functions should go through this file.  This file was originally
 * written for support under Borland C on the IBM PC, so it has calls
 * to functions that were defined there.  These functions must be defined
 * in the OS-specific files included with the sparrow distribution.
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
#include <term.h>
#include <string.h>
#include <signal.h>
#include "tclib.h"
#include "keymap.h"		/* extended keycodes */
#include "termio.h"

extern void *calloc();

char *termtype = NULL;		/* user-specified terminal type */
int visual = 0;			/* visual mode flag (set block cursor) */
static int visual_flag = 0;     /* keep track of actual visual mode */

#define TERMVAR		"TERM"	/* terminal environment variable */

/* Termcap declarations */
extern char PC, *BC, *UP;
extern short ospeed;

/* Global variables */
int tc_color = 0;		/* color capable monitor */

/* Static function declarations */
static int tc_check_match(unsigned char *, int *, int *);

/* Termcap capabilities - see termcap(5) & tc_init() */
static char *bc, *cr, *dn, *nd, *up, *cd, *ke, *ks, *vs, *ve, *cl;
static char *al, *ce, *ch, *dc, *dl, *ei, *im, *ic, *bl, *ti, *te;
static char *cm, *so, *se, *AF, *AB, *md, *mh, *me;
static int co, li;              /* Number of columns/lines on the screen */
static char imf;                /* Insert mode flag */
static int dc_len;              /* Cost of deleting a single character */

char tc_bp[1024];               /* Termcap buffer */
#define CAPBUF_SIZE 1024

/* Save default character strings in known locations */
static char *tc_cr = "\r", *tc_nl = "\n", *tc_bs = "\b", *tc_bl = "\007";
char *capbuf = NULL;

/* Save keypad mappings */
struct keypad_entry {
    char *name;                 /* Termcap capability name */
    int code;                   /* Key code */
    char *seq;                  /* Keypad character sequence */
} key[] = {
	{"kl", K_LEFT},    {"kr", K_RIGHT},   {"ku", K_UP},
	{"kd", K_DOWN},    {"kh", K_HOME},    {"ke", K_END},
	{"ki", K_INS},     {"kx", K_DEL},     {"k0", K_F10},
	{"k1", K_F1},      {"k2", K_F2},      {"k3", K_F3},
	{"k4", K_F4},      {"k5", K_F5},      {"k6", K_F6},
	{"k7", K_F7},      {"k8", K_F8},      {"k9", K_F9},
	{NULL, -1}
};

/* Initialize strings used by termcap routines */
int tc_init(int vflg)
{
    int i;
    char *area;

    if (capbuf == NULL) {
	char *term = termtype;
	if (term == NULL) term = getenv(TERMVAR);

#       ifdef BSD42
	/* Make sure TERM exists for BSD42 (otherwise we dump core) */
	if (term == NULL || tgetent(tc_bp, term) != 1) {
	    if (vflg) 
		fprintf(stderr, "can't initialize terminal \"%s\"\n",term);
	    return -1;
	}

#       else
	if (tgetent(tc_bp, term) != 1) {
	    if (vflg)
		fprintf(stderr, "can't initialize terminal \"%s\"\n",term);
	    return -1;      /* JAD 8/90 */
	}
#       endif

	/* Allocate space for all the capabilities */
	if ((capbuf = (char *) calloc(CAPBUF_SIZE, 1)) == NULL) return -1;
	area = capbuf;

	/* Initialize capability strings */
	al = tgetstr("al", &area);
	bc = tgetflag("bs") ? tc_bs : tgetstr("bc", &area);
	if (bc == NULL) bc = tgetstr("kb", &area);
	ce = tgetstr("ce", &area);
	cd = tgetstr("cd", &area);
	cl = tgetstr("cl", &area);
	ch = tgetstr("ch", &area);
	if ((cr = tgetstr("cr", &area)) == NULL) cr = tc_cr;
	dl = tgetstr("dl", &area);
	dn = tc_nl;
	nd = tgetstr("nd", &area);
	up = tgetstr("up", &area);
	im = tgetstr("im", &area);
	ic = tgetstr("ic", &area);
	ei = tgetstr("ei", &area);
	imf = (im != NULL) || (ic != NULL);
	co = tgetnum("co");
	li = tgetnum("li");  if (li == 0) li = 24;
	dc = tgetstr("dc", &area);
	cm = tgetstr("cm", &area);
	ks = tgetstr("ks", &area);	ke = tgetstr("ke", &area);
	vs = tgetstr("vs", &area);	ve = tgetstr("ve", &area);
	so = tgetstr("so", &area);	se = tgetstr("se", &area);
	ti = tgetstr("ti", &area);  te = tgetstr("te", &area);
	if ((bl = tgetstr("bl", &area)) == NULL) bl = tc_bl;
	md = tgetstr("md", &area);	me = tgetstr("me", &area);
	mh = tgetstr("mh", &area);

	AF = tgetstr("AF", &area);	AB = tgetstr("AB", &area);
	if (AF != NULL) tc_color = 1;

	/* Keep track of the cost to delete a character */
	dc_len = (dc == NULL) ? 100 : strlen(dc);

	/* Keyboard capabilities */
	for (i = 0; key[i].name != NULL; ++i)
	    key[i].seq = tgetstr(key[i].name, &area);

	/* Make sure we have the basic capabilities */
	if (bc == NULL || nd == NULL || up == NULL) {
	    if (vflg) fprintf(stderr, "terminal does not support editing\n");
	    tc_free();
	    return -1;
	}

	/* If this terminal has a cursor motion mode, run SST in that mode */
	/* This is needed for the sun command window */
	tc_enter_termcap();

	/* For sparrow, run everything in bold colors */
	if (md != NULL) tputs(md, 1, co_outc);
    }
    return 0;
}

/* Open the terminal for editing */
void tc_open()
{
    /* Make sure all output is echoed */
    fflush(stdout);

    /* Ignore changes in window size */
    signal(SIGWINCH, SIG_IGN);

    /* Set up the terminal mode */
    co_setraw();

    /* Initialize keypad mode and set up cursor */
    if (visual) {
	visual_flag = 1;
#       ifdef HARDWARE_CURSOR
	setvisual();
#       endif
	if (vs != NULL) tputs(vs, 1, co_outc);
    }
    if (ks != NULL) tputs(ks, 1, co_outc);         /* enter keypad mode */
}

/* Read a keystroke and decode it */
int tc_getc()
{
    int c, partial_match, full_match;
    static unsigned char sequence[16];
    static int seqcount = 0, seqoffset = 0;

    /* See if we have any characters from a mismatched sequence */
    if (seqcount != 0) {
#       ifdef RMM
	dbg_printf("tc_getc", "using stored sequence (%d)\r", seqcount);
#       endif
	--seqcount;

#       ifdef RMM
	dbg_printf("tc_getc", "returning %d\r", sequence[seqoffset+1]);
#       endif
	return(sequence[++seqoffset]);
    }

    /* Start storing the sequence at the beginning */
    seqoffset = 0;

    /* Read another character from the terminal */
    while ((c = co_cget()) != EOF) {

	/* Store the character in the next sequence spot */
	sequence[seqoffset++] = c;
	sequence[seqoffset] = '\0';
	++seqcount;

	/* See if the sequence matches anything */
	c = tc_check_match(sequence, &partial_match, &full_match);

	/* If we had a full match or no partial matches we are done */
	if (full_match || !partial_match) {
	    if (full_match) {
		/* Clear the sequence buffer */
		seqoffset = seqcount = 0;
	    } else {
		/* Return the first character in the sequence */
		seqoffset = 0;
		--seqcount;
	    }
	    break;
	}
#       ifdef RMM
	dbg_printf("tc_getc", "storing sequence (%d)\r", seqcount);
#       endif
    }

    /* Return the key code */
#   ifdef RMM
    dbg_printf("tc_getc", "returning %d\r", c);
#   endif
    return(c);
}

/* Check a key sequence for matches */
static int tc_check_match(unsigned char *s, int *partial, int *full)
{
    int i, len = strlen((char *)s);

    /* We start out with no matches */
    *partial = *full = 0;

    /* See if the sequence matches anything */
    for (i = 0; key[i].name != NULL; ++i) {
	/* This first character may be '\0' so compare by hand */
	if (key[i].seq != NULL && key[i].seq[0] == *s) {
	    /* Check for a partial match */
	    if (len == 1 || strncmp(key[i].seq+1, (char *)(s+1), len-1) == 0)
		++(*partial);

	    /* Check for a full match */
	    if (strcmp(key[i].seq+1, (char *)(s+1)) == 0) {
		++(*full);
		break;
	    }
	}
    }
    /* Return the key code if there was a match, otherwise the first char */
    return (*full ? key[i].code : *s);
}

/* Move the cursor from one point to another */
void tc_rmove(int row, int col, int nrow, int ncol)
{
    /* Row motion - assume this lies on the screen */
    while (row < nrow) { tputs(dn, 1, co_outc); ++row; }
    while (row > nrow) { tputs(up, 1, co_outc); --row; }

    /* Trap cursor at left edge of display */
    if (ncol >= co-1) ncol = co-2;
    if ( col >= co-1)  col = co-2;

    /* Check cursor motion for special cases */
    if (ncol == 0) {
	/* This is too easy to ignore */
	tputs(cr, 1, co_outc);

    } else if (ch != NULL) {
	/* Use directly horizontal motion if it is available */
	if (ncol != col) tputs(tgoto(ch, ncol, row), 1, co_outc);

    } else {
	/* Use cursor motion commands */
	while (col < ncol) { tputs(nd, 1, co_outc); ++col; }
	while (col > ncol) { tputs(bc, 1, co_outc); --col; }
    }
}

/* Move to an absolute cursor position */
void tc_amove(int row, int col)
{
    if (cm != 0) tputs(tgoto(cm, col, row), 1, co_outc);
}

/* Clear the the end of the line and leave the cursor in place */
void tc_clear_to_eol(int col)
{
    if (ce != NULL)
	tputs(ce, 1, co_outc);

    else {
	int ocol = col;

	/* Write out blanks and go back to where we started */
	while (col++ < co - 1) co_outc(' ');
	tc_rmove(0, col, 0, ocol);
    }
}

/* Clear to the end of the display */
void tc_clear(int n)
{
    if (cd != NULL)
	tputs(cd, n, co_outc);

    else {
	int row = 0;
	while (row < n) { tc_clear_to_eol(0); tputs(dn, 1, co_outc); ++row; }
	tc_rmove(row, 0, 0, 0);
    }
}

/* Insert a string of characters */
void tc_insert(char *s, int len, int col)
     // char *s;                        /* Contents of the rest of the line */
     // int len;                        /* Length of insertion */
     // int col;                        /* Current column */
{
    if (imf) {
	/* Use the insert capability */
	if (im != NULL) tputs(im, 1, co_outc);
	while (len-- > 0) {
	    if (ic != NULL) tputs(ic, 1, co_outc);
	    co_outc((int) *s++);
	}
	if (ei != NULL) tputs(ei, 1, co_outc);

    } else {
	/* Print the rest of the line */
	char *p = s;
	int ocol = col;

	while (*p && col < co-1) { co_outc((int) *p++); ++col; }
	tc_rmove(0, col, 0, ocol);  co_outc((int) *s);
    }
}

/* Delete characters under the cursor */
void tc_delete(char *s, int n, int col)
     // char *s;                        /* Remainder of the line */
     // int n;                          /* Number of characters to delete */
     // int col;                        /* Current column */
{
    if (dc != NULL) {
	/* Use the delete character capability */
	while (n-- > 0) tputs(dc, 1, co_outc);

    } else {
	int ocol = col;

	/* Redraw the rest of the line and blank trailing characters */
	while (*s && col < co-1) { co_outc((int) *s++); ++col; }
	while (n-- > 0 && col < co-1) { co_outc((int) ' '); ++col; }
	tc_rmove(0, col, 0, ocol);
    }
}

/* Enter termcap mode (save screen; to be used in *cooked* mode only) */
void tc_enter_termcap()
{
    if (ti != NULL) {
	co_setraw();
	tputs(ti, 1, co_outc);                     /* enter visual mode */
	if (cl != NULL) tputs(cl, li, co_outc);    /* clear the new screen */
	co_unsetraw();
    }
}

/* Leave termcap mode (restore screen; to be used in *cooked* mode only) */
void tc_exit_termcap()
{
    if (te != NULL) {
	co_setraw();
	tputs(te, 1, co_outc);     /* exit visual mode */
	co_outf();                 /* make sure the data is sent */
	co_unsetraw();

    }
}

/* Print a space under the cursor and leave cursor in place */
void tc_space() { co_outc((int) ' '); tputs(bc, 1, co_outc); }

/* Short utility routines */
void tc_bell() { tputs(bl, 1, co_outc); }  /* ring the bell */
void tc_down() { tputs(dn, 1, co_outc); }  /* linefeed */
int tc_lines() { return(li); }          /* return screen length */

/* Close the terminal for editing - back to cooked mode */
void tc_close()
{
    /* Take terminal out of keypad transmit mode and fix cursor */
    if (visual_flag) {
	visual_flag = 0;
#       ifdef HARDWARE_CURSOR
	unsetvisual();
#       endif
	if (ve != NULL) tputs(ve, 1, co_outc);
    }
    if (ke != NULL) tputs(ke, 1, co_outc);

    /* Reset the terminal mode */
    (void) co_unsetraw();
}

/* Free termcap storage */
void tc_free()
{
    /* Free capabilities if they still exist */
    if (capbuf != NULL) {
	tc_exit_termcap();      /* exit termcap mode (restore screen) */
	/* Get rid of capabilities */
	free(capbuf);
	capbuf = NULL;
	free(termtype);
    }
}

/* Switch between normal and reverse video */
void tc_reverse() { if (so != NULL) tputs(so, 1, co_outc); }
void tc_normal() { if (so != NULL) tputs(se, 1, co_outc); }

/* Set foregrcound and background color */
void tc_setfg(int color) { 
  //  if (me != NULL) tputs(me, 1, co_outc);
  //  if (color > 7 && md != NULL) tputs(md, 1, co_outc);
  if (AF != NULL) tputs(tparm(AF, color%8 ,1), 1, co_outc); 
}
void tc_setbg(int color) { 
  //  if (me != NULL) tputs(me, 1, co_outc);
  //  if (color > 7 && md != NULL) tputs(md, 1, co_outc);
  if (AB != NULL) tputs(tparm(AB, color%8, 1), 1, co_outc); 
}
