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

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "servo.h"
#include <stdio.h>

static pthread_t servo_thread;	        /* servo thread */
static void *isr_handler(void *arg);	/* servo routine */
static void (*isr_userisr)()  = NULL;	/* user servo function */
int servo_freq = -1;		        /* servo frequency */
int isr_count = 0;                      /* counter */
unsigned long servo_period;	        /* servo period time in 10^-6 sec */
struct timeval tv;                      /* for calls of gettimeofday */
unsigned long long time1 = 0;           /* to measure time intervals */
unsigned long long time2 = 0;
double temp_time1 = 0.0;
double temp_time2 = 0.0;                /* to measure time intervals and */
int n = 0;                              /* compute an average period time */
//#define MEASURE_AVG_PERIOD

int servo_setup(void (*routine)(), int freq, int flags)
{
  servo_period = 1000000/freq;
  
  servo_freq = freq;
  
  isr_userisr = routine;
  
  // initialization
  gettimeofday(&tv,NULL);
  time1 = (unsigned long long)tv.tv_usec + 1000000ULL * tv.tv_sec;

  if (isr_userisr != NULL){
    pthread_create(&servo_thread, NULL, isr_handler, (void *) NULL);
  }

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
	  
    temp_time1 = ((double)time1)/1000000.0;

    // time before
    gettimeofday(&tv,NULL);
    time1 = (unsigned long long)tv.tv_usec + 1000000ULL * tv.tv_sec;

    // for measuring purposes, this variable can be displayed by the dd
    actual_period_time = ((double)time1)/1000000.0 - temp_time1;
    
#ifdef MEASURE_AVG_PERIOD
    n++;
    temp_time2 = temp_time2 + actual_period_time;
    fprintf(stderr,"%1.10f, %d",temp_time2/n,n);
#endif
    
    // servo function
    (*isr_userisr)();
    
    isr_count++;

    // time afterwards
    gettimeofday(&tv,NULL);
    time2 = (unsigned long long)tv.tv_usec + 1000000ULL * tv.tv_sec;

    // for measuring purposes, this variable can be displayed by the dd
    time_it_took = ((double)(time2 - time1))/1000000.0;
    
    if((servo_period)>((time2 - time1) + SERVO_OVERHEAD)){
      usleep(servo_period - (time2 - time1) - SERVO_OVERHEAD);
    }else{
      // execution time > period time !!!
    }
  }
}
