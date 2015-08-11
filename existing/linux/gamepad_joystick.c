
  // Uses both sticks of a Logitech gamepad for tank drive control of a Propller BOE-Bot running Mike Green's code.
  // The following uses a few internet examples, I'm not sure of the origianl author of the examples.  
  // If you know either let me know, so I can credit them.  - J. A. Streich.
  
#include <termios.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <sys/signal.h>
#include <errno.h>
#include "casper.h"
#include <string.h>


// Define where the Joystick device and XBee are.  Joystick is typicall /dev/js0 or /dev/input/js0
// First USB serial device is ttyUSB0
#define JOY_DEV "/dev/input/js0"
#define XBEE_DEV "/dev/ttyUSB0"

int initport(int fd) {
	struct termios options;
	// Get the current options for the port...
	tcgetattr(fd, &options);
	// Set the baud rates to 19200...
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	// Enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;

	// Set the new options for the port...
	tcsetattr(fd, TCSANOW, &options);
	return 1;
}

int main()
{
        //Define variables.
	int joy_fd, *axis=NULL, num_of_axis=0, num_of_buttons=0, x, xbee_fd, i;
	char *button=NULL, name_of_joystick[80], servoleft[250], servoright[250], stopleft[30] = "srvleft = 1500", 
             stopright[30] = "srvright = 1500";
        char sResult[1024];
	struct js_event js;

        // Open Joystick for reading
	if( ( joy_fd = open( JOY_DEV , O_RDONLY)) == -1 )
	{
		printf( "Couldn't open joystick\n" );
		return -1;
	}

        // Open USB Serial TTY for read and write...
        if( ( xbee_fd = open( XBEE_DEV , O_RDWR | O_NOCTTY | O_NDELAY)) == -1)
        {
          printf( "Couldn't open xbee\n" );
          return -1;
        }

        //XBee control stuff, like init and baud.
        fcntl(xbee_fd, F_SETFL, 0);
        printf("baud=%d\n", getbaud(xbee_fd));
        initport(xbee_fd);
        printf("baud=%d\n", getbaud(xbee_fd));

/*  // If you want to see version sting and such.  Commented out so this doesn't block if Bot is already on...
	if (!readport(xbee_fd,sResult)) {
		printf("read failed\n");
		close(xbee_fd);
		return 1;
	}
	printf("readport=%s\n", sResult);
*/

	ioctl( joy_fd, JSIOCGAXES, &num_of_axis );
	ioctl( joy_fd, JSIOCGBUTTONS, &num_of_buttons );
	ioctl( joy_fd, JSIOCGNAME(80), &name_of_joystick );
	// Char ruler ro see if I'm setting the right chars.
//                         0         1
//                         012345678901234
        strcpy(servoleft, "srvleft =     ");
        strcpy(servoright,"srvright =     ");

	// Get what buttons and analog sticks
	axis = (int *) calloc( num_of_axis, sizeof( int ) );
	button = (char *) calloc( num_of_buttons, sizeof( char ) );

	// Show what's availible.
	printf("Joystick detected: %s\n\t%d axis\n\t%d buttons\n\n"
		, name_of_joystick
		, num_of_axis
		, num_of_buttons );

	// Don't block for reading the joystick.
	fcntl( joy_fd, F_SETFL, O_NONBLOCK );	/* use non-blocking mode */

	while( 1 ) 	/* infinite loop */
	{

			/* read the joystick state */
		read(joy_fd, &js, sizeof(struct js_event));
		
			/* see what to do with the event */
		switch (js.type & ~JS_EVENT_INIT)
		{
			case JS_EVENT_AXIS:
				//If it's the up down of either stick, ignoring (for now) left right
                                if(js.number == 1 || js.number == 3)
                                {                
                                  axis [ js.number-1 ] = axis [js.number];
                                  if(js.number == 1)
                                  {
				    // Translate up so 0 => 1500 (stoped), 
                                    // and scale so that 32,767 => 2000 (full on)
                                    //   and -32,000 => 1000 (full back)
	 			    axis   [ js.number ] = (int) ( ( (float) (-js.value) * .0152592547 ) + 1500) ;
				  }else{
				    axis   [ js.number ] = (int) ( ( (float) (js.value) * .0152592547 ) + 1500) ;
				  }

				  // Axis 1 -> left servo, 
				  // If the stick still maps to the speed we're already going, do nothing.
                                  if(js.number == 1 && axis [0] != axis[1])
                                  {
                                        servoleft[10] = (char)(axis[js.number]/1000+'0');
                                        servoleft[11] = (char)((axis[js.number]%1000)/100+'0');
                                        servoleft[12] = (char)((axis[js.number]%100)/10+'0');
                                        servoleft[13] = (char)((axis[js.number]%10)+'0'); 
                                        servoleft[14] = (char)0;
					servoleft[15] = (char)0;

                                        printf(servoleft);
                                	
                                       if(!writeport(xbee_fd,servoleft))
                                        {
						printf("Error writing to XBee!");
                                        }
                                        // Removed readport call from here, ignoring errors the bot sends,
					// reading can block, and if Bot doesn't send reply (or is off) we get stuck here
                                  }
                                  else if( js.number == 3 && axis[2] != axis[3]) // same thing, axis 3 => right servo
                                  {
                                        servoright[11] = (char)(axis[js.number]/1000+'0');
                                        servoright[12] = (char)((axis[js.number]%1000)/100+'0');
                                        servoright[13] = (char)((axis[js.number]%100)/10+'0');
                                        servoright[14] = (char)((axis[js.number]%10)+'0');
                                        servoright[15] = (char)0;
                                        servoright[16] = (char)0;

                                        printf(servoright);
                                        if(!writeport(xbee_fd,servoright))
                                        {
                                                printf("Error writing to XBee!");
                                        }
					// Here, again, the read is removed, see comment above.
                                  }
                                }
				break;
			case JS_EVENT_BUTTON:
				button [ js.number ] = js.value; 
				if(js.number == 9 && js.value == 1) // If buton was 9 (button labeled "10" on the pad itself)
				{
                                  writeport(xbee_fd,stopleft);  // Stop left
				  writeport(xbee_fd,stopright); // Stop right
                                  close(xbee_fd);		// close Serial Port
				  close(joy_fd);		// close JoyStick
				  return 0;			// quit, exit status 0, no errors
				}
				break;
		}
	}

	close( joy_fd );	/* Just in case something really bizare happens.*/
        close( xbee_fd );
	return 0;
}
