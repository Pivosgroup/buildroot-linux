
#ifndef AMVIDEO_UTILS_H
#define AMVIDEO_UTILS_H

#ifdef  __cplusplus
extern "C" {
#endif


    int     amvideo_utils_get_global_offset();
    int     amvideo_utils_set_position(int32_t x, int32_t y, int32_t w, int32_t h, int rotation);
    int     amvideo_utils_set_virtual_position(int32_t x, int32_t y, int32_t w, int32_t h, int rotation);
    int     amvideo_utils_set_absolute_position(int32_t x, int32_t y, int32_t w, int32_t h, int rotation);

    int     amvideo_utils_get_position(int32_t *x, int32_t *y, int32_t *w, int32_t *h);

#ifdef  __cplusplus
}
#endif


#endif

