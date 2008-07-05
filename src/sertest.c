/*
 * sertest.c - simple serial driver test routine
 *
 * \author Richard M. Murray
 * \date 16 Dec 04
 *
 * This device driver reads and writes from a file passed as an
 * argument in the config.dev file.  For a simple test, use a hardware
 * address of /dev/zero.  For a more complicated test, use ptycat to
 * generate a new psuedo terminal (/dev/pts/nn) and enter numbers
 * remotely.
 *
 * The operation of the driver depends on the number of channels that
 * are requested:
 *
 *   * 1 channel - write only operation
 *   * 2 channel - read/write operation 
 *   * 3 channel - read/write/status (faked)
 *
 * With 1 channel defined, the serial port will be sent the current
 * value of the first channel.  If 2 channels are used, the serial
 * port will be queried for a floating point number that will be
 * recorded as the (second) channel value.  Finally, if 3 channels are
 * used, a status code will be written to the third channel.  This
 * status bit is currently defined as the number of times the a value
 * has been read from the serial port.
 *
 * This driver serves an example of how to implement a serial device
 * driver and also a serves as a test driver for chntest.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>			/* file access types */
#include <pthread.h>			/* process thread library */
#include <string.h>			/* need for atof */
#include <stdarg.h>			/* variable arguments */
#include <termios.h>
#include "channel.h"			/* channel structure definitions */
#include "flag.h"

/* 
 * Device-specific channel data
 *
 * This structure is used to store data that is specific to the
 * device.  We create a copy of the file pointer, the thread process,
 * and buffers for the output, input and status values.  While Sparrow
 * generally allows a device-dependent structure for each channel,
 * here we will only associate this with the first channel.
 *
 * Putting this information into the device specific data structure
 * allows multiple serial devices to use the same driver.
 *
 */

struct sertest_sp {
  int sfd; 				/* file descriptor for input */
  pthread_t ser_thread;			/* read/write process thread */
  double obuf;				/* buffered output */
  double ibuf;				/* buffered input */
  double sbuf;				/* buffered status */
  pthread_mutex_t sermutex;		/* mutually exclusive access */
};

/*
 * Function and variable declarations
 *
 * Declare the thread that is used to read and write from the device
 */

void *sertest_rw(void *);		/* read/write thread */
int ser_debug = 0;			/* set to turn on debugging info */

/*
 * Device driver routine
 *
 * This is a standard Sparrow device driver.  On Init, it creates a
 * thread that is used to actually talk to the device.  This allows
 * writes and reads to the serial port to be done without blocking.
 * The buffers that are part of the device specific data structure are
 * used to pass data back and forth, in principle so that we can
 * implent mutually exclusive access (not yet done).
 *
 */

int sertest_driver(DEV_ACTION action, ...)
{
  int i, status = 0;
  va_list ap;

  va_start(ap, action);
  DEVICE *dp = va_arg(ap, DEVICE *);
  CHANNEL *cp = va_arg(ap, CHANNEL *);
  struct sertest_sp *sp = (struct sertest_sp *) cp->dev_sp;
  FILE *cfp;

  switch (action) {

  case NewChannels:
    /*
     * Allocate space for device specific data
     *
     * This action is called during channel configuration and should
     * be used to create the data structures associated with the
     * device and with the channels.  For now, all we do is create the
     * device specific data structure and assign it to channel 0.
     *
     */
    if (cp->dev_sp != NULL) free(cp->dev_sp);
    cp->dev_sp = (struct sertest_sp *) malloc(sizeof(struct sertest_sp));
    ((struct sertest_sp *)(cp->dev_sp))->sfd = -1;
    break;

  case HandleFlag:			/* no supported flags */
    /* 
     * Parse configuration options
     *
     * This version of the driver currently doesn't allow any optoins,
     * but this is where you could put things like baud rate and other
     * serial port settions for the channel.
     *
     * This should eventually be used to parse baud rate and other
     * settings, including whether to turn on debugging info.
     *
     */
    status = -1;
    break;

  case Init:				/* initialize driver */
    /*
     * Initialize the driver (and read R/W thread)
     *
     * This action is called after the configuration options have been
     * parsed and the various data structures initialized.  The main
     * function of this block of code is to open up the serial port
     * for read/write access and start the process thread that will
     * read and write from the device.
     *
     * The port is opened using non-buffered I/O, so that all reads
     * and writes are synchronous with the device.
     *
     * TBD: add more error checking
     *
     */
    {
      FILE *fp;				/* file pointer for device */
      struct termios term;		/* use to set term attributes */

      /* Initialize data buffers */
      sp->ibuf = sp->obuf = sp->sbuf = 0;
      /* for(i=0; i<dp->size; ++i){ cp[i].data.d = 0; } */

      /* Open the indicated device for reading and writing */
      if (dp->devname[0] != '\0') {
	if ((sp->sfd = open(dp->devname, O_RDWR)) < 0) {
	  perror("sertest");
	  status = -1;
	}
      }

      /* Set up the terminal for raw interface */
      cfmakeraw(&term);
      tcsetattr(sp->sfd, TCSANOW, &term);
    }
     
    /* Create a mutex to synchronize data access */
    pthread_mutex_init(&sp->sermutex, NULL);

    /* Now create a thread to read and write data */
    if (pthread_create(&(((struct sertest_sp *)(cp->dev_sp))->ser_thread),
		       NULL, sertest_rw, (void *) cp) != 0) {
      perror("sertest");
      status = -1;
    }
    break;

    /*
     * Read/Write actions
     *
     * The Read and Write actions just move data back and forth from
     * the buffers.
     */

  case Write:		    /* Store data in buffer for later usage */
    /* TBD: we should probably convert units here */
    pthread_mutex_lock(&sp->sermutex);
    sp->obuf = cp[0].data.d;
    pthread_mutex_unlock(&sp->sermutex);
    break;

  case Read: 				/* Copy data from buffer */
    {
      static int count;

      /* If a read channel is declared, copy data to it */
      pthread_mutex_lock(&sp->sermutex);
      if (dp->size > 1)
	cp[1].data.d = (double) sp->ibuf + (double) (count++ % 1000) / 1000;

      /* If a status channel is declared, copy data to it */
      if (dp->size > 2)
	cp[2].data.d = sp->sbuf;
      pthread_mutex_unlock(&sp->sermutex);

      break; 
    }
  }

  va_end(ap);
  return status;
  }

/*
 * sertest_rw - thread to read and write from serial device
 *
 * This thread continuously writes the latest value of the output to
 * the device and reads back information from the device.  The status
 * of the device is just set to the value of a counter.
 *
 * For more complete drivers, this is the location where you send
 * commands to the device and read back the current value of the
 * device (eg, its positions, if this is a motor) and the status of
 * the device.  Note that in the version presented here, we always
 * read from the device, even if we only have a write channel.
 *
 * For now, this device always reads and writes from the device,
 * independent of the number of channels that are declared.  The real
 * way to do this depends on the specific device that is being used.
 * 
 */

void *sertest_rw(void *ptr)
{
  CHANNEL *cp = (CHANNEL *) ptr;
  struct sertest_sp *sp = (struct sertest_sp *) cp->dev_sp;
  static int count = 0;			/* keep track of # of calls */

  while (1) {
    int sfd = ((struct sertest_sp *)(cp[0].dev_sp))->sfd;

    if (sfd >= 0) {
      char msg[128];

      /* Write the data to the port */
      if (ser_debug) flag(0, 'o', 0);
      sprintf(msg, "%g\n", sp->obuf);
      if (write(sfd, msg, strlen(msg)) < 0) perror("write()");
      if (ser_debug) flag(0, 'O', 0);

      /* Read the data from the port and save its status */
      if (ser_debug) flag(0, 'i', 0);
      int i = 0;
      while (read(sfd, msg+i, 1) >= 0 && i < 127 && msg[i] != '\n') ++i;
      msg[i+1] = '\0';
      if (ser_debug) flag(0, 'I', 0);

      pthread_mutex_lock(&sp->sermutex);
      sp->ibuf = atof(msg);

      /* Update status count */
      sp->sbuf = ++count;

      pthread_mutex_unlock(&sp->sermutex);

    } else 
      sp->sbuf = -1;

    /* Sleep for a bit to let the computer get other things done */
    usleep(10000);
  }

  return NULL;
}
