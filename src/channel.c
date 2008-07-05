/*
 * channel.c - i/o routines for channel interface library
 * 
 * Richard M. Murray
 * 17 January 1993
 * 
 */

#include <stdio.h>
#include <string.h>
#include "channel.h"
#include "hook.h"

/* Local function declarations */
int chn_filter(CHANNEL *cp);

/* Allocate space for write hooks */
DECL_HOOKLIST(chn_write_hooks, 4);

/*
 * Main channel I/O routines
 *
 * chn_init	        (re-)initialize the channel interface
 * chn_read		read all input channels
 * chn_write		write all output channels
 * chn_close            re-initialize all channels
 * chn_add_device       add device driver to table
 * chn_zero             zero a specific channel
 *
 */

/* Initialize the channel package from file */
int chn_init()
{
    register int dev, offset;
    DEVICE *dp = chn_devtbl;

    int i, j, k;
    FILTER *filtp;

    /* Go through and initialize each driver */
    for (offset = dev = 0; dev < chn_ndev; ++dev) {
	(dp->driver) (Init, dp, chn_chantbl + offset);
	offset += dp->size;
	++dp;
    }

    /*
     * Initialize channel filters; make the chn_filters list of channels that
     * get filtered and set initial values for the filters
     */
    for (i = 0, j = 0; i < chn_nchan; i++) {
	/* See if there is a filter on this channel */
	if ((filtp = chn_chantbl[i].filter) != NULL) {
	    /* add channel to filter list */
	    chn_filters[j++] = i;

	    /* set filter data indices */
	    filtp->xi = 0;
	    filtp->yi = 0;

	    /* zero out inital values of filter */
	    for (k = 0; k < filtp->nb; k++) filtp->x[k] = 0;
	    for (k = 0; k < filtp->na; k++) filtp->y[k] = 0;
	}
    }
    chn_filters[j] = -1;	/* mark end of list */

    /* Return the number of devices installed */
    return chn_ndev;
}

/* Read and process input channels */
int chn_read()
{
    DEVICE *dp = chn_devtbl;
    int chni, offset = 0;
    int *filtchns;

    /* Read data from hardware */
    for (chni = 0; chni < chn_ndev; ++chni) {
	/* Call device driver to perform read/conversion */
	(*dp->driver) (Read, dp, chn_chantbl + offset);

	/* Update the offset into the channel table */
	offset += dp->size;
	++dp;
    }

    /* Do any desired channel filtering */
    filtchns = chn_filters;
    while (*filtchns != -1) {	/* end of filter list indicated by -1 */
	chn_filter((chn_chantbl + *filtchns));
	filtchns++;
    }

    return 0;
}

/* Filter a channel */
int chn_filter(CHANNEL *cp)
{
    double store = 0.0;
    FILTER *fp = cp->filter;
    int di, ci;			/* data and coefficient indexes */

    if (fp->nb > 0) {
	di = fp->xi + 1;	/* find current index for input */
	if (di == fp->nb)
	    di = 0;		/* go to beginning of circular buffer if at
				 * end */
	fp->x[di] = cp->data.d;	/* store current input in x history buffer */
	fp->xi = di;		/* remember where the current x went */

	/* add linear combination of past inputs */
	ci = 0;			/* index into b coefficients */
	while (ci < fp->nb) {	/* haven't gone thru all coeffs yet */
	    store += fp->b[ci] * fp->x[di];
	    if (di == 0)
		di = fp->nb;	/* if at beginning, go to end */
	    di--;
	    ci++;
	}
    }
    if (fp->na > 0) {
	/* add linear combination of past outputs */
	di = fp->yi;		/* yi points to last output */
	ci = 0;			/* index into a coefficients */
	while (ci < fp->na) {	/* haven't gone thru all coeffs yet */
	    store -= fp->a[ci] * fp->y[di];
	    if (di == 0)
		di = fp->na;	/* if at beginning, go to end */
	    di--;
	    ci++;
	}

	di = fp->yi + 1;	/* find current index for output */
	if (di == fp->na)
	    di = 0;
	fp->yi = di;		/* remember where we put the current y */
	fp->y[di] = store;	/* store current output in y history buffer */
    }
    chn_data(fp->out_chn) = store;	/* also put output in correct chan */
    return 0;
}


/* Write and process input channels */
int chn_write()
{
    DEVICE *dp = chn_devtbl;
    int chni, status, offset = 0;

    for (chni = 0; chni < chn_ndev; ++chni) {
	/* Write raw data to hardware */
	status = (*dp->driver) (Write, dp, chn_chantbl + offset);

	offset += dp->size;
	++dp;
    }

    /* Call hooks (used to capture data; see capture.c and adcap.c) */
    hook_execute(chn_write_hooks);

    return status;
}

/* Close all channels and reset outputs to "safe" values */
int chn_close()
{
    DEVICE *dp = chn_devtbl;
    int chni, offset = 0;

    /* Read data from hardware */
    for (chni = 0; chni < chn_ndev; ++chni) {
	/* Call device driver to re-initialize all channels */
	(*dp->driver) (Init, dp, chn_chantbl + offset);

	/* Update the offset into the channel table */
	offset += dp->size;
	++dp;
    }
    return 0;
}


/* Reset an individual channel */
/*! This can conflict with channel_read if called simulataneously !*/
int chn_zero(int index)
{
    CHANNEL *cp;
    DEVICE *dp;

    /* Make sure the channel number is in range */
    if (index < 0 || index >= chn_nchan)
	return -1;

    /* Get pointers to the device and channel */
    cp = chn_chantbl + index;
    dp = chn_devtbl + cp->devid;

    /*! This should also clean out hooks !*/

    return (*dp->driver) (Zero, dp, cp);
}
