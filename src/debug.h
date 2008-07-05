/*
 * debug.h - header file for SST debug routines
 *
 * Richard M. Murray
 * 29 December 2003
 *
 * This is the header file for using the SST debugging routines.
 *
 * By default, this file redefines dbg_* to fprintf commands (or nothing)
 *
 * If you include "dbglib.h" instead of this file, you will get the actual
 * debug library (dbglib defines DEBUG)
 */

#include <stdio.h>

/* Set up replacement functions to eliminate linking errors */
#define dbg_info(...)
#define dbg_warn(...) fprintf(stdin, __VA_ARGS__)
#define dbg_error(...) fprintf(stderr, __VA_ARGS__)
#define dbg_fatal(...) fprintf(stderr, __VA_ARGS__)
