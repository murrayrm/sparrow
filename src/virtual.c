/*
 * virtual.c - device driver for channels which are set by user
 *
 * \author Richard M. Murray
 * \date 21 Jul 93
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
