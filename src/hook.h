/*!
 * \file hook.h 
 * \brief include file for using hook functions
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

#ifndef _HOOK_INCLUDED_
#define _HOOK_INCLUDED_

/* Hook list */
struct hook_list {
    int length;                 /* length of list */
    int nhooks;                 /* number of hooks currently defined */
    int lock; 
    int (**fcnlist)(void);      /* list of hook functions */
};
typedef struct hook_list HOOK_LIST;

/* Macro for allocating space */
#define DECL_HOOKLIST(name, length) \
  static int (*_##name##_list[length])() = {NULL}; \
  HOOK_LIST name##_data = {length, 0, 0, _##name##_list}; \
  HOOK_LIST *name = &name##_data;

/* Function declarations */
extern int hook_add(HOOK_LIST *, int (*)());
extern int hook_remove(HOOK_LIST *, int (*)());
extern int hook_execute(HOOK_LIST *);
extern int hook_clear(HOOK_LIST *);

/* standard hooks used in the sparrow system */
extern HOOK_LIST *hook_foreground;
extern HOOK_LIST *hook_update;
extern HOOK_LIST *hook_exit;
extern HOOK_LIST *dd_loop_hooks;

#endif


