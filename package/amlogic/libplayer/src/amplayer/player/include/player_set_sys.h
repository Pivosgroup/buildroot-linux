#ifndef _PLAYER_SET_DISP_H_
#define _PLAYER_SET_DISP_H_

#define MID_800_400_FREESCALE (0x10001) //mid 800*400

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum
{
    DISP_MODE_480I  = 1,
    DISP_MODE_480P  = 2,
    DISP_MODE_576I  = 3,
    DISP_MODE_576P  = 4,
    DISP_MODE_720P  = 5,
    DISP_MODE_1080I = 6,
    DISP_MODE_1080P = 7
} display_mode;

typedef struct {
    display_mode disp_mode;
    int osd_disble_coordinate[8];
    int osd_enable_coordinate[8];
    int video_enablecoordinate[4];
    int fb0_freescale_width;
    int fb0_freescale_height;
    int fb1_freescale_width;
    int fb1_freescale_height;
} freescale_setting_t;

typedef struct
{
}sys_h264_profile_t;

typedef struct
{
	int progressive_enable;
	int interlace_enable;
	int wmv1_enable;
	int wmv2_enable;
	int wmv3_enable;
}sys_vc1_profile_t;

typedef struct 
{
	int es_support;
	int exceed_720p_enable;
}sys_real_profile_t;

typedef struct 
{
	
}sys_mpeg12_profile_t;

typedef struct 
{
	
}sys_mpeg4_profile_t;

typedef struct 
{
	
}sys_mjpeg_profile_t;

typedef struct _system_para_
{
	sys_h264_profile_t 		h264_para;
	sys_vc1_profile_t  		vc1_para;
	sys_real_profile_t 		real_para;
	sys_mpeg12_profile_t  	mpeg12_para;	
	sys_mpeg4_profile_t  	mpeg4_para;	
	sys_mjpeg_profile_t		mjpeg_para;
}vdec_profile_t;

int get_vdec_profile(vdec_profile_t *vdec_profiles);

int set_sysfs_str(const char *path, const char *val);
int  get_sysfs_str(const char *path, char *valstr, int size);
int set_sysfs_int(const char *path, int val);
int get_sysfs_int(const char *path);

int set_black_policy(int blackout);
int get_black_policy();
int set_tsync_enable(int enable);
int set_tsync_discontinue(int enable);
int get_pts_discontinue();
int set_fb0_blank(int blank);
int set_fb1_blank(int blank);
int set_subtitle_num(int num);
int av_get_subtitle_curr();
int set_subtitle_startpts(int pts);
int set_subtitle_fps(int fps);
int set_subtitle_subtype(int subtype);
void get_display_mode(char *mode);
int set_fb0_freescale(int freescale);
int set_fb1_freescale(int freescale);
int set_display_axis(int *coordinate);
int set_video_axis(int *coordinate);
int set_fb0_scale_width(int width);
int set_fb0_scale_height(int height);
int set_fb1_scale_width(int width);
int set_fb1_scale_height(int height);
int check_audiodsp_fatal_err(void);
int set_stb_source_hiu(void);
int set_stb_demux_source_hiu(void);

int set_subtitle_enable(int num);
int set_subtitle_curr(int num);
int check_file_same(char *filename2);

//player sysfs API channel
int set_amutils_enable(int isOn);
int set_amutils_cmd(const char* cmd);
int get_amutils_cmd(char* cmd);

#ifdef  __cplusplus
}
#endif

#endif
