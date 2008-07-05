/*
 *
 * cygwin.c - Terminal support routines for cygwin
 *
 * Richard M. Murray
 * 26 December 2003
 *
 * This file is a replacement for termio.c for use under cygwin.  It
 * uses the system call to execute stty for switching terminal modes.  Crude
 * but it works.
 *
 * To compile sprwdisp under cygwin:
 *   1. Replace termio.* with cygwin.* in Makefile
 *   2. Remove -lcurses from Makefile
 *   3. Replace <term.h> with <termcap.h> in tclib.c, conio.c
 */
 
#include <stdlib.h>
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

/* raw mode - use cbreak to allow typeahead */
int co_setraw()
{
  system("stty raw cbreak -echo");
}

/* Reset the terminal state */
void co_unsetraw()
{
  system("stty echo -cbreak -raw");
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
