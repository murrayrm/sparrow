/*!
 * \file channel.h 
 * \brief Definitions for channel I/O library
 *
 * \author Richard M. Murray
 * \date 18 Jan 93
 * 
 * This file contains the structures and functions needed to access
 * the Sparrow I/O library (device drivers).
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

#ifndef __CHANNEL_INCLUDED__
#define __CHANNEL_INCLUDED__

#include <stdio.h>			/* for FILE declaration */

#define CHN_MAXDEV 64			/* max number of channels allowed */
 
/*!
 * \enum chn_driver_action 
 * \brief Device driver actions 
 *
 * This enum lists the types of actions that a device can be asked to
 * perform.  The device driver should interpret each of these actions and
 * act on any that make sense for the driver.
 *
 * The AsynRW action is not actively used in this versio of Sparrow.
 * In Sparrow v2.1g, there was a capability to read/write devices as
 * part of the display loop (in chnasyn.c).  This is not needed under
 * linux since we can just use process threads.
 * 
 */
enum chn_driver_action {
    Init,			  /*!< initialize the driver */
    Read,			  /*!< read data from hardware */
    Write,                        /*!< write data to hardware */
    Zero,                         /*!< reset input/output to zero */
    NewChannels,                  /*!< make driver-specific part of channels */
    HandleFlag,                   /*!< handle a driver-specific flag */
    DeviceSpecific,               /*!< routine necessary for devtest */
    AsynRW			  /*!< asynchronous R/W (not currently used) */
};
typedef enum chn_driver_action DEV_ACTION;

/*!
 * \struct chn_device_entry
 * \brief Device driver table entry 
 *
 */
#define CHNDEVLEN 40			/*!< max length of driver/path name  */
struct chn_device_entry {
    int (*driver)(DEV_ACTION, ...);	/*!< device driver */
    int size;				/*!< number of channels in device */
    int address;			/*!< device address */
    char devname[CHNDEVLEN+1];		/*!< device path */
    int index;                          /*!< device index */
    char name[CHNDEVLEN+1];             /*!< device name */
};
typedef struct chn_device_entry DEVICE;

/* Channel option flag types for the chn_config routine and option parsing */
enum chn_flag_typ {
  Unknown,
  Device,
  Channel
};
typedef enum chn_flag_typ CHN_FLAG_TYPE;

/*!
 * \struct chn_filter_entry
 * \brief channel filter structure 
 *
 * Channel filters allow the data on a channel to be filtered through
 * a digital filter and sent to another channel input.
 */
struct chn_filter_entry {
  double *a,*b;			/* filter coefficients */
  int na, nb;			/* length of filter coeff lists */

  /* Note: na is one less than the total number of a coefficients
     because the first a coeff is used to normalize the rest, and thus
     it is not saved in the list of coefficients */

  double *x,*y;			/* circular buffers holding data history */
  int xi, yi;			/* current posn in x, y circular buffers */
  int out_chn;			/* output channel number */
};
typedef struct chn_filter_entry FILTER;

/* Channel types */
enum channel_type {
    Double, 	         		/* double precision float */
    Short                               /* signed integer */
};

/*!
 * \struct chn_channel_entry
 * \brief Channel structure 
 */
struct chn_channel_entry {
  short raw;				/* raw data from hardware */
  
  /* Data type for this channel */
  enum channel_type type;

  /* Storage for processed data */
  union { double d; short s; } data;
  
  int devid;				/* device driver for this channel */
  int chnid;                            /* channel offset _within_ device */
  int offset;				/* scale offset */
  double scale;  			/* scale factor */
  unsigned dumpf;			/* dump data to disk? */
  FILTER *filter;	 /* data needed for possible filtering of the channel */
  void *dev_sp;
};
typedef struct chn_channel_entry CHANNEL;

/*!
 * \struct chn_device_lookup
 * \brief Device driver lookup table 
 *
 * This function defines the name of a device driver and the actual
 * function that implements that driver.  Used to look up drivers when
 * reading a configuration file.
 *
 */
struct chn_device_lookup {
  char *name;				/* name of device driver */
  int (*driver)(DEV_ACTION, ...);	/* device driver */
};
typedef struct chn_device_lookup DEV_LOOKUP;

/* External declarations */
extern DEVICE chn_devtbl[];     	/* device driver table */
extern CHANNEL chn_chantbl[];		/* channel table */
extern int chn_capture_flag;		/* data capture status flag */
extern unsigned chn_capture_offset;     /* capture buffer offset */
extern int chn_ndev;
extern int chn_nchan;
extern char chn_flag_name[];            /* data storage for flag parsing */
extern char chn_flag_value[];           /* routines */
extern CHN_FLAG_TYPE chn_flag_type;     /* for device configuration flags */
extern int chn_adcap_flag;              /* adcap capturing on? */
extern char chn_adcap_default_prefix[32];/* adcap default file naming */
extern char chn_adcap_rawdatafile[32];  /* temporary file name for adcap's raw data output */
extern int chn_filters[];	        /* list of channels that get filtered */

int chn_config(char *);
int chn_init();
int chn_read(void);
int chn_write(void);
int chn_close(void);
int chn_add_driver(char *name, int (*driver)(DEV_ACTION, ...));
int chn_add_device(DEVICE *dp);
int chn_zero(int index);

int chn_capture_setup(double time, double rate, unsigned long int mem_left);
int chn_capture_on(), chn_capture_on_cb(long);
int chn_capture_off(), chn_capture_off_cb(long);
int chn_capture_resume();
int chn_capture_dump(char *filename), chn_capture_dump_cb(long);
int chn_capture(void);
unsigned chn_capture_size(unsigned size);

#define chn_data(i)     chn_chantbl[i].data.d
#define chn_bits(i)     chn_chantbl[i].data.s
#define chn_raw(i)	chn_chantbl[i].raw

/* Define the default drivers in the library */
int virtual_driver(DEV_ACTION, ...);
int fcn_driver(DEV_ACTION, ...);

#endif /* __CHANNEL_INCLUDED__ */
