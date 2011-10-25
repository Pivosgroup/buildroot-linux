/****************************************************
*
* transition effect of moving.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>


static blend_step=2;
static blend_delay=(1000*10);
static int effect_blend_565(screen_area_t* src,screen_area_t* dst) {
	int i,const_color;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	if(init_ge2d()==-1) return -1;
	for(i=0;i<=255;i+=blend_step) {
		//const_color=(i&0xff)|((i>>3<<3)<<8)|((i>>2<<2)<<16)|((i>>3<<3)<<24);
		const_color=(i&0xff)|((i)<<8)|((i)<<16)|((i)<<24);
		fb_blend(&src_scr,&dst_scr,&dst_scr,const_color);
		usleep(blend_delay);
	}	
	//fb_bitbld(&src_scr,&dst_scr);
	uninit_ge2d();
	return 0;
}
static int effect_blend_888(screen_area_t* src,screen_area_t* dst) {
	int i,const_color;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	if(init_ge2d()==-1) return -1;
	for(i=0;i<=255;i+=blend_step) {
		const_color=(i)|(i<<8)|(i<<16)|(i<<24);
		fb_blend(&src_scr,&dst_scr,&dst_scr,blend_step);
		usleep(blend_delay);
	}	
	fb_bitbld(&src_scr,&dst_scr);
	uninit_ge2d();
	return 0;
}
int effect_blend(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	if(extra==0)
		effect_blend_565(src,dst);
	else 
		effect_blend_888(src,dst);
}
