/*
	fbv  --  simple image viewer for the linux framebuffer
	Copyright (C) 2000, 2001, 2003, 2004  Mateusz 'mteg' Golicz

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
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

#include <pic_app.h>
#include <pic_uop.h>
#include <vo.h>

#include "filelistop.h"

int opt_clear = 1,
	   opt_alpha = 0,
	   opt_hide_cursor = 1,
	   opt_image_info = 1,
	   opt_stretch = 0,
	   opt_delay = 0,
	   opt_enlarge = 0,
	   opt_ignore_aspect = 0;

extern int show_image(char *filename,video_out_t* vo);
extern void sighandler(int s);
extern void setup_console(int t);

void help(char *name)
{
	printf("Usage: %s [options] image1 image2 image3 ...\n\n"
		   "Available options:\n"
		   " --help        | -h : Show this help\n"
		   " --alpha       | -a : Use the alpha channel (if applicable)\n"
		   " --dontclear   | -c : Do not clear the screen before and after displaying the image\n"
		   " --donthide    | -u : Do not hide the cursor before and after displaying the image\n"
		   " --noinfo      | -i : Supress image information\n"
		   " --stretch     | -f : Strech (using a simple resizing routine) the image to fit onto screen if necessary\n"
		   " --colorstretch| -k : Strech (using a 'color average' resizing routine) the image to fit onto screen if necessary\n"
		   " --enlarge     | -e : Enlarge the image to fit the whole screen if necessary\n"
		   " --ignore-aspect| -r : Ignore the image aspect while resizing\n"
           " --delay <d>   | -s <delay> : Slideshow, 'delay' is the slideshow delay in tenths of seconds.\n\n"
		   " --play_folder <dir> | -d <dir> : slideshow a folder.   "
		   "Keys:\n"
		   " r            : Redraw the image\n"
		   " a, d, w, x   : Pan the image\n"
		   " f            : Toggle resizing on/off\n"
		   " k            : Toggle resizing quality\n"
		   " e            : Toggle enlarging on/off\n"
		   " i            : Toggle respecting the image aspect on/off\n"
		   " n            : Rotate the image 90 degrees left\n"
		   " m            : Rotate the image 90 degrees right\n"
		   " p            : Disable all transformations\n"
		   "amlogic.\n", name);
}

int main(int argc, char **argv)
{
	static struct option long_options[] =
	{
		{"help",			no_argument,		0, 'h'},
		{"noclear",		 	no_argument, 		0, 'c'},
		{"alpha", 			no_argument, 		0, 'a'},
		{"unhide",  		no_argument, 		0, 'u'},
		{"noinfo",  		no_argument, 		0, 'i'},
		{"stretch", 		no_argument, 		0, 'f'},
		{"colorstrech", 	no_argument, 		0, 'k'},
		{"delay", 			required_argument, 	0, 's'},
		{"enlarge",			no_argument,		0, 'e'},
		{"ignore-aspect", 	no_argument,		0, 'r'},
		{"play_folder",		required_argument, 	0, 'd'},
		{0, 0, 0, 0}
	};
	int c, i;
	char* filelist=NULL;
	video_out_t vo={0};
	vo.name="gles";
	
	if(argc < 2)
	{
		help(argv[0]);
		fprintf(stderr, "Error: Required argument missing.\n");
		return(1);
	}
	
	while((c = getopt_long_only(argc, argv, "hcauifks:er", long_options, NULL)) != EOF)
	{
		switch(c)
		{
			case 'a':
				opt_alpha = 1;
				break;
			case 'c':
				opt_clear = 0;
				break;
			case 's':
				opt_delay = atoi(optarg);
				break;
			case 'u':
				opt_hide_cursor = 0;
				break;
			case 'h':
				help(argv[0]);
				return(0);
			case 'i':
				opt_image_info = 0;
				break;
			case 'f':
				opt_stretch = 1;
				break;
			case 'k':
				opt_stretch = 2;
				break;
			case 'e':
				opt_enlarge = 1;
				break;
			case 'r':
				opt_ignore_aspect = 1;
				break;
			case 'd':
				filelist = create_filelist(optarg);
				break;
		}
	}
	
#if 0	
	if(!argv[optind])
	{
		fprintf(stderr, "Required argument missing! Consult %s -h.\n", argv[0]);
		return(1);
	}
#endif

	if(vo_cfg(&vo)!=VO_ERROR_OK) {
		printf("video out device invalid\n");
		exit(1);
	}
	vo_preinit(&vo);
	
#ifndef WIN32
	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGSEGV, sighandler);
	signal(SIGTERM, sighandler);
	signal(SIGABRT, sighandler);
	
	if(opt_hide_cursor)
	{
		printf("\033[?25l");
		fflush(stdout);
	}
	
	setup_console(1);
#endif

	if(filelist) {
		filelist_t* pre=NULL;
		char* filename;
		while(enum_file(filelist,&pre,&filename)!=-1) {
			if(filename&&strlen(filename)>3) 
				show_image(filename,&vo);
		}
		free_filelist(filelist);
	} else {
		for(i = optind; argv[i]; )
		{
			int r = show_image(argv[i],&vo);
		
			if(!r) break;
			
			i += r;
			if(i < optind)
				i = optind;
		}
	}

	setup_console(0);

	if(opt_hide_cursor)
	{
		printf("\033[?25h");
		fflush(stdout);
	}
	
	vo_uninit(&vo);
	return(0);	
}
