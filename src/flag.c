/*!
 * \file flag.c 
 * \brief poor man's flags for use under unix
 *
 * \author Richard M. Murray
 * \date 4 Jan 04
 *
 * \ingroup debug
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

#include "conio.h"
#include "flag.h"

char flag_sline[FLAG_MAXFLG+1];		/* storage for flag symbol */
char flag_cline[FLAG_MAXFLG+1];		/* storage for flag character */
int flag_row = 1, flag_col = 72;	/* location of flag lines */
static int flag_initflg = 0;		/* 1 => module initialized */

/* Initalize the character lines to be blank */
static void flag_initmod() {
  int i;
  for (i = 0; i < FLAG_MAXFLG; ++i) 
    flag_sline[i] = flag_cline[i] = ' ';
  flag_sline[FLAG_MAXFLG] = flag_cline[FLAG_MAXCOL] = '\0';
  flag_initflg++;
}

/* Display the current contents of the flags line */
void flag_update() {
  int i;
  if (!flag_initflg) return;		/* leave if uninitialized */

  /* Write out the flag line */
  co_gotoxy(flag_col, flag_row);		/* move to position on screen */
  for (i = 0; i < FLAG_MAXFLG; ++i) 
    if (flag_sline[i]) co_putch(flag_sline[i] ? flag_sline[i] : ' ');

  /* Write out the status/color line */
  co_gotoxy(flag_col, flag_row+1);		/* move to position on screen */
  for (i = 0; i < FLAG_MAXFLG; ++i) 
    if (flag_cline[i]) co_putch(flag_cline[i] ? flag_cline[i] : ' ');
}

/* Set the flag in the display;  */
void flag(int offset, int symbol, int color) {
  if (!flag_initflg) flag_initmod();
  flag_sline[offset % FLAG_MAXFLG] = symbol;
  flag_cline[offset % FLAG_MAXFLG] = '0' + color % FLAG_MAXCOL;
}

/* Set the flag symbol and color separately */
void flag_symbol(int offset, int symbol) { if (!flag_initflg) flag_initmod(); }
void flag_color(int offset, int color) { if (!flag_initflg) flag_initmod(); }

void flag_init(int offset, int symbol) {
  if (!flag_initflg) flag_initmod();
  flag_sline[offset % FLAG_MAXFLG] = symbol;
  flag_cline[offset % FLAG_MAXFLG] = ' ';
}

/* Functions for setting the flag "color"; must initialized first */
void flag_clear(int offset) { flag_cline[offset % FLAG_MAXFLG] = ' '; }
void flag_on(int offset) { flag_cline[offset % FLAG_MAXFLG] = '+'; }
void flag_off(int offset) { flag_cline[offset % FLAG_MAXFLG] = '-'; }
void flag_error(int offset) { flag_cline[offset % FLAG_MAXFLG] = '!'; }



