
/* picture operation. */

#ifndef __VIDEO_OUT_H___
#define __VIDEO_OUT_H___

/* the default vo device. */
#define DEFAULT_VO "fb"

typedef enum {
	VO_ERROR_OK=0,
	VO_ERROR_NOT_CONFIGED,
	VO_ERROR_NO_MATCH_DEV,
	VO_ERROR_DEV_INVALID,
	VO_ERROR_INIT_FAIL,
} e_vo_error_no_t;

typedef struct s_video_out_t {
	char* 	name;
	int		vo_handle;	/* read only. */
	void* 	arg;
	int		vo_errno;
} video_out_t;

int vo_cfg(video_out_t* vo);
int vo_preinit(video_out_t* vo);
void vo_display(video_out_t* vo,aml_image_info_t* img,int x_pan, int y_pan, int x_offs, int y_offs);
void vo_getCurrentRes(video_out_t* vo,int *x, int *y);
void vo_uninit(video_out_t* vo);

#endif //__VIDEO_OUT_H___