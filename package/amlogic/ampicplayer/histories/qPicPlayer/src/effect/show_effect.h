/*********************************************
*
* api for transition effect.
*
**********************************************/
#ifndef SHOW_EFFECT_H
#define SHOW_EFFECT_H

#ifdef __cplusplus
extern "C" {
#endif

#define name_effect_over "effect_over"
extern int effect_over(screen_area_t* src,screen_area_t* dst, unsigned extra);

#define name_effect_blend "effect_blend"
extern int effect_blend(screen_area_t* src,screen_area_t* dst, unsigned extra);

#define name_effect_wave "effect_wave"
extern int effect_wave(screen_area_t* src,screen_area_t* dst, unsigned extra);

#define name_effect_cornercover "effect_cornercover"
extern int effect_cornercover(screen_area_t* src,screen_area_t* dst, unsigned extra);

#define name_effect_move "effect_move"
extern int effect_move(screen_area_t* src,screen_area_t* dst, unsigned extra);

extern int copy_area(screen_area_t* src,screen_area_t* dst, unsigned extra);
#ifdef __cplusplus
}
#endif

#endif
