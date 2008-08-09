/*!
 * \file flag.h 
 * \brief define standard video flag locations
 *
 * \author Richard M. Murray
 * \date 30 Apr 94
 *
 * \ingroup debug
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

#define CORRUPT_FLAG    73	/* capture data has been corrupted (adcap) */
#define CAPTURE_FLAG	74 	/* visual status flag location */
#define SERVO_FLAG	75	/* visual status flag location */
#define ZEBRA_FLAG	76	/* zebra overflow flag */
#define DISPLAY_FLAG    77	/* display loop status */

extern void flag(int offset, int symbol, int color);
extern void flag_symbol(int offset, int symbol);
extern void flag_color(int offset, int color);
extern void flag_init(int offset, int symbol);
extern void flag_clear(int offset);
extern void flag_on(int offset);
extern void flag_off(int offset);
extern void flag_error(int offset);
extern void flag_update();

#define FLAG_MAXFLG 8
#define FLAG_MAXCOL 10
extern char flag_sline[FLAG_MAXFLG+1];
extern char flag_cline[FLAG_MAXFLG+1];
