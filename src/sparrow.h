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
