/****************************************************
*
* transition effect boxin.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>


static int effect_delay=(1000*10);
static int effect_step=80;

int effect_cornercover(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	int xstep,ystep,show_width,show_height;
	int i;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	screen_area_t fill_rect;
	if(init_ge2d()==-1) return -1;

	xstep=src->width/effect_step;
	ystep=src->height/effect_step;
	show_width=xstep;
	show_height=ystep;
    for(i=0;i<effect_step;i++)
	{
		if(extra==0) {  /* from left top. */
			src_scr.width=show_width;
			src_scr.height=show_height;
			src_scr.x=src->x+src->width-show_width;
			src_scr.y=src->y+src->height-show_height;

			dst_scr.width=show_width;
			dst_scr.height=show_height;
			dst_scr.x=dst->x;
			dst_scr.y=dst->y;
		} else if(extra==1) { /* from right top. */
			src_scr.width=show_width;
			src_scr.height=show_height;
			src_scr.x=src->x;
			src_scr.y=src->y+src->height-show_height;

			dst_scr.width=show_width;
			dst_scr.height=show_height;
			dst_scr.x=dst->x+dst->width-show_width;
			dst_scr.y=dst->y;
		}else if(extra==2) { /* from left bottom */
			src_scr.width=show_width;
			src_scr.height=show_height;
			src_scr.x=src->x+src->width-show_width;
			src_scr.y=src->y;

			dst_scr.width=show_width;
			dst_scr.height=show_height;
			dst_scr.x=dst->x;
			dst_scr.y=dst->y+dst->height-show_height;
		} else { /* from right bottom. */
			src_scr.width=show_width;
			src_scr.height=show_height;
			src_scr.x=src->x;
			src_scr.y=src->y;

			dst_scr.width=show_width;
			dst_scr.height=show_height;
			dst_scr.x=dst->x+dst->width-show_width;
			dst_scr.y=dst->y+dst->height-show_height;
		}

		show_width+=xstep;
		show_height+=ystep;
		fb_bitbld(&src_scr,&dst_scr);
		usleep(effect_delay);
    } /* loop end. */

	uninit_ge2d();
	return 0;
}
