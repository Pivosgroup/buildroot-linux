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
#include <config.h>
#include <pic_app.h>
#include <pic_load_impl.h>

/**
*
* function to get thep information of picture.
*
**/
int get_pic_info(image_t* image) {
	int i;
	
	/* check validility. */
	if(!image) return FH_ERROR_OTHER;
	if(configure_number_of_decoder<=1) return FH_ERROR_FORMAT;
	
	/* try to detect the picture type. */
	for(i=1;i<configure_number_of_decoder;i++) {
		if(decoder_funs[i].fd_id(image->fn)) {
			if(decoder_funs[i].fh_getsize(image->fn, &image->width, &image->height)
					== FH_ERROR_OK) {
				image->pic_type=i;
				return FH_ERROR_OK;
			}
		}
	}

	return FH_ERROR_FORMAT;
}

/**
*
* function to load a picture to pbuff.
*
**/
int load_pic(image_t* image) {
	if(image->pic_type<1 || image->pic_type>=configure_number_of_decoder)
		return FH_ERROR_FORMAT;
	return decoder_funs[image->pic_type].fh_load(image->fn, image->rgb, &image->alpha, image->width, image->height);
}