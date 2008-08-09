/*!
 * \file ddsave.c 
 * \brief routines for saving and reloading data from display tables
 *
 * \author RLB
 * \date Sep 94
 *
 * These functions allow you to save data stored in a display table
 * to disk and reload that data at a later time.
 *
 * \ingroup display
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
#include <stdio.h>
#include <string.h>
#include "display.h"

/*!
 * \fn int dd_save(char *filename)
 * \brief save the display variables for the current table
 */
int dd_save(char *filename)
{
  return dd_tbl_save(filename, ddtbl);
}

/*!
 * \fn int dd_save_tbl(char *filename, DD_IDENT *tbl)
 * \brief save a specific table
 */
int dd_save_tbl(char *filename, DD_IDENT *tbl)
{
  int i;
  char tempbuf[256], response[10];
  FILE *fp;
  DD_IDENT *tmptbl;

#ifdef UNUSED
  struct ffblk file_search;
  i = findfirst(filename, &file_search, FA_NORMAL);
  if(i==0){
    strcpy(tempbuf, filename);
    strcat(tempbuf, " already exists. Overwrite(y/n)? ");
    while (DD_SCANF(tempbuf, "%c", response) != 1 || (*response!='y' && *response!='n'));
    if(*response=='n')
      return -1;
  }
#endif
  if((fp=fopen(filename,"w"))==NULL){
    DD_PROMPT("Unable to open file.");
    return -1;
  }

  tmptbl = ddtbl;		/* store current table */
  ddtbl = tbl;			/* switch current table, to load values */
  /* Search thru table for variables to save; a saveable variable is
     indicated by a non-null varname */
  for (i = 0; ddtbl[i].value != NULL; i++) {
    if(*(ddtbl[i].varname) != '\0'){
      /* the manager function is responsible for formatting and storing the
	 number it wants to save in dd_save_string, max length 256 */
      (*ddtbl[i].function)(Save, i); 
      fprintf(fp,"%s\t%s\n",ddtbl[i].varname,dd_save_string);
    }
  }

  fclose(fp);
  if(tmptbl != tbl)
    ddtbl = tmptbl;		/* go back to current table */

  return 1;
}

/*!
 * \fn dd_load(char *filename)
 * \brief load saved display vars into current table
 */
int dd_load(char *filename){
  return dd_tbl_load(filename, ddtbl);
}

/*!
 * \fn dd_tbl_load(char *filename, DD_IDENT *tbl)
 * \brief load saved display vars into a specific table
 */
int dd_tbl_load(char *filename, DD_IDENT *tbl){
  int line=0, i;
  FILE *fp;
  char buf[256];
  DD_IDENT *tmptbl;

  if ((fp = fopen(filename,"r"))==NULL) {
#   ifdef UNUSED			/* don't print error msg here */
    perror("dd_load");
    strcpy(buf,"dd_load: error opening inputfile: ");
    strcat(buf, filename); 
    DD_PROMPT(buf);
#   endif
    return -1;
  }

  tmptbl = ddtbl;		/* store current table */
  ddtbl = tbl;			/* switch current table, to load values */
  while( (chn_gettok(fp, buf, 256, " \t\n", &line)) !=EOF){
    /* see if this variable is defined in the _current_ display */
    for (i = 0; tbl[i].value != NULL; ++i)
      if(!strcmp(tbl[i].varname, buf)) /* found */
	break;
    chn_gettok(fp, buf, 256, " \t\n", &line);
    if(tbl[i].value != NULL){	/* found the varname */
      /* the manager function is responsible for using the string in dd_save_string
	 to load its value */
      strcpy(dd_save_string, buf);
      (*ddtbl[i].function)(Load, i); 
    }
  }
  fclose(fp);
  
  if(tmptbl != tbl)
    ddtbl = tmptbl;		/* go back to current table */

  return 1;
}


