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
