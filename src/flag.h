/*!
 * \file flag.h 
 * \brief define standard video flag locations
 *
 * \author Richard M. Murray
 * \brief 30 Apr 94
 *
 * \ingroup debug
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
