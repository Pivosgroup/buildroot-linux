#ifndef FB_AREA_H_
#define FB_AREA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct screen_area {
	int x;
	int y;
	int width;
	int height;
} screen_area_t;

#ifdef __cplusplus
};
#endif

#endif
