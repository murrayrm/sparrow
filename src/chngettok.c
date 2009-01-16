/*!
 * \file chngettok.c 
 * \brief parser for channel configuration file
 * 
 * \author Eric Wemhoff
 * \date September 1993
 * 
 * \ingroup channel
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
#include <string.h>
#include "display.h"

/*
 * Parser for configuration file; gets strings separated by _delimiters, from
 * input stream _fp.  _line keeps track of newline characters. A maximum of
 * _length characters is returned in _string.
 */
int chn_gettok(FILE * fp, char *string, int length, char *delimiters, int *line)
{
  int i = 0;
  int flag = 0;
  char ch;

  while(string[i] != '\0')
    string[i++] = ' ';
  i = 0;

  /* ignore preceeding whitespace */
  do {
    ch = fgetc(fp);
    if (ch == '#') {	/* ignore stuff from # to end of line
			 * (comments) */
      while (ch != '\n' && !feof(fp))
	ch = fgetc(fp);
    }
    if (ch == '\n')
      ++(*line);
  } while (ch == ' ' || ch == '\t' || ch == '\n');
  
  while (i < length - 1) {
    if (ch == '\n')
      ++(*line);
    if (strchr(delimiters, (int) ch) != NULL || feof(fp)) {
      if (strchr(string, '=') != NULL) flag = 1;
      while (ch == ' ' || ch == '\t') /* allow spaces or tabs between */
	ch = fgetc(fp);               /* flag and equal sign as well as */
      if (ch == '=')                  /* equal sign and flag value */
	flag = 1;
      else if ((flag) && (((ch >= '0') && (ch <= '9')) || (ch == '.')))
	continue;
      else {
	flag = 0;
	if (!feof(fp) && ch != '\n' && ch != ';')
	  ungetc(ch,fp);
	string[i] = '\0';
	return ch;
      }
    }
    if (ch == '#') {	/* ignore stuff from # to end of line
			 * (comments) */
      while (ch != '\n' && !feof(fp))
	ch = fgetc(fp);
    }
    string[i++] = ch;
    ch = fgetc(fp);
  }
  string[i] = '\0';
  return -1;			/* overflow */
}
