#ifndef _PLAYER_PARA_H_
#define _PLAYER_PARA_H_

#include <libavformat/avformat.h>
#include <stream_format.h>

struct play_para;

typedef enum
{
    STREAM_UNKNOWN = 0,
    STREAM_TS,
    STREAM_PS,
    STREAM_ES,
    STREAM_RM,
    STREAM_AUDIO,
    STREAM_VIDEO,
}pstream_type;


typedef struct
{
    int 		    has_video;
    vformat_t	    video_format;
    signed short    video_index;
    unsigned short  video_pid;
    unsigned int	video_width;
    unsigned int	video_height;   
    unsigned int	video_ratio;
    int             check_first_pts;
    int				flv_flag;
    int64_t  	    start_time;		
    float		    video_duration;
    float		    video_pts;
	unsigned int    video_rate;
    unsigned int    video_codec_rate;
    vdec_type_t	    video_codec_type;
    int             vdec_buf_len;        
    int             extradata_size;
    unsigned char   *extradata;
}v_stream_info_t;

typedef struct
{
    int 		    has_audio;
    int             resume_audio;
    aformat_t   	audio_format;
    signed short    audio_index;
    unsigned short  audio_pid;
    int			    audio_channel;
    int 		    audio_samplerate;
    int             check_first_pts;
    int64_t 	    start_time;
    int             adec_buf_len;  
    float		    audio_duration;    
    void   *adts_header;
}a_stream_info_t;

typedef struct
{
    int 		    has_sub;
    signed short    sub_index;
    unsigned short  sub_pid;
    unsigned int    sub_type;
    int64_t  	    start_time;	
    float		    sub_duration;
    float		    sub_pts;
    int             check_first_pts;
}s_stream_info_t;

typedef  struct
{
    unsigned int search_flag:1;
    unsigned int read_end_flag:1;
    unsigned int video_end_flag:1;   
    unsigned int audio_end_flag:1;
    unsigned int end_flag:1;
    unsigned int pts_valid:1;
    unsigned int sync_flag:1;
    unsigned int reset_flag:1;
    unsigned int need_reset:1;
    unsigned int no_audio_flag:1;
    unsigned int no_video_flag:1;
    unsigned int has_sub_flag:1;
    unsigned int loop_flag:1;
    unsigned int is_started:1;
    unsigned int black_out:1;
    unsigned int raw_mode:1;
    unsigned int pause_flag:1;
    unsigned int fast_forward:1;
    unsigned int fast_backward:1;
    unsigned int init_ff_fr:1;
    unsigned int audio_switch_flag:1;
    unsigned int audio_mute:1;
    unsigned int time_point;
    unsigned int f_step;
    unsigned int switch_audio_id;
    unsigned int switch_sub_id;    
}p_ctrl_info_t;

//void player_para_init(struct play_para *p_para);
int player_dec_init(struct play_para *p_para);
int player_decoder_init(struct play_para *p_para);
void player_para_reset(struct play_para *para);
int player_dec_reset(struct play_para *p_para);
#endif
