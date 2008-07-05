/* 
 * fcn_gen.c - definitions for function generator drivers
 * 
 * ELW, 3 Oct 93
 */

int fcn_driver(DEV_ACTION, ...);
int fcn_change_frequency(int, double);

enum fcn_type{
  Sine,
  Square,
  Triangle,
};
typedef enum fcn_type FCN_TYPE;

extern int fcn_channel(long);

