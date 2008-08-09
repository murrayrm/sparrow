/*
 * \file curslib.c 
 * \brief curses/terminfo display management
 *
 * \author Richard M. Murray
 * \date 27 Jul 92
 *
 * This file provides an interface to the unix curses library.  All
 * display functions should go through this file.  This file replaces
 * tclib.c, conio.c and termio.c in the original sparrow distribution.
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
#include <curses.h>
#include "keymap.h"		/* extended keycodes */

/* Functions defined later in this file */
int co_cget();

/* Initialize strings used by termcap routines */
int tc_init(int vflg) 
{ 
  /* This function is not used by the curses version */
  return 0; 
}

/* Open the terminal for editing */
void tc_open()
{
  (void) initscr();      /* initialize the curses library */
  keypad(stdscr, TRUE);  /* enable keyboard mapping */
  (void) nonl();         /* tell curses not to do NL->CR/NL on output */
  (void) cbreak();       /* take input chars one at a time, no wait for \n */
  (void) noecho();       /* no echoing of input */

  /* Initialize colors */
  if (has_colors()) {
    start_color();
    /* Set up colors according to sparrow definitions */
    init_pair( 1, COLOR_RED,     COLOR_BLACK);
    init_pair( 2, COLOR_GREEN,   COLOR_BLACK);
    init_pair( 3, COLOR_YELLOW,  COLOR_BLACK);
    init_pair( 4, COLOR_BLUE,    COLOR_BLACK);
    init_pair( 5, COLOR_CYAN,    COLOR_BLACK);
    init_pair( 6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair( 7, COLOR_WHITE,   COLOR_BLACK);
    init_pair( 8, COLOR_BLACK,   COLOR_RED);
    init_pair( 9, COLOR_BLACK,   COLOR_GREEN);
    init_pair(10,COLOR_BLACK,    COLOR_YELLOW);
    init_pair(11,COLOR_BLACK,    COLOR_BLUE);
    init_pair(12,COLOR_BLACK,    COLOR_CYAN);
    init_pair(13,COLOR_BLACK,    COLOR_MAGENTA);
    init_pair(14,COLOR_BLACK,    COLOR_WHITE);
  }
}

/* Read a keystroke and decode it */
/*! This should be more intelligent !*/
int tc_getc()
{
  return co_cget();
}

/* Move to an absolute cursor position */
void tc_amove(int row, int col)
{
  move(row, col);
}

/* Enter termcap mode (save screen; to be used in *cooked* mode only) */
void tc_enter_termcap()
{
}

/* Leave termcap mode (restore screen; to be used in *cooked* mode only) */
void tc_exit_termcap()
{
}

/* Close the terminal for editing - back to cooked mode */
void tc_close()
{
    nocbreak();
    echo();
    endwin();
}

/*
 * conio.c - mimic Borland console routines
 *
 * Richard M. Murray
 * 27 Jul 92
 *
 * This file converts Borland C console calls into termcap calls (in tclib.c)
 *
 */

#include <stdio.h>
#include <term.h>
#include <curses.h>
#include "conio.h"
#include "tclib.h"
#include "termio.h"

int co_getch()		{ return tc_getc(); }
void co_putch(int ch)	{ co_outc((int) ch); co_outf(); }
int co_kbhit()		{ return co_cready(); }
int co_clreol() 	{ clrtoeol(); return 0; }
void co_clrscr()	{ erase(); redrawwin(stdscr); refresh(); }
void co_beep()		{ beep(); }

/* Set the cursor type (ignored) */
void co_setcursortype(int type) { curs_set(type); }

/* Get information about the display */
void co_gettextinfo(struct text_info *tip)
{
  int maxrows, maxcols;
  getmaxyx(stdscr, maxrows, maxcols);
  tip->screenheight = maxrows;
  tip->screenwidth = maxcols;
}

/* Goto an x, y location */
int co_gotoxy(int x, int y)
{
  tc_amove(y-1, x-1);
  refresh();
  return 0;
}

/*
 * Color management
 *
 * These routines have to map the color scheme for sparrow/termcap into
 * the color pair scheme of curses.
 *
 */

static int foreground = 7, background = 0;
void co_setcolor(int fg, int bg)
{
  /* Look for the best matching color pair */
  /* For now, only support simple reversed color pairs */
  switch ((fg & 0x0f) | ((bg << 4) & 0xf0)) {
  case 0x01:	color_set( 1, NULL);	break;
  case 0x02:	color_set( 2, NULL);	break;
  case 0x03:	color_set( 3, NULL);	break;
  case 0x04:	color_set( 4, NULL);	break;
  case 0x05:	color_set( 5, NULL);	break;
  case 0x06:	color_set( 6, NULL);	break;
  case 0x07:	color_set( 7, NULL);	break;
  case 0x10:	color_set( 8, NULL);	break;
  case 0x20:	color_set( 9, NULL);	break;
  case 0x30:	color_set(10, NULL);	break;
  case 0x40:	color_set(11, NULL);	break;
  case 0x50:	color_set(12, NULL);	break;
  case 0x60:	color_set(13, NULL);	break;
  case 0x70:	color_set(14, NULL);	break;
  default:	color_set( 0, NULL); 	break;
  }
}


/* Change the foreground/background colors; key off of background */
void co_textcolor(int color) {
  co_setcolor(foreground = color, background);
}

void co_textbackground(int color)
{
  co_setcolor(foreground, background = color);
}

/*
 *
 * termio.c - Terminal support routines for unix
 *
 * Richard M. Murray
 * November 7, 1987
 *
 * This file contains low level routines for manipulating screen input
 * and output.  These functions were originally defined as part of the
 * Borland C library under MSDOS, but have been converted here to run
 * under unix.  If BSD is defined, we make use of BSD system calls,
 * otherwise we use curses() equivalents.
 */
 
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/select.h>

/*
 * Input functions
 *
 * co_cget		Input a single character
 * co_cready		Check to see if a character is ready
 *
 * These routines are derived from the routines in Rochkind[85] and [88].
 */

#include <curses.h>

/* Read a character from the keyboard */
int co_cget()
{
  int ch;

  switch (ch = getch()) {
  case KEY_DOWN:	ch = K_DOWN;		break;
  case KEY_UP:		ch = K_UP;		break;
  case KEY_LEFT:	ch = K_LEFT;		break;
  case KEY_RIGHT:	ch = K_RIGHT;		break;
  case KEY_HOME:	ch = K_HOME;		break;
  case KEY_END:		ch = K_END;		break;
  case KEY_PPAGE:	ch = K_PGUP;		break;
  case KEY_NPAGE:	ch = K_PGDN;		break;
  case KEY_IC:		ch = K_INS;		break;
  case KEY_DC:		ch = K_DEL;		break;
  case KEY_BACKSPACE:	ch = K_BS;		break;

  case KEY_F(1):	ch = K_F1;		break;
  case KEY_F(2):	ch = K_F2;		break;
  case KEY_F(3):	ch = K_F3;		break;
  case KEY_F(4):	ch = K_F4;		break;
  case KEY_F(5):	ch = K_F5;		break;
  case KEY_F(6):	ch = K_F6;		break;
  case KEY_F(7):	ch = K_F7;		break;
  case KEY_F(8):	ch = K_F8;		break;
  case KEY_F(9):	ch = K_F9;		break;
  case KEY_F(10):	ch = K_F10;		break;

  default:
    if (ch > K_MAX) ch = 0;
    break;
  }
  return ch;
}

/* See how many characters are in the input stream */
int co_cready()
{
    fd_set fds;
    struct timeval tv;

   /* Check to see if a key has been pressed */
    FD_ZERO ( &fds ); FD_SET ( 0, &fds );
    tv.tv_sec = 0; tv.tv_usec = 0;

    return select( 1, &fds, NULL, NULL, &tv);
}

/*
 * Output functions
 *
 * co_outc				Output a single character
 * co_outf				Flush the output buffer
 *
 */

int co_outc(int c) 
{
  return addch(c);
}

void co_outf()  { refresh(); }
