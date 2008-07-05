/*!
 * \file hook.c 
 * \brief small library of routines for managing hooks
 *
 * \author Richard M. Murray
 * date 29 Oct 94
 *
 * Copyright (c) 1996, California Institute of Technology
 * Permission is granted to copy, modify, and reditribute this file
 * provided this header message is retained. 
 *
 * \ingroup hook
 */

#include <stdio.h>
#include "hook.h"

/* our standard sparrow hooks */

DECL_HOOKLIST(hook_foreground,8);
DECL_HOOKLIST(hook_update,8);
DECL_HOOKLIST(hook_exit,8);


/*! Add a function to a hook list !*/
int hook_add(HOOK_LIST *hl, int (*fcn)(void))
{ 
  hl->lock=1;
  if(hl->nhooks<hl->length)
    hl->fcnlist[hl->nhooks++] = fcn;
  hl->lock=0;
 return hl->nhooks;
}

/*! Remove a function from a hook list !*/
int hook_remove(HOOK_LIST *hl, int (*fcn)(void))
{
  register int i;
  hl->lock=1; 
  /* Look for the function in the list; return -1 if not found */
  for (i = 0; i < hl->nhooks; ++i) if (hl->fcnlist[i] == fcn) break;
  if (i == hl->nhooks) return -1;
  
  /* Shift other hooks down to close up gap */
  for (hl->nhooks--; i < hl->nhooks; ++i) hl->fcnlist[i] = hl->fcnlist[i+1];
  hl->fcnlist[hl->nhooks] = NULL;
  
  hl->lock=0;
  /* Return the number of hooks that are left */
  return hl->nhooks;
}

/*! Clear all hooks in this list !*/
int hook_clear(HOOK_LIST *hl)
{ hl->lock=1; 
  hl->nhooks = 0;
  hl->lock=0;
  return 0;
}

/*! Call all functions on a hook list !*/
int hook_execute(HOOK_LIST *hl)
{
  int i, status = 0;

  if(hl->lock!=0)return 0;

  /* Execute hooks in order; abort on error */
  for (i = 0; i < hl->nhooks; ++i) {
    status = (hl->fcnlist[i])();
    if (status < 0) break;
  }
  return status;
}





