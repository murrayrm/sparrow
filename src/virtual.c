/*
 * virtual.c - device driver for channels which are set by user
 *
 * \author Richard M. Murray
 * \date 21 Jul 93
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include "channel.h"
#include "virtual.h"

/* Device driver routine */
int virtual_driver(DEV_ACTION action, ...)
{
  va_list ap;
  register int i;
  int status = 0;

  va_start(ap, action);
  DEVICE *dp = va_arg(ap, DEVICE *);
  CHANNEL *cp = va_arg(ap, CHANNEL *);
  
  switch (action) {
  case Write:
    for (i = 0; i < dp->size; ++i)
      cp[i].raw = (int) (cp[i].data.d / cp[i].scale) + cp[i].offset;
    status = 1;
    break;
    
  case Init:
    for (i = 0; i < dp->size; ++i) cp[i].raw = 0;
    status = 1;
    break;
    
  case NewChannels:
    break;         /* no dev-sp fields supported by virtual */

  case HandleFlag:
    break;         /* no dev-sp flags supported by virtual */
  }
  va_end(ap);
  return status;
}
