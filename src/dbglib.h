/*
 * dbglib.h - header file for enabling SST debug routines
 *
 * Richard M. Murray
 * 29 December 2003
 *
 * This is the header file for using the SST debugging routines.  This
 * file sets the DEBUG variable and then includes debug.h.
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
