/*---------------------------------------------------------------------------
 * filename - cgets.c
 *
 * function(s)
 *        cgets - reads string from console
 *--------------------------------------------------------------------------*/

/*[]------------------------------------------------------------[]*/
/*|                                                              |*/
/*|     C/C++ Run Time Library - Version 4.0                     |*/
/*|                                                              |*/
/*|                                                              |*/
/*|     Copyright (c) 1987, 1991 by Borland International        |*/
/*|     All Rights Reserved.                                     |*/
/*|                                                              |*/
/*[]------------------------------------------------------------[]*/



/*--------------------------------------------------------------------------*

Name            cgets - reads string from console

Usage           char *cgets(char *string);

Prototype in    conio.h

Description     cgets reads a string of characters from the console,
                storing the string (and the string length) in the
                location pointed to by string.

                cgets reads characters until it encounters a CR/LF or
                until the maximum allowable number of characters have
                been read.

                Before cgets is called, string[0] should be set to the
                maximum length of the string to be read.

Return value    string[1] is set to the number of characters actually
                read.
                &string[2], a pointer to the string of characters that
                were read.  There is no error return.

*---------------------------------------------------------------------------*/

#include "conio.h"

char * cgets( char *s )
  {
  int c, len, maxlen;
  char *p = s + 2;

  len = 0;
  maxlen = s[ 0 ] & 0xff;

  while( 1 )
    {
    switch( c = co_getch() )
      {
      case 0:     if( co_getch() != 75 )  break;      /* keypad left arrow */
      case '\b':  if( len )  
                    {
                    co_putch( '\b' );
                    co_putch( ' ' );
                    co_putch( '\b' );
                    --len;
                    --p;
                    }
                  break;

      case '\r':  *p = '\0';
                  s[ 1 ] = len;   
                  return( s + 2 );

      default:    if( len < maxlen - 1 )
                    {
                    co_putch( c );
                    *p++ = c;
                    ++len;
                    }
      }
    }
  }
