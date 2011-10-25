/*******************************************************
 * name	: player_ps.c
 * function: ps play relative funtions
 * date	: 2010.2.10
 *******************************************************/
#include <player.h>
#include <log_print.h>
#include "stream_decoder.h"
#include "player_av.h"
#include "player_ps.h"

static int stream_ps_init(play_para_t *p_para)
{
	v_stream_info_t *vinfo=&p_para->vstream_info;
	a_stream_info_t *ainfo=&p_para->astream_info;
    s_stream_info_t *sinfo=&p_para->sstream_info;
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
		if(codec->video_type == VFORMAT_H264)
			codec->am_sysinfo.format = vinfo->video_codec_type;
	}
	if(ainfo->has_audio)
	{
		codec->has_audio = 1;
		codec->audio_type=ainfo->audio_format;
		codec->audio_pid=ainfo->audio_pid;		
		codec->audio_channels = ainfo->audio_channel;
		codec->audio_samplerate = ainfo->audio_samplerate;	
	}
    if(sinfo->has_sub)
    {
        codec->has_sub = 1;
        codec->sub_pid = sinfo->sub_pid;
        codec->sub_type = sinfo->sub_type;
    }
	codec->stream_type= stream_type_convert(p_para->stream_type, codec->has_video, codec->has_audio);	
	if(codec_init(codec) != 0)
		goto error1;
	
	p_para->codec = codec;	
	return PLAYER_SUCCESS;
error1:
	log_print("[ps]codec_init failed!\n");
	codec_free(codec);	
	return DECODER_INIT_FAILED;
}
static int stream_ps_release(play_para_t *p_para)
{
	if(p_para->codec)
	{
    	codec_close_cntl(p_para->codec);
    	codec_close(p_para->codec);
    	codec_free(p_para->codec);
	}
	p_para->codec=NULL;
	return 0;
}

static const stream_decoder_t ps_decoder=
{
	.name="PS",
	.type=STREAM_PS,
	.init=stream_ps_init,
	.add_header=NULL,
	.release=stream_ps_release,
};

int ps_register_stream_decoder()
{
	return register_stream_decoder(&ps_decoder);
}

