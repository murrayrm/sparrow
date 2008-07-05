/*!
 * \file errlog.h
 * \brief error log capability
 * 
 * \author Richard M. Murray
 * \date 30 December 2006
 *
 * Header file for error logging capability.
 *
 * \ingroup display
 */

#ifndef ERRLOG_INCLUDED
#define ERRLOG_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

/* Function calls for setting up error logging */
extern int dd_errlog_init(int);
extern int dd_errlog_hook();
extern void dd_errlog_clear();
void dd_errlog_bindkey();

/* Callback functions */
int dd_errlog_prev(long);
int dd_errlog_next(long);
int dd_errlog_start(long);
int dd_errlog_end(long);
int dd_errprint_toggle(long);

#ifdef __cplusplus
}
#endif

#endif /* ERRLOG_INCLUDED */
