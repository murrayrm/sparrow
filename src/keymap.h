/*
 * keymap.h - definitions and structions for key mapping routines
 *
 * \author Richard M. Murray
 * \date 24 Jan 94
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

#ifndef __KEYMAP_INCLUDED__
#define __KEYMAP_INCLUDED__

/* Key binding table */
typedef int (*KM_BINDING)(long); /* bindings are pointers to fcns */

/* Function declarations */
extern KM_BINDING *km_createmap(void);
extern KM_BINDING *km_copymap(KM_BINDING *, KM_BINDING *);
extern KM_BINDING *km_initmap(KM_BINDING *);
extern void km_freemap(KM_BINDING *);
extern int km_bindkey(KM_BINDING *, int, int (*)(long));
extern int km_unbound(long);

/* Macro to execute a key binding */
#define km_execkey(keymap, key)	(*keymap[key])((long) 0)

/* Standardized keycodes */
#define K_LEFT          128     /* Left arrow       */
#define K_RIGHT         129     /* Right arrow      */
#define K_UP            130     /* Up arrow         */
#define K_DOWN          131     /* Down arrow       */
#define K_HOME          132     /* Home             */
#define K_END           133     /* End              */
#define K_PGUP          134     /* Page Up          */
#define K_PGDN          135     /* Page Down        */
#define K_INS           136     /* Insert           */
#define K_DEL           137     /* Delete           */
#define K_BS            138     /* Backspace key    */

#define K_F1            141     /* Function key 1   */
#define K_F2            142     /* Function key 2   */
#define K_F3            143     /* Function key 3   */
#define K_F4            144     /* Function key 4   */
#define K_F5            145     /* Function key 5   */
#define K_F6            146     /* Function key 6   */
#define K_F7            147     /* Function key 7   */
#define K_F8            148     /* Function key 8   */
#define K_F9            149     /* Function key 9   */
#define K_F10           150     /* Function key 10  */

/* Shift and control function keys (for MSDOS) */
#ifdef MSDOS
#define K_SHIFT_F1      151     /* Function key 1 - shifted */
#define K_SHIFT_F2      152     /* Function key 2 - shifted */
#define K_SHIFT_F3      153     /* Function key 3 - shifted */
#define K_SHIFT_F4      154     /* Function key 4 - shifted */
#define K_SHIFT_F5      155     /* Function key 5 - shifted */
#define K_SHIFT_F6      156     /* Function key 6 - shifted */
#define K_SHIFT_F7      157     /* Function key 7 - shifted */
#define K_SHIFT_F8      158     /* Function key 8 - shifted */
#define K_SHIFT_F9      159     /* Function key 9 - shifted */
#define K_SHIFT_F10     160     /* Function key 10- shifted */

#define K_CNTRL_F1      161     /* Function key 1 - control */
#define K_CNTRL_F2      162     /* Function key 2 - control */
#define K_CNTRL_F3      163     /* Function key 3 - control */
#define K_CNTRL_F4      164     /* Function key 4 - control */
#define K_CNTRL_F5      165     /* Function key 5 - control */
#define K_CNTRL_F6      166     /* Function key 6 - control */
#define K_CNTRL_F7      167     /* Function key 7 - control */
#define K_CNTRL_F8      168     /* Function key 8 - control */
#define K_CNTRL_F9      169     /* Function key 9 - control */
#define K_CNTRL_F10     170     /* Function key 10- control */

#define K_ALT_F1        171     /* Function key 1 - alt */
#define K_ALT_F2        172     /* Function key 2 - alt */
#define K_ALT_F3        173     /* Function key 3 - alt */
#define K_ALT_F4        174     /* Function key 4 - alt */
#define K_ALT_F5        175     /* Function key 5 - alt */
#define K_ALT_F6        176     /* Function key 6 - alt */
#define K_ALT_F7        177     /* Function key 7 - alt */
#define K_ALT_F8        178     /* Function key 8 - alt */
#define K_ALT_F9        179     /* Function key 9 - alt */
#define K_ALT_F10       180     /* Function key 10- alt */

#define K_CNTRL_LEFT    181	/* control-left arrow */
#define K_CNTRL_RIGHT   182	/* control-right arrow */
#define K_CNTRL_UP      183	/* control-up arrow */
#define K_CNTRL_DOWN    184	/* control-down arrow */
#define K_ALT_LEFT      185	/* alt-left arrow */
#define K_ALT_RIGHT     186	/* alt-right arrow */
#define K_ALT_UP        187	/* alt-up arrow */
#define K_ALT_DOWN      188	/* alt-down arrow */

#define K_MAX           195     /* Maximum keycode used (+1) */
#else
#define K_MAX           160
#endif

#endif /* __KEYMAP_INCLUDED__ */

