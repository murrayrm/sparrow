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
#include <sys/ioctl.h>
#include <sys/select.h>

/*
 * Input functions
 *
 * co_setraw		Initialize terminal for raw input
 * co_unsetraw		Restore the terminal flags
 * co_cgetc		Input a single character
 * co_cready		Check to see if a character is ready
 *
 * These routines are derived from the routines in Rochkind[85] and [88].
 */

#ifdef BSD
#include <sgtty.h>
static struct sgttyb tbufsave;
static struct tchars tchrsave;
static struct ltchars ltchsave;
#else
#include <curses.h>
#endif

extern short ospeed;		/* Terminal speed for termcap routines */
#ifdef STANDALONE
short ospeed;			/* info routines just need co_setraw() */
#endif

/* raw mode - use cbreak to allow typeahead */
int co_setraw()
{
#ifdef BSD
    struct sgttyb tbuf;
    struct tchars tchr;
    struct ltchars ltch;

    ioctl(0, TIOCGETP, &tbuf); ioctl(0, TIOCGETP, &tbufsave);
    ioctl(0, TIOCGETC, &tchr); ioctl(0, TIOCGETC, &tchrsave);
    ioctl(0, TIOCGLTC, &ltch); ioctl(0, TIOCGLTC, &ltchsave);

    ospeed = tbuf.sg_ospeed;
    /* tbuf.sg_flags |= RAW | CBREAK; */
    tbuf.sg_flags |= CBREAK;
    tbuf.sg_flags &= ~(CRMOD | ECHO);
    ioctl(0, TIOCSETN, &tbuf);		/* leave characters on stack */

    /* Turn off special character processing */
    tchr.t_intrc = '\377';  tchr.t_quitc = '\377';
    tchr.t_eofc = '\377';   tchr.t_brkc = '\377';

    /* Leave ^S and ^Q alone for hss.caltech.edu and others */
    /* tchr.t_startc = '\377'; tchr.t_stopc = '\377'; */
    ioctl(0, TIOCSETC, &tchr);

    /* Turn off BSD special character processing */
    ltch.t_suspc = '\377';   ltch.t_dsuspc = '\377';
    ltch.t_rprntc = '\377';  ltch.t_flushc = '\377';
    ltch.t_werasc = '\377';  ltch.t_lnextc = '\377';
    ioctl(0, TIOCSLTC, &ltch);
#else
    initscr();
    cbreak();
    noecho();
#endif

    return 0;
}

/* Reset the terminal state */
void co_unsetraw()
{
#ifdef BSD
    (void) ioctl(0, TIOCSETP, &tbufsave);
    (void) ioctl(0, TIOCSETC, &tchrsave);
    (void) ioctl(0, TIOCSLTC, &ltchsave);
#else
    nocbreak();
    echo();
    endwin();
#endif
}

/* Read a character from the keyboard */
int co_cget()
{
#   ifndef RMM
    /* Only send back 7 bits */
    int ch = fgetc(stdin);
    return(ch == -1 ? -1 : (ch&0x7f));

#   else
    /* Send back all 8 bits for debugging tvi920c */
    return(fgetc(stdin));
#   endif
}

/* See how many characters are in the input stream */
int co_cready()
{
#ifdef BSD
    int x;			/* holds # of pending chars */
    return((ioctl(0,FIONREAD,&x) < 0) ? 0 : x);
#else
    fd_set fds;
    struct timeval tv;

   /* Check to see if a key has been pressed */
    FD_ZERO ( &fds ); FD_SET ( 0, &fds );
    tv.tv_sec = 0; tv.tv_usec = 0;

    return select( 1, &fds, NULL, NULL, &tv);
#endif
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
    return fputc(c, stdout);
}

void co_outf()  { (void) fflush(stdout); }
