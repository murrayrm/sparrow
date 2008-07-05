/*
 * tclib.h
 *
 * Termcap definitions
 *
 */

extern int tc_color;			/* monitor is color capable */

extern void tc_enter_termcap();
extern void tc_free();

int tc_init(int vflg);
void tc_open();
int tc_getc();
void tc_rmove(int, int, int, int);
void tc_amove(int, int);
void tc_clear_to_eol(int);
void tc_clear(int);
void tc_insert(char *, int, int);
void tc_delete(char *, int, int);
void tc_enter_termcap();
void tc_exit_termcap();
void tc_space();
void tc_bell();
void tc_down();
int tc_lines();
void tc_close();
void tc_free();
void tc_reverse(), tc_normal();
void tc_setfg(int), tc_setbg(int);
