/*
 * chntest.c - test out channel interface
 *
 * Richard M. Murray
 * 9 August 1992
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
#include <math.h>
#include "display.h"		/* dynamic display package */
#include "keymap.h"
#include "channel.h"
#include "servo.h"
// #include "vscope.h"
#include "fcn_gen.h"

/* Declare global variables for this module */
int channel_read_button(long), channel_write_button(long), channel_init_button(long);
int channel_servo_button(long);
int channel_capture_button(long), channel_dump_button(long);
int chntest_fcn_config(long);
int user_quit(long);

static int isr_rate= 200;   	/* servo interrupt rate */
static char dumpfile[20] = "chntest.dat";
static char cfgfile[20] = "config.dev";
double enc1, enc2;		/* encoder variables */

int chntest_chnid; /* variable for testing fcn_gen */
double chntest_frequency; /* variable for testing fcn_gen */
int chntest_fcn_test(long);
char dummystring[30];

#include "chntest.h"		/* menu definition */

int main(int argc, char **argv)
{
  freopen("error.msg", "w", stderr); /* write errors to a file */

  /* Start up the display package and run everything from there */
  if (dd_open() < 0) {
    fprintf(stderr, "diag: can't initialize display\n");
    exit(1);
  }
  dd_debug = 1;			/* turn debugging info on */
  
  /* Define a few custom keys */
  dd_bindkey('Q', user_quit);
  dd_bindkey(K_F1, channel_servo_button);
  dd_bindkey(K_F2, channel_capture_button);
  dd_bindkey(K_F3, channel_dump_button);
  dd_bindkey(K_F4, channel_init_button);
  dd_bindkey('r', dd_redraw);
  dd_usetbl(chnmenu);		/* start off on the main menu */
  dd_loop();			/* start the display manager */
  dd_close();			/* close up the screen */
  
  printf("turning off servo\n");
  
  /* Reset the servo handler if it is running */
  servo_cleanup();
  
  return 0;
}

/* Globally defined callbacks */
int user_quit(long arg) { return DD_EXIT_LOOP; }

/*
 * Servo code
 *
 * This servo loop just reads and writes channel data
 *
 */
 
static void isr()
{
    chn_read();
    chn_write();
}

/*
 * Display callbacks
 *
 * These functions are all called by pressing buttons on the display
 *
 */

/* Dummy functions (should be defined in channel interface) */
int channel_read_button(long arg) { chn_read(); return 0; }
int channel_write_button(long arg) { chn_write(); return 0; }
int channel_capture_button(long arg) { chn_capture_on(); return 0; }
int channel_dump_button(long arg) { chn_capture_dump("diag.dat"); return 0; }
/*chntest_fcn_config(long arg){fcn_config(long arg); return 0;}*/
int chntest_fcn_config(long arg){ return 0; };
int adcap_dump_file(long arg) { chn_capture_dump("diag.dat"); return 0; }
int adcap_toggle_capture(long arg) {
    chn_capture_flag ? chn_capture_off() : chn_capture_on();
    return 0;
}

int channel_init_button(long arg) { chn_config(cfgfile); return 0; }

/* Start up a servo loop to read/write values */
int channel_servo_button(long arg)
{
    static int init = 0, running = 0;
    int status;

    /* Initialize the channel data if this is the first call */
    if (!init) { 
      /* Add non-default device drivers */
      /* None */
	
      chn_config(cfgfile); 
      init = 1; 
    }

    /* Toggle the status of the servo routine */
    if (!running) {
	/* servo_sethandler(isr_fast_handler); */
        status = servo_setup(isr, isr_rate, SERVO_OVFL_ABORT);
	if (status <= 0) DD_PROMPT("chntest: couldn't start servo");

        status = servo_enable();
	if (status <= 0) DD_PROMPT("chntest: couldn't enable servo");
    } else {
        servo_disable();
    }

    running = !running;
    return 0;
}

int chntest_fcn_test(long arg){
/*
  fcn_change_frequency(chntest_chnid, chntest_frequency);
*/
  return 0;
}

int mem_usage(long arg)
{
#   ifdef MSDOS
    unsigned long farcoresize, coresize;
    char msg[80];

    farcoresize = farcoreleft();
    coresize = coreleft();
    sprintf(msg, "farcoreleft = %ld, coreleft = %ld", farcoresize, coresize);

    dd_prompt(msg);
#   endif
    return 0;
}
    
