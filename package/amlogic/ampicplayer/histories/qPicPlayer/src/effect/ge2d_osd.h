/******************************************************
*
*	operation for ge2d for frame buffer utils.
*
*******************************************************/
#ifndef GE2D_FOR_FRAME_BUFFER_
#define GE2D_FOR_FRAME_BUFFER_
#include "screen_area.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int init_ge2d();
extern void uninit_ge2d();
extern void fb_blend(screen_area_t* src1_pos, screen_area_t* src2_pos, screen_area_t* dst_pos,int factor);
extern void fb_bitbld(screen_area_t* src_pos, screen_area_t* dst_pos);
extern void fb_fillrect(screen_area_t* src_pos, unsigned argb);

#ifdef __cplusplus
}
#endif

#endif
