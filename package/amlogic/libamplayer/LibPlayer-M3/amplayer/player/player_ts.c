/*******************************************************
 * name : player_ts.c
 * function: ts play relative funtions
 * date : 2010.2.5
 *******************************************************/
#include <player.h>
#include <log_print.h>
#include "stream_decoder.h"
#include "player_av.h"
#include "player_ts.h"

static int stream_ts_init(play_para_t *p_para)
{
    v_stream_info_t *vinfo = &p_para->vstream_info;
    a_stream_info_t *ainfo = &p_para->astream_info;
    s_stream_info_t *sinfo = &p_para->sstream_info;
	AVCodecContext  *pCodecCtx;
    codec_para_t *codec ;
    int ret = CODEC_ERROR_NONE;

    codec = codec_alloc();
    if (!codec) {
        return PLAYER_EMPTY_P;
    }
    MEMSET(codec, 0, sizeof(codec_para_t));

    codec->noblock = !!p_para->buffering_enable;
    if (vinfo->has_video) {
        codec->has_video = 1;
        codec->video_type = vinfo->video_format;
        codec->video_pid = vinfo->video_pid;
        if ((codec->video_type == VFORMAT_H264) || (codec->video_type == VFORMAT_H264MVC)) {
            codec->am_sysinfo.format = vinfo->video_codec_type;
            codec->am_sysinfo.width = vinfo->video_width;
            codec->am_sysinfo.height = vinfo->video_height;
        } else if (codec->video_type == VFORMAT_VC1) {
            codec->am_sysinfo.format = vinfo->video_codec_type;
            codec->am_sysinfo.width = vinfo->video_width;
            codec->am_sysinfo.height = vinfo->video_height;
            codec->am_sysinfo.rate = vinfo->video_rate;
        }
    }
    if (ainfo->has_audio) {
        codec->has_audio = 1;
        codec->audio_type = ainfo->audio_format;
        codec->audio_pid = ainfo->audio_pid;
        codec->audio_channels = ainfo->audio_channel;
        codec->audio_samplerate = ainfo->audio_samplerate;
		pCodecCtx = p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec;	
        /*if ((codec->audio_type == AFORMAT_ADPCM) || (codec->audio_type == AFORMAT_WMA)
            || (codec->audio_type == AFORMAT_WMAPRO) || (codec->audio_type == AFORMAT_PCM_S16BE)
            || (codec->audio_type == AFORMAT_PCM_S16LE) || (codec->audio_type == AFORMAT_PCM_U8)
            || (codec->audio_type == AFORMAT_PCM_BLURAY)||(codec->audio_type == AFORMAT_AMR)) {*/
          if(IS_AUIDO_NEED_EXT_INFO(codec->audio_type)){
            codec->audio_info.bitrate = pCodecCtx->sample_fmt;
            codec->audio_info.sample_rate = pCodecCtx->sample_rate;
            codec->audio_info.channels = pCodecCtx->channels;
            codec->audio_info.codec_id = pCodecCtx->codec_id;
            codec->audio_info.block_align = pCodecCtx->block_align;
            codec->audio_info.extradata_size = pCodecCtx->extradata_size;
            if (codec->audio_info.extradata_size > 0) {
		     if(codec->audio_info.extradata_size > 	AUDIO_EXTRA_DATA_SIZE)
		     {
	      			log_print("[%s:%d],extra data size exceed max  extra data buffer,cut it to max buffer size ", __FUNCTION__, __LINE__);
				codec->audio_info.extradata_size = 	AUDIO_EXTRA_DATA_SIZE;
	  	     }
                memcpy((char*)codec->audio_info.extradata, pCodecCtx->extradata, codec->audio_info.extradata_size);
            }
            codec->audio_info.valid = 1;

        }
	 codec->avsync_threshold = p_para->start_param->avsync_threshold;
     	 log_print("[%s:%d]audio bitrate=%d sample_rate=%d channels=%d codec_id=%x block_align=%d,extra size\n",
                  __FUNCTION__, __LINE__, codec->audio_info.bitrate, codec->audio_info.sample_rate, codec->audio_info.channels,
                  codec->audio_info.codec_id, codec->audio_info.block_align,codec->audio_info.extradata_size);
       }
    if (sinfo->has_sub) {
        codec->has_sub = 1;
        codec->sub_pid = sinfo->sub_pid;
        codec->sub_type = sinfo->sub_type;
    }

    codec->stream_type = stream_type_convert(p_para->stream_type, codec->has_video, codec->has_audio);
    codec->packet_size = p_para->pFormatCtx->orig_packet_size;
    ret = codec_init(codec);
    if (ret != CODEC_ERROR_NONE) {
        if (ret != CODEC_OPEN_HANDLE_FAILED) {
            codec_close(codec);
        }
        goto error1;
    }

    p_para->codec = codec;
    return PLAYER_SUCCESS;
error1:
    log_print("[ts]codec_init failed!\n");
    codec_free(codec);
    return DECODER_INIT_FAILED;
}
static int stream_ts_release(play_para_t *p_para)
{
    if (p_para->codec) {
        codec_close(p_para->codec);
        codec_free(p_para->codec);
    }
    p_para->codec = NULL;
    return 0;
}

static const stream_decoder_t ts_decoder = {
    .name = "TS",
    .type = STREAM_TS,
    .init = stream_ts_init,
    .add_header = NULL,
    .release = stream_ts_release,
};

int ts_register_stream_decoder()
{
    return register_stream_decoder(&ts_decoder);
}

