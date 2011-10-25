/*******************************************************
 * name	: player_ts.c
 * function: ts play relative funtions
 * date	: 2010.2.5
 *******************************************************/
#include <player.h>
#include <log_print.h>
#include "stream_decoder.h"
#include "player_av.h"
#include "player_ts.h"

static int stream_ts_init(play_para_t *p_para)
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
		else if (codec->video_type == VFORMAT_VC1)
		{
			codec->am_sysinfo.format = vinfo->video_codec_type;
			codec->am_sysinfo.width = vinfo->video_width;
			codec->am_sysinfo.height = vinfo->video_height;
			codec->am_sysinfo.rate = vinfo->video_rate;
		}
	}
	if(ainfo->has_audio)
	{
		codec->has_audio = 1;
		codec->audio_type=ainfo->audio_format;
		codec->audio_pid=ainfo->audio_pid;		
		codec->audio_channels = ainfo->audio_channel;
		codec->audio_samplerate = ainfo->audio_samplerate;
		if(codec->audio_type == AFORMAT_ADPCM)
		{
			codec->audio_info.sample_rate = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->sample_rate;
			codec->audio_info.channels = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->channels;
			codec->audio_info.codec_id = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->codec_id;
			codec->audio_info.block_align = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->block_align;
			codec->audio_info.extradata_size = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->extradata_size;
			if(codec->audio_info.extradata_size > 0)
				memcpy((char*)codec->audio_info.extradata, p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->extradata, codec->audio_info.extradata_size);
			codec->audio_info.valid = 1;
			
		}
	}
    if(sinfo->has_sub)
    {
        codec->has_sub = 1;
        codec->sub_pid = sinfo->sub_pid;
        codec->sub_type = sinfo->sub_type;
    }
	
	codec->stream_type= stream_type_convert(p_para->stream_type, codec->has_video, codec->has_audio);
    codec->packet_size= p_para->pFormatCtx->orig_packet_size;
	if(codec_init(codec) != 0)
		goto error1;
	
	p_para->codec = codec;	
	return PLAYER_SUCCESS;
error1:
	log_print("[ts]codec_init failed!\n");
	codec_free(codec);	
	return DECODER_INIT_FAILED;
}
static int stream_ts_release(play_para_t *p_para)
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

static const stream_decoder_t ts_decoder=
{
	.name="TS",
	.type=STREAM_TS,
	.init=stream_ts_init,
	.add_header=NULL,
	.release=stream_ts_release,
};

int ts_register_stream_decoder()
{
	return register_stream_decoder(&ts_decoder);
}

