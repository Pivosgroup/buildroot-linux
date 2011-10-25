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

/* decoder function of kinds of picture type. */
int fh_bmp_id(char *name);
int fh_bmp_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_bmp_getsize(char *name,int *x,int *y);

int fh_jpeg_id(char *name);
int fh_jpeg_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_jpeg_getsize(char *name,int *x,int *y);

int fh_png_id(char *name);
int fh_png_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_png_getsize(char *name,int *x,int *y);

int fh_gif_id(char *name);
int fh_gif_load(char *name,unsigned char *buffer, unsigned char **alpha, int x,int y);
int fh_gif_getsize(char *name,int *x,int *y);

#define CONFIGURE_MAXIMUM_DECODER   50

typedef int (*fh_id_t)(char*);
typedef int (*fh_load_t)(char*,unsigned char*, unsigned char**, int x,int y);
typedef int (*fh_getinfo_t)(char*,int*,int*);

typedef struct s_decoder_fun_t {
	char*			name;
	fh_id_t 		fd_id;
	fh_load_t		fh_load;
	fh_getinfo_t	fh_getsize;
}decoder_fun_t;

decoder_fun_t decoder_funs[] = {
	{(char*)0,(fh_id_t)0,(fh_load_t)0,(fh_getinfo_t)0},
	
#ifdef USE_JPEG
	{"jpeg",fh_jpeg_id,fh_jpeg_load,fh_jpeg_getsize},
#endif

#ifdef USE_PNG
	{"png",fh_png_id,fh_png_load,fh_png_getsize},
#endif

#ifdef USE_BMP
	{"bmp",fh_bmp_id,fh_bmp_load,fh_bmp_getsize},
#endif

#ifdef USE_GIF
	{"gif",fh_gif_id,fh_gif_load,fh_gif_getsize},
#endif
};

const configure_number_of_decoder =
  (sizeof(decoder_funs) / sizeof(decoder_fun_t));

