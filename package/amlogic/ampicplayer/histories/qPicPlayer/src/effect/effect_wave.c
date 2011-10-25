/****************************************************
*
* transition effect of wave cover.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>


static int effect_delay=(1000*100);
static int effect_wave_l2r(screen_area_t* src,screen_area_t* dst) {
	int i, w, wd, dir;
    int wave = 0;
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	screen_area_t fill_rect;
	if(init_ge2d()==-1) return -1;

	/* effect begine. */

    for (i = 0; i < src->width; i+=64) {

		dst_scr.x=dst->x+i;
		dst_scr.y=dst->y+0;
		dst_scr.height=dst->height;
		dst_scr.width=64;
		src_scr.x =src->x+i;
		src_scr.y=src->y+0;
		src_scr.width = 64;
		src_scr.height=src->height;
		fb_bitbld(&src_scr,&dst_scr);

        src_scr.width = 16;
        dst_scr.width = 16;

        dir = 1;
        w = wd = i + 64;
        wave = 0;
		while (w < src->width) {
			dst_scr.x =dst->x+wd;
			dst_scr.y =dst->y+(wave/2);
			dst_scr.height = dst->height - wave;
    
            src_scr.x =src->x+w;
            fb_bitbld(&src_scr,&dst_scr);

            if (wave > 0) {
				fill_rect.x=dst->x+w;
				fill_rect.y=0;
				fill_rect.width=16;
				fill_rect.height=wave/2;
                fb_fillrect(&fill_rect,0);
				fill_rect.x=dst->x+w;
				fill_rect.y=dst->height - wave/2;
				fill_rect.width=16;
				fill_rect.height=wave/2;
                fb_fillrect(&fill_rect,0);
            }

            wd += dst_scr.width;
            w  += src_scr.width;

            if (dir == 0) {
                wave -= 2;
                dst_scr.width = 16;
                
                if (wave <= 0) {
                    dir = 1;
                }
            } else {
                wave += 2;
                dst_scr.width = 14;

                if (wave >= 20) {
                    dir = 0;
                }
            }
        }
		usleep(effect_delay);
    }

	uninit_ge2d();
	return 0;
}
int effect_wave(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	effect_wave_l2r(src,dst);
}
