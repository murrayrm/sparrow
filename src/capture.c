/*!
 * \file capture.c
 * \brief capture and dump routines for channel interface library
 *
 * \author Richard M. Murray
 * \date 17 January 1993
 *
 * \ingroup capture
 */

#include <stdlib.h>
#include "channel.h"
#include "flag.h"
#include "hook.h"

extern HOOK_LIST *chn_write_hooks;

/*--------------------------------------------------------------------------*/
/*
 * Channel data capture
 *
 * These routines control the data capture facility.  Data is stored
 * in a single circular buffer and can be dumped to file in binary or
 * ASCII format.
 *
 */
 
int chn_capture_flag = 0;	/* set if data is being captured */
static double *capbuf;		/* capture buffer */
unsigned chn_capture_offset;    /* current offset into capture buffer */

#define CAPBUFSIZ 0x8000	/* Size of capture buffer */

/* Turn the capture flag on and off */
int chn_capture_off_cb(long arg) { return chn_capture_off(); }
int chn_capture_off() {
    flag_off(CAPTURE_FLAG);
    return chn_capture_flag = 0;
}

/* Start taking more data from where we left off */
int chn_capture_resume() 
{
    /* Make sure the buffer isn't already on */
    if (chn_capture_offset >= CAPBUFSIZ) return -1;
    flag_on(CAPTURE_FLAG);
    return chn_capture_flag = 1;
}

/* Reset buffer pointer and turn on data capture */
int chn_capture_on_cb(long arg) { return chn_capture_on(); }
int chn_capture_on() 
{ 
    static int init = 0;   

    if (!init++) {
        /* Allocate some space for captured data */
	if (chn_capture_size(CAPBUFSIZ) == 0) {
	    chn_capture_flag = 0;
	    return -1;
	}

	/* Insert a hook into chn_write to save data */
	hook_add(chn_write_hooks, chn_capture);
    }
    
    /* Reset the offset into the buffer */
    if (!chn_capture_flag) chn_capture_offset = 0;

    /* Intialize a status flag and turn it on */
    flag_init(CAPTURE_FLAG, 'C');
    flag_on(CAPTURE_FLAG);

    return ++chn_capture_flag;
}

/* Set the size of the capture buffer */
unsigned int chn_capture_size(unsigned int size)
{
    /* Allocate the buffer and make sure we really got the space */
    capbuf = (double *) calloc((long) size, sizeof(double));
    if (capbuf == NULL) {
	perror("capture");
	fprintf(stderr, "capture: farcoreleft = %ld\n", (long) 0);

	/* Turn off the capture flag to try to avoid core dumps */
	chn_capture_flag = 0;
	return 0;
    }

    /* Reset the offset to the beginning of the buffer */
    /*! This may be out of sync with the capture routine !*/
    chn_capture_offset = 0;

    /* Return the amount of space allocated */
    return size;
}


/* Main capture routine; called by channel_write */
int chn_capture()
{
    register int i;
    
    /* See if data capture is turned on */
    if (!chn_capture_flag) return 0;

    /* Go through all of the channels and store data */    
    /*! It is possible for this code to store a partial record !*/
    for (i = 0; i < chn_nchan && chn_capture_offset < CAPBUFSIZ; ++i) {
        if (!chn_chantbl[i].dumpf) continue;
	switch (chn_chantbl[i].type) {
	case Double:
	  capbuf[chn_capture_offset++] =  chn_chantbl[i].data.d;
	  break;
	case Short:
	  capbuf[chn_capture_offset++] =  (double) chn_chantbl[i].data.d;
	  break;
	}
    }
    
    /* Check to see if the buffer is full */
    if (chn_capture_offset >= CAPBUFSIZ) chn_capture_off();
    return 0;
}

/* Dump captured data to file */
int chn_capture_dump_cb(long arg) { return chn_capture_dump((char *) arg); }
int chn_capture_dump(char *file)
{
    FILE *fp;
    register int off = 0, chn;
    
    /* Open the dump file for writing */
    if ((fp = fopen(file, "w")) == NULL) {
        perror(file);
	return -1;
    }
    
    /* Go through the buffer and then through each channel */
    while (off < chn_capture_offset) {
	/* Let the user see that something is happening */
	if ((off % 100) == 0)
	    flag_symbol(CAPTURE_FLAG, "|/-\\|/-\\"[(off/100)%8]);

        for (chn = 0; chn < chn_nchan; ++chn) {
            /* Skip this channel if it wasn't dumped */
            if (!chn_chantbl[chn].dumpf) continue;

            /* Dump depending on the type of data stored */
            switch (chn_chantbl[chn].type) {
	    case Double:
	      /* Convert data to a double and then dump */
	      fprintf(fp, "%g\t", capbuf[off]);
	      break;	        

	    case Short:
	      /* Convert data to a short and then dump */
	      fprintf(fp, "%d\t", (short) capbuf[off]);
	      break;
            }
            /* Go to next record in buffer */
            ++off;
        }
        fputc('\n', fp);
    }
    /* Close the file and return the number of items written */
    fclose(fp);
    flag_symbol(CAPTURE_FLAG, 'C');
    return off;
}

