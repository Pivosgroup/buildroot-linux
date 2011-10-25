#ifndef _PLAYER_PRIV_H_
#define _PLAYER_PRIV_H_
 //header file
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <pthread.h>

 /* ffmpeg headers */
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include <codec_type.h> 

#include <player.h>
#include <log_print.h>

#include "player_id.h"
#include "player_para.h"




struct stream_decoder;

#define  MALLOC(s)      malloc(s)
#define  FREE(d)		free(d)
#define  MEMCPY(d,s,l)	memcpy(d,s,l)
#define  MEMSET(d,s,l)	memset(d,s,l)
#define  MIN(x,y)		((x)<(y)?(x):(y))

#define VB_SIZE             (0x100000)
#define AB_SIZE             (0x60000)
#define MAX_BURST_WRITE     (VB_SIZE/32)
#define MAX_RAW_DATA_SIZE   (0x10000)       //64k
#define MIN_RAW_DATA_SIZE   (0x1000)        //4k
#define RESERVE_VIDEO_SIZE	(256)
#define RESERVE_AUDIO_SIZE	(64)
#define MAX_PACKET_SIZE		(2*1024*1024)
#define FILE_BUFFER_SIZE	(1024*128)//(1024*512)	
#define CHECK_END_COUNT     (10)
#define CHECK_END_INTERVAL  (100)   //ms

#define SEARCH_INVALID		(0)
#define SEARCH_SEND_CMD		(1)
#define SEARCH_GET_CMD		(2)
#define SEARCH_ACK_CMD		(3)
#define FF_FB_BASE_STEP     (2)

#define TRICKMODE_NONE       0x00
#define TRICKMODE_I          0x01
#define TRICKMODE_FFFB       0x02

#define UNIT_FREQ   96000
#define PTS_FREQ    90000
#define AV_SYNC_THRESH    PTS_FREQ*10
#define INT64_0     INT64_C(0x8000000000000000)

#define EXTERNAL_PTS		(1)
#define ADTS_HEADER_SIZE	(7)

#define SUBTITLE_SYNC_HIGH  0x414d4c55
#define SUBTITLE_SYNC_LOW   0xaa000000

typedef struct message_pool
{
 player_cmd_t *message_list[MESSAGE_MAX];
 int message_in_index;
 int message_out_index;
 int message_num;
 pthread_mutex_t msg_mutex;
}message_pool_t;


typedef struct player_thread_mgt
{
	pthread_mutex_t  pthread_mutex;
	pthread_cond_t   pthread_cond;
	pthread_t		 pthread_id;
	player_status	 player_state;
}player_thread_mgt_t;

typedef union
{
    int64_t      total_bytes;
    unsigned int vpkt_num;
    unsigned int apkt_num;
    unsigned int spkt_num;
}read_write_size;

typedef struct
{
    unsigned long old_time_ms;
    int end_count; 
    int interval;
}check_end_info_t;

typedef struct play_para
{
	play_control_t  *start_param;
	int player_id;
    char 		    *file_name;
    pfile_type 	    file_type;
    int64_t 	 	file_size;	
    int64_t         data_offset;
    pstream_type    stream_type;
    int             vstream_num;
    int             astream_num;
    int             sstream_num;   
    int             first_index;
    int             max_raw_size;
    check_end_info_t check_end;

    read_write_size read_size;
    read_write_size write_size;
    
    p_ctrl_info_t	playctrl_info;	
    v_stream_info_t	vstream_info;
    a_stream_info_t	astream_info;
    s_stream_info_t sstream_info;
	media_info_t    media_info;

    AVFormatContext *pFormatCtx; 

    codec_para_t 	*vcodec;
    codec_para_t 	*acodec;
    codec_para_t    *scodec;
    codec_para_t 	*codec;	

    player_info_t state;
	const struct stream_decoder *decoder;   
    callback_t update_state;
	message_pool_t message_pool;
	player_thread_mgt_t thread_mgt;
	unsigned long extern_priv;/*used for uper level controler*/
}play_para_t;

typedef struct media_type_t
{
    const char   *file_ext;
    const pfile_type      file_type; 
    const pstream_type    stream_type;
}media_type;

//private functions declearation
void player_thread(play_para_t *player);
int message_pool_init(play_para_t *para);
//video
codec_para_t *get_video_codec(play_para_t *para);
//audio
codec_para_t *get_audio_codec(play_para_t *para);

int send_message(play_para_t *para,player_cmd_t *cmd);
int send_message_by_pid(int pid,player_cmd_t *cmd);
void clear_all_message(play_para_t *para);

player_cmd_t * get_message(play_para_t *para);
int update_player_states(play_para_t *para,int force);
void set_player_error_no(play_para_t *player,int error_no);
int register_update_callback(callback_t *cb,update_state_fun_t up_fn,int interval_s);

#endif
