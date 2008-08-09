/*
 * conio.c - mimic Borland console routines
 *
 * Richard M. Murray
 * 27 Jul 92
 *
 * This file converts Borland C console calls into termcap calls (in tclib.c).
 *
 * 30 Dec 06, RMM: added co_beep() [non-Borland]
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

#include <stdio.h>
#include <term.h>
#include <curses.h>
#include "conio.h"
#include "tclib.h"
#include "termio.h"

int co_getch()		{ return tc_getc(); }
void co_putch(int ch)	{ co_outc((int) ch); co_outf(); }
int co_kbhit() 	{ return co_cready(); }
int co_clreol() 	{ tc_clear_to_eol(0); return 0; }
void co_clrscr()	{ tc_clear(24); }
void co_beep()		{ co_putch(0x07); }

/* Set the cursor type (ignored) */
void co_setcursortype(int type) { curs_set(type); }

/* Get information about the display */
void co_gettextinfo(struct text_info *tip)
{
    tip->screenheight = tgetnum("li");
    tip->screenwidth = 80;
}

/* Goto an x, y location */
int co_gotoxy(int x, int y)
{
    tc_amove(y-1, x-1);
    return 0;
}

/* Change the foreground/background colors; key off of background */
void co_textcolor(int color) {
#ifdef COLOR
  if (tc_color) tc_setfg(color);
#endif
}

void co_textbackground(int color)
{
#ifdef COLOR
  if (tc_color) tc_setbg(color);
  else
#endif
    switch (color) {
    case 0:				/* assume BLACK = 0 */
	tc_normal();
	break;
    default:
	tc_reverse();
	break;
    }
}

