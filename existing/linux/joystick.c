/*
linux/drivers/char/joystick.c
   Copyright (C) 1992, 1993 Arthur C. Smith
   Joystick driver for Linux running on an IBM compatible computer.

VERSION INFO:
01/08/93	ACS	0.1: Works but needs multi-joystick support
01/13/93	ACS	0.2: Added multi-joystick support (minor 0 and 1)
		  	     Added delay between measuring joystick axis
		   	     Added scaling ioctl
02/16/93	ACS	0.3: Modified scaling to use ints to prevent kernel
			     panics 8-)
02/28/93	ACS	0.4: Linux99.6 and fixed race condition in js_read.
			     After looking at a schematic of a joystick card
                             it became apparent that any write to the joystick
			     port started ALL the joystick one shots. If the
			     one that we are reading is short enough and the
			     first one to be read, the second one will return
			     bad data if it's one shot has not expired when
			     the joystick port is written for the second time.
			     Thus solves the mystery delay problem in 0.2!
05/05/93	ACS/Eyal 0.5:Upgraded the driver to the 99.9 kernel, added
			     joystick support to the make config options,
			     updated the driver to return the buttons as
			     positive logic, and read both axis at once
			     (thanks Eyal!), and added some new ioctls.
02/12/94    Jeff Tranter 0.6:Made necessary changes to work with 0.99pl15
                             kernel (and hopefully 1.0). Also did some
			     cleanup: indented code, fixed some typos, wrote
			     man page, etc...
05/17/95    Dan Fandrich 0.7.3:Added I/O port registration, cleaned up code
04/03/96	Matt Rhoten 0.8: many minor changes:
			new read loop from Hal Maney <maney@norden.com>
			cleaned up #includes to allow #include of joystick.h with
				gcc -Wall and from g++
			made js_init fail if it finds zero joysticks
			general source/comment cleanup
			use of MOD_(INC|DEC)_USE_COUNT
			changes from Bernd Schmidt <crux@Pool.Informatik.RWTH-Aachen.DE>
				to compile correctly under 1.3 in kernel or as module
*/

#include <linux/module.h>
#include <linux/joystick.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <asm/io.h>

static struct JS_DATA_SAVE_TYPE JS_DATA[JS_MAX];	/* misc data */
static int JS_EXIST;			/* which joysticks' axis exist? */
static int JS_READ_SEMAPHORE;	/* to prevent two processes from trying
								   to read different joysticks at the
								   same time */

/* get_timer0():
   returns the current value of timer 0. This is a 16 bit counter that starts
   at LATCH and counts down to 0 */
inline int get_timer0 (void)
{
	int t0, t1;
	outb (0, PIT_MODE);
	t0 = (int) inb (PIT_COUNTER_0);
	t1 = ((int) inb (PIT_COUNTER_0) << 8) + t0;
	return (t1);
}

/* find_axes():

   returns which axes are hooked up, in a bitfield. 2^n is set if
   axis n is hooked up, for 0 <= n < 4.

   REVIEW: should update this to handle eight-axis (four-stick) game port
   cards. anyone have one of these to test on? mattrh 3/23/96 */
inline int find_axes(void)
{
	int j;

	outb (0xff, JS_PORT);		/* trigger oneshots */
								/* and see what happens */

	for (j = JS_DEF_TIMEOUT; (0x0f & inb (JS_PORT)) && j; j--)
		;						/* do nothing; wait for the timeout */

	JS_EXIST = inb (JS_PORT) & 0x0f; /* get joystick status byte */

	JS_EXIST = (~JS_EXIST) & 0x0f;

	printk("find_axes: JS_EXIST is %d (0x%04X)\n", JS_EXIST, JS_EXIST);

	return JS_EXIST;
}

static int js_ioctl (struct inode *inode,
		     struct file *file,
		     unsigned int cmd,
		     unsigned long arg)
{
	unsigned int minor, i, save_busy;
	char *c;
	minor = MINOR (inode->i_rdev);
	if (MAJOR (inode->i_rdev) != JOYSTICK_MAJOR)
		return -EINVAL;
	if (minor >= JS_MAX)
		return -ENODEV;
	if ((((inb (JS_PORT) & 0x0f) >> (minor << 1)) & 0x03) == 0x03)	/*js minor exists?*/
		return -ENODEV;
	switch (cmd) {
	case JS_SET_CAL:	/*from struct *arg to JS_DATA[minor]*/
		verify_area (VERIFY_READ, (void *) arg,
			     sizeof (struct JS_DATA_TYPE));
		c = (char *) &JS_DATA[minor].JS_CORR;
		for (i = 0; i < sizeof (struct JS_DATA_TYPE); i++)
			*c++ = get_fs_byte ((char *) arg++);
		break;
	case JS_GET_CAL:	/*to struct *arg from JS_DATA[minor]*/
		verify_area (VERIFY_WRITE, (void *) arg,
			     sizeof (struct JS_DATA_TYPE));
		c = (char *) &JS_DATA[minor].JS_CORR;
		for (i = 0; i < sizeof (struct JS_DATA_TYPE); i++)
			put_fs_byte (*c++, (char *) arg++);
		break;
	case JS_SET_TIMEOUT:
		verify_area (VERIFY_READ, (void *) arg,
			     sizeof (JS_DATA[0].JS_TIMEOUT));
		c = (char *) &JS_DATA[minor].JS_TIMEOUT;
		for (i = 0; i < sizeof (JS_DATA[0].JS_TIMEOUT); i++)
			*c++ = get_fs_byte ((char *) arg++);
		break;
	case JS_GET_TIMEOUT:
		verify_area (VERIFY_WRITE, (void *) arg,
			     sizeof (JS_DATA[0].JS_TIMEOUT));
		c = (char *) &JS_DATA[minor].JS_TIMEOUT;
		for (i = 0; i < sizeof (JS_DATA[0].JS_TIMEOUT); i++)
			put_fs_byte (*c++, (char *) arg++);
		break;
	case JS_SET_TIMELIMIT:
		verify_area (VERIFY_READ, (void *) arg,
			     sizeof (JS_DATA[0].JS_TIMELIMIT));
		c = (char *) &JS_DATA[minor].JS_TIMELIMIT;
		for (i = 0; i < sizeof (JS_DATA[0].JS_TIMELIMIT); i++)
			*c++ = get_fs_byte ((char *) arg++);
		break;
	case JS_GET_TIMELIMIT:
		verify_area (VERIFY_WRITE, (void *) arg,
			     sizeof (JS_DATA[0].JS_TIMELIMIT));
		c = (char *) &JS_DATA[minor].JS_TIMELIMIT;
		for (i = 0; i < sizeof (JS_DATA[0].JS_TIMELIMIT); i++)
			put_fs_byte (*c++, (char *) arg++);
		break;
	case JS_GET_ALL:
		verify_area (VERIFY_WRITE, (void *) arg,
			     sizeof (struct JS_DATA_SAVE_TYPE));
		c = (char *) &JS_DATA[minor];
		for (i = 0; i < sizeof (struct JS_DATA_SAVE_TYPE); i++)
			put_fs_byte (*c++, (char *) arg++);
		break;
	case JS_SET_ALL:
		verify_area (VERIFY_READ, (void *) arg,
			     sizeof (struct JS_DATA_SAVE_TYPE));
		save_busy = JS_DATA[minor].BUSY;
		c = (char *) &JS_DATA[minor];
		for (i = 0; i < sizeof (struct JS_DATA_SAVE_TYPE); i++)
			*c++ = get_fs_byte ((char *) arg++);
		JS_DATA[minor].BUSY = save_busy;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/* js_open():
   device open routine. increments module usage count, initializes
   data for that joystick.

   returns: 0 or
   -ENODEV: asked for joystick other than #0 or #1
   -ENODEV: asked for joystick on axis where there is none
   -EBUSY: attempt to open joystick already open
*/
static int js_open (struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR (inode->i_rdev);
	int j;

	if (minor >= JS_MAX)
		return -ENODEV;	/*check for joysticks*/

	for (j = JS_DEF_TIMEOUT; (JS_EXIST & inb (JS_PORT)) && j; j--);
	cli ();			/*block js_read while JS_EXIST is being modified*/
	/*js minor exists?*/
	if ((((JS_EXIST = inb (JS_PORT)) >> (minor << 1)) & 0x03) == 0x03) {
		JS_EXIST = (~JS_EXIST) & 0x0f;
		sti ();
		return -ENODEV;
	}
	JS_EXIST = (~JS_EXIST) & 0x0f;
	sti ();

	if (JS_DATA[minor].BUSY)
		return -EBUSY;
	JS_DATA[minor].BUSY = JS_TRUE;
	JS_DATA[minor].JS_CORR.x = JS_DEF_CORR;	/*default scale*/
	JS_DATA[minor].JS_CORR.y = JS_DEF_CORR;
	JS_DATA[minor].JS_TIMEOUT = JS_DEF_TIMEOUT;
	JS_DATA[minor].JS_TIMELIMIT = JS_DEF_TIMELIMIT;
	JS_DATA[minor].JS_EXPIRETIME = CURRENT_JIFFIES;

	MOD_INC_USE_COUNT;
	return 0;
}

static void js_release (struct inode *inode, struct file *file)
{
	unsigned int minor = MINOR (inode->i_rdev);
	inode->i_atime = CURRENT_TIME;
	JS_DATA[minor].BUSY = JS_FALSE;
	MOD_DEC_USE_COUNT;
}

/* js_read() reads the buttons x, and y axis from both joysticks if a
 * given interval has expired since the last read or is equal to
 * -1l. The buttons are in port 0x201 in the high nibble. The axis are
 * read by writing to 0x201 and then measuring the time it takes the
 * one shots to clear.
 */

static int js_read (struct inode *inode, struct file *file, char *buf, int count)
{
	int j, chk, jsmask;
	int t0, t_x0, t_y0, t_x1, t_y1;
	char *c;
	unsigned int minor, minor2;
	int buttons;

	if (count != JS_RETURN)
		return -EOVERFLOW;
	verify_area (VERIFY_WRITE, (void *) buf, sizeof (struct JS_DATA_TYPE));
	minor = MINOR (inode->i_rdev);
	inode->i_atime = CURRENT_TIME;
	if (CURRENT_JIFFIES >= JS_DATA[minor].JS_EXPIRETIME) {
		minor2 = minor << 1;
		j = JS_DATA[minor].JS_TIMEOUT;
		for (; (JS_EXIST & inb (JS_PORT)) && j; j--);
		if (j == 0)
			return -ENODEV;	/*no joystick here*/
		while (1) {	/*Make sure no other proc is using port*/
			cli ();
			if (!JS_READ_SEMAPHORE) {
				JS_READ_SEMAPHORE++;
				sti ();
				break;
			}
			sti ();
		}
		buttons = ~(inb (JS_PORT) >> 4);
		JS_DATA[0].JS_SAVE.buttons = buttons & 0x03;
		JS_DATA[1].JS_SAVE.buttons = (buttons >> 2) & 0x03;
		j = JS_DATA[minor].JS_TIMEOUT;
		jsmask = 0;

		cli ();		/*no interrupts!*/
		outb (0xff, JS_PORT);	/*trigger one-shots*/
		/*get init timestamp*/
		t_x0 = t_y0 = t_x1 = t_y1 = t0 = get_timer0 ();
		/*wait for an axis' bit to clear or timeout*/
		while (j-- && (chk = (inb (JS_PORT) & JS_EXIST ) | jsmask)) {
			if (!(chk & JS_X_0)) {
				t_x0 = get_timer0();
				jsmask |= JS_X_0;
			}
			if (!(chk & JS_Y_0)) {
				t_y0 = get_timer0();
				jsmask |= JS_Y_0;
			}
			if (!(chk & JS_X_1)) {
				t_x1 = get_timer0();
				jsmask |= JS_X_1;
			}
			if (!(chk & JS_Y_1)) {
				t_y1 = get_timer0();
				jsmask |= JS_Y_1;
			}
		}
		sti ();					/* allow interrupts */

		JS_READ_SEMAPHORE = 0;	/* allow other reads to progress */
		if (j == 0)
			return -ENODEV;	/*read timed out*/
		JS_DATA[0].JS_EXPIRETIME = CURRENT_JIFFIES +
			JS_DATA[0].JS_TIMELIMIT;	/*update data*/
		JS_DATA[1].JS_EXPIRETIME = CURRENT_JIFFIES +
			JS_DATA[1].JS_TIMELIMIT;
		JS_DATA[0].JS_SAVE.x = DELTA_TIME (t0, t_x0) >>
			JS_DATA[0].JS_CORR.x;
		JS_DATA[0].JS_SAVE.y = DELTA_TIME (t0, t_y0) >>
			JS_DATA[0].JS_CORR.y;
		JS_DATA[1].JS_SAVE.x = DELTA_TIME (t0, t_x1) >>
			JS_DATA[1].JS_CORR.x;
		JS_DATA[1].JS_SAVE.y = DELTA_TIME (t0, t_y1) >>
			JS_DATA[1].JS_CORR.y;
	}

	for (c = (char *) &JS_DATA[minor].JS_SAVE, j = 0; j < JS_RETURN; j++)
		put_fs_byte (*c++, buf++);	/*copy to user space*/
	return JS_RETURN;
}


static struct file_operations js_fops =
{
	NULL,			/* js_lseek*/
	js_read,		/* js_read */
	NULL,			/* js_write*/
	NULL,			/* js_readaddr*/
	NULL,			/* js_select */
	js_ioctl,		/* js_ioctl*/
	NULL,			/* js_mmap */
	js_open,		/* js_open*/
	js_release,		/* js_release*/
	NULL			/* js_sync */
};

#ifdef MODULE

#define joystick_init init_module

void cleanup_module (void)
{
	if (unregister_chrdev (JOYSTICK_MAJOR, "joystick"))
		printk ("joystick: cleanup_module failed\n");
	release_region(JS_PORT, 1);
}

#endif /* MODULE */

int joystick_init(void)
{
	int js_num;
	int js_count;

	if (check_region(JS_PORT, 1)) {
		printk("js_init: port already in use\n");
		return -EBUSY;
	}

	js_num = find_axes();
	js_count = !!(js_num & 0x3) + !!(js_num & 0xC),

	printk ("js_init: found %d joystick%c.\n",
			js_count,
		    (js_num == 1) ? ' ' : 's');

	if (js_count == 0) {
		printk("No joysticks found.\n");
		return -ENODEV;
		/* if the user boots the machine, which runs insmod, and THEN
		   decides to hook up the joystick, well, then we do the wrong
		   thing. But it's a good idea to avoid giving out a false sense
		   of security by letting the module load otherwise. */
	}

	if (register_chrdev (JOYSTICK_MAJOR, "joystick", &js_fops)) {
		printk ("Unable to get major=%d for joystick\n",
					JOYSTICK_MAJOR);
		return -EBUSY;
	}
	request_region(JS_PORT, 1, "joystick");
		
	for (js_num = 0; js_num < JS_MAX; js_num++)
		  JS_DATA[js_num].BUSY = JS_FALSE;
	JS_READ_SEMAPHORE = 0;
	return 0;
}
