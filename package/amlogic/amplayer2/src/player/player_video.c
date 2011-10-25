/*******************************************************
 * name	: player_video.c
 * function: ts play relative funtions
 * date	: 2010.2.5
 *******************************************************/
#include <player.h>
#include <log_print.h>
#include "stream_decoder.h"
#include "player_av.h"
#include "player_video.h"

static int stream_video_init(play_para_t *p_para)
{
    int ret = -1;
	v_stream_info_t *vinfo=&p_para->vstream_info;
	
	codec_para_t *codec ;		
	codec=codec_alloc();
	if(!codec)
		return PLAYER_EMPTY_P;
	MEMSET(codec, 0, sizeof(codec_para_t));
    if (codec_init_cntl(codec) != 0)
        goto error1;
	if(vinfo->has_video)
	{
		codec->has_video = 1;
		codec->video_type = vinfo->video_format;
		codec->video_pid = vinfo->video_pid;		
	}	
	codec->stream_type= stream_type_convert(p_para->stream_type, codec->has_video, codec->has_audio);
    ret = codec_init(codec);
	if(ret != 0)
		goto error1;
	
	p_para->vcodec = codec;	   
	return PLAYER_SUCCESS;
error1:
	log_print("[video]codec_init failed!ret=%x stream_type=%d\n",ret,codec->stream_type);
	codec_free(codec);	
	return DECODER_INIT_FAILED;
}
static int stream_video_release(play_para_t *p_para)
{
	if(p_para->vcodec)
	{
        codec_close_cntl(p_para->vcodec);
        codec_close(p_para->vcodec);
        codec_free(p_para->vcodec);
	}
	p_para->vcodec=NULL;
	return 0;
}

static const stream_decoder_t video_decoder=
{
	.name="VIDEO",
	.type=STREAM_VIDEO,
	.init=stream_video_init,
	.add_header=NULL,
	.release=stream_video_release,
};

int video_register_stream_decoder()
{
	return register_stream_decoder(&video_decoder);
}

