/*
 * debug.c - SST debug package
 *
 * Original: RMM 1/8/89
 * Revised: RMM, 30 Dec 03
 *
 * These routines implement a simple debugging package for handling 
 * error messages.  There are four primary routines that are defined:
 *
 *     dbg_info(fmt, ...)	informational message
 *     dbg_warn(fmt, ...)	warning message
 *     dbg_error(fmt, ...)	error message
 *     dbg_panic(fmt, ...)	something catastrophic
 *
 * Each function uses standard printf format.  For this release, the
 * messsages just print to stderr.  Future versions will send messages
 * out to a logger, etc.
 *
 * Which messages are printed are controlled by a set of global variables
 * and functions:
 *
 *     dbg_flag		        enable debugging package
 *     dbg_all			turn on debugging for all modules
 *     dbg_capture		turn on debugging for modules as seen
 *     dbg_outc			turn on screen output
 *     dbg_logf			turn on logfile output
 *     dbg_add_module(name)	print messages from file "name"
 *     dbg_delete_module	remove module from list
 *
 * Finally, there are a few functions that were original intended for
 * a command line processing program, but still might be useful:
 *
 *     dbg_execute		prompt for commands and execute them
 *     dbg_execute_command	execute a single command
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "dbglib.h"			/* turn on debugging */

/* Global variables used by debugging routines */
int dbg_flag = 0;               /* 1 => parse debug calls */
int dbg_all = 0;                /* 1 => parse all debug calls */
int dbg_outf = 0;		/* 1 => print errors to stderr */
int dbg_logf = 0;		/* 1 => send errors to log file */
int dbg_ddflg = 0;		/* 1 => display routines are running */
int dbg_capture = 0;		/* 1 => capture modules names */
FILE *dbg_outfile = NULL;	/* debug spool file handle */
FILE *dbg_logfile = NULL;	/* debug log file handle */

/* Hooks to alter behavior */
int (*dbg_pre_hook)() = NULL;	/* set up screen for display */
int (*dbg_post_hook)() = NULL;	/* return screen to proper mode */

extern int dbg_module_count;    /* number of modules in list */
extern char *dbg_module_list[]; /* list of module names */

int _dbg_puts(char *);		/* internal function to print strings */

/* Command types */
enum  cmd_token {
  Use, Unuse, Quit, List, On, Off, Help, All, Unknown, Output
};


/* Command table */
static struct cmd_entry {
    char *name;
    enum cmd_token token;
    char *help;
} cmdtbl[] = {
    {"use",     Use,        "add module to debug list"},
    {"unuse",   Unuse,      "delete module from debug list"},
    {"quit",    Quit,       "exit interactive debug mode"},
    {"show",    List,       "list currently active modules"},
    {"on",      On,         "turn debuggging on"},
    {"off",     Off,        "turn debugging off"},
    {"output",  Output,	    "turn output on/off"},
    {"help",    Help,       "print this list"},
    {"all",     All,        "turn on debugging for all modules"},
    {NULL,      Unknown,    NULL}
};

/*VARARGS2*/
/* Print a message if a module is active */
void _dbg_printf(enum dbg_type type, char *module, int line, char *fmt, ...)
{
    va_list arg_ptr;
    register int index = 0;

    /* See if we should run */
    if (!dbg_flag) return;

    /* Make sure outfile was initialized */
    if (dbg_outfile == NULL) dbg_outfile = stderr;

    /* Search module list to see if it is active */
    index = dbg_find_module(module);
    if (index < 0) {
	if (dbg_all) {
	    /* Insert the module in the list */
	  if (dbg_capture) dbg_add_module(module);
	} else
	    return;
    }

    /* Check to see if the screen display needs to be set up */
    if (dbg_pre_hook != NULL) 		/* see if screen hook exists */
      if ((*dbg_pre_hook)() < 0) 	/* call screen hook */
	return;				/* break out on error */

    /* Print the information */
    if (dbg_outf & type) {
      va_start(arg_ptr, fmt);
      fprintf(dbg_outfile, "%s [%d]: ", module, line);
      vfprintf(dbg_outfile, fmt, arg_ptr);
      if (fmt[strlen(fmt)-1] != '\n') fprintf(dbg_outfile, "\n");
      va_end(arg_ptr);
    }

    /* Let the screen display know that we are done */
    if (dbg_post_hook != NULL) (*dbg_post_hook)();

    /* Log the information */
    if ((dbg_logf & type) && dbg_logfile != NULL) {
      va_start(arg_ptr, fmt);
      time_t now = time(NULL); struct tm *timeval = localtime(&now);
      fprintf(dbg_logfile, "%2x %s [%d] @ %d.%d: ", type, module, line,
	      timeval->tm_min, timeval->tm_sec);
      vfprintf(dbg_logfile, fmt, arg_ptr);
      if (fmt[strlen(fmt)-1] != '\n') fprintf(dbg_logfile, "\n");
      va_end(arg_ptr);
    }

}

/* Open a log file */
int dbg_openlog(char *file, char *mode)
{
  /* Open up the log file */
  if ((dbg_logfile = fopen(file, mode)) == NULL) {
    dbg_error("can't open log file %s", file);
    return -1;
  }

  /* Write a header saying we are starting a new log file */
  time_t now = time(NULL);
  if (fprintf(dbg_logfile, "====\n dbg_logfile: new logfile - %s====\n", 
	      ctime(&now)) < 0) {
    dbg_error("can't write to log file %s", file);
    return -1;
  }

  /* Everything was OK; turn on file logging and return */
  dbg_logf = 0xff;
  return 0;
}

int dbg_closelog()
{
  if (dbg_logfile != NULL) fclose(dbg_logfile);
  dbg_logfile = NULL;
  dbg_logf = 0;
  return 0;
}

int dbg_flushlog()
{
  if (dbg_logfile != NULL) fflush(dbg_logfile);
  return 0;
}

/* Parse and execute a debug command */
static int dbg_execute_command(char *command)
{
    char *parse, *end;
    int index;

    /* Find the command token */
    dbg_token(command, &parse, &end);

    /* Look up the token in the list */
    for (index = 0; cmdtbl[index].name != NULL; ++index)
	if (strcasecmp(cmdtbl[index].name, parse) == 0)
	    break;
    parse = end;

    switch (cmdtbl[index].token) {
    case On:  dbg_flag = 1; _dbg_puts("DEBUG is on\n"); break;
    case Off: dbg_all = dbg_flag = 0; _dbg_puts("DEBUG is off\n"); break;
    case All: dbg_flag = dbg_all = 1; break;

    case Output:
	dbg_outf = dbg_outf ? 0 : 1;
	printf("Terminal output is %s\n", dbg_outf ? "on" : "off");
	break;

    case Use:                   /* Put module name in list */
	dbg_token(parse, &parse, &end);
	dbg_add_module(parse);
	break;

    case Unuse:
	dbg_token(parse, &parse, &end);
	dbg_delete_module(parse);
	break;

    case List:
	printf("Listing of modules currently active:\n");
	for (index = 0; index < dbg_module_count; index++)
	    printf("%s\n", dbg_module_list[index]);
	printf("Debug status = %d\n", dbg_flag);
	printf("All mode status = %d\n", dbg_all);
	break;

    case Help:
	/* Print out all the available commands */
	for (index = 0; cmdtbl[index].name != NULL; ++index) {
	    if (cmdtbl[index].help != NULL)
		printf("%s - %s\n", cmdtbl[index].name,
			   cmdtbl[index].help);
	}
	break;

    default:
	/* Command not found */
	printf("Command not recognized\n");
	break;
    }
    return(1);
}

/* Prompt for commands and execute them */
int dbg_execute(char *s)
{
    char command[80];

    /* If we were passed a command, execute it and return */
    if (s != NULL) return(dbg_execute_command(s));

    while (1) {
	printf("dbg> ");
	if (fgets(command, sizeof(command), stdin) == NULL ||
	    strncasecmp(command, "quit", 4) == 0)
	    break;

	dbg_execute_command(command);
    }
    return(1);
}

/* Return the start, end of a token */
void dbg_token(char *command, char **start, char **end)
{
    for (*start = command; isspace(**start); ++*start);
    for (*end = *start; **end != '\0' && !isspace(**end); ++*end);

    /* Terminate token; leave end pointing at the next valid char */
    if (**end != '\0') { **end = '\0'; ++*end; }
}

/*
 * Module list maintenance routines
 *
 * These routines are used to manipulate the module list
 */
#define DBG_MODULE_MAX 256      /* maximum number of modules allowed */
char *dbg_module_list[256];     /* list of modules currently active */
int dbg_module_count = 0;       /* number of modules currently active */

/* Add a module to the list */
int dbg_add_module(char *name)
{
    register int index;
    char *s;

    /* Don't overflow the module list */
    if (dbg_module_count == DBG_MODULE_MAX) return(-1);

    /* First check to see if the module is already there (exactly) */
    for (index = 0; index < dbg_module_count; ++index)
	if (strcmp(dbg_module_list[index], name) == 0)
	    return(0);

    /* Add the module to the list */
    s = dbg_module_list[dbg_module_count] =
	(char *) calloc((unsigned) strlen(name)+1, sizeof(char));
    if (s != NULL) (void) strcpy(s, name);
    ++dbg_module_count;
	 return(1);
}

/* Delete a module from the list */
int dbg_delete_module(char *name)
{
    int index, head = -1, tail;

    /* Find the module in the list */
    while ((index = dbg_find_module(name)) != -1) {
	/* Delete this module from the list */
	free(dbg_module_list[index]);
	dbg_module_list[index] = (char *) NULL;

	/* Save the first element we deleted */
	if (head == -1 || index < head) head = index;
    }

    /* Clean up the module list */
    if (head != -1) {
	for (tail = head; tail < dbg_module_count; ++tail)
	    if (dbg_module_list[tail] != NULL)
		dbg_module_list[head++] = dbg_module_list[tail];
	dbg_module_count = head;
    }
    return(1);
}

/* Find a module in the list */
int dbg_find_module(char *name)
{
    register int index;
    for (index = 0; index < dbg_module_count; ++index) {
	if (dbg_module_list[index] == NULL) continue;
	if (wildcard_match(dbg_module_list[index], name))
	    return(index);
    }
    return(-1);
}


/* Write a string to stdout and any spool files */
int _dbg_puts(char *s)
{
  if (dbg_outf) {
      if (dbg_outfile == NULL) dbg_outfile = stderr;
      fputs(s, dbg_outfile);
  }
  return(1);
}


/* Check a wildcard expression and a string to see if they match */
/* Recursive tree traversal algorithm compliments of R. Lord */
/*! These routine does not handle quotes (wildcard_check does though) !*/
#define ESCAPE_CHAR '\\'

int wildcard_match(char *expr, char *string)
{
    /* Take care of escapes first */
    if (*expr == ESCAPE_CHAR && (expr[1] == '*' || expr[1] == '?'))
	/* Match the next character exactly and continue */
	return((*(expr+1) == *string) &&
	       wildcard_match(expr+2, string+1));

    /* Check terminal conditions */
    if (*expr == '\0' && *string == '\0')
	/* Both strings are finished */
	return(1);
    else if (*expr == '\0')
	/* We still have some unmatched characters left in the string */
	return(0);
    else if (*string == '\0' && *expr != '*')
	/* We still have unmatched characters left in the wildcard expr */
	return(0);

    /* So much for the easy cases - check the next character in the expr */
    switch (*expr) {
    case '*':
	if (*string == '\0')
	    /* Make sure the rest of expr matches '\0' */
	    return(wildcard_match(expr+1, string));
	else
	    /* Match many, zero or one characters (respectively) */
	    return(wildcard_match(expr, string+1) ||
		   wildcard_match(expr+1, string) ||
		   wildcard_match(expr+1, string+1));
    case '?':
	/* Match one character */
	return(wildcard_match(expr+1, string+1));
    }
    /* Default case - this character matches and so does the rest of expr */
    return((*expr == *string) && wildcard_match(expr+1, string+1));
}

