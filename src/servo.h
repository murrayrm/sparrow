/*!
 * \file servo.h 
 * \brief declarations and definitions for servo package
 *
 * \author Richard M. Murray
 * \date 6 Aug 92
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

#ifndef __SERVO_INCLUDED__
#define __SERVO_INCLUDED__

/* Flags for servo_setup */
#define SERVO_OVFL_ABORT	0x01 	/* abort servo on overflow  */
#define ISR_TIMING_BITS		0x02 	/* turn out timing bit output */
 
/* Global variables declared in servo.c */
extern int servo_running, servo_overflow; 	/* flags */
extern int servo_freq;				/* frequency */
extern int isr_count;		/* counter, incremented by handler */

/* Function prototypes */
int servo_setup(void (*)(), int, int);
int servo_alloc(int, int);
int servo_enable(void);
void servo_cleanup(void);
void servo_disable(void);
int servo_start(void (*)(), int);
void servo_stop();

double time_it_took;
double actual_period_time;
#define SERVO_OVERHEAD 90
// adjust this as needed
// examples:
// 90 for a desktop computer,
// 10000 for a gumstix

#endif /* __SERVO_INCLUDED__ */
