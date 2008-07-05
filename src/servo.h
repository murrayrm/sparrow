/*!
 * \file servo.h 
 * \brief declarations and definitions for servo package
 *
 * \author Richard M. Murray
 * \date 6 Aug 92
 *
 * \ingroup servo
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

#endif /* __SERVO_INCLUDED__ */
