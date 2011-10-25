/****************************************************
*
* utilities for transition effect.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>

int copy_area(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	init_ge2d();
	fb_bitbld(src,dst);
	uninit_ge2d();
}