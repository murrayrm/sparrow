/*!
 * \file servo.c 
 * \brief linux version of servo routines
 *
 * \author Richard M. Murray
 * \date 11 Dec 04
 *
 * WARNING: these routines are *very* preliminary; no error checking yet.
 *
 * \ingroup servo
 */

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "servo.h"

static pthread_t servo_thread;		/* servo thread */
static void *isr_handler(void *arg);	/* servo routine */
static void (*isr_userisr)()  = NULL;	/* user servo function */
int servo_freq = -1;			/* servo frequency */
unsigned long servo_usec;		/* servo sleep time */

int servo_setup(void (*routine)(), int freq, int flags)
{
  unsigned long period = 1000000/freq;

  pthread_create(&servo_thread, NULL, isr_handler, (void *) NULL);
  servo_freq = 1000000/period;
  servo_usec = period;
  isr_userisr = routine;

  return servo_freq;
}

void servo_cleanup()
{
}

int servo_enable()
{
  return servo_freq;
}

void servo_disable()
{
}


/*
 * Internal routines
 *
 * isr_handler		interrupt service routine
 *
 */

static void *isr_handler(void *arg)
{
  while (1) {
    if (isr_userisr != NULL) (*isr_userisr)();
    usleep(servo_usec);
  }
}

