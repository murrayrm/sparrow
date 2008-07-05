/*!
 * \file chngettok.c 
 * \brief parser for channel configuration file (from sparrow)
 * 
 * \author Eric Wemhoff
 * \date September 1993
 * 
 * 3 Jan 03, RMM: stripped code from sparrow for DGC
 * 
 * \ingroup channel
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
      while (ch != '\n' && ch != EOF)
	ch = fgetc(fp);
    }
    if (ch == '\n')
      ++(*line);
  } while (ch == ' ' || ch == '\t' || ch == '\n');
  
  while (i < length - 1) {
    if (ch == '\n')
      ++(*line);
    if (strchr(delimiters, (int) ch) != NULL || ch == EOF) {
      if (strchr(string, '=') != NULL) flag = 1;
      while (ch == ' ' || ch == '\t') /* allow spaces or tabs between */
	ch = fgetc(fp);               /* flag and equal sign as well as */
      if (ch == '=')                  /* equal sign and flag value */
	flag = 1;
      else if ((flag) && (((ch >= '0') && (ch <= '9')) || (ch == '.')))
	continue;
      else {
	flag = 0;
	if (ch != EOF && ch != '\n' && ch != ';')
	  ungetc(ch,fp);
	string[i] = '\0';
	return ch;
      }
    }
    if (ch == '#') {	/* ignore stuff from # to end of line
			 * (comments) */
      while (ch != '\n' && ch != EOF)
	ch = fgetc(fp);
    }
    string[i++] = ch;
    ch = fgetc(fp);
  }
  string[i] = '\0';
  return -1;			/* overflow */
}
