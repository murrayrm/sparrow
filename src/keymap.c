/*!
 * \file keymap.c 
 * \brief functions for manipulating keymaps
 *
 * \author Richard M. Murray
 * \date 24 Jan 94
 *
 * Revisions:
 * 24 Jan 94 (RMM)
 *	* moved key binding functions from keymap.c
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

#include <stdlib.h>
#include <stdio.h>
#ifdef MSDOS
#include <alloc.h>
#endif
#include "keymap.h"		/* header file for this module */

/* Create a new key map */
KM_BINDING *km_createmap()
{
    register int i;
    KM_BINDING *mp;

    /* Allocate space for the table and fill with unbound functions */
    if ((mp = (KM_BINDING *) calloc(K_MAX, sizeof(KM_BINDING))) != NULL)
	for (i = 0; i < K_MAX; ++i) mp[i] = km_unbound;

    return mp;
}

/* Initialize a key map */
KM_BINDING *km_initmap(KM_BINDING *src)
{
    register int i;
    for (i = 0; i < K_MAX; ++i) src[i] = km_unbound;
    return 0;
}

/* Copy one key map to another */
KM_BINDING *km_copymap(KM_BINDING *dst, KM_BINDING *src)
{
    register int i;

    /* Copy the bindings from one map to the other */
    for (i = 0; i < K_MAX; ++i) dst[i] = src[i];

    return dst;
}

/* Free the space associated with a key map */
void km_freemap(KM_BINDING *keymap)
{
    free(keymap);
}

/* Bind a key to a function */
int km_bindkey(KM_BINDING *keymap, int key, int (*fcn)(long))
{
    /* Make sure the key and the function are valid */
    if (key >= K_MAX || key < 0) return -1;
    if (fcn == NULL) fcn = km_unbound;

    /* Assign the key to a function */
    keymap[key] = fcn;
    return 0;
}

/*
 * Key actions 
 *
 * km_unbound		don't do anything
 */

int km_unbound(long arg) { return 0; }
