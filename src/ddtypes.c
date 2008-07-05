/*!
 * \file ddtypes.c 
 * \brief display managers for standard data types
 *
 * \author Richard M. Murray
 * \date 31 Dec 03 (rebuilt)
 *
 * \ingroup display
 */

#include <stdio.h>
#include <string.h>
#include "display.h"

/* Check and display doubles (double precision) */
int dd_double(DD_ACTION action, int id)
{
  DD_IDENT *dd = ddtbl + id;
  char ibuf[32];
  double *value = (double *)dd->value, *current = (double *)dd->current;
  double dtmp;
  
  switch (action) {
  case Update:
    if (dd->current != NULL && (!dd->initialized || *value != *current)) {
      snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
      dd_puts(dd, ibuf);
      dd->initialized = 1;
    }
    break;
    
  case Refresh:
    snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
    dd_puts(dd, ibuf);
    dd->initialized = 1;
    break;
    
  case Input:
    if (DD_SCANF("Float: ", "%lf", &dtmp) == 1)
      *value = dtmp;
    break;
    
  case Save:
    sprintf(dd_save_string, "%lg", *value);
    break;
    
  case Load:
    if(sscanf(dd_save_string, "%lf", &dtmp)==1)
      *value = dtmp;
    break;
    
  }
  return 0;
}

/* Check and display short integers */
int dd_short(DD_ACTION action, int id)
{
  DD_IDENT *dd = ddtbl + id;
  char ibuf[12];

  int *value = (int *)dd->value, *current = (int *)dd->current;
  
  switch (action) {
  case Update:
    if (dd->current != NULL && (!dd->initialized || *value != *current)) {
      snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
      dd_puts(dd, ibuf);
      dd->initialized = 1;
    }
    break;
    
  case Refresh:
    snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
    dd_puts(dd, ibuf);
    dd->initialized = 1;
    break;
    
  case Input:
    DD_SCANF("Integer: ", "%d", value);
    break;
    
  case Save:
    sprintf(dd_save_string, "%d", *value);
    break;
    
  case Load:
    sscanf(dd_save_string, "%d", value);
    break;

  }
  return 0;
}

/* Check and display bytes */
int dd_byte(DD_ACTION action, int id)
{
    DD_IDENT *dd = ddtbl + id;
    char ibuf[8];
    char *value = (char *)dd->value, *current = (char *)dd->current;
    int itmp;
    
    switch (action) {
    case Update:
        if (dd->current != NULL && (!dd->initialized || *value != *current)) {
    	    snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
	    dd_puts(dd, ibuf);
	    dd->initialized = 1;
	}
        break;

    case Refresh:
	snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
	dd_puts(dd, ibuf);
	dd->initialized = 1;
	break;

    case Input:
      if (DD_SCANF("Byte: ", "%d", &itmp) == 1)
	*value = itmp;
      break;
	
      case Save:
	printf("can't save byte\n");
	break;
      case Load: 
	printf("can't load byte\n");
	break;
    }
    return 0;
}

/* Check and display floats (single precision) */
int dd_float(DD_ACTION action, int id)
{
  DD_IDENT *dd = ddtbl + id;
  char ibuf[8];
  float *value = (float *)dd->value, *current = (float *)dd->current;
  float ftmp;
  
  switch (action) {
  case Update:
    if (dd->current != NULL && (!dd->initialized || *value != *current)) {
      snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
      dd_puts(dd, ibuf);
      dd->initialized = 1;
    }
    break;
    
  case Refresh:
    snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
    dd_puts(dd, ibuf);
    dd->initialized = 1;
    break;
    
  case Input:
    if (DD_SCANF("Float: ", "%f", &ftmp) == 1)
      *value = ftmp;
    break;
    
  case Save:
    sprintf(dd_save_string, "%g", *value);
    break;
    
  case Load:
    if(sscanf(dd_save_string, "%f", &ftmp)==1)
      *value = ftmp;
    break;
    
  }
    return 0;
}

int dd_label(DD_ACTION action, int id)
{
    DD_IDENT *dd = ddtbl + id;
    switch (action) {
    case Update:
       if (!dd->initialized) {
	   dd_puts(dd, (char *) dd->value);
           dd->initialized = 1;
       }
       break;

    case Refresh:
       dd_puts(dd, (char *) dd->value);
       dd->initialized = 1;
       break;
    }
    return 0;
}

/* Check and display long integers */
int dd_long(DD_ACTION action, int id)
{
  DD_IDENT *dd = ddtbl + id;
  char ibuf[32];
  long *value = (long *)dd->value, *current = (long *)dd->current;
  
  switch (action) {
  case Update:
    if (dd->current != NULL && (!dd->initialized || *value != *current)) {
      snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
      dd_puts(dd, ibuf);
      dd->initialized = 1;
    }
    break;
    
  case Refresh:
    snprintf(ibuf, sizeof(ibuf), dd->format, *current = *value);
    dd_puts(dd, ibuf);
    dd->initialized = 1;
    break;
    
  case Input:
    DD_SCANF("Integer: ", "%ld", value);
    break;
    
  case Save:
    sprintf(dd_save_string, "%ld", *value);
    break;
  case Load:
    sscanf(dd_save_string, "%ld", value);
    break;
  }
  return 0;
}

/* Display manager function for dynamic strings */
int dd_string(DD_ACTION action, int id)
{
  DD_IDENT *dd = ddtbl + id;
  int oldfg = dd->foreground;
  char ibuf[256];
  
  switch (action) {
  case Update:
    /* Assume that string never changes on its own */
    if(dd->initialized) break;
    /* Drop through to refresh */
    
  case Refresh:
    if(dd->value) /* check for NULL string */
      strcpy(ibuf, (char *) dd->value);
    else          /* copy in an empty string if pointer is NULL */
      strcpy(ibuf, "");
    dd_puts(dd, ibuf);
    dd->initialized = 1;
    break;
    
  case Input:
    /* Read string into temporary buffer */
    dd_read("String : ", ibuf, (int) (dd->length));
    
    /* If the string is empty, do nothing */
    if (*ibuf == '\0') break;
    
    /* Otherwise erase the old string and display the new */
    dd->foreground = dd->background;
    dd_puts(dd, (char *) dd->value);
    dd->foreground = oldfg;
    
    /* Copy temporrary buffer into storage and redisplay */
    strcpy((char *) dd->value, ibuf);
    dd_puts(dd, ibuf);
    break;
    
  case Save:
    /* put string in dd_save_string */
    strcpy(dd_save_string, (char *) dd->value);
    break;
    
  case Load:
    strcpy((char *) dd->value, dd_save_string);
    break;
    
  }
  return 0;
}

/* Display manager function for messages */
int dd_message(DD_ACTION action, int id)
{
  DD_IDENT *dd = ddtbl + id;
  char **value = (char **)dd->value, **current = (char **)dd->current;
  
  switch (action) {
  case Update:
    if (dd->current != NULL && (!dd->initialized || *value != *current)) {
      if (*value != NULL) dd_puts(dd, *current = *value);
      dd->initialized = 1;
    }
    break;
    
  case Refresh:
    if (*current != NULL) dd_puts(dd, *current = *value);
    dd->initialized = 1;
    break;
    
  case Input:
  case Save:				/* Not implemented */
  case Load:
    break;
    
  }
  return 0;
}

int dd_nilmgr(DD_ACTION action, int id) { return 0; }
