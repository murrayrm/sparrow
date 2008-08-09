/*!
 * \file errlog.c
 * \brief error log capability
 * 
 * \author Richard M. Murray
 * \date 30 December 2006
 *
 * This file implements the ability to keep a log of error messages
 * and replay elements of that log.  The functionality is implemented
 * via display hooks and only included if the dd_errlog_init()
 * function is called.
 *
 * \ingroup display
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

#include <stdlib.h>			/* for calloc() */
#include <string.h>			/* for strdup() */
#include "display.h"
#include "hook.h"
#include "errlog.h"

/* External variables accessed by the error log */
extern char dd_errlog[];		/* error log buffer */
extern int dd_errprint;			/* error print flag */
extern int dd_errlog_new;		/* flag to indicate new contents */

/* Static variables */
static char **logbuf;			/* buffered messages */
static int logsize;			/* size of error log */
static int logoff;			/* offset into log (cyclic) */
static int logcur = -1;			/* currently displayed message */
static int logcnt = 0;			/* count number of message */

/* Static functions */
static void errlog_display(int);	/* display message from log */

/*!
 * \fn dd_errlog_init(int bufsize)
 * \brief enable error logging
 *
 * This function enables the storage and replay of error messages
 * generated that are redirected from stderr.  It establishes a
 * display hook that keeps track of new error messages and stores them
 * in a buffer.
 */

int dd_errlog_init(int bufsize)
{
  /* Free up any previously allocated space */
  if (logbuf != NULL) dd_errlog_clear();

  /* Allocate the memory required for the log buffer */
  if (bufsize <= 0 ||
      (logbuf = (char **) calloc(bufsize, sizeof(char *))) == NULL) 
    return -1;
  logsize = bufsize;			/* store size of the buffer */
  logoff = 0;				/* store from start of buffer */

  /* Set up a display loop hook to store the log buffer */
  if (hook_add(dd_loop_hooks, dd_errlog_hook) < 0) return -1;

  return 0;
}

/*!
 * \fn int dd_errlog_hook()
 * \brief Hook function for storing error log
 * 
 * The dd_errlog_hook() function is a hook function for dd_loop() that
 * keeps a copy of any messages posted to the error buffer.
 *
 */
int dd_errlog_hook()
{
  if (!dd_errlog_new) return 0;		/* no action required */

  /* Make sure we have been initialized properly */
  if (logbuf == NULL || logsize == 0) return -1;

  /* Store the message in our buffer */
  if (logbuf[logoff] != NULL) free(logbuf[logoff]);
  logbuf[logoff] = strdup(dd_errlog);
  dd_errlog_new = 0;			/* reset new flag  */

  /* Increment offset for next run */
  logoff = (logoff + 1) % logsize;
  logcnt++;
  
  return 0;
}

/*!
 * \fn dd_errlog_clear()
 * \brief Clear the contents of the error log and free resources
 *
 */
void dd_errlog_clear()
{
  int i;
  if (logbuf == NULL || logsize == 0); return;
  for (i = 0; i < logsize; ++i)
    if (logbuf[i] != NULL) free (logbuf[i]);
  free(logbuf);
  logsize = logcnt = 0;
}

/*
 * Display callbacks
 *
 * The callbacks below can be used to display information from the 
 * error log.
 *
 * dd_errlog_keybind	set up standard key bindings for errlog
 * dd_errlog_prev	display previous entry in the log
 * dd_errlog_next	display next entry in the log
 * dd_errprint_toggle	turn off/on printing of stderr output
 *
 */

/*!
 * \fn void dd_errlog_bindkey()
 * \brief set up standard key binds for errlog
 *
 * This function sets up the standard key bindings for errlog:
 *   * ^A	beginning of errror log
 *   * ^E	end of errror log
 *   * ^N	next error
 *   * ^P	previous error
 *   * ^Oi	toggle printing of error messages
 */

void dd_errlog_bindkey()
{
  dd_bindkey('P'-'A' + 1, dd_errlog_prev);	/* ^P - scroll errlog */
  dd_bindkey('N'-'A' + 1, dd_errlog_next);	/* ^N - scroll errlog */
  dd_bindkey('A'-'A' + 1, dd_errlog_start);	/* ^N - start of log */
  dd_bindkey('E'-'A' + 1, dd_errlog_end);	/* ^N - end of log */
  dd_bindkey('O'-'A' + 1, dd_errprint_toggle);	/* ^O - toggle printing */

  return;
}

/*!
 * \fn int dd_errlog_start(long arg)
 * \brief Display the first message in error logging buffer
 *
 * This function allows you to scroll to the beginning of the error
 * buffer.
 */

int dd_errlog_start(long arg)
{
  errlog_display(logcur = 0);
  return 0;
}

/*!
 * \fn int dd_errlog_prev(long arg)
 * \brief Display the previous message in error logging buffer
 *
 * This function allows you to scroll backwards through the error
 * buffer.
 */

int dd_errlog_prev(long arg)
{
  /* Move to the prev message, unless we are already at beginning */
  if (logcur >= 0) --logcur;

  /* Make sure the message exists */
  if (logcur < 0) {
    DD_PROMPT("errlog: start of buffer"); 
    return 0; 
  }

  errlog_display(logcur);
  return 0;
}

/*!
 * \fn int dd_errlog_next(long arg)
 * \brief Display the next message in error logging buffer
 *
 * This function allows you to scroll forwards through the error
 * buffer.  
 */

int dd_errlog_next(long arg)
{
  /* Move to the next message, unless we are already at end */
  if (logcur < logcnt) ++logcur;

  /* Make sure the message we want exists */
  if (logcur >= logcnt) {
    DD_PROMPT("errlog: end of buffer");
    return 0; 
  }

  errlog_display(logcur);
  return 0;
}

/*!
 * \fn int dd_errlog_end(long arg)
 * \brief Display the last message in the error log
 *
 * This function allows you to scroll to the end of the error buffer.
 */

int dd_errlog_end(long arg)
{
  if (logcnt == 0) {
    DD_PROMPT("errlog: end of buffer");
    return 0;
  }
  errlog_display(logcur = logcnt-1);
  return 0;
}

/*!
 * \fn int dd_errprint_toggle(long arg)
 * \brief Toggle printing of error messages
 *
 * This function toggles whether or not error messages are printed 
 * to the screen.
 */

int dd_errprint_toggle(long arg)
{
  dd_errprint = !dd_errprint;
  return 0;
}

/*
 * Utility functions
 *
 * errlog_display	display message from error log
 *
 */

/*!
 * \fn void errlog_display(int offset)
 * \brief Display message at the current location in the log
 *
 * If the message is no longer available (because it has scrolled out
 * of the buffer), it prints an error message and sets the offset to
 * the oldest message available.
 */

void errlog_display(int offset)
{
  /* Check to make sure the message is in the buffer */
  if (logcnt - offset > logsize) { 
    DD_PROMPT("errlog: message not available; resetting to start of buffer");
    logcur = logcnt - logsize;
    return;
  }

  /* Now construct the message (with limited line length) */  
  const int msglen = 80;
  static char msgbuf[81];

  sprintf(msgbuf, "%d: ", offset);
  strncat(msgbuf, logbuf[offset % logsize], msglen-strlen(msgbuf));
  DD_PROMPT(msgbuf);

  return;
}
