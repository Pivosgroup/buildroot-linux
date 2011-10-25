/*
    ampicplayer.
    Copyright (C) 2010 amlogic.

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

/*
 * Preinitializes driver (real INITIALIZATION)
 *   arg - currently it's vo_subdevice
 *   returns: zero on successful initialization, non-zero on error.
 */
typedef void* (*preinit_t)(char *arg);
typedef void (*display_pic_t)(void* arg,unsigned char *argbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
typedef void (*getCurrentRes_t)(int *x, int *y);
typedef void (*uninit_t)(const char *arg); /* Closes driver. Should restore the original state of the system.*/
/* ======================current vo device================================ */

/* frame buffer. */
void fb_display(void* arg,unsigned char *argbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
void fb_getCurrentRes(int *x, int *y);

/* gles */
void*  gles_preinit(void* arg);
void gles_display(void* arg,unsigned char *argbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs);
void gles_getCurrentRes(int *x, int *y);
void gles_uninit(void* *arg);
/* ==============================end================================ */

typedef struct s_video_out_fun_t {
	char*				name;	/* driver name ("fb") */
	preinit_t			preinit;
	display_pic_t		display;
	getCurrentRes_t		getCurrentRes;
	uninit_t			uninit;
} video_out_fun_t;

video_out_fun_t video_out_funs[] = {
	{(char*)0,(preinit_t)0,(display_pic_t)0,(getCurrentRes_t)0,(uninit_t)0},
	
#ifdef USE_FB
	{"fb",(preinit_t)0,fb_display,fb_getCurrentRes,(uninit_t)0},
#endif

#ifdef USE_GLES
	{"gles",gles_preinit,gles_display,gles_getCurrentRes,gles_uninit},
#endif

};

const int configure_number_of_vo = \
  (sizeof(video_out_funs) / sizeof(video_out_fun_t));

