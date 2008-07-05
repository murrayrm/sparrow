/*!
 * \file sparrow.h
 * \brief Main include file for Sparrow library
 *
 * \author Richard M. Murray
 * \date 22 Dec 04
 *
 * This file includes all of the standard headers for the sparrow
 * library.  It can be used in place of including the individual
 * sparrow header files. 
 *
 */

#include "display.h"			/* display function */
#include "channel.h"			/* channel interface */
#include "servo.h"			/* servo loop control */
#include "debug.h"			/* debugging library */

/*!
 * \mainpage Sparrow Documentation
 * \section intro_sec Introduction
 *
 * Sparrow is a software package for real-time control on an IBM-PC.
 * The DGC version of sparrow is designed to be used for the DARPA
 * Grand Challenge and runs in a linux environment.  This manual
 * describes how to use the DGC version of the sparrow library and
 * access its features.
 * 
 * The user documentation for Sparrow is contained in a separate set
 * of documentation that is built as part of the sparrow installation.
 *
 * \defgroup display	Dynamic display 
 * \defgroup debug	Debugging support
 * \defgroup hook	Hook library
 * \defgroup servo	Servo loop control (experimental)
 * \defgroup channel	Device drivers (obsolete)
 * \defgroup capture	Data capture (obsolete)
 *
 */
