/*
 * chnconf.c - configuration routines for channel library
 * 
 * Eric Wemhoff
 * September 1993
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
#include <stdlib.h>
#include <string.h>
#include "channel.h"
#include "display.h"

#ifdef OLD_MATLABV4
#include "matlab.h"
#endif

/* Function declarations */
int chn_parse_option(DEVICE *, CHANNEL *, char *, int *, 
  double *, unsigned *, FILTER **, int);
int chn_gettok(FILE *, char *, int, char *, int *);

extern DEV_LOOKUP chn_devlut[];	/* device lookup table */

/*
 * Included functions:
 *   chn_config()
 *   chn_parse_option()
 *   chn_add_device()
 *   chn_add_driver()
 *
 */

/* Global variables */
#define MAXCHN 256
#define FLEN 40			/* max string length allowed for flags,
				 * including preceeding - and possibly an =
				 * sign and flag value */
int chn_ndev = 0;		/* number of devices installed */
int chn_nchan = 0;		/* number of channels */
int chn_filters[MAXCHN];	/* list of channels to filter (needs to be
				 * filled by chnconf */

DEVICE chn_devtbl[CHN_MAXDEV];	/* device driver table */
CHANNEL chn_chantbl[MAXCHN];	/* channel table */

/* table of defined standard config.dev flags */
enum flags {
    TABLEEND, INDEX, OFFSET, SCALE, NODUMP,
    MFILTER,			/* filter, specified in matlab format */
    FILTOUT,			/* channel to output filter to */
    DEBUG,			/* turn on debugging information */
};

/* table for parsing channel configuration file flags */
static struct {
    char name[20];
    int number;
} chn_flags[] = {
    {"index", INDEX}, {"offset", OFFSET}, {"scale", SCALE},
    {"nodump", NODUMP}, {"filter", MFILTER}, {"filtout", FILTOUT},
    {"debug", DEBUG},
    {"tableend", TABLEEND}
};

char chn_flag_name[20];
char chn_flag_value[20];
CHN_FLAG_TYPE chn_flag_type;

int chn_dev_debug = 0;		/* turn on device debugging info */
int chn_chn_debug = 0;		/* turn on channel debugging info */

/*
 * Read device configuration from file and set up channel table. Return 1 if
 * everything's OK, -1 if an error occurs.
 */
int chn_config(char *file)
{
    FILE *fp;
    int line = 1, status, i;
    char buf[FLEN + 1], delims[5] = " \t\n;";
    char ch;
    int num, address, offset;
    unsigned dumpf;
    double scale;
    CHANNEL *chnp;
    int errorflag = 0;
    FILTER *filtp;
    char tempbuf[100];

    /* Open up the configuration file */
    if ((fp = fopen(file, "r")) == NULL) {
	perror("chn_config");
	fprintf(stderr, "chn_config: trouble opening file %s\n", file);
	return -1;
    }
    /* initialize */
    chn_ndev = 0;
    chn_nchan = 0;
    chn_flag_type = Unknown;
    *buf = '\0';
    ch = ' ';
    chn_filters[0] = -1;

    /* main loop searches for occurences of "device:" or "channel:" */
    do {
	ch = chn_gettok(fp, buf, FLEN, delims, &line);

	if (strcmp(buf, "device:") == 0) {
	    /* get the required device parameters */
	    chn_gettok(fp, buf, FLEN, delims, &line);	/* look for device */

#	    ifdef DEBUG
	    /* Look for the driver name in the lookup table */
	    fprintf(stderr,
		    "Using devlut = 0x%x; devlut[0].name = %s\n",
		    chn_devlut, chn_devlut[0].name);
#	    endif /* DEBUG */

	    for (i = 0; chn_devlut[i].name != NULL; ++i)
		if (strcmp(chn_devlut[i].name, buf) == 0)
		    break;

	    if (chn_devlut[i].name == NULL) {
		if (strcmp(buf,"end")==0) {
	            /* artifical end of file */
	            fprintf(stderr,"Device \"%s\" found in line %d, skipping rest of .dev file.\n", buf, line);
	            break;
	        }
		fprintf(stderr,
		 "Device \"%s\" unknown, skipping. (line %d)\n", buf, line);
		chn_flag_type = Unknown;
		errorflag++;
		continue;
	    }
	    chn_gettok(fp, buf, FLEN, delims, &line);	/* look for no. of
							 * channels */
	    if (sscanf(buf, "%d", &num) != 1) {
		fprintf(stderr, "Bad size parameter, skipping device. (line %d)\n", line);
		chn_flag_type = Unknown;
		errorflag++;
		continue;
	    }
	    ch = chn_gettok(fp, buf, FLEN, delims, &line);	/* look for address */
	    if (sscanf(buf, "0x%x", &address) == 1 ||
		sscanf(buf, "0X%x", &address) == 1) {
	      chn_devtbl[chn_ndev].address = address;
	      chn_devtbl[chn_ndev].devname[0] = '\0';

	    } else if (sscanf(buf, "/dev/%*s") == 0) {
	      /* Copy the device name to the channel structure */
	      strncpy(chn_devtbl[chn_ndev].devname, buf, CHNDEVLEN);
	      chn_devtbl[chn_ndev].devname[CHNDEVLEN] = '\0';
	      chn_devtbl[chn_ndev].address = 0;

	    } else {
		    fprintf(stderr, "Bad address parameter, skipping device. (line %d)\n", line);
		    chn_flag_type = Unknown;
		    errorflag++;
		    continue;
	    }

	    /* store the device driver information */
	    /* Note: address, devname stored during parsing */
	    chn_devtbl[chn_ndev].driver = chn_devlut[i].driver;
	    chn_devtbl[chn_ndev].size = num;
	    chn_devtbl[chn_ndev].index = 0;	/* default */
	    strcpy(chn_devtbl[chn_ndev].name, chn_devlut[i].name);

	    /* basic channel defaults */
	    offset = 0;
	    dumpf = 1;
	    scale = 1;
	    filtp = NULL;

	    /* now parse the options */
	    chn_flag_type = Device;	/* so parsing functions know it's a
					 * device */
	    status = 1;
	    chn_dev_debug = 0;		/* debugging off by default */
	    while (ch != ';') {
		ch = chn_gettok(fp, buf, FLEN, delims, &line);
		status = chn_parse_option(chn_devtbl + chn_ndev, chn_chantbl + chn_nchan, buf, &offset, &scale, &dumpf, &filtp, line);
		if (status == -1) {	/* if an error occurs skip device */
		    fprintf(stderr, "Device option error (%s), skipping device. (line %d)\n", buf, line);
		    break;
		}
		if (status == 0) {	/* if flag is unrecognised, skip flag
					 * but not device */
		    fprintf(stderr, "Unsupported device option (%s), skipping it. (line %d)\n", buf, line);
		    errorflag++;
		}
	    }
	    if (status == -1) {
		chn_flag_type = Unknown;
		errorflag++;
		continue;	/* don't add the device */
	    }
	    /* Everything went OK. Add the device's channels */
	    for (i = 0; i < num; i++) {
		chn_chantbl[chn_nchan].devid = chn_ndev;
		chn_chantbl[chn_nchan].chnid = i;
		chn_chantbl[chn_nchan].offset = offset;
		chn_chantbl[chn_nchan].scale = scale;
		chn_chantbl[chn_nchan].dumpf = dumpf;
		chn_chantbl[chn_nchan].filter = filtp;
		chn_nchan++;	/* add this channel */
	    }
	    /* let the device driver setup its special channel entries */
	    status = (*chn_devtbl[chn_ndev].driver) (NewChannels,
		       chn_devtbl + chn_ndev, (chn_chantbl + chn_nchan - num));
	    if (status == -1) {
		fprintf(stderr, "Device couldn't set up NewChannels properly, skipping device. (line %d)\n", line);
		chn_nchan -= num;	/* take out the new channels, this
					 * device went bad */
		errorflag++;
		continue;
	    }
	    chn_ndev++;		/* add the device */
	} else if (strcmp(buf, "channel:") == 0) {
	    if (chn_flag_type == Unknown) {
		fprintf(stderr, "No device for this channel, skipping channel. (line %d)\n", line);
		*buf = '\0';	/* reset to look for the next occurrence of
				 * channel: or device: */
		errorflag++;
		continue;
	    }
	    /* get channel number; check its validity */
	    chn_gettok(fp, buf, FLEN, delims, &line);
	    if (sscanf(buf, "%d", &num) != 1) {
		fprintf(stderr, "Bad channel number, skipping channel. (line %d)\n", line);
		errorflag++;
		continue;
	    }
	    if (num >= chn_devtbl[chn_ndev - 1].size || num < 0) {
		fprintf(stderr, "Bad channel number, skipping channel. (line %d)\n", line);
		errorflag++;
		continue;
	    }
	    for (i = 0, chnp = chn_chantbl + num; i < chn_ndev - 1; i++)
		chnp += chn_devtbl[i].size;	/* find current place in
						 * channel table */
	    /* now parse the options */
	    chn_flag_type = Channel;	/* so parsing functions know it's a
					 * channel */
	    chn_chn_debug = chn_dev_debug; /* inherit debugging status */
	    while (ch != ';') {
		ch = chn_gettok(fp, buf, FLEN, delims, &line);
		status = chn_parse_option(chn_devtbl + chn_ndev - 1, chnp, buf, &chnp->offset, &chnp->scale, &chnp->dumpf, &filtp, line);
		if (status == -1) {	/* stop the parsing */
		    fprintf(stderr, "Channel option error (%s), skipping it. (line %d)\n", buf, line);
		    errorflag++;
		    continue;
		}
		if (status == 0) {	/* print error and continue parsing */
		    fprintf(stderr, "Unsupported channel option (%s), skipping it. (line %d)\n", buf, line);
		    errorflag++;
		}
	    }
	} else if (*buf != '\0') {
	    fprintf(stderr, "chn_config: Ignoring unknown text (%s) in config file, line %d\n", buf, line);
	    errorflag++;
	}
	if (feof(fp))
	    break;
    } while (1);		/* end of configuration file parsing loop */


    fclose(fp);			/* close the file */

    if (errorflag) {
	strcpy(tempbuf, "chnconf(): An error occured during channel configuration. Continue(y/n)? ");
	if (dd_modef == 0)
	    while (DD_SCANF(tempbuf, "%c", buf) != 1 || (*buf != 'y' && *buf != 'n'));
	/*
	 * else if(dd_modef==1) while(gdd_scanf(tempbuf, "%c", buf)!=1 ||
	 * (*buf!='y' && *buf!='n'));
	 *//* graphics display not supported */
	else
	    do {
		printf("%s", tempbuf);
		ch = getchar();
	    } while (ch != 'n' && ch != 'y');

	if (*buf == 'n') {
	    fclose(stderr);
	    exit(0);
	}
    }
    chn_init();			/* initialize the channels */

#ifdef UNUSED
#ifdef DEBUG
    printf("done with config.dev, here's the tables:\n");
    tbl_diag(chn_devtbl, chn_chantbl);
    getchar();			/* let user see messages */
    dd_redraw();		/* redraw screen (hopefully you're using
				 * display package) */
#endif
#endif

    if (errorflag)
	return -1;
    else
	return 1;
}


/* Handler for the options in the configuration file */
/*
 * Protocol for return value: -1 if an error occurs, such as a flag not
 * having the right type of associated value, or wrong flag_type; or 0 if the
 * flag is not known;  the Handleflag part of the device drivers have to be
 * consistent with this
 */
/*
 * If the flag is part of a device line, then dp should point to the
 * appropriate device.  If part of a channel line, dp and cp should both be
 * set to the proper places. buf contains the flag and its value with the -
 * and = signs.
 */
int chn_parse_option(
  DEVICE *dp, CHANNEL *cp, char *buf, int *offset,
  double *scale, unsigned *dumpf, FILTER **fpp,
  int line)
{
    int i, status = -1;
    int inttemp;
    double doubletemp;
    struct matlab_entry *amat, *bmat;
    FILTER *filtp;

    if (*buf != '-')
	return status;

    /* copy flag name & value into the global flag holders */
    buf++;
    for (i = 0; buf[i] != '\0' && buf[i] != '='; i++)
	chn_flag_name[i] = buf[i];
    chn_flag_name[i] = '\0';
    if (buf[i] == '=') {
	buf = buf + i + 1;
	for (i = 0; buf[i] != '\0'; i++)
	    chn_flag_value[i] = buf[i];
	chn_flag_value[i] = '\0';
    } else
	*chn_flag_value = '\0';
    /* lookup flag number in flag table */
    for (i = 0; chn_flags[i].number != 0; i++) {
	if (strcmp(chn_flag_name, chn_flags[i].name) == 0)
	    break;
    }

    switch (chn_flags[i].number) {
    case TABLEEND:		/* flag not supported by channel.c */
	if (chn_flag_type == Unknown) {
	    fprintf(stderr, "No device for \"%s\", skipping. (line %d)\n", chn_flag_name, line);
	    break;
	}
	/* Non-standard flag, let the device driver handle it */
	status = (*dp->driver) (HandleFlag, dp, cp);
	break;

    case DEBUG:		/* Turn on debuggings */
	if (chn_flag_type == Device) chn_dev_debug = atoi(chn_flag_value);
	if (chn_flag_type == Channel) chn_chn_debug = atoi(chn_flag_value);
	status = 1;
	break;

    case INDEX:		/* index flag */
	if (chn_flag_type != Device) {
	    fprintf(stderr, "No device for flag \"index\", skipping. (line %d)\n", line);
	    break;
	}
	if (sscanf(chn_flag_value, "0x%x", &dp->index) == 1) {
	    status = 1;
	    break;
	}
	if (sscanf(chn_flag_value, "0X%x", &dp->index) == 1) {
	    status = 1;
	    break;
	}
	if (sscanf(chn_flag_value, "%d", &dp->index) == 1) {
	    status = 1;
	    break;
	}
	fprintf(stderr, "Bad index value, skipping. (line %d)\n", line);
	dp->index = 0;
	break;

    case OFFSET:		/* offset flag */
	if (chn_flag_type == Unknown) {
	    fprintf(stderr, "No device for flag \"offset\", skipping. (line %d)\n", line);
	    break;
	}
	if (sscanf(chn_flag_value, "%d", &inttemp) != 1) {
	    fprintf(stderr, "Bad offset value, skipping. (line %d)\n", line);
	    break;
	}
	switch (chn_flag_type) {
	case Device:
	    *offset = inttemp;
	    status = 1;
	    break;
	case Channel:
	    cp->offset = inttemp;
	    status = 1;
	    break;
	}
	break;

    case SCALE:		/* scale flag */
	if (chn_flag_type == Unknown) {
	    fprintf(stderr, "No device for flag \"scale\", skipping. (line %d)\n", line);
	    break;
	}
	if (sscanf(chn_flag_value, "%lf", &doubletemp) != 1) {
	    fprintf(stderr, "Bad scale value (line %d)\n", line);
	    break;
	}
	switch (chn_flag_type) {
	case Device:
	    *scale = doubletemp;
	    status = 1;
	    break;
	case Channel:
	    cp->scale = doubletemp;
	    status = 1;
	    break;
	}
	break;

    case NODUMP:		/* nodump flag */
	if (chn_flag_type == Unknown) {
	    fprintf(stderr, "No device for flag \"nodump\", skipping. (line %d)\n", line);
	    break;
	}
	switch (chn_flag_type) {
	case Device:
	    *dumpf = 0;
	    status = 1;
	    break;
	case Channel:
	    cp->dumpf = 0;
	    status = 1;
	    break;
	}
	break;

    case MFILTER:		/* load a filter from a matlab-format file */
#ifdef OLD_MATLABV4
	if (chn_flag_type == Unknown) {
	    fprintf(stderr, "No device for flag \"filter\", skipping. (line %d)\n", line);
	    break;
	}
	inttemp = matlab_open(chn_flag_value);
	if (inttemp == -1) {
	    perror("chn_parse_option");
	    fprintf(stderr, "Couldn't read file \"%s\". (line %d)\n", chn_flag_value, line);
	    break;
	}
	filtp = (FILTER *) malloc(sizeof(FILTER));
	if (filtp == NULL) {
	    fprintf(stderr, "Could not allocate memory for filter structure. (line %d)\n", line);
	    break;
	}
	/* matrices are stored in the matlab_entry struct defined in matlab.h */
	/* get the a matrix in amat */
	if (chn_chn_debug) printf("looking for amat...");
	if ((amat = matlab_find("a")) == NULL) {
	    fprintf(stderr, "Could not find the a matrix for \"filter\", skipping. (line %d)\n", line);
	    break;
	}
	if (chn_chn_debug) printf("done\n");
	if (chn_chn_debug) mat_print(amat);
	/*
	 * amat must be a vector (row or column) and must have at least one
	 * element
	 */
	if (amat->nrows < 1 || amat->ncols < 1 || (amat->nrows > 1 && amat->ncols > 1)) {
	    fprintf(stderr, "Bad a matrix dimensions: %dx%d, skipping. (line %d)\n", amat->nrows, amat->ncols, line);
	    printf("bad matrix a\n");
	    (void) getchar();
	    free(amat);
	    free(filtp);
	    filtp = NULL;
	    break;
	}
	filtp->na = ((amat->ncols > amat->nrows) ? amat->ncols : amat->nrows) - 1;	/* max */
	doubletemp = *(amat->real);

	/* get the b matrix in bmat */
	if ((bmat = matlab_find("b")) == NULL) {
	    fprintf(stderr, "Could not find the b matrix for \"filterm\", skipping. (line %d)\n", line);
	    break;
	}
	/* bmat must be a vector (row or column) but could have zero elements */
	if (bmat->nrows > 1 && bmat->ncols > 1) {
	    fprintf(stderr, "Bad b matrix dimensions: %dx%d, skipping. (line %d)\n", bmat->nrows, bmat->ncols, line);
	    printf("bad matrix b\n");
	    (void) getchar();
	    free(bmat);
	    free(filtp);
	    filtp = NULL;
	    break;
	}
	filtp->nb = ((bmat->ncols > bmat->nrows) ? bmat->ncols : bmat->nrows);	/* max */

	/* Now put the filter coefficients in the filter structure */
	filtp->a = amat->real + 1;
	filtp->b = bmat->real;

	/* Normalize coefficients by doubletemp */
	for (i = 0; i < filtp->na; i++)
	    filtp->a[i] = amat->real[i+1] / (amat->real)[0];
	for (i = 0; i < filtp->nb; i++)
	    filtp->b[i] = bmat->real[i] / (amat->real)[0];

	/* Allocate space for the data vectors */
	if (filtp->nb > 0)
	    filtp->x = (double *) malloc(filtp->nb * sizeof(double));
	if (filtp->na > 0)
	    filtp->y = (double *) malloc(filtp->na * sizeof(double));
	if ((filtp->nb > 0 && filtp->x == NULL) || (filtp->na > 0 && filtp->y == NULL)) {
	    fprintf(stderr, "Trouble allocating memory for filter, skipping. (line %d)\n", line);
	    free(amat);
	    free(bmat);
	    free(filtp);
	    break;
	}
	switch (chn_flag_type) {
	case Device:
	    *fpp = filtp;
	    status = 1;
	    break;
	case Channel:
	    cp->filter = filtp;
	    status = 1;
	    break;
	}
	break;
#else
	fprintf(stderr, "Channel filters disabled in this release\n");
	break;
#endif

    case FILTOUT:
	if (chn_flag_type != Channel) {
	    fprintf(stderr, "\"filtout\" is a channel flag only. (line %d)\n", line);
	    break;
	}
	if (cp->filter == NULL) {
	    fprintf(stderr, "No filter has been defined for \"filtout\" flag. (line %d)\n", line);
	    break;
	}
	if (sscanf(chn_flag_value, "%d", &cp->filter->out_chn) != 1) {
	    fprintf(stderr, "Bad out_chn value (line %d).\n", line);
	    break;
	}
	status = 1;
	break;

    }
    return status;
}

/* Add a device driver to the table of known devices */
int chn_add_device(DEVICE *dp)
{
    chn_devtbl[chn_ndev] = *dp;
    return chn_ndev++;
}


/* Add a driver to the list of available drivers */
int chn_add_driver(char *name, int (*driver)(DEV_ACTION, ...))
{
    int offset;

    /* Check to make sure the driver is not already installed */
    /* and figure out where the lookup table current ends. */
    for (offset = 0; chn_devlut[offset].name != NULL; ++offset)
	/* Don't allow name duplications */
	if (strcmp(name, chn_devlut[offset].name) == 0)
	    return -1;

    /* Make sure there is room to install a new device entry */
    if (offset >= CHN_MAXDEV - 2)
	return -1;

    /* Add the entry */
    chn_devlut[offset].name = name;
    chn_devlut[offset].driver = driver;
    /* Mark the table's end */
    chn_devlut[offset + 1].name = NULL;
    chn_devlut[offset + 1].driver = NULL;

    /* Return something non-negative */
    return offset;
}
