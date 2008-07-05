/*
 * ptycat - simple program to read/write from psuedo tty
 *
 * \author Richard M. Murray
 * \date 23 Dec 04
 *
 * This program is designed for use in testing out the serial device 
 * interface to sparrow.  It opens up a pty and allows input/output
 * via stdin/stdout.
 *
 * Usage: run 'ptycat', which will print out the device that it has created.
 * Use that device in place of the serial device in your config.dev file.
 *
 * Compilation: g++ ptycat.c -o ptycat -lreadline -lpthread
 *
 * TBD:
 *   * consider turning this into a more general program that can act
 *     as a monitor for serial ports, showing/logging all commands
 *   * add dynamic display interface instead of command line
 *   * add functions to clear input buffer (for synchronization)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <pthread.h>
#include <termios.h>
#include <readline/readline.h>

static int ready(FILE *);		/* Check to see if file has data */

int main (int argc, char **argv)
{
  int c, errflg = 0;
  int ptyfd;				/* file descriptor for pty */
  int ptynum = -1;			/* pty number to use */
  char *inpline, ptydev[64];		/* string buffers */
  struct termios term;			/* use to set term attributes */

  extern int optind;
  extern char *optarg;

  /* Parse command line arguments */
  while ((c = getopt(argc, argv, "?")) != EOF)
    switch (c) {
	default:
	    errflg++;		/* print usage information */
	    break;
	}

  if (errflg) {
    fprintf(stderr, "usage: %s [-h]\n", argv[0]);
    exit(2);
  }

  /* Open up the pty for reading and writing */
  if ((ptyfd = open("/dev/ptmx", O_RDWR)) < 0) {
    perror("ptycat");
    exit(2);
  }

  /* Allow access to psuedo tty */
  if (grantpt(ptyfd) < 0) { perror("grantpt"); exit(2); }
  if (unlockpt(ptyfd) < 0) { perror("unlockpt"); exit(2); }

  /* Set up the terminal for raw interface */
  cfmakeraw(&term);
  tcsetattr(ptyfd, TCSANOW, &term);
  
  /* Print out the pty number */
  printf("pty = %s\n", ptsname(ptyfd));

  /*
   * Main loop - read from serial port and send back info
   *
   * The main loop reads whatever is sent across the serial port and
   * then send s back the information typed by the user.  Use '.' to 
   * send back no information.
   *
   */

  while (1) {
    char msg[128];
    int status, i = 0;

    /* Read up until a new line */
    while ((status = read(ptyfd, msg+i, 1)) >= 0 && i < 127 &&
	   msg[i] != '\n') ++i;
    msg[i+1] = '\0';

    /* Print out what we read */
    printf("data (%d, %d): %s", i, status, msg);

    /* Now read a line from the terminal as a response */
    if ((inpline = readline("response: ")) == NULL) break;
    sprintf(msg, "%s\n", inpline);
    free(inpline);

    /* Skip the response if we received '.' */
    if (msg[0] == '.' && msg[1] == '\n') continue;
   
    /* Now write the characters out and clear the buffer */
    write(ptyfd, msg, strlen(msg));
  }
  
  return 1;
}
