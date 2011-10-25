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

#define PAN_STEPPING 20
#include <pic_app.h>
#include <pic_uop.h>
#include <vo.h>

extern int opt_clear,opt_alpha,opt_image_info,opt_stretch,opt_delay,opt_enlarge,opt_ignore_aspect;

int show_image(char *filename,video_out_t* vo)
{
	unsigned char * image = NULL;
	unsigned char * alpha = NULL;
	
	int x_size, y_size, screen_width, screen_height;
	int x_pan, y_pan, x_offs, y_offs, refresh = 1, c, ret = 1;
	int delay = opt_delay, retransform = 1;
	
	int transform_stretch = opt_stretch, transform_enlarge = opt_enlarge, transform_cal = (opt_stretch == 2),
	    transform_iaspect = opt_ignore_aspect, transform_rotation = 0;
	
	aml_image_info_t i={0};
	aml_dec_para_t para = {0};
	para.fn=filename;


	if(get_pic_info(&para) != FH_ERROR_OK) {
		fprintf(stderr, "%s: Unable to access file or file format unknown.\n", filename);
		return(1);
	}

	if(!(image = (unsigned char*) malloc(i.width * i.height * 3)))
	{
		fprintf(stderr, "%s: Out of memory.\n", filename);
		goto error_mem;
	}
	
//	i.rgb = image;
	if(load_pic(&para , &i) != FH_ERROR_OK)
	{
		fprintf(stderr, "%s: Image data is corrupt or image unsupported?\n", filename);
		goto error_mem;
	}
	
//	alpha=i.alpha;
	if(!opt_alpha)
	{
		free(alpha);
		alpha = NULL;
	}
	
	vo_getCurrentRes(vo,&screen_width, &screen_height);
	i.do_free = 0;
#if 0	
	while(1)
	{
		if(retransform)
		{
			if(i.do_free)
			{
				free(i.rgb);
				free(i.alpha);
			}
			
			i.rgb = image;
			i.alpha = alpha;
			i.do_free = 0;
	
	
			if(transform_rotation)
				do_rotate(&i, transform_rotation);
				
			if(transform_stretch)
				do_fit_to_screen(&i, screen_width, screen_height, transform_iaspect, transform_cal);
	
			if(transform_enlarge)
				do_enlarge(&i, screen_width, screen_height, transform_iaspect);

			x_pan = y_pan = 0;
			refresh = 1; retransform = 0;
			if(opt_clear)
			{
				printf("\033[H\033[J");
				fflush(stdout);
			}
			if(opt_image_info)
				printf("ampicplayer - \n%s\n%d x %d\n", filename, x_size, y_size); 
		}
		if(refresh)
		{
			if(i.width < screen_width)
				x_offs = (screen_width - i.width) / 2;
			else
				x_offs = 0;
			
			if(i.height < screen_height)
				y_offs = (screen_height - i.height) / 2;
			else
				y_offs = 0;
			
			vo_display(vo, &i, x_pan, y_pan, x_offs, y_offs);
			refresh = 0;
		}
#ifdef WIN32
		if(delay) usleep(delay*100);
		delay=0;
		break;
#else
		if(delay)
		{
			struct timeval tv;
			fd_set fds;
			tv.tv_sec = delay / 10;
			tv.tv_usec = (delay % 10) * 100000;
			FD_ZERO(&fds);
			FD_SET(0, &fds);
			
			if(select(1, &fds, NULL, NULL, &tv) <= 0)
				break;
			delay = 0;
		}	
#endif		
		c = getchar();
		switch(c)
		{
			case EOF:
			case 'q':
				ret = 0;
				goto done;
			case ' ': case 10: case 13: 
				goto done;
			case '>': case '.':
				goto done;
			case '<': case ',':
				ret = -1;
				goto done;
			case 'r':
				refresh = 1;
				break;
			case 'a': case 'D':
				if(x_pan == 0) break;
				x_pan -= i.width / PAN_STEPPING;
				if(x_pan < 0) x_pan = 0;
				refresh = 1;
				break;
			case 'd': case 'C':
				if(x_offs) break;
				if(x_pan >= (i.width - screen_width)) break;
				x_pan += i.width / PAN_STEPPING;
				if(x_pan > (i.width - screen_width)) x_pan = i.width - screen_width;
				refresh = 1;
				break;
			case 'w': case 'A':
				if(y_pan == 0) break;
				y_pan -= i.height / PAN_STEPPING;
				if(y_pan < 0) y_pan = 0;
				refresh = 1;
				break;
			case 'x': case 'B':
				if(y_offs) break;
				if(y_pan >= (i.height - screen_height)) break;
				y_pan += i.height / PAN_STEPPING;
				if(y_pan > (i.height - screen_height)) y_pan = i.height - screen_height;
				refresh = 1;
				break;
			case 'f': 
				transform_stretch = !transform_stretch;
				retransform = 1;
				break;
			case 'e':
				transform_enlarge = !transform_enlarge;
				retransform = 1;
				break;
			case 'k':
				transform_cal = !transform_cal;
				retransform = 1;
				break;
			case 'i':
				transform_iaspect = !transform_iaspect;
				retransform = 1;
				break;
			case 'p':
				transform_cal = 0;
				transform_iaspect = 0;
				transform_enlarge = 0;
				transform_stretch = 0;
				retransform = 1;
				break;
			case 'n':
				transform_rotation -= 1;
				if(transform_rotation < 0)
					transform_rotation += 4;
				retransform = 1;
				break;
			case 'm':
				transform_rotation += 1;
				if(transform_rotation > 3)
					transform_rotation -= 4;
				retransform = 1;
				break;	
		}
	}
#endif
done:
	if(opt_clear)
	{
		printf("\033[H\033[J");
		fflush(stdout);
	}
	
error_mem:
	free(image);
	free(alpha);
	if(i.do_free)
	{
		free(i.rgb);
		free(i.alpha);
	}
	return(ret);

}

