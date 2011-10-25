/*******************************************************
 * name	: player_audio.c
 * function: ts play relative funtions
 * date	: 2010.2.20
 *******************************************************/
#include <player.h>
#include <log_print.h>

#include "stream_decoder.h"
#include "player_av.h"
#include "player_audio.h"

static int stream_audio_init(play_para_t *p_para)
{
    int ret= -1;   
	a_stream_info_t *ainfo=&p_para->astream_info;
	codec_para_t *codec ;		
	codec=codec_alloc();
	if(!codec)
		return PLAYER_EMPTY_P;
	MEMSET(codec, 0, sizeof(codec_para_t));    	
	if(ainfo->has_audio)
	{
		codec->has_audio = 1;
		codec->audio_type=ainfo->audio_format;
		codec->audio_pid=ainfo->audio_pid;		
		codec->audio_channels = ainfo->audio_channel;
		codec->audio_samplerate = ainfo->audio_samplerate;	
	}   
	codec->stream_type= stream_type_convert(p_para->stream_type, codec->has_video, codec->has_audio);
    ret = codec_init(codec);
	if(ret != 0)
		goto error1;  
    log_print("[%s:%d]codec init finished! mute_on:%d\n",__FUNCTION__,__LINE__,p_para->playctrl_info.audio_mute);
	ret = codec_set_mute(codec,p_para->playctrl_info.audio_mute);
    if(ret != 0)
		goto error1;   
	p_para->acodec = codec;	   
	return PLAYER_SUCCESS;
	
error1:
	log_print("[audio]codec_init failed!ret=%x stream_type=%d\n",ret,codec->stream_type);
	codec_free(codec);	
	return DECODER_INIT_FAILED;
}
static int stream_audio_release(play_para_t *p_para)
{
	if(p_para->acodec)
	{
        codec_close(p_para->acodec);
        codec_free(p_para->acodec);
	}
	p_para->acodec=NULL;
	return PLAYER_SUCCESS;
}

static const stream_decoder_t audio_decoder=
{
	.name="AUDIO",
	.type=STREAM_AUDIO,
	.init=stream_audio_init,
	.add_header=NULL,
	.release=stream_audio_release,
};

int audio_register_stream_decoder()
{
	return register_stream_decoder(&audio_decoder);
}

