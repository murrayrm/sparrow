/*
 * devlut.c - device lookup table for all known devices 
 *
 * \author Richard M. Murray
 * \date 25 Oct 94
 *
 * This table only gets linked in if you don't provide a table.  Since
 * it includes all known drivers, if you are worried about space you
 * should provide your own table.
 */

#include <stdio.h>
#include "channel.h"

extern int virtual_driver(DEV_ACTION, ...);
extern int fcn_driver(DEV_ACTION, ...);
extern int sertest_driver(DEV_ACTION, ...);

DEV_LOOKUP chn_devlut[CHN_MAXDEV] = {		
    {"virtual", virtual_driver},	/* virtual channels */
    {"function-gen", fcn_driver},	/* fcn_gen.c */
    {"sertest", sertest_driver},	/* sertest.c */
    {NULL, NULL}			/* end of table */
};
