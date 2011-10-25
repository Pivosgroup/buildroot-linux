/*****************************************
 * name	: player_es.c
 * function: es player relative functions
 * date		: 2010.2.4
 *****************************************/
#include <codec.h>
#include <player_error.h>

#include "player_hwdec.h"
#include "player_priv.h"
#include "stream_decoder.h"
#include "player_es.h"

static int stream_es_init(play_para_t *p_para)
{
	v_stream_info_t *vinfo= &p_para->vstream_info;
	a_stream_info_t *ainfo= &p_para->astream_info;
    s_stream_info_t *sinfo= &p_para->sstream_info;
	codec_para_t *v_codec, *a_codec, *s_codec; 	
  
	if(vinfo->has_video)
	{         
		v_codec= codec_alloc();
		if(!v_codec)
			return PLAYER_EMPTY_P;
		MEMSET(v_codec, 0, sizeof(codec_para_t));       
        if (codec_init_cntl(v_codec) != 0)
        {
            codec_free(v_codec);
            return DECODER_INIT_FAILED;
        }        
		v_codec->has_video = 1;
		v_codec->video_type 		= vinfo->video_format;
		v_codec->video_pid 			= vinfo->video_pid;		
		v_codec->am_sysinfo.format 	= vinfo->video_codec_type;
		v_codec->am_sysinfo.height 	= vinfo->video_height;
		v_codec->am_sysinfo.width 	= vinfo->video_width;
		v_codec->am_sysinfo.rate 	= vinfo->video_rate;
		v_codec->am_sysinfo.ratio	= vinfo->video_ratio;
        if ((vinfo->video_format == VFORMAT_MPEG4) 
            || (vinfo->video_format == VFORMAT_H264))
		    v_codec->am_sysinfo.param	    = (void *)EXTERNAL_PTS;
        else
            v_codec->am_sysinfo.param	    = (void *)0;
		//v_codec->stream_type        = STREAM_TYPE_ES_VIDEO;			
		v_codec->stream_type        = stream_type_convert(p_para->stream_type, v_codec->has_video, 0);	
        log_print("[%s:%d]video stream_type=%d rate=%d\n",__FUNCTION__,__LINE__,v_codec->stream_type,v_codec->am_sysinfo.rate);
        if ( p_para->vstream_info.video_format != VFORMAT_SW )
        {
			if(codec_init(v_codec) != 0)		
				return DECODER_INIT_FAILED;
        }
		p_para->vcodec = v_codec;        
	}	

	if(ainfo->has_audio)
	{  
		a_codec=codec_alloc();
		if(!a_codec)
			return PLAYER_EMPTY_P;

		MEMSET(a_codec, 0, sizeof(codec_para_t));
        if (codec_init_cntl(a_codec) != 0)
        {
            codec_free(a_codec);
            return DECODER_INIT_FAILED;
        }

		a_codec->has_audio = 1;
		a_codec->audio_type=ainfo->audio_format;
		a_codec->audio_pid=ainfo->audio_pid;		
		a_codec->audio_channels = ainfo->audio_channel;
		a_codec->audio_samplerate = ainfo->audio_samplerate;	
		//a_codec->stream_type = STREAM_TYPE_ES_AUDIO;		
		a_codec->stream_type= stream_type_convert(p_para->stream_type, 0, a_codec->has_audio);	

		log_print("[%s:%d]audio stream_type=%d afmt=%d apid=%d asample_rate=%d achannel=%d\n",
			__FUNCTION__,__LINE__,a_codec->stream_type,a_codec->audio_type,a_codec->audio_pid,
			a_codec->audio_samplerate,a_codec->audio_channels);
		
		if((a_codec->audio_type == AFORMAT_ADPCM)||(a_codec->audio_type == AFORMAT_WMA))
		{
			a_codec->audio_info.bitrate= p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->bit_rate;
			a_codec->audio_info.sample_rate = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->sample_rate;
			a_codec->audio_info.channels = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->channels;
			a_codec->audio_info.codec_id = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->codec_id;
			a_codec->audio_info.block_align = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->block_align;
			a_codec->audio_info.extradata_size = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->extradata_size;
			if(a_codec->audio_info.extradata_size > 0)
				memcpy((char*)a_codec->audio_info.extradata, p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->extradata, a_codec->audio_info.extradata_size);
			a_codec->audio_info.valid = 1;
		}
		
		if(codec_init(a_codec) != 0)			
		{
            return DECODER_INIT_FAILED;
		}	    
		p_para->acodec = a_codec;
	}    

    if(sinfo->has_sub)
    {
        s_codec=codec_alloc();
        if(!s_codec)
            return PLAYER_EMPTY_P;

        MEMSET(s_codec, 0, sizeof(codec_para_t));
        s_codec->has_sub = 1;
        s_codec->sub_type = sinfo->sub_type;
        s_codec->sub_pid = sinfo->sub_pid;
        s_codec->stream_type = STREAM_TYPE_ES_SUB;
        if(codec_init(s_codec) != 0)
        {
            return DECODER_INIT_FAILED;
        }
        p_para->scodec = s_codec;
    }
    
    if(!p_para->vcodec && !p_para->acodec)
    {
        log_print("[stream_es_init] no audio and no video codec init!\n");
        return DECODER_INIT_FAILED;
    }
	return PLAYER_SUCCESS;
}
static int stream_es_release(play_para_t *p_para)
{
    int r = -1;
	if(p_para->acodec)
	{
    	r = codec_close(p_para->acodec);
        if(r < 0)
        {
            log_error("[stream_es_release]close acodec failed, r= %x\n",r);
            return r;
        }
    	codec_free(p_para->acodec);
        p_para->acodec=NULL;
	}
	if(p_para->vcodec)
	{
	    r = codec_close_cntl(p_para->vcodec);
        if(r < 0)
        {
            log_error("[stream_es_release]close vcodec control handle failed, r= %x\n",r);
            return r;
        }
    	r = codec_close(p_para->vcodec);
        if(r < 0)
        {
            log_error("[stream_es_release]close vcodec failed, r= %x\n",r);
            return r;
        }
    	codec_free(p_para->vcodec);
        p_para->vcodec=NULL;
	}
	return 0;
}

static const stream_decoder_t es_decoder=
{
	.name="ES",
	.type=STREAM_ES,
	.init=stream_es_init,
	.add_header=NULL,
	.release=stream_es_release,
};

int es_register_stream_decoder()
{
	return register_stream_decoder(&es_decoder);
}
