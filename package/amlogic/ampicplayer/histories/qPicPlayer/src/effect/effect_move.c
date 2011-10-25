/****************************************************
*
* transition effect move left/right.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>


static int effect_delay=(1000);
static int effect_step=80;
int effect_move_l(screen_area_t* src,screen_area_t* dst) {
	int xstep,show_width;
	int i,j;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	screen_area_t fill_rect;
	if(init_ge2d()==-1) return -1;

	xstep=src->width/effect_step;
	show_width=xstep;

	src_scr.y=src->y;
	src_scr.height=src->height;
	dst_scr.y=dst->y;
	dst_scr.height=dst->height;
	for(i=0;i<dst->width;i+=xstep)
	{
		for(j=0;j<dst->width-i;j+=xstep) {
			src_scr.width=xstep;
			src_scr.x=dst->x+xstep+j;
			dst_scr.width=src_scr.width;
			dst_scr.x=dst->x+j;
			fb_bitbld(&src_scr,&dst_scr);
		}
		src_scr.width=i+xstep;
		src_scr.x=src->x;
		dst_scr.width=src_scr.width;
		dst_scr.x=dst->x+dst->width-src_scr.width;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(effect_delay);
    } /* loop end. */

	uninit_ge2d();
	return 0;
}
int effect_move_r(screen_area_t* src,screen_area_t* dst) {
	int xstep,show_width;
	int i,j;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	screen_area_t fill_rect;
	if(init_ge2d()==-1) return -1;

	xstep=src->width/effect_step;
	show_width=xstep;

	src_scr.y=src->y;
	src_scr.height=src->height;
	dst_scr.y=dst->y;
	dst_scr.height=dst->height;
	for(i=xstep;i<dst->width;i+=xstep)
	{
		for(j=dst->width-xstep;j>=i-xstep;j-=xstep) {
			src_scr.width=xstep;
			src_scr.x=dst->x-xstep+j;
			dst_scr.width=src_scr.width;
			dst_scr.x=dst->x+j;
			fb_bitbld(&src_scr,&dst_scr);
		}
		src_scr.width=i+xstep;
		src_scr.x=src->x+src->width-src_scr.width;
		dst_scr.width=src_scr.width;
		dst_scr.x=dst->x;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(effect_delay);
    } /* loop end. */

	uninit_ge2d();
	return 0;
}
int effect_move(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	if((extra&1)==0) {  /* r2l. */
		effect_move_l(src,dst);
	} else { /* from right top. */
		effect_move_r(src,dst);
	}
	uninit_ge2d();
	return 0;
}