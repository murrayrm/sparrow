/*
 * conio.h - duplicate the functionality of Borland's console interface
 *
 * Ricahrd M. Murray
 * July 27, 1992
 */

/* Structure for returning screen information */
struct text_info {
    int screenheight;
    int screenwidth;
};

/* Cursor types for _setcursor */
#define _NOCURSOR	0
#define _NORMALCURSOR	1

int co_getch();
void co_putch(int);
int co_kbhit();
void co_beep();
int co_clreol();
void co_clrscr();
void co_gettextinfo(struct text_info *);
void co_setcursortype(int);
int co_gotoxy(int, int);
void co_textcolor(int);
void co_textbackground(int);
