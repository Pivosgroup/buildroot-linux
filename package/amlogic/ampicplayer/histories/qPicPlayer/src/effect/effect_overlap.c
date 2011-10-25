/****************************************************
*
* transition effect of moving.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>


static overlap_step=10;
static overlap_delay=(1000*10);
static int effect_over_l2r(screen_area_t* src,screen_area_t* dst) {
	int i;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	init_ge2d();
	for(i=overlap_step;i<dst->width;i+=overlap_step) {
		src_scr.width=i;
		dst_scr.width=i;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(overlap_delay);
	}	
	uninit_ge2d();
}
static int effect_over_r2l(screen_area_t* src,screen_area_t* dst) {
	int i;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	init_ge2d();
	for(i=overlap_step;i<dst->width;i+=overlap_step) {
                src_scr.x=src->x+src->width-i;
                dst_scr.x=dst->x+dst->width-i;
		src_scr.width=i;
		dst_scr.width=i;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(overlap_delay);
	}	
	uninit_ge2d();
}
static int effect_over_t2b(screen_area_t* src,screen_area_t* dst) {
	int i;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	init_ge2d();
	for(i=overlap_step;i<dst->height;i+=overlap_step) {
		src_scr.height=i;
		dst_scr.height=i;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(overlap_delay);
	}	
	uninit_ge2d();
}
static int effect_over_b2t(screen_area_t* src,screen_area_t* dst) {
	int i;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	init_ge2d();
	for(i=overlap_step;i<dst->height;i+=overlap_step) {
		src_scr.y=dst->height-i;
		dst_scr.y=src_scr.y;
		src_scr.height=i;
		dst_scr.height=i;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(overlap_delay);
	}	
	uninit_ge2d();
}
int effect_over(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	switch(extra) {
		case 0:
		effect_over_l2r(src,dst);
		break;
		case 1:
		effect_over_r2l(src,dst);
		break;
		case 2:
		effect_over_t2b(src,dst);
		break;
		default:
		effect_over_b2t(src,dst);
		break;
	}
}
