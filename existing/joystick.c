/*
   simple test program for joystick driver

   usage: js 0       (to test first joystick)
   usage: js 1       (to test second joystick)
*/

#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main (int argc, char **argv)
{
	int fd, status;
	char *fname;
	struct JS_DATA_TYPE js;

	/* should be one argument, and it should be "0" or "1" */
	if (argc != 2 || (strcmp (argv[1], "0") && strcmp (argv[1], "1"))) {
		fprintf (stderr, "usage: js 0|1\n");
		exit (1);
	}

	/* pick appropriate device file */
	if (!strcmp (argv[1], "0"))
		fname = "/dev/js0";
	else if (!strcmp (argv[1], "1"))
		fname = "/dev/js1";
	else
		fname = NULL;

	/* open device file */
	fd = open (fname, O_RDONLY);
	if (fd < 0) {
		perror ("js");
		exit (1);
	}

	printf ("Joystick test program (interrupt to exit)\n");

	while (1) {
		status = read (fd, &js, JS_RETURN);
		if (status != JS_RETURN) {
			perror ("js");
			exit (1);
		}

		fprintf (stdout, "button 0: %s  button 1: %s  X position: %4d  Y position: %4d\r",
			 (js.buttons & 1) ? "on " : "off",
			 (js.buttons & 2) ? "on " : "off",
			 js.x,
			 js.y);
		fflush (stdout);

		/* give other processes a chance */
		usleep (100);
	}

	exit (0);
}
