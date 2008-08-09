/*!
 * \file hook.c 
 * \brief small library of routines for managing hooks
 *
 * \author Richard M. Murray
 * \date 29 Oct 94
 *
 * \ingroup hook
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





