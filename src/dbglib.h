/*
 * dbglib.h - header file for enabling sparrow debug routines
 *
 * Richard M. Murray
 * 29 December 2003
 *
 * This is the header file for using the sparrow debugging routines.  This
 * file sets the DEBUG variable and then includes debug.h.
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

#ifndef DBGLIB
#define DBGLIB

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

enum dbg_type {DBG_INFO=0x01, DBG_WARN=0x02, DBG_ERROR=0x04, DBG_PANIC=0x08};

/* Define dbg_printf to give us the file name and line of the message */
#define dbg_info(...) _dbg_printf(DBG_INFO, __FILE__, __LINE__, __VA_ARGS__);
#define dbg_warn(...) _dbg_printf(DBG_WARN, __FILE__, __LINE__, __VA_ARGS__);
#define dbg_error(...) _dbg_printf(DBG_ERROR, __FILE__, __LINE__, __VA_ARGS__);
#define dbg_panic(...) _dbg_printf(DBG_PANIC, __FILE__, __LINE__, __VA_ARGS__);

/* Debug routines */
void _dbg_printf(enum dbg_type, char *file, int line, char *fmt, ...);
int dbg_openlog(char *, char *);
int dbg_closelog();
int dbg_flushlog();
int dbg_execute(char *);
void dbg_token(char *, char **, char **);
int dbg_add_module(char *);
int dbg_delete_module(char *);
int dbg_find_module(char *);
int wildcard_match(char *, char *);

/* Global variables */
extern int dbg_flag;			/* print debug information? */
extern int dbg_all;			/* debug all modules? */
extern int dbg_outf;			/* print debugging output (+ mask) */
extern int dbg_logf;			/* log debugging output (+ mask) */
extern FILE *dbg_infofile;		/* file for informational output */
extern FILE *dbg_warnfile;		/* file for warning output */
extern FILE *dbg_errorfile;		/* file for error output */
extern FILE *dbg_panicfile;		/* file for panic output */
  
#ifdef __cplusplus
}
#endif

#endif
