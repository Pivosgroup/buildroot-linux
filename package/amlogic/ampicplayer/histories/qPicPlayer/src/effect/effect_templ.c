/****************************************************
*
* transition effect template.
*
*****************************************************/
#include "ge2d_osd.h"
#include "screen_area.h"
#include <unistd.h>


static int effect_delay=(1000*10);
static int effect_template_custom(screen_area_t* src,screen_area_t* dst) {
	screen_area_t src_scr=*src;
	screen_area_t dst_scr=*dst;
	screen_area_t fill_rect;
	if(init_ge2d()==-1) return -1;

    /* loop begin*/
	{
		/* do some effect. */
		usleep(effect_delay);
    } /* loop end. */

	uninit_ge2d();
	return 0;
}
int effect_template(screen_area_t* src,screen_area_t* dst, unsigned extra) {
	effect_template_custom(src,dst);
}
