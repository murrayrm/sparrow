/*
 * dbgdisp.c - display routines for sparrow debugger 
 *
 * \author Richard M. Murray
 * \date 7 Aug 04
 *
 * This file contains routines that are used for displaying information
 * about the debug messaging module.
 *
 */

#include "dbglib.h"
#include "display.h"
#include "conio.h"
#include "tclib.h"

extern FILE *dbg_outfile;		/* file used for output errors */
static FILE *sav_outfile;		/* save status */

/* Function to prepare screen for displaying debug messages */
int dd_dbgout_setup()
{
  co_gotoxy(1, 1);			/* move cursor to upper left */
  sav_outfile = dbg_outfile;		/* save file pointer */
  dbg_outfile = stdout;			/* Use screen display */

  return 0;
}

/* Function to return screen to proper state */
void dd_dbgout_cleanup()
{
  dbg_outfile = sav_outfile;		/* restore file pointer */
}
