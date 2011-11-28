/************************************************
 * name : player_para.c
 * function: ffmpeg file relative and set player parameters functions
 * date     : 2010.2.4
 ************************************************/
#include <codec.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <player_set_sys.h>

#include "thread_mgt.h"
#include "stream_decoder.h"
#include "player_priv.h"
#include "player_hwdec.h"
#include "player_update.h"
#include "player_ffmpeg_ctrl.h"
#include "system/systemsetting.h"

DECLARE_ALIGNED(16, uint8_t, dec_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2]);

static int try_decode_picture(play_para_t *p_para, int video_index)
{
    AVCodecContext *ic = NULL;
    AVCodec *codec = NULL;
    AVFrame *picture = NULL;
    int got_picture = 0;
    int ret = 0;
    int read_packets = 0;
    int64_t cur_pos;
    AVPacket avpkt;

    ic = p_para->pFormatCtx->streams[video_index]->codec;

    codec = avcodec_find_decoder(ic->codec_id);
    if (!codec) {
        log_print("[%s:%d]Codec not found\n", __FUNCTION__, __LINE__);
        goto exitf1;
    }

    if (avcodec_open(ic, codec) < 0) {
        log_print("[%s:%d]Could not open codec\n", __FUNCTION__, __LINE__);
        goto exitf1;
    }

    picture = avcodec_alloc_frame();
    if (!picture) {
        log_print("[%s:%d]Could not allocate picture\n", __FUNCTION__, __LINE__);
        goto exitf;
    }

    cur_pos = url_ftell(p_para->pFormatCtx->pb);
    log_print("[%s:%d]codec id 0x%x, cur_pos 0x%llx, video index %d\n", 
        __FUNCTION__, __LINE__, ic->codec_id, cur_pos, video_index);
    av_init_packet(&avpkt);

    /* get the first video frame and decode it */
    while (!got_picture) {
        do {
            ret = av_read_frame(p_para->pFormatCtx, &avpkt);
            if (ret < 0) {
                if (AVERROR(EAGAIN) != ret) {
                    /*if the return is EAGAIN,we need to try more times*/
                    log_error("[%s:%d]av_read_frame return (%d)\n", __FUNCTION__, __LINE__, ret);
                    url_fseek(p_para->pFormatCtx->pb, cur_pos, SEEK_SET);
                    av_free_packet(&avpkt);
                    goto exitf;
                } else {
                    av_free_packet(&avpkt);
                    continue;
                }
            }
        } while (avpkt.stream_index != video_index);

        avcodec_decode_video2(ic, picture, &got_picture, &avpkt);
        av_free_packet(&avpkt);
        read_packets++;
    }

    log_print("[%s:%d]got one picture\n", __FUNCTION__, __LINE__);
    if (picture) {
        av_free(picture);
    }

    url_fseek(p_para->pFormatCtx->pb, cur_pos, SEEK_SET);
	avcodec_close(ic);
    return 0;
    
exitf:
    if (picture) {
        av_free(picture);
    }
	avcodec_close(ic);
exitf1:
	
    if (read_packets) {
        return read_packets;
    } else {
        return ret;
    }
}

static void get_av_codec_type(play_para_t *p_para)
{
    AVFormatContext *pFormatCtx = p_para->pFormatCtx;
    AVStream *pStream;
    AVCodecContext  *pCodecCtx;
    int video_index = p_para->vstream_info.video_index;
    int audio_index = p_para->astream_info.audio_index;
    int sub_index = p_para->sstream_info.sub_index;
    log_print("[%s:%d]vidx=%d aidx=%d sidx=%d\n", __FUNCTION__, __LINE__, video_index, audio_index, sub_index);

	if (video_index != -1) {
        pStream = pFormatCtx->streams[video_index];
        pCodecCtx = pStream->codec;
        p_para->vstream_info.video_format   = video_type_convert(pCodecCtx->codec_id);
        if (pCodecCtx->codec_id == CODEC_ID_FLV1) {
            pCodecCtx->codec_tag = CODEC_TAG_F263;
            p_para->vstream_info.flv_flag = 1;
        } else {
            p_para->vstream_info.flv_flag = 0;
        }
        if ((pCodecCtx->codec_id == CODEC_ID_MPEG1VIDEO)
            || (pCodecCtx->codec_id == CODEC_ID_MPEG2VIDEO)
            || (pCodecCtx->codec_id == CODEC_ID_MPEG2VIDEO_XVMC)) {
            mpeg_check_sequence(p_para);
        }
        if (p_para->stream_type == STREAM_ES && pCodecCtx->codec_tag != 0) {
            p_para->vstream_info.video_codec_type = video_codec_type_convert(pCodecCtx->codec_tag);
        } else {
            p_para->vstream_info.video_codec_type = video_codec_type_convert(pCodecCtx->codec_id);
        }
		
        if ((p_para->vstream_info.video_format < 0) ||
            (p_para->vstream_info.video_format >= VFORMAT_MAX) ||
            (IS_NEED_VDEC_INFO(p_para->vstream_info.video_format)&&
             p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_UNKNOW)) {
            p_para->vstream_info.has_video = 0;            
        }else if (p_para->vstream_info.video_format == VFORMAT_UNSUPPORT){
        	p_para->vstream_info.has_video = 0;     
        }

		if (p_para->vstream_info.video_format == VFORMAT_VC1){
        	if(p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WMV3) {
                if (pFormatCtx->video_avg_frame_time != 0) {
                    p_para->vstream_info.video_rate = pFormatCtx->video_avg_frame_time * 96 / 10000;
                }

                if (pCodecCtx->extradata && !(pCodecCtx->extradata[3] & 1)) { // this format is not supported
                    p_para->vstream_info.has_video = 0;                    
                }
        	}				
        }else if (p_para->vstream_info.video_format == VFORMAT_SW){
        	p_para->vstream_info.has_video = 0;     
        }
		
        if (p_para->vstream_info.has_video) {
            p_para->vstream_info.video_pid      = (unsigned short)pStream->id;
            if (0 != pStream->time_base.den) {
                p_para->vstream_info.video_duration = ((float)pStream->time_base.num / pStream->time_base.den) * UNIT_FREQ;
                p_para->vstream_info.video_pts      = ((float)pStream->time_base.num / pStream->time_base.den) * PTS_FREQ;
            }
            p_para->vstream_info.video_width    = pCodecCtx->width;
            p_para->vstream_info.video_height   = pCodecCtx->height;
            p_para->vstream_info.video_ratio    = (pStream->sample_aspect_ratio.num << 16) | pStream->sample_aspect_ratio.den;
            p_para->vstream_info.video_ratio64  = (pStream->sample_aspect_ratio.num << 32) | pStream->sample_aspect_ratio.den;

			log_print("[%s:%d]time_base=%d/%d,r_frame_rate=%d/%d ratio=%d/%d video_pts=%.3f\n", __FUNCTION__, __LINE__, \
						pCodecCtx->time_base.num, pCodecCtx->time_base.den, \
						pStream->r_frame_rate.den, pStream->r_frame_rate.num, \
						pStream->sample_aspect_ratio.num, pStream->sample_aspect_ratio.den, p_para->vstream_info.video_pts);
			
            if (0 != pCodecCtx->time_base.den) {
                p_para->vstream_info.video_codec_rate = (int64_t)UNIT_FREQ * pCodecCtx->time_base.num / pCodecCtx->time_base.den;
            }

            if (0 != pStream->r_frame_rate.num) {
                p_para->vstream_info.video_rate = (int64_t)UNIT_FREQ * pStream->r_frame_rate.den / pStream->r_frame_rate.num;
            }
            log_print("[%s:%d]video_codec_rate=%d,video_rate=%d\n", __FUNCTION__, __LINE__, p_para->vstream_info.video_codec_rate, p_para->vstream_info.video_rate);

            if (p_para->vstream_info.video_format != VFORMAT_MPEG12) {
                p_para->vstream_info.extradata_size = pCodecCtx->extradata_size;
                p_para->vstream_info.extradata      = pCodecCtx->extradata;
            }
            p_para->vstream_info.start_time = pStream->start_time * pStream->time_base.num * PTS_FREQ / pStream->time_base.den;
          
            /* added by Z.C for mov file frame duration */
            if ((p_para->file_type == MOV_FILE) || (p_para->file_type == MP4_FILE)) {
                if (pStream->nb_frames && pStream->duration && pStream->time_base.den && pStream->time_base.num) {
                    unsigned int fix_rate;
                    if ((0 != pStream->time_base.den) && (0 != pStream->nb_frames)) {
                        fix_rate = UNIT_FREQ * pStream->duration * pStream->time_base.num / pStream->time_base.den / pStream->nb_frames;
                    }
                    p_para->vstream_info.video_rate = fix_rate;
                    log_print("[%s:%d]video_codec_rate=%d,video_rate=%d\n", __FUNCTION__, __LINE__, p_para->vstream_info.video_codec_rate, p_para->vstream_info.video_rate);

                }
            } else if (p_para->file_type == FLV_FILE) {
                if (pStream->special_fps > 0) {
                    p_para->vstream_info.video_rate = UNIT_FREQ / pStream->special_fps;
                }
            } 			
        }
    } else {
        p_para->vstream_info.has_video = 0;
        log_print("no video specified!\n");
    }
    if (audio_index != -1) {
        pStream = pFormatCtx->streams[audio_index];
        pCodecCtx = pStream->codec;
        p_para->astream_info.audio_pid      = (unsigned short)pStream->id;
        p_para->astream_info.audio_format   = audio_type_convert(pCodecCtx->codec_id, p_para->file_type);
		p_para->astream_info.audio_channel  = pCodecCtx->channels;
		p_para->astream_info.audio_samplerate = pCodecCtx->sample_rate;
		log_print("[%s:%d]afmt=%d apid=%d asr=%d ach=%d aidx=%d\n",
              __FUNCTION__, __LINE__, p_para->astream_info.audio_format, 
              p_para->astream_info.audio_pid, p_para->astream_info.audio_samplerate,
              p_para->astream_info.audio_channel, p_para->astream_info.audio_index);
        /* only support 2ch flac,cook,raac */
        if ((p_para->astream_info.audio_channel > 2) && 
			(IS_AUDIO_NOT_SUPPORT_EXCEED_2CH(p_para->astream_info.audio_format))) {
            log_print(" afmt=%d channel=%d ******** we do not support more than 2ch \n", \
				p_para->astream_info.audio_format, p_para->astream_info.audio_channel);
            p_para->astream_info.has_audio = 0;           
        }

        if (p_para->astream_info.audio_format == AFORMAT_AAC) {
            pCodecCtx->profile = FF_PROFILE_UNKNOWN;
            AVCodecContext  *pCodecCtx = p_para->pFormatCtx->streams[audio_index]->codec;
            uint8_t *ppp = pCodecCtx->extradata;
            if (ppp != NULL) {
                char profile = (*ppp) >> 3;
                log_print(" aac profile = %d  ********* { MAIN, LC, SSR } \n", profile);

                if (profile == 1) {
                    pCodecCtx->profile = FF_PROFILE_AAC_MAIN;
					p_para->astream_info.has_audio = 0;
					log_print("AAC MAIN not support yet!!\n");
                }
                //else
                //  p_para->astream_info.has_audio = 0;
            } else {

                AVCodec * aac_codec = avcodec_find_decoder_by_name("aac");

                if (aac_codec) {
                    int len;
                    int data_size;
                    AVPacket packet;

                    avcodec_open(pCodecCtx, aac_codec);

                    av_init_packet(&packet);
                    av_read_frame(p_para->pFormatCtx, &packet);

                    data_size = sizeof(dec_buf);
                    len = avcodec_decode_audio3(pCodecCtx, (int16_t *)dec_buf, &data_size, &packet);
                    if (len < 0) {
                        log_print("[%s,%d] decode failed!\n", __func__, __LINE__);
                    }

                    avcodec_close(pCodecCtx);
                }
            }
        }
        if ((p_para->astream_info.audio_format < 0) ||
            (p_para->astream_info.audio_format >= AFORMAT_MAX)) {
            p_para->astream_info.has_audio = 0;           
            log_print("audio format not support!\n");
        }else if (p_para->astream_info.audio_format == AFORMAT_UNSUPPORT){
        	p_para->astream_info.has_audio = 0;     
        }
		
        if (p_para->astream_info.has_audio) {
            if (0 != pStream->time_base.den) {
                p_para->astream_info.audio_duration = PTS_FREQ * ((float)pStream->time_base.num / pStream->time_base.den);
            }
            p_para->astream_info.start_time = pStream->start_time * pStream->time_base.num * PTS_FREQ / pStream->time_base.den;
        }
    } else {
        p_para->astream_info.has_audio = 0;
        log_print("no audio specified!\n");
    }
    if (sub_index != -1) {
        pStream = pFormatCtx->streams[sub_index];
        p_para->sstream_info.sub_pid = (unsigned short)pStream->id;
        p_para->sstream_info.sub_type = pStream->codec->codec_id;
        if (pStream->time_base.num && (0 != pStream->time_base.den)) {
            p_para->sstream_info.sub_duration = UNIT_FREQ * ((float)pStream->time_base.num / pStream->time_base.den);
            p_para->sstream_info.sub_pts = PTS_FREQ * ((float)pStream->time_base.num / pStream->time_base.den);
            p_para->sstream_info.start_time = pStream->start_time * pStream->time_base.num * PTS_FREQ / pStream->time_base.den;
        } else {
            p_para->sstream_info.start_time = pStream->start_time * PTS_FREQ;
        }
    } else {
        p_para->sstream_info.has_sub = 0;
    }
    return;
}

static void get_stream_info(play_para_t *p_para)
{
    unsigned int i;
    AVFormatContext *pFormat = p_para->pFormatCtx;
    AVStream *pStream;
    AVCodecContext *pCodec;
    int video_index = p_para->vstream_info.video_index;
    int audio_index = p_para->astream_info.audio_index;
    int sub_index = p_para->sstream_info.sub_index;
    int temp_vidx = -1, temp_aidx = -1, temp_sidx = -1;
    int bitrate = 0;
    int read_packets = 0;
    int ret = 0;

    p_para->first_index = pFormat->first_index;
    
    /* caculate the stream numbers */
    p_para->vstream_num = 0;
    p_para->astream_num = 0;
    p_para->sstream_num = 0;

    for (i = 0; i < pFormat->nb_streams; i++) {
        pStream = pFormat->streams[i];
        pCodec = pStream->codec;
        if (pCodec->codec_type == CODEC_TYPE_VIDEO) {
            p_para->vstream_num ++;
            if (p_para->file_type == RM_FILE) {
                /* find max bitrate */
                if (pCodec->bit_rate > bitrate) {
                    /* only support RV30 and RV40 */
                    if ((pCodec->codec_id == CODEC_ID_RV30)
                        || (pCodec->codec_id == CODEC_ID_RV40)) {
                        ret = try_decode_picture(p_para, i);
                        if (ret == 0) {
                            bitrate = pCodec->bit_rate;
                            temp_vidx = i;
                        } else if (ret > read_packets) {
                            read_packets = ret;
                            temp_vidx = i;
                        }
                    }
                }
            } else {
                if (temp_vidx == -1) {
                    temp_vidx = i;
                }
            }
        } else if (pCodec->codec_type == CODEC_TYPE_AUDIO) {
            p_para->astream_num ++;
            if (p_para->file_type == RM_FILE) {
                if ((temp_aidx == -1)
                    && (CODEC_ID_SIPR != pCodec->codec_id)) { // SIPR not supported now
                    temp_aidx = i;
                }
            } else {
                if (temp_aidx == -1) {
                    temp_aidx = i;
                }
            }
        } else if (pCodec->codec_type == CODEC_TYPE_SUBTITLE) {
            p_para->sstream_num ++;
            if (temp_sidx == -1) {
                temp_sidx = i;
            }
        }
    }

    if (p_para->vstream_num >= 1) {
        p_para->vstream_info.has_video = 1;
    } else {
        p_para->vstream_info.has_video = 0;
        p_para->vstream_info.video_format = -1;
    }

    if (p_para->astream_num >= 1) {
        p_para->astream_info.has_audio = 1;
    } else {
        p_para->astream_info.has_audio = 0;
        p_para->astream_info.audio_format = -1;
    }


    if (p_para->sstream_num >= 1) {
        p_para->sstream_info.has_sub = 1;
    } else {
        p_para->sstream_info.has_sub = 0;
    }

    if ((p_para->vstream_num >= 1) ||
        (p_para->astream_num >= 1) ||
        (p_para->sstream_num >= 1)) {
        if ((video_index > (p_para->vstream_num + p_para->astream_num)) || (video_index < 0)) {
            video_index = temp_vidx;
        }

        if (audio_index > (p_para->vstream_num + p_para->astream_num) || audio_index < 0) {
            audio_index = temp_aidx;
        }

        if ((sub_index > p_para->sstream_num) || (sub_index < 0)) {
            sub_index = temp_sidx;
        }
    }
    if (p_para->astream_info.has_audio && audio_index!= -1) {		
        p_para->astream_info.audio_channel = pFormat->streams[audio_index]->codec->channels;
        p_para->astream_info.audio_samplerate = pFormat->streams[audio_index]->codec->sample_rate;
    }

    p_para->vstream_info.video_index = video_index;
    p_para->astream_info.audio_index = audio_index;
    p_para->sstream_info.sub_index = sub_index;
    log_print("Video index %d and Audio index %d to be played (index -1 means no stream)\n", video_index, audio_index);
    if (p_para->sstream_info.has_sub) {
        log_print("Subtitle index %d detected\n", sub_index);
    }

    get_av_codec_type(p_para);

    if (p_para->stream_type == STREAM_RM && video_index != -1) {
        if (p_para->pFormatCtx->streams[video_index]->stream_offset > 0) {
            p_para->data_offset = p_para->pFormatCtx->streams[video_index]->stream_offset;
        } else {
            p_para->data_offset = p_para->pFormatCtx->data_offset;
        }
        log_print("[%s:%d]real offset %lld\n", __FUNCTION__, __LINE__, p_para->data_offset);
		
        if (p_para->vstream_info.video_height > 720) {
            log_print("[%s:%d]real video_height=%d, exceed 720 not support!\n", __FUNCTION__, __LINE__, p_para->vstream_info.video_height);
            p_para->vstream_info.has_video = 0;            
        }
    }
     else{
	 	p_para->data_offset = p_para->pFormatCtx->data_offset;
		log_print("[%s:%d]data start offset %lld\n", __FUNCTION__, __LINE__, p_para->data_offset);
     }

    if (video_index != -1) {
        if ((p_para->vstream_info.video_width > 1920) 
            || (p_para->vstream_info.video_height > 1088)) {
            log_error("[%s]can't support exceeding video \n", __FUNCTION__);
            set_player_error_no(p_para, PLAYER_UNSUPPORT_VIDEO);
            update_player_states(p_para, 1);
            p_para->vstream_info.has_video = 0;
            p_para->vstream_info.video_index = -1;
        }
    }
    
    if (p_para->vstream_info.video_format == VFORMAT_VC1 && video_index != -1) {

		if (p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WVC1 &&
			p_para->vstream_info.video_width > 1920){
			log_error("[%s]can't support wvc1 exceed 1920\n", __FUNCTION__);
			p_para->vstream_info.has_video = 0; 
		}else{
		#if 0
			/* process vc1 packet to detect interlace or progressive */
	        int64_t cur_pos;
	        AVPacket avpkt;
	        int ret;

	        cur_pos = url_ftell(p_para->pFormatCtx->pb);
	        av_init_packet(&avpkt);

	        /* get the first video frame */
	        do {
	            ret = av_read_frame(p_para->pFormatCtx, &avpkt);
	            if (ret < 0) {
	                if (AVERROR(EAGAIN) != ret) {
	                    /*if the return is EAGAIN,we need to try more times*/
	                    log_error("[%s:%d]av_read_frame return (%d)\n", __FUNCTION__, __LINE__, ret);
	                    url_fseek(p_para->pFormatCtx->pb, cur_pos, SEEK_SET);
	                    av_free_packet(&avpkt);
	                    return;
	                } else {
	                    av_free_packet(&avpkt);
	                    continue;
	                }
	            }
	        } while (avpkt.stream_index != video_index);

	        ret = get_vc1_di(avpkt.data, avpkt.size);
	        if (ret == 1) {// interlace, not support
	            log_print("[%s:%d]vc1 interlace video, not support!\n", __FUNCTION__, __LINE__);
	            set_player_error_no(p_para, PLAYER_UNSUPPORT_VIDEO);
	            p_para->vstream_info.has_video = 0;
	            p_para->vstream_info.video_index = -1;
	        }
	        av_free_packet(&avpkt);

	        url_fseek(p_para->pFormatCtx->pb, cur_pos, SEEK_SET);
        #else
            if (p_para->pFormatCtx->streams[video_index]->codec->frame_interlace) {
                log_print("[%s:%d]vc1 interlace video, not support!\n", __FUNCTION__, __LINE__);
	            set_player_error_no(p_para, PLAYER_UNSUPPORT_VIDEO);
	            p_para->vstream_info.has_video = 0;
	            p_para->vstream_info.video_index = -1;
            }
        #endif
		}
    }

	if ((p_para->vstream_info.video_format == VFORMAT_H264 || p_para->vstream_info.video_format == VFORMAT_H264MVC ) && video_index != -1){
		if (p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_H264 &&
			p_para->vstream_info.video_height > 1088){
			log_error("[%s]can't support h264 height exceed 1088\n", __FUNCTION__);
			p_para->vstream_info.has_video = 0; 
		}
	}

    return;
}

static int set_decode_para(play_para_t*am_p)
{
    signed short audio_index = am_p->astream_info.audio_index;
    int ret = -1;
    int rev_byte = 0;
    int total_rev_bytes = 0;
	vformat_t vfmt;
	int filter_vfmt = 0;
    unsigned char* buf;
    AVIOContext *pb = am_p->pFormatCtx->pb;

    get_stream_info(am_p);
    log_print("[%s:%d]has_video=%d vformat=%d has_audio=%d aformat=%d", __FUNCTION__, __LINE__, \
              am_p->vstream_info.has_video, am_p->vstream_info.video_format, \
              am_p->astream_info.has_audio, am_p->astream_info.audio_format);
	
	filter_vfmt = PlayerGetVFilterFormat("media.amplayer.disable-vcodecs");		
	if (((1 << am_p->vstream_info.video_format) & filter_vfmt) != 0) {
		log_error("Can't support video codec! filter_vfmt=%x vfmt=%x  (1<<vfmt)=%x\n", \
			filter_vfmt, am_p->vstream_info.video_format, (1 << am_p->vstream_info.video_format));
		return PLAYER_UNSUPPORT_VCODEC;
	}
	
	if (am_p->playctrl_info.no_video_flag) {
        set_player_error_no(am_p, PLAYER_SET_NOVIDEO);
        update_player_states(am_p, 1);
    } else if (!am_p->vstream_info.has_video) {
        if (am_p->file_type == RM_FILE) {
            log_error("Can't support rm file without video!\n");
            return PLAYER_UNSUPPORT;
        } else if (am_p->astream_info.has_audio) {
        	if(IS_VFMT_VALID(am_p->vstream_info.video_format)){
				set_player_error_no(am_p, PLAYER_UNSUPPORT_VIDEO);
	            update_player_states(am_p, 1);
        	}else{
	            set_player_error_no(am_p, PLAYER_NO_VIDEO);
	            update_player_states(am_p, 1);
        	}
        } else {
            if(IS_AFMT_VALID(am_p->astream_info.audio_format)){
				set_player_error_no(am_p, PLAYER_UNSUPPORT_AUDIO);
	            update_player_states(am_p, 1);
			}else{
	            set_player_error_no(am_p, PLAYER_NO_AUDIO);
	            update_player_states(am_p, 1);
			}
            log_error("[%s:%d]Can't support the file!\n", __FUNCTION__, __LINE__);
            return PLAYER_UNSUPPORT;
        }
    }

    if (am_p->playctrl_info.no_audio_flag) {
        set_player_error_no(am_p, PLAYER_SET_NOAUDIO);
        update_player_states(am_p, 1);
    } else if (!am_p->astream_info.has_audio) {
        if (am_p->vstream_info.has_video) {
			//log_print("[%s:%d]afmt=%d IS_AFMT_VALID(afmt)=%d\n", __FUNCTION__, __LINE__, am_p->astream_info.audio_format, IS_AFMT_VALID(am_p->astream_info.audio_format));
			if(IS_AFMT_VALID(am_p->astream_info.audio_format)){
				set_player_error_no(am_p, PLAYER_UNSUPPORT_AUDIO);
	            update_player_states(am_p, 1);
			}else{
	            set_player_error_no(am_p, PLAYER_NO_AUDIO);
	            update_player_states(am_p, 1);
			}
        } else {
            log_error("Can't support the file!\n");
            return PLAYER_UNSUPPORT;
        }
    }

	if ((am_p->stream_type == STREAM_ES) && 
		(am_p->vstream_info.video_format == VFORMAT_REAL)){
		log_print("[%s:%d]real ES not support!\n", __FUNCTION__, __LINE__);      
        return PLAYER_UNSUPPORT;
	}	
	
    if (am_p->playctrl_info.no_audio_flag) {
        am_p->astream_info.has_audio = 0;
    }

    if (am_p->playctrl_info.no_video_flag) {
        am_p->vstream_info.has_video = 0;
    }

	if (!am_p->vstream_info.has_video){
		am_p->vstream_num = 0;
	}
	
	if (!am_p->astream_info.has_audio){
		am_p->astream_num = 0;
	}

	if ((!am_p->playctrl_info.has_sub_flag) && (!am_p->sstream_info.has_sub)){
		am_p->sstream_num = 0;
	}

    am_p->sstream_info.has_sub &= am_p->playctrl_info.has_sub_flag;
    am_p->astream_info.resume_audio = am_p->astream_info.has_audio;

    if (am_p->vstream_info.has_video == 0) {
        am_p->playctrl_info.video_end_flag = 1;
    } else {
        am_p->playctrl_info.audio_end_flag = 1;
    }

    if (am_p->astream_info.has_audio) {

        if (!am_p->playctrl_info.raw_mode &&
            am_p->astream_info.audio_format == AFORMAT_AAC) {
            adts_header_t *adts_hdr;
            adts_hdr = MALLOC(sizeof(adts_header_t));
            if (adts_hdr == NULL) {
                log_print("no memory for adts_hdr\n");
                return PLAYER_NOMEM;
            }
            ret = extract_adts_header_info(am_p);
            if (ret != PLAYER_SUCCESS) {
                log_error("[%s:%d]extract adts header failed! ret=0x%x\n", __FUNCTION__, __LINE__, -ret);
                return ret;
            }
        } else if (am_p->astream_info.audio_format == AFORMAT_COOK ||
                   am_p->astream_info.audio_format == AFORMAT_RAAC) {
            log_print("[%s:%d]get real auido header info...\n", __FUNCTION__, __LINE__);
            url_fseek(pb, 0, SEEK_SET); // get cook info from the begginning of the file
            buf = MALLOC(AUDIO_EXTRA_DATA_SIZE);
            if (buf) {
                do {
                    buf += total_rev_bytes;
                    rev_byte = get_buffer(pb, buf, (AUDIO_EXTRA_DATA_SIZE - total_rev_bytes));
                    log_print("[%s:%d]rev_byte=%d total=%d\n", __FUNCTION__, __LINE__, rev_byte, total_rev_bytes);
                    if (rev_byte < 0) {
                        if (rev_byte == AVERROR(EAGAIN)) {
                            continue;
                        } else {
                            log_error("[stream_rm_init]audio codec init faile--can't get real_cook decode info!\n");
                            return PLAYER_REAL_AUDIO_FAILED;
                        }
                    } else {
                        total_rev_bytes += rev_byte;
                        if (total_rev_bytes == AUDIO_EXTRA_DATA_SIZE) {
                            if (am_p->astream_info.extradata) {
                                FREE(am_p->astream_info.extradata);
                                am_p->astream_info.extradata = NULL;
                                am_p->astream_info.extradata_size = 0;
                            }
                            am_p->astream_info.extradata = buf;
                            am_p->astream_info.extradata_size = AUDIO_EXTRA_DATA_SIZE;
                            break;
                        } else if (total_rev_bytes > AUDIO_EXTRA_DATA_SIZE) {
                            log_error("[%s:%d]real cook info too much !\n", __FUNCTION__, __LINE__);
                            return PLAYER_FAILED;
                        }
                    }
                } while (1);
            } else {
                log_error("[%s:%d]no enough memory for real_cook_info\n", __FUNCTION__, __LINE__);
                return PLAYER_NOMEM;
            }
        }

    }

	if (am_p->vstream_info.has_video) {
		if (am_p->vstream_info.video_format == VFORMAT_MJPEG && 
			am_p->vstream_info.video_width >= 1280) {
			am_p->vstream_info.discard_pkt = 1;
            log_error("[%s:%d]HD mjmpeg, discard some vpkt, rate=%d\n", __FUNCTION__, __LINE__,am_p->vstream_info.video_rate);
			am_p->vstream_info.video_rate <<= 1;
            log_error("[%s:%d]HD mjmpeg, set vrate=%d\n", __FUNCTION__, __LINE__, am_p->vstream_info.video_rate);
		}
	}

	if (am_p->sstream_info.has_sub) {
		am_p->sstream_info.sub_has_found = 1;
	}
    return PLAYER_SUCCESS;
}

static int fb_reach_head(play_para_t *para)
{
    para->playctrl_info.time_point = 0;
    set_player_state(para, PLAYER_FB_END);
    update_playing_info(para);
    update_player_states(para, 1);
    return 0;
}

static int ff_reach_end(play_para_t *para)
{
    //set_black_policy(para->playctrl_info.black_out);
    para->playctrl_info.f_step = 0;
    if (para->playctrl_info.loop_flag) {
        para->playctrl_info.time_point = 0;
        para->playctrl_info.init_ff_fr = 0;
        log_print("ff reach end,loop play\n");
    } else {
        para->playctrl_info.time_point = para->state.full_time;
        log_print("ff reach end,stop play\n");
    }
    set_player_state(para, PLAYER_FF_END);
    update_playing_info(para);
    update_player_states(para, 1);
    return 0;
}

static void player_ctrl_flag_reset(p_ctrl_info_t *cflag)
{
    cflag->video_end_flag = 0;
    cflag->audio_end_flag = 0;
    cflag->end_flag = 0;
    cflag->read_end_flag = 0;
    cflag->video_low_buffer = 0;
    cflag->audio_low_buffer = 0;
	cflag->audio_ready = 0;
    cflag->audio_switch_vmatch = 0;
    cflag->audio_switch_smatch = 0;
    //cflag->pause_flag = 0;
}

void player_clear_ctrl_flags(p_ctrl_info_t *cflag)
{
    cflag->fast_backward = 0;
    cflag->fast_forward = 0;
    cflag->search_flag = 0;
    cflag->reset_flag = 0;
    cflag->f_step = 0;
}

void player_para_reset(play_para_t *para)
{
    player_ctrl_flag_reset(&para->playctrl_info);
    if (!url_support_time_seek(para->pFormatCtx->pb)) {
        para->discontinue_point = 0;
    }
    para->discontinue_flag = 0;
    //para->playctrl_info.pts_valid = 0;
    para->playctrl_info.check_audio_ready_ms = 0;
	
    MEMSET(&para->write_size, 0, sizeof(read_write_size));
    MEMSET(&para->read_size, 0, sizeof(read_write_size));
}

int player_dec_reset(play_para_t *p_para)
{
    const stream_decoder_t *decoder;
    int ret = PLAYER_SUCCESS;
    AVFormatContext *pFormatCtx = p_para->pFormatCtx;;
    unsigned int time_point = p_para->playctrl_info.time_point;
    int64_t timestamp = 0;
    int mute_flag = 0;

    timestamp = (int64_t)time_point * AV_TIME_BASE;
    if (p_para->vstream_info.has_video
        && (timestamp != pFormatCtx->start_time)
        && (p_para->stream_type == STREAM_ES)) {
        if (p_para->astream_info.has_audio && p_para->acodec) {
            codec_audio_automute(p_para->acodec->adec_priv, 1);
            mute_flag = 1;
        }
        if (p_para->vcodec) {
            codec_set_dec_reset(p_para->vcodec);
        }
    }

    decoder = p_para->decoder;
    if (decoder == NULL) {
        log_error("[player_dec_reset:%d]decoder null!\n", __LINE__);
        return PLAYER_NO_DECODER;
    }

    if (decoder->release(p_para) != PLAYER_SUCCESS) {
        log_error("[player_dec_reset] deocder release failed!\n");
        return DECODER_RESET_FAILED;
    }
	/*make sure have enabled.*/
	if(p_para->astream_info.has_audio && p_para->vstream_info.has_video) {
		set_tsync_enable(1);
		
        p_para->playctrl_info.avsync_enable = 1;
	}else{
		set_tsync_enable(0);
        p_para->playctrl_info.avsync_enable = 0;
	}
    if (decoder->init(p_para) != PLAYER_SUCCESS) {
        log_print("[player_dec_reset] deocder init failed!\n");
        return DECODER_RESET_FAILED;
    }

    if (p_para->astream_info.has_audio && p_para->acodec) {
        p_para->codec = p_para->acodec;
        if (p_para->vcodec) {
            p_para->codec->has_video = 1;
        }
        log_print("[%s:%d]para->codec pointer to acodec!\n", __FUNCTION__, __LINE__);
    } else if (p_para->vcodec) {
        p_para->codec = p_para->vcodec;
        log_print("[%s:%d]para->codec pointer to vcodec!\n", __FUNCTION__, __LINE__);
    }

    if (p_para->playctrl_info.fast_forward) {
        if (p_para->playctrl_info.time_point >= p_para->state.full_time && 
			p_para->state.full_time > 0) {			
            p_para->playctrl_info.end_flag = 1;
			set_black_policy(p_para->playctrl_info.black_out);
			log_print("[%s]ff end: tpos=%d black=%d\n", __FUNCTION__, p_para->playctrl_info.time_point, p_para->playctrl_info.black_out);
            return ret;
        }

        log_print("[player_dec_reset]time_point=%d step=%d\n", p_para->playctrl_info.time_point, p_para->playctrl_info.f_step);
        p_para->playctrl_info.time_point += p_para->playctrl_info.f_step;
        if (p_para->playctrl_info.time_point >= p_para->state.full_time &&
			p_para->state.full_time > 0) {
            ff_reach_end(p_para);
            log_print("reach stream end,play end!\n");
        }
    } else if (p_para->playctrl_info.fast_backward) {
        if (p_para->playctrl_info.time_point == 0) {
            p_para->playctrl_info.init_ff_fr = 0;
            p_para->playctrl_info.f_step = 0;
        }
        if ((p_para->playctrl_info.time_point >= p_para->playctrl_info.f_step) && 
			(p_para->playctrl_info.time_point > 0)) {
            	p_para->playctrl_info.time_point -= p_para->playctrl_info.f_step;        	
        } else {
            fb_reach_head(p_para);
            log_print("reach stream head,fast backward stop,play from start!\n");
        }
    } else {
        if (!p_para->playctrl_info.search_flag && p_para->playctrl_info.loop_flag) {
            p_para->playctrl_info.time_point = 0;
        }
    }
    if (p_para->stream_type == STREAM_AUDIO) {
        p_para->astream_info.check_first_pts = 0;
    }

    ret = time_search(p_para);
    if (ret != PLAYER_SUCCESS) {
        log_error("[player_dec_reset]time search failed !ret = -%x\n", -ret);
    } else {
        /*clear the maybe end flags*/
        p_para->playctrl_info.audio_end_flag = 0;
        p_para->playctrl_info.video_end_flag = 0;
        p_para->playctrl_info.read_end_flag = 0;
        p_para->playctrl_info.video_low_buffer = 0;
        p_para->playctrl_info.audio_low_buffer = 0;
    }

    if (mute_flag) {
        log_print("[%s:%d]audio_mute=%d\n", __FUNCTION__, __LINE__, p_para->playctrl_info.audio_mute);
        codec_audio_automute(p_para->acodec->adec_priv, p_para->playctrl_info.audio_mute);
    }
    p_para->state.last_time = p_para->playctrl_info.time_point;
    return ret;
}
static int check_ctx_bitrate(play_para_t *p_para)
{
    AVFormatContext *ic = p_para->pFormatCtx;
    AVStream *st;
    int bit_rate = 0;
    unsigned int i;
    int flag = 0;
    for (i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (p_para->file_type == RM_FILE) {
            if (st->codec->codec_type == CODEC_TYPE_VIDEO ||
                st->codec->codec_type == CODEC_TYPE_AUDIO) {
                bit_rate += st->codec->bit_rate;
            }
        } else {
            bit_rate += st->codec->bit_rate;
        }
        if (st->codec->codec_type == CODEC_TYPE_VIDEO && st->codec->bit_rate == 0) {
            flag = -1;
        }
        if (st->codec->codec_type == CODEC_TYPE_AUDIO && st->codec->bit_rate == 0) {
            flag = -1;
        }
    }
    log_print("[check_ctx_bitrate:%d]bit_rate=%d ic->bit_rate=%d\n", __LINE__, bit_rate, ic->bit_rate);
    if (p_para->file_type == ASF_FILE) {
        if (ic->bit_rate == 0) {
            ic->bit_rate = bit_rate;
        }
    } else {
        if (bit_rate > ic->bit_rate || (ic->bit_rate - bit_rate) > 1000000000) {
            ic->bit_rate = bit_rate ;
        }
    }
    log_print("[check_ctx_bitrate:%d]bit_rate=%d ic->bit_rate=%d\n", __LINE__, bit_rate, ic->bit_rate);
    return flag;
}

static void subtitle_para_init(play_para_t *player)
{
    AVFormatContext *pCtx = player->pFormatCtx;
    int frame_rate_num, frame_rate_den;
    float video_fps;
	char out[20];
	char default_sub = "firstindex";

	if (player->vstream_info.has_video) {
        video_fps = (UNIT_FREQ) / (float)player->vstream_info.video_rate;
        set_subtitle_fps(video_fps * 100);
    }
    set_subtitle_num(player->sstream_num);

	//FFT: get proerty from build.prop
	GetSystemSettingString("media.amplayer.divx.certified", out, &default_sub);
	log_print("[%s:%d]out = %s !\n", __FUNCTION__, __LINE__, out);
	
	//FFT: set default subtitle index for divx certified
	if (strcmp(out, "enable")==0){	
		set_subtitle_enable(0);
		set_subtitle_curr(0xff);
		log_print("[%s:%d]set default subtitle index !\n", __FUNCTION__, __LINE__);
	}
    if (player->sstream_info.has_sub) {
        if (player->sstream_info.sub_type == CODEC_ID_DVD_SUBTITLE) {
            set_subtitle_subtype(0);
        } else if (player->sstream_info.sub_type == CODEC_ID_HDMV_PGS_SUBTITLE) {
            set_subtitle_subtype(1);
        } else if (player->sstream_info.sub_type == CODEC_ID_XSUB) {
            set_subtitle_subtype(2);
        } else if (player->sstream_info.sub_type == CODEC_ID_TEXT || \
                   player->sstream_info.sub_type == CODEC_ID_SSA) {
            set_subtitle_subtype(3);
        } else {
            set_subtitle_subtype(4);
        }
    } else {
        set_subtitle_subtype(0);
    }
    if (player->astream_info.start_time != -1) {
        set_subtitle_startpts(player->astream_info.start_time);
        log_print("player set startpts is 0x%llx\n", player->astream_info.start_time);
    } else if (player->vstream_info.start_time != -1) {
        set_subtitle_startpts(player->vstream_info.start_time);
        log_print("player set startpts is 0x%llx\n", player->vstream_info.start_time);
    } else {
        set_subtitle_startpts(0);
    }
}

///////////////////////////////////////////////////////////////////
int player_dec_init(play_para_t *p_para)
{
    pfile_type file_type = UNKNOWN_FILE;
    pstream_type stream_type = STREAM_UNKNOWN;
    int ret = 0;
    int full_time = 0;

    ret = ffmpeg_parse_file(p_para);
    if (ret != FFMPEG_SUCCESS) {
        log_print("[player_dec_init]ffmpeg_parse_file failed(%s)*****ret=%x!\n", p_para->file_name, ret);
        return ret;
    }
    dump_format(p_para->pFormatCtx, 0, p_para->file_name, 0);

    ret = set_file_type(p_para->pFormatCtx->iformat->name, &file_type, &stream_type);
    if (ret != PLAYER_SUCCESS) {
        set_player_state(p_para, PLAYER_ERROR);
        p_para->state.status = PLAYER_ERROR;
        log_print("[player_dec_init]set_file_type failed!\n");
        goto init_fail;
    }

    if (STREAM_ES == stream_type) {
        p_para->playctrl_info.raw_mode = 0;
    } else {
        p_para->playctrl_info.raw_mode = 1;
    }

    p_para->file_size = p_para->pFormatCtx->file_size;
	if(p_para->file_size < 0)
		p_para->pFormatCtx->valid_offset = INT64_MAX;

	if (p_para->pFormatCtx->duration != -1) {
    	p_para->state.full_time = p_para->pFormatCtx->duration / AV_TIME_BASE;
	} else {
		p_para->state.full_time = -1;
	}
		
    p_para->state.name = p_para->file_name;
    p_para->file_type = file_type;
    p_para->stream_type = stream_type;
    log_print("[player_dec_init:%d]fsize=%lld full_time=%d bitrate=%d\n", __LINE__, p_para->file_size, p_para->state.full_time, p_para->pFormatCtx->bit_rate);

    if (p_para->stream_type == STREAM_AUDIO) {
        p_para->astream_num = 1;
    } else if (p_para->stream_type == STREAM_VIDEO) {
        p_para->vstream_num = 1;
    }

    ret = set_decode_para(p_para);
    if (ret != PLAYER_SUCCESS) {
        log_error("set_decode_para failed, ret = -0x%x\n", -ret);
        goto init_fail;
    }

    if (p_para->stream_type != STREAM_TS && p_para->stream_type != STREAM_PS) {
        if (check_ctx_bitrate(p_para) == 0) {
            if ((0 != p_para->pFormatCtx->bit_rate) && (0 != p_para->file_size)) {
                full_time = (int)((p_para->file_size << 3) / p_para->pFormatCtx->bit_rate);
                log_print("[player_dec_init:%d]bit_rate=%d file_size=%lld full_time=%d\n", __LINE__, p_para->pFormatCtx->bit_rate, p_para->file_size, full_time);

                if (abs(p_para->state.full_time - full_time) > 600) {
                    p_para->state.full_time = full_time;
                }
            }
        }
    }

    if (p_para->state.full_time <= 0) {
        if (p_para->stream_type == STREAM_PS || p_para->stream_type == STREAM_TS) {
            check_ctx_bitrate(p_para);
            if ((0 != p_para->pFormatCtx->bit_rate) && (0 != p_para->file_size)) {
                p_para->state.full_time = (int)((p_para->file_size << 3) / p_para->pFormatCtx->bit_rate);
            } else {
                p_para->state.full_time = -1;
            }
        } else {
            p_para->state.full_time = -1;
        }
        if (p_para->state.full_time == -1) {
            if (p_para->pFormatCtx->pb) {
                int duration = url_ffulltime(p_para->pFormatCtx->pb);
                if (duration > 0) {
                    p_para->state.full_time = duration;
                }
            }
        }

    }
    log_print("[player_dec_init:%d]bit_rate=%d file_size=%lld file_type=%d stream_type=%d full_time=%d\n", __LINE__, p_para->pFormatCtx->bit_rate, p_para->file_size, p_para->file_type, p_para->stream_type, p_para->state.full_time);

    if (p_para->pFormatCtx->iformat->flags & AVFMT_NOFILE) {
        p_para->playctrl_info.raw_mode = 0;
    }
    if (p_para->playctrl_info.raw_mode) {
        if (p_para->pFormatCtx->bit_rate > 0) {
            p_para->max_raw_size = (p_para->pFormatCtx->bit_rate >> 3) >> 2 ; //KB/s /4
            if (p_para->max_raw_size < MIN_RAW_DATA_SIZE) {
                p_para->max_raw_size = MIN_RAW_DATA_SIZE;
            }
            if (p_para->max_raw_size > MAX_RAW_DATA_SIZE) {
                p_para->max_raw_size = MAX_RAW_DATA_SIZE;
            }
        } else {
            p_para->max_raw_size = MAX_BURST_WRITE;
        }
        log_print("====bitrate=%d max_raw_size=%d\n", p_para->pFormatCtx->bit_rate, p_para->max_raw_size);
    }
    if (p_para->playctrl_info.time_point >= 0) {
        ret = time_search(p_para);
        if (ret != PLAYER_SUCCESS) {
			set_player_state(p_para, PLAYER_ERROR);
		    ret = PLAYER_SEEK_FAILED;
            log_error("[%s:%d]time_search to pos:%ds failed!", __FUNCTION__, __LINE__, p_para->playctrl_info.time_point);
            goto init_fail;
        }
		
		if(p_para->playctrl_info.time_point < p_para->state.full_time){
			p_para->state.current_time = p_para->playctrl_info.time_point;
			p_para->state.current_ms=p_para->playctrl_info.time_point*1000;
		}
    } else if (p_para->playctrl_info.raw_mode) {
        //log_print("*****data offset 0x%x\n", p_para->data_offset);
        url_fseek(p_para->pFormatCtx->pb, p_para->data_offset, SEEK_SET);
    }
    subtitle_para_init(p_para);
    //set_tsync_enable(1);        //open av sync
    //p_para->playctrl_info.avsync_enable = 1;
    return PLAYER_SUCCESS;

init_fail:
    ffmpeg_close_file(p_para);
    return ret;
}

int player_decoder_init(play_para_t *p_para)
{
    int ret;
    const stream_decoder_t *decoder = NULL;
    decoder = find_stream_decoder(p_para->stream_type);
    if (decoder == NULL) {
        log_print("[player_dec_init]can't find decoder!\n");
        ret = PLAYER_NO_DECODER;
        goto failed;
    }
	
	if(p_para->astream_info.has_audio && p_para->vstream_info.has_video) {
		set_tsync_enable(1);
		
        p_para->playctrl_info.avsync_enable = 1;
	}else{
		set_tsync_enable(0);
        p_para->playctrl_info.avsync_enable = 0;
	}
	if(p_para->vstream_info.has_video){
		/*
		if we have video,we need to clear the pcrsrc to 0.
		if not the pcrscr maybe a big number..
		*/
		set_sysfs_str("/sys/class/tsync/pts_pcrscr","0x0");
	}
	
    if (decoder->init(p_para) != PLAYER_SUCCESS) {
        log_print("[player_dec_init] codec init failed!\n");
        ret = DECODER_INIT_FAILED;
        goto failed;
    }
    p_para->decoder = decoder;
    p_para->check_end.end_count = CHECK_END_COUNT;
	p_para->check_end.interval = CHECK_END_INTERVAL;	 
    p_para->abuffer.check_rp_change_cnt = CHECK_AUDIO_HALT_CNT;
	p_para->vbuffer.check_rp_change_cnt = CHECK_VIDEO_HALT_CNT;	

    if (p_para->astream_info.has_audio && p_para->acodec) {
        p_para->codec = p_para->acodec;
        if (p_para->vcodec) {
            p_para->codec->has_video = 1;
        }
        log_print("[%s:%d]para->codec pointer to acodec!\n", __FUNCTION__, __LINE__);
    } else if (p_para->vcodec) {
        p_para->codec = p_para->vcodec;
        log_print("[%s:%d]para->codec pointer to vcodec!\n", __FUNCTION__, __LINE__);
    }
	
	
    return PLAYER_SUCCESS;
failed:
    ffmpeg_close_file(p_para);
    return ret;
}

