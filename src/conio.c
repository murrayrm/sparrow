/*
 * conio.c - mimic Borland console routines
 *
 * Richard M. Murray
 * 27 Jul 92
 *
 * This file converts Borland C console calls into termcap calls (in tclib.c).
 *
 * 30 Dec 06, RMM: added co_beep() [non-Borland]
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

