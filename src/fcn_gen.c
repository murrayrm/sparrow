/*!
 * \file fcn_gen.c
 * \brief function generating device driver
 *
 * ELW, 3 Oct 93
 *
 * This file contains a pseudo-device driver which, instead 
 * of reading an actual physical device, generates functions.
 * It can be used to generate sine, square, and triangle
 * waves, and behaves like a normal device driver.
 * The output of the function generator device is a function
 * of real time, the calculation of which depends on the frequency
 * the servo routine is running at (servo_frequency, declared
 * in servo.h), and assumes that the device is "read" once
 * per servo cycle.
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

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "channel.h"
#ifdef SERVO
#include "servo.h"
#else				/* if no servo, use gettimeofday */
#include <sys/time.h>
#include <time.h>
#endif
#include "display.h"
#include "fcn_gen.h"

#define PI 3.14159265

/* Local function declarations */
int fcn_channel_scan(long type);

/* device-specific channel part */
struct fcn_gen_sp{
  double frequency;
  double dc_offset;
  FCN_TYPE type;
};
typedef struct fcn_gen_sp SPECIFIC3;

/*!
 * \fn int fcn_driver(DEV_ACTION action, ...)
 * \brief function generator device
 *
 */
int fcn_driver(DEV_ACTION action, ...)
{
  va_list ap;
  static double time = 0;
  register int i;
  int status = 0;
  double temp, dummy;

  static double frequency=1;	/* defaults for channel configuration */
  static double dc_offset=0;
  static FCN_TYPE type=Sine;
  
  va_start(ap, action);
  DEVICE *dp = va_arg(ap, DEVICE *);
  CHANNEL *cp = va_arg(ap, CHANNEL *);
  
  switch (action) {
  case Read:
#   ifdef SERVO
    time += (double)1/servo_freq;
#   else
    {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      time = tv.tv_sec + ((double) tv.tv_usec)/1000.0;
    }
#   endif // SERVO
    for(i=0; i<dp->size; i++){
      switch(((SPECIFIC3*)cp[i].dev_sp)->type){
      case Sine:
	cp[i].data.d = ((SPECIFIC3*)cp[i].dev_sp)->dc_offset + cp[i].scale * sin((double)2*PI*((SPECIFIC3*)cp[i].dev_sp)->frequency*time + cp[i].offset*PI/180);
	break;

      case Square:
	cp[i].data.d = ((SPECIFIC3*)cp[i].dev_sp)->dc_offset + cp[i].scale * ((sin((double)2*PI*((SPECIFIC3*)cp[i].dev_sp)->frequency*time+cp[i].offset*PI/180) > 0) ? 1 : -1);
	break;

      case Triangle:
	temp = modf((double)(time*((SPECIFIC3*)cp[i].dev_sp)->frequency+cp[i].offset/360), &dummy);  /* ranging from 0 to 1 each period*/
	cp[i].data.d = ((SPECIFIC3*)cp[i].dev_sp)->dc_offset + cp[i].scale * (4*fabs((double)0.5-temp)-1);
	break;
      }
    }
    break;

  case Init:
    break;
    
  case NewChannels:
    /* create fcn_gen specific part of the channel entries */
    for(i=0; i<dp->size; i++){
      if ((cp+i)->dev_sp != NULL)
	free((cp+i)->dev_sp);          /* don't want to clutter memory */
      ((cp+i)->dev_sp = (SPECIFIC3 *)malloc(sizeof(SPECIFIC3)));
      if( (cp+i)->dev_sp == NULL){
	 status=-1; 
	 break;
       }
      ((SPECIFIC3*)((cp+i)->dev_sp))->frequency = frequency; 
      ((SPECIFIC3*)((cp+i)->dev_sp))->dc_offset = dc_offset;
      ((SPECIFIC3*)((cp+i)->dev_sp))->type = type;   
    }
    frequency=1;		/* reset defaults for next fcn_gen */
    dc_offset=0;
    type=Sine;
    status=1;
    break; 

  case HandleFlag:
    switch(chn_flag_type) {
    case Device:
      if(!strcmp(chn_flag_name, "type")){
	if(sscanf(chn_flag_value, "%d", (int *) &type))
	  status = 1;
	else status=-1;
	break;
      }
      else if(!strcmp(chn_flag_name, "frequency")){
	if(sscanf(chn_flag_value, "%lf", &frequency))
	  status = 1;
	else status=-1;
	break;
      }
      else if(!strcmp(chn_flag_name, "dc_offset")){
	if(sscanf(chn_flag_value, "%lf", &dc_offset))
	  status = 1;
	else status=-1;
	break;
      }
      break;
      
    case Channel:
      if(!strcmp(chn_flag_name, "type")){
	if(sscanf(chn_flag_value, "%d", 
		  (int *) &((SPECIFIC3*)cp->dev_sp)->type))
	  status = 1;
	else status=-1;
	break;
      }
      else if(!strcmp(chn_flag_name, "frequency")){
	if(sscanf(chn_flag_value, "%lf", &((SPECIFIC3*)cp->dev_sp)->frequency))
	  status = 1;
	else status=-1;
	break;
      }
      else if(!strcmp(chn_flag_name, "dc_offset")){
	if(sscanf(chn_flag_value, "%lf", &((SPECIFIC3*)cp->dev_sp)->dc_offset))
	  status = 1;
	else status=-1;
	break;
      }
      break;
      
    }
  }
  va_end(ap);
  return status;
}



int fcn_change_frequency(int chnid, double frequency){
  int status=0;
  DEVICE *dp = chn_devtbl;
  CHANNEL *cp = chn_chantbl;
  
  if(chnid>=0 && chnid<chn_nchan){
    if(!strcmp(dp[cp[chnid].devid].name, "function-gen")){
      ((SPECIFIC3*)cp[chnid].dev_sp)->frequency = frequency;
      status = 1; /*success*/
    }
  }
  else{
    fprintf(stderr, "Bad channel number for _fcn_change_frequency. \n");
    DD_PROMPT("Bad channel number for _fcn_change_frequency.");
  }
  return status;
}


/* -------------------------------------------------------------------
 * function generator channel configuration table stuff
 */
/* storage space for configuration table entries */
int fcn_chnid[3]={-1,-1,-1};
double fcn_freq[3]={0,0,0};
double fcn_amp[3]={0,0,0};
int fcn_phase[3]={0,0,0};
double fcn_dc_offset[3]={0,0,0};
FCN_TYPE fcn_type[3]={Sine,Sine,Sine};
char fcn_save_file[13] = "fcn_chn1";
char fcn_load_file[13] = "fcn_chn1";

int fcn_channel(long);        /* call this to enter the fcn chn conf stuff */
int fcn_channel_scan(long);   /* scan in specified channels */
int fcn_channel_update(long); /* update a specific channel */
int fcn_channel_return(long); /* return from the fcn chn conf stuff */
int fcn_channel_save(long);   /* save a table of channel settings */
int fcn_channel_load(long);   /* load a table of channel settings */

#include "fcn_tbl.h"

/* Fcn_channel_config is the callback function to start the channel 
 * configuration table.  It takes no arguments, and uses
 * the current channel table to fill in the table.
 */

int fcn_config(long arg){
  fcn_channel_scan((long) 1);
  dd_usetbl(fcn_tbl);		/* start off on the main menu */
  return 0;
}


int fcn_channel_scan(long type){
		    /* type 0 is scan user-indicated channels only */
		    /* type 1 is scan for all possible fcn_gen channels */
  int i, chnid;
  CHANNEL *cp = chn_chantbl;
  char tempbuf[64];
  
  switch(type){
  case 0:
    for(i=0; i<3; i++){
      chnid = fcn_chnid[i];
      if(chnid>=0 && chnid<chn_nchan){
	if(strcmp(chn_devtbl[cp[chnid].devid].name, "function-gen")==0){
	  fcn_freq[i] = ((SPECIFIC3*)cp[chnid].dev_sp)->frequency;
	  fcn_amp[i] = cp[chnid].scale;
	  fcn_phase[i] = cp[chnid].offset;
	  fcn_dc_offset[i] = ((SPECIFIC3*)cp[chnid].dev_sp)->dc_offset;
	  fcn_type[i] = ((SPECIFIC3*)cp[chnid].dev_sp)->type;
	  continue;
	}
      }
      if(chnid!=-1){             /* assume that they know a -1 channel doesn't exist */
	sprintf(tempbuf, "%d", chnid);
	strcat(tempbuf, " is a bad channel number, not a 'function-generator' device.");
	DD_PROMPT(tempbuf);
      }
    }
    break;

  case 1:
    for(chnid=0,i=0; chnid<chn_nchan && i<3; chnid++){
      if(!strcmp(chn_devtbl[cp[chnid].devid].name, "function-gen")){
	fcn_chnid[i]=chnid;
	fcn_freq[i] = ((SPECIFIC3*)cp[chnid].dev_sp)->frequency;
	fcn_amp[i] = cp[chnid].scale;
	fcn_phase[i] = cp[chnid].offset;
	fcn_dc_offset[i] = ((SPECIFIC3*)cp[chnid].dev_sp)->dc_offset;
	fcn_type[i] = ((SPECIFIC3*)cp[chnid].dev_sp)->type;
	i++;
      }
    }
    break;
  }

  return 0;
}


int fcn_channel_update(long id){
  CHANNEL *cp = chn_chantbl;
  int chnid = fcn_chnid[id];
  char tempbuf[64];

  /* first check if it's a valid channel */
  if(chnid>=0 && chnid<chn_nchan){
    if(!strcmp(chn_devtbl[cp[chnid].devid].name, "function-gen")){
#     ifdef SERVO
      servo_disable();    /* don't want the servo routines to interrupt us */
#     endif
      ((SPECIFIC3*)cp[chnid].dev_sp)->frequency = fcn_freq[id];
      cp[chnid].scale = fcn_amp[id];
      cp[chnid].offset = fcn_phase[id];
      ((SPECIFIC3*)cp[chnid].dev_sp)->dc_offset = fcn_dc_offset[id];
      ((SPECIFIC3*)cp[chnid].dev_sp)->type = fcn_type[id];
#     ifdef SERVO
      servo_enable();     /* values have all been updated */
#     endif
      return 0;
    }
  }
  if(chnid!=-1){
    sprintf(tempbuf, "%d", chnid);
    strcat(tempbuf, " is a bad channel number, not a 'function-generator' device.");
    DD_PROMPT(tempbuf);
  }
  return 0;
}


int fcn_channel_return(long arg){
  dd_prvtbl();
  return 0;
}


int fcn_channel_save(long arg){
  FILE *fp;
  int i;
  char tempbuf[255], response[10];
#ifdef UNUSED
  struct ffblk searchstruct;
  
				/* check if save file already exists */
  getcwd(tempbuf, 255);
  if(strlen(tempbuf) + strlen(fcn_save_file) <254){
    strcat(tempbuf, "\\");
    strcat(tempbuf,fcn_save_file);
  }
  if(findfirst(tempbuf, &searchstruct, FA_NORMAL) == 0){
    strcpy(tempbuf, fcn_save_file);
    strcat(tempbuf, " already exists. Overwrite(y/n)? ");
    while(DD_SCANF(tempbuf, "%c", response)!=1 || (*response!='y' && *response!='n'));
    if(*response=='n')
      return 0;
  }
#endif
  
  fp = fopen(fcn_save_file, "w");
  if(fp == NULL){
    strcpy(tempbuf, "Could not open ");
    strcat(tempbuf, fcn_save_file);
    strcat(tempbuf, ", skipping.");
    DD_PROMPT(tempbuf);
    return 0;
  }

  fprintf(fp, "Function Generator Channel Configuration File\n");   
  for(i=0; i<3; i++){
    fprintf(fp, "\nchannel:\n");
    fprintf(fp, "chnid %d\ntype %d\nfrequency %lf\n", fcn_chnid[i], fcn_type[i], fcn_freq[i]);
    fprintf(fp, "amplitude %lf\nphase %d\ndc_offset %lf\n", fcn_amp[i], fcn_phase[i], fcn_dc_offset[i]);
  }

  fclose(fp);
  DD_PROMPT("channel configuration saved");
  return 0;
}

int fcn_channel_load(long arg){
  FILE *fp;
  int i;
  char tempbuf[80];

  fp = fopen(fcn_load_file, "rt");
  if(fp == NULL){
    strcpy(tempbuf, "Could not open ");
    strcat(tempbuf, fcn_load_file);
    strcat(tempbuf, ", skipping.");
    DD_PROMPT(tempbuf);
    return 0;
  }

  fgets(tempbuf, 80, fp);
  if(strcmp(tempbuf, "Function Generator Channel Configuration File\n")){
    strcpy(tempbuf, fcn_load_file);
    strcat(tempbuf, " is not a valid channel configuration file, skipping. ");
    DD_PROMPT(tempbuf);
    return 0;
  }
    
  for(i=0; i<3; i++){
    fscanf(fp, "\nchannel:\n chnid%d\n type%d\n frequency%lf\n", &fcn_chnid[i], &fcn_type[i], &fcn_freq[i]);
    fscanf(fp, "amplitude%lf\n phase%d\n dc_offset%lf\n", &fcn_amp[i], &fcn_phase[i], &fcn_dc_offset[i]);
  }
  fclose(fp);
  return 0;  
}












