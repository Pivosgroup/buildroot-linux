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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <config.h>
#include <pic_app.h>
#include <vo.h>
#include <vo_impl.h>

int vo_cfg(video_out_t* vo) {
	int i=1;
	if(configure_number_of_vo<=1) { 
		vo->vo_errno=VO_ERROR_DEV_INVALID;
		return VO_ERROR_DEV_INVALID;
	}
	if(!vo->name) vo->name=DEFAULT_VO;
	for(;i<configure_number_of_vo;i++) {
		if(!strcmp(vo->name,video_out_funs[i].name)) {
			vo->vo_handle=i;
			vo->vo_errno=VO_ERROR_OK;
			return VO_ERROR_OK;
		}
	}
	vo->vo_errno=VO_ERROR_DEV_INVALID;
	return VO_ERROR_DEV_INVALID;
}

int vo_preinit(video_out_t* vo) {
	if(configure_number_of_vo<=1) {
		vo->vo_errno=VO_ERROR_DEV_INVALID;
		return VO_ERROR_DEV_INVALID;
	}
	if(!vo->vo_handle) {
		vo->vo_errno=VO_ERROR_NOT_CONFIGED;
		return VO_ERROR_NOT_CONFIGED;
	}
	if(video_out_funs[vo->vo_handle].preinit) {
		vo->arg=video_out_funs[vo->vo_handle].preinit(vo->arg);
		if(vo->arg)
			vo->vo_errno=VO_ERROR_INIT_FAIL;
		else
			vo->vo_errno=VO_ERROR_OK;
	}
	return vo->vo_errno;
}

void vo_display(video_out_t* vo,image_t* img,int x_pan, int y_pan, int x_offs, int y_offs) {
	if(configure_number_of_vo<=1) {
		vo->vo_errno=VO_ERROR_DEV_INVALID;
		return VO_ERROR_DEV_INVALID;
	}
	if(!vo->vo_handle) {
		vo->vo_errno=VO_ERROR_NOT_CONFIGED;
		return VO_ERROR_NOT_CONFIGED;
	}
	if(video_out_funs[vo->vo_handle].display) 
		video_out_funs[vo->vo_handle].display(vo->arg,img->rgb, img->alpha, img->width, img->height, x_pan, y_pan, x_offs, y_offs);
}

void vo_getCurrentRes(video_out_t* vo,int *x, int *y) {
	if(configure_number_of_vo<=1) {
		vo->vo_errno=VO_ERROR_DEV_INVALID;
		return VO_ERROR_DEV_INVALID;
	}
	if(!vo->vo_handle) {
		vo->vo_errno=VO_ERROR_NOT_CONFIGED;
		return VO_ERROR_NOT_CONFIGED;
	}
	if(video_out_funs[vo->vo_handle].getCurrentRes) 
		video_out_funs[vo->vo_handle].getCurrentRes(x,y);
}

void vo_uninit(video_out_t* vo){
	if(configure_number_of_vo<=1) {
		vo->vo_errno=VO_ERROR_DEV_INVALID;
		return VO_ERROR_DEV_INVALID;
	}
	if(!vo->vo_handle) {
		vo->vo_errno=VO_ERROR_NOT_CONFIGED;
		return VO_ERROR_NOT_CONFIGED;
	}
	if(video_out_funs[vo->vo_handle].uninit) 
		video_out_funs[vo->vo_handle].uninit(vo->arg);
}
