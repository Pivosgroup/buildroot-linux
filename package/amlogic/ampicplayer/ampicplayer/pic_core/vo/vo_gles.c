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
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <config.h>
#include "gles/gles_api.h"

int gles_preinit(const char *arg) {
	local_init(arg);
}

void gles_display(const char *arg,unsigned char *rgbbuff, unsigned char * alpha, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs) {
	local_display(arg,rgbbuff,x_size,y_size,x_pan,y_pan,x_offs,y_offs);
}

void gles_getCurrentRes(int *x, int *y) {
	local_getCurrentRes(NULL,x,y);
}

void gles_uninit(const char *arg) {
	local_release(arg);
}