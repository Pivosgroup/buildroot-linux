/*
    ampicplayer

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
#include <aml_common.h>
#define FH_ERROR_OK 0
#define FH_ERROR_FILE 1		/* read/access error */
#define FH_ERROR_FORMAT 2	/* file format error */
#define FH_ERROR_OTHER 3

typedef struct s_image_t
{
	int width, height;
	unsigned char *rgb;
	unsigned char *alpha;
	int do_free;
	char* fn;
	
	unsigned pic_type;		/* pointer to load function. */
}image_t;

extern int get_pic_info(aml_dec_para_t* para);
extern int load_pic(aml_dec_para_t* para , aml_image_info_t* image);
extern int show_pic(aml_image_info_t* image,int flag);
