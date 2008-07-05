/*
 * keymap.h - Keymap tables for redo
 *
 * Richard M. Murray
 * November 1, 1987
 *
 */

#ifndef DDKEYMAP
#define DDKEYMAP

/* Define a few macros to simplify the keymap table */
#define F0 dd_unbound
#define F1 dd_next
#define F2 dd_prev
#define F3 dd_input
#define F4 dd_exit_loop
#define F5 dd_redraw
#define F6 dd_up
#define F7 dd_down
#define F8 dd_left
#define F9 dd_right
#define F10 dd_exec_callback
#define F11 dd_toggle_beep_cb

/* default keymap table - numbers refer to function_tbl index */
static KM_BINDING dd_defkeymap[K_MAX] = {
    F0,  F0,  F0,  F0,  F0,  F0,  F0, F11,    /* ^@ to ^G */
    F0,  F0,  F0,  F0,  F5, F10,  F0,  F0,    /* ^H to ^O */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* ^P to ^W */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* ^X to US,  ^_ */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* space to ' */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* (  to /  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /*F0  to 7  */
    F0,  F0,  F0,  F0,  F0,  F3,  F0,  F0,    /* 8  to ?  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* @  to G  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* H  to O  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* P  to W  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* X  to _  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* `  to g  */
    F8,  F0,  F7,  F6,  F9,  F0,  F0,  F0,    /* h  to o  */
    F0,  F4,  F0,  F0,  F0,  F0,  F0,  F0,    /* p  to w  */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* x  to ^? */
    F8,  F9,  F6,  F7,  F0,  F0,  F0,  F0,    /* lft rgt up dn strt end ? ? */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* ins del backspace */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0,    /* 144   */
    F0,  F0,  F0,  F0,  F0,  F0,  F0,  F0    /* 152   */
};

#endif
