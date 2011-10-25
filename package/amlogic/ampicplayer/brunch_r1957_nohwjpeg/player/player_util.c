#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <termios.h>
#include <signal.h>
#endif
#include <config.h>

extern int opt_hide_cursor;

void setup_console(int t)
{
#ifdef WIN32

#else
	struct termios our_termios;
	static struct termios old_termios;

	if(t)
	{
		tcgetattr(0, &old_termios);
		memcpy(&our_termios, &old_termios, sizeof(struct termios));
		our_termios.c_lflag &= !(ECHO | ICANON);
		tcsetattr(0, TCSANOW, &our_termios);
	}
	else
		tcsetattr(0, TCSANOW, &old_termios);
#endif
}

#ifndef WIN32
void sighandler(int s)
{
	if(opt_hide_cursor)
	{
		printf("\033[?25h");
		fflush(stdout);
	}
	setup_console(0);
	_exit(128 + s);
	
}
#endif