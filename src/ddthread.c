/*!
 * \file ddthread.c
 * \brief start display manager as a thread 
 *
 * \ingroup display
 */

#include <stdlib.h>
#include <pthread.h>
#include "display.h"

void *dd_loop_thread(void *arg)
{
  dd_loop();
  return NULL;
}
