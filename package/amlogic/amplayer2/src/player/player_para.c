/************************************************
 * name	: player_para.c
 * function: ffmpeg file relative and set player parameters functions
 * date		: 2010.2.4
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

static void get_av_codec_type(play_para_t *p_para)
{
	AVFormatContext *pFormatCtx = p_para->pFormatCtx;
	AVStream *pStream;
    AVCodecContext  *pCodecCtx; 
	int video_index = p_para->vstream_info.video_index;
	int audio_index = p_para->astream_info.audio_index;
    int sub_index = p_para->sstream_info.sub_index;
	log_print("[%s:%d]vidx=%d aidx=%d sidx=%d\n",__FUNCTION__,__LINE__,video_index,audio_index,sub_index);
	if(video_index != -1)
	{        
		pStream = pFormatCtx->streams[video_index];
	    pCodecCtx=pStream->codec;			
		p_para->vstream_info.video_format 	= video_type_convert(pCodecCtx->codec_id);
		if ( p_para->vstream_info.video_format == VFORMAT_SW )
			pCodecCtx->codec_tag = pCodecCtx->codec_id;
		if ( pCodecCtx->codec_id == CODEC_ID_FLV1 )
		{
			pCodecCtx->codec_tag = CODEC_TAG_F263;
			p_para->vstream_info.flv_flag = 1;
		}
		else
			p_para->vstream_info.flv_flag = 0;
		
        if(pCodecCtx->codec_id == CODEC_ID_MPEG1VIDEO)
            mpeg_check_sequence(p_para);
		
		if(p_para->stream_type == STREAM_ES)		
            p_para->vstream_info.video_codec_type= video_codec_type_convert(pCodecCtx->codec_tag);		
		else		
            p_para->vstream_info.video_codec_type= video_codec_type_convert(pCodecCtx->codec_id);			

        if(p_para->vstream_info.video_format < 0 || 
           p_para->vstream_info.video_format >= VFORMAT_MAX ||
           (((p_para->vstream_info.video_format == VFORMAT_MPEG4) ||
            (p_para->vstream_info.video_format == VFORMAT_REAL)) && 
            p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_UNKNOW))
        {
            p_para->vstream_info.has_video = 0;
			set_player_error_no(p_para, PLAYER_UNSUPPORT_VIDEO);
	        update_player_states(p_para, 1);
			set_tsync_enable(0);
        }
		if(p_para->vstream_info.has_video)
		{
	        p_para->vstream_info.video_pid 		= (unsigned short)pStream->id;	
	        if(0!=pStream->time_base.den)
	       	{
	            p_para->vstream_info.video_duration = ((float)pStream->time_base.num / pStream->time_base.den) * UNIT_FREQ;
			    p_para->vstream_info.video_pts 		= ((float)pStream->time_base.num / pStream->time_base.den) * PTS_FREQ;
	        }
			p_para->vstream_info.video_width 	= pCodecCtx->width;
		    p_para->vstream_info.video_height 	= pCodecCtx->height;
		    p_para->vstream_info.video_ratio 	= pStream->sample_aspect_ratio.num/pStream->sample_aspect_ratio.den;
	        log_print("[%s:%d]time_base=%d/%d,r_frame_rate=%d/%d\n",__FUNCTION__,__LINE__,pCodecCtx->time_base.num,pCodecCtx->time_base.den,pStream->r_frame_rate.den,pStream->r_frame_rate.num);
	        if(0!=pCodecCtx->time_base.den)
	            p_para->vstream_info.video_codec_rate = UNIT_FREQ * pCodecCtx->time_base.num / pCodecCtx->time_base.den;

	        if(0!=pStream->r_frame_rate.num)
	            p_para->vstream_info.video_rate = UNIT_FREQ * pStream->r_frame_rate.den / pStream->r_frame_rate.num;
			if ( p_para->vstream_info.video_format == VFORMAT_VC1 && p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WMV3 )
				p_para->vstream_info.video_rate = (unsigned int)(p_para->pFormatCtx->video_avg_frame_time*96/10000);
	        log_print("[%s:%d]video_codec_rate=%d,video_rate=%d\n",__FUNCTION__,__LINE__,p_para->vstream_info.video_codec_rate,p_para->vstream_info.video_rate);

	        if(p_para->vstream_info.video_format != VFORMAT_MPEG12)
	        {
	            p_para->vstream_info.extradata_size = pCodecCtx->extradata_size;
	            p_para->vstream_info.extradata      = pCodecCtx->extradata;
	        }
	        p_para->vstream_info.start_time = pStream->start_time * pStream->time_base.num * PTS_FREQ/ pStream->time_base.den;
	        //p_para->vstream_info.start_time = pStream->start_time;
	        /* added by Z.C for mov file frame duration */
	        if ((p_para->file_type == MOV_FILE) || (p_para->file_type == MP4_FILE))
	        {
	            if (pStream->nb_frames && pStream->duration && pStream->time_base.den && pStream->time_base.num)
	            {
	                unsigned int fix_rate;
	                if((0!=pStream->time_base.den) && (0!=pStream->nb_frames))
	                    fix_rate = UNIT_FREQ * pStream->duration * pStream->time_base.num / pStream->time_base.den / pStream->nb_frames;
	                p_para->vstream_info.video_rate = fix_rate;
	                log_print("[%s:%d]video_codec_rate=%d,video_rate=%d\n",__FUNCTION__,__LINE__,p_para->vstream_info.video_codec_rate,p_para->vstream_info.video_rate);

	            }
	        }
	        else if (p_para->file_type == FLV_FILE)
	        {
	            if (pStream->special_fps > 0)
	                p_para->vstream_info.video_rate = UNIT_FREQ / pStream->special_fps;
	        }
		}
	}
	else
	{
        log_print("no video specified!\n");
	}
	if(audio_index != -1)
	{       
		pStream = pFormatCtx->streams[audio_index];
	    pCodecCtx=pStream->codec;	 
		p_para->astream_info.audio_pid 		= (unsigned short)pStream->id;	  
		p_para->astream_info.audio_format 	= audio_type_convert(pCodecCtx->codec_id, p_para->file_type);
        if(p_para->astream_info.audio_format < 0 || p_para->astream_info.audio_format >= AFORMAT_MAX)
        {
			p_para->astream_info.has_audio = 0;			
            set_player_error_no(p_para, PLAYER_UNSUPPORT_AUDIO);
            update_player_states(p_para, 1);
            set_tsync_enable(0);        
        }
		if(p_para->astream_info.has_audio)
		{
	        if(0!=pStream->time_base.den)
			    p_para->astream_info.audio_duration = PTS_FREQ * ((float)pStream->time_base.num / pStream->time_base.den);	       
	        p_para->astream_info.start_time = pStream->start_time * pStream->time_base.num * PTS_FREQ/ pStream->time_base.den;
	        //p_para->vstream_info.start_time = pStream->start_time;
		}
    }
	else
	{
        log_print("no audio specified!\n");    
	}
    if(sub_index != -1)
    {
        pStream = pFormatCtx->streams[sub_index];
        p_para->sstream_info.sub_pid = (unsigned short)pStream->id;
        p_para->sstream_info.sub_type = pStream->codec->codec_id;
        if (pStream->time_base.num &&(0!=pStream->time_base.den))
        {
            p_para->sstream_info.sub_duration = UNIT_FREQ * ((float)pStream->time_base.num / pStream->time_base.den);
            p_para->sstream_info.sub_pts = PTS_FREQ * ((float)pStream->time_base.num / pStream->time_base.den);
            p_para->sstream_info.start_time = pStream->start_time * pStream->time_base.num * PTS_FREQ/ pStream->time_base.den;
        }
        else
        {
            p_para->sstream_info.start_time = pStream->start_time * PTS_FREQ;
        }
    }
    return;
}

static void get_stream_info(play_para_t *p_para)
{  
	int i;   	
	AVFormatContext *pFormat = p_para->pFormatCtx;
    AVStream *pStream;
    AVCodecContext *pCodec;
	int video_index = p_para->vstream_info.video_index;
	int audio_index = p_para->astream_info.audio_index;
    int sub_index = p_para->sstream_info.sub_index;
    int temp_vidx=-1,temp_aidx=-1, temp_sidx=-1;
    int bitrate = 0;	      

	/* caculate the stream numbers */    
	p_para->vstream_num = 0;    
	p_para->astream_num = 0;
    p_para->sstream_num = 0;
    
	for (i=0; i<pFormat->nb_streams; i++)    
	{   
	    pStream = pFormat->streams[i];
        pCodec = pStream->codec;
		if(pCodec->codec_type == CODEC_TYPE_VIDEO)        
		{   
            p_para->vstream_num ++;
		    if (p_para->file_type == RM_FILE)
	        {
	            /* find max bitrate */
                if (pCodec->bit_rate > bitrate) 
                {
                    /* only support RV30 and RV40 */
                    if ((pCodec->codec_id == CODEC_ID_RV30)
                        || (pCodec->codec_id == CODEC_ID_RV40))
                    {
                        bitrate = pCodec->bit_rate;
                        temp_vidx = i;
                    }
                }
	        }
            else
			{
			    if(temp_vidx == -1)		 		
				    temp_vidx = i;
            }
		}     
		else if(pCodec->codec_type == CODEC_TYPE_AUDIO)    
		{
            p_para->astream_num ++;
		    if (p_para->file_type == RM_FILE)
	        {
	            if ((temp_aidx == -1)
                    && (CODEC_ID_SIPR != pCodec->codec_id)) // SIPR not supported now
                {
                    temp_aidx = i;
                }
	        }
            else
			{
			    if(temp_aidx==-1)	            
				    temp_aidx = i;
            }
		} 
        else if(pCodec->codec_type == CODEC_TYPE_SUBTITLE)
        {			
            p_para->sstream_num ++;
            if(temp_sidx == -1)
                temp_sidx = i;
        }
	}  
    
	if(p_para->vstream_num >= 1)	
		p_para->vstream_info.has_video = 1;	
	else
		p_para->vstream_info.has_video = 0;

	if(p_para->astream_num >= 1)	
		p_para->astream_info.has_audio= 1;	
	else
		p_para->astream_info.has_audio= 0;

	
    if(p_para->sstream_num >= 1)    
		p_para->sstream_info.has_sub = 1;    
    else
        p_para->sstream_info.has_sub = 0;

	if ((p_para->vstream_num >= 1) || 
        (p_para->astream_num >= 1) || 
        (p_para->sstream_num >= 1))    
	{	  
		if((video_index > (p_para->vstream_num + p_para->astream_num)) || (video_index < 0))			
			video_index = temp_vidx;	   
		
		if(audio_index > (p_para->vstream_num + p_para->astream_num) || audio_index< 0)			
			audio_index = temp_aidx;

        if((sub_index > p_para->sstream_num) || (sub_index<0))
            sub_index = temp_sidx;
	}
	if(p_para->astream_info.has_audio)
	{
		p_para->astream_info.audio_channel = pFormat->streams[audio_index]->codec->channels;			
		p_para->astream_info.audio_samplerate = pFormat->streams[audio_index]->codec->sample_rate;    
	}
    
	p_para->vstream_info.video_index = video_index;
	p_para->astream_info.audio_index = audio_index;
    p_para->sstream_info.sub_index = sub_index;
    if (p_para->sstream_info.has_sub)
        log_print("Subtitle index %d detected\n", sub_index);
	get_av_codec_type(p_para);    
	return;
}

static int set_decode_para(play_para_t*am_p)
{
	AVFormatContext *pFCtx = am_p->pFormatCtx;
	AVCodecContext  *pCodecCtx;		
	signed short audio_index = am_p->astream_info.audio_index;   
    get_stream_info(am_p);	     
    if((am_p->vstream_info.video_format == -1)&&(am_p->astream_info.audio_format == -1))
    {        
        log_error("Can't support the video_format and audio_format!\n");
        return PLAYER_UNSUPPORT;
    }
	else if((!am_p->vstream_info.has_video) && (am_p->file_type == RM_FILE))
	{
		log_error("Can't support rm audio file!\n");
        return PLAYER_UNSUPPORT;
	}
    if(am_p->playctrl_info.no_audio_flag)        am_p->astream_info.has_audio = 0;
    if(am_p->playctrl_info.no_video_flag)        am_p->vstream_info.has_video = 0;
    
    am_p->sstream_info.has_sub &= am_p->playctrl_info.has_sub_flag;
    am_p->astream_info.resume_audio = am_p->astream_info.has_audio;

    if(am_p->vstream_info.has_video == 0)    
        am_p->playctrl_info.video_end_flag = 1;     
	
    if(am_p->astream_info.has_audio == 0)    
        am_p->playctrl_info.audio_end_flag = 1;        
         
	if(am_p->astream_info.has_audio)
	{       
        audio_index = am_p->astream_info.audio_index;          
		if(audio_index == -1)
			pCodecCtx= pFCtx->streams[0]->codec;		    
		else
			pCodecCtx= pFCtx->streams[audio_index]->codec;
		am_p->astream_info.audio_channel = pCodecCtx->channels;		    
		am_p->astream_info.audio_samplerate = pCodecCtx->sample_rate;	
        
        if(!am_p->playctrl_info.raw_mode&&am_p->astream_info.audio_format == AFORMAT_AAC)
    	{            
            adts_header_t *adts_hdr;            
            adts_hdr = MALLOC(sizeof(adts_header_t));      
            if(adts_hdr == NULL)
            {
                log_print("no memory for adts_hdr\n");
                return PLAYER_NOMEM;
            }             
    		extract_adts_header_info(pCodecCtx->extradata, pCodecCtx->extradata_size,adts_hdr);                        
            am_p->astream_info.adts_header=adts_hdr;           
    	}       
	}    
	return PLAYER_SUCCESS;
}

static int fb_reach_head(play_para_t *para)
{     
    para->playctrl_info.time_point = 0;      
    set_player_state(para,PLAYER_FB_END);
    update_playing_info(para); 
    update_player_states(para,1);   
    return 0;
}

static int ff_reach_end(play_para_t *para)
{    
    set_black_policy(para->playctrl_info.black_out);
    para->playctrl_info.f_step = 0;
    if(para->playctrl_info.loop_flag)
    {                
        para->playctrl_info.time_point = 0;        
        para->playctrl_info.init_ff_fr = 0;
        log_print("ff reach end,loop play\n");
    }
    else
    {        
        para->playctrl_info.time_point = para->state.full_time;
        log_print("ff reach end,stop play\n");
    }   
    set_player_state(para,PLAYER_FF_END);
    update_playing_info(para); 
    update_player_states(para,1); 
    return 0;
}

static void player_ctrl_flag_reset(p_ctrl_info_t *cflag)
{	
    cflag->video_end_flag = 0;
    cflag->audio_end_flag = 0;
    cflag->end_flag = 0;
    cflag->read_end_flag = 0;
    cflag->pause_flag = 0;
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
        && (p_para->stream_type == STREAM_ES))
    {   
		if(p_para->astream_info.has_audio)
        {
       		codec_audio_automute(1);
			mute_flag = 1;
		}
        if (p_para->codec)
            codec_set_dec_reset(p_para->codec);
        else if (p_para->vcodec)
            codec_set_dec_reset(p_para->vcodec);
    }
    
    decoder = p_para->decoder;
    if(decoder == NULL)
    {
        log_error("[player_dec_reset:%d]decoder null!\n", __LINE__);
        return PLAYER_NO_DECODER;
    }

    if(decoder->release(p_para) != PLAYER_SUCCESS)		
	{
		log_print("[player_dec_reset] deocder release failed!\n");
		return DECODER_RESET_FAILED;
	}   
    
	if(decoder->init(p_para) != PLAYER_SUCCESS)		
	{
		log_print("[player_dec_reset] deocder init failed!\n");
		return DECODER_RESET_FAILED;
	}   
    
    if(p_para->playctrl_info.fast_forward)
    {
        if(p_para->playctrl_info.time_point >= p_para->state.full_time)
        {
            p_para->playctrl_info.end_flag = 1;
            return ret;
        }
        
        log_print("[player_dec_reset]time_point=%d step=%d\n",p_para->playctrl_info.time_point,p_para->playctrl_info.f_step);
        p_para->playctrl_info.time_point += p_para->playctrl_info.f_step;        
        if(p_para->playctrl_info.time_point >= p_para->state.full_time)
        {
            ff_reach_end(p_para);           
            log_print("reach stream end,play end!\n");          
        }
    }
    else if (p_para->playctrl_info.fast_backward)
    {
        if(p_para->playctrl_info.time_point == 0)
        {
            p_para->playctrl_info.init_ff_fr = 0;            
            p_para->playctrl_info.f_step = 0;
        }
        if(p_para->playctrl_info.time_point >= p_para->playctrl_info.f_step)
        {
            p_para->playctrl_info.time_point -= p_para->playctrl_info.f_step;              
        }
        else
        {
            fb_reach_head(p_para);
            log_print("reach stream head,fast backward stop,play from start!\n"); 
        }
    }
    else
    {        
        if( !p_para->playctrl_info.search_flag && p_para->playctrl_info.loop_flag)
            p_para->playctrl_info.time_point = 0;       
    }
    p_para->state.last_time = 0;
    if(p_para->stream_type == STREAM_AUDIO)
        p_para->astream_info.check_first_pts = 0;
    
    ret = time_search(p_para);    

    if (mute_flag)
    {        
        log_print("[%s:%d]audio_mute=%d\n",__FUNCTION__, __LINE__,p_para->playctrl_info.audio_mute);
        codec_audio_automute(p_para->playctrl_info.audio_mute);
    }
    
    return ret;     
}
static void check_ctx_bitrate(AVFormatContext *ic)
{
    AVStream *st;
    int bit_rate = 0;
    int i;    
    for(i=0;i<ic->nb_streams;i++) 
    {
        st = ic->streams[i];
        bit_rate += st->codec->bit_rate;
    }
    //log_print("[check_ctx_bitrate:%d]bit_rate=%d ic->bit_rate=%d\n",__LINE__,bit_rate, ic->bit_rate);
    if(bit_rate > ic->bit_rate)
        ic->bit_rate = bit_rate ;   
    //log_print("[check_ctx_bitrate:%d]bit_rate=%d ic->bit_rate=%d\n",__LINE__,bit_rate, ic->bit_rate);
    
}
///////////////////////////////////////////////////////////////////
int player_dec_init(play_para_t *p_para)
{
	pfile_type file_type;
	pstream_type stream_type;		
    int ret;
    int full_time;

    ret = ffmpeg_parse_file(p_para);
	if(ret != FFMPEG_SUCCESS)
	{	
		log_print("[player_dec_init]ffmpeg_parse_file failed(%s)*****ret=%x!\n",p_para->file_name,ret);
        return ret;	
	}    
    dump_format(p_para->pFormatCtx, 0,p_para->file_name, 0);         
    ret = set_file_type(p_para->pFormatCtx->iformat->name, &file_type, &stream_type);
	if(ret != PLAYER_SUCCESS)
	{
		set_player_state(p_para,PLAYER_ERROR);
        p_para->state.status = PLAYER_ERROR;
		log_print("[player_dec_init]set_file_type failed!\n");
		goto init_fail;		
	}
    if(STREAM_ES == stream_type)
        p_para->playctrl_info.raw_mode = 0;
    else
        p_para->playctrl_info.raw_mode = 1;

    p_para->file_size=p_para->pFormatCtx->file_size;    
    p_para->state.full_time = p_para->pFormatCtx->duration/AV_TIME_BASE;   
    p_para->state.name = p_para->file_name;
	p_para->file_type = file_type;
	p_para->stream_type = stream_type;
    
    if((p_para->stream_type!=STREAM_TS) && (p_para->stream_type!=STREAM_PS))
    {
        check_ctx_bitrate(p_para->pFormatCtx);
        if(0!=p_para->pFormatCtx->bit_rate)
            full_time = (int)((p_para->file_size<<3)/p_para->pFormatCtx->bit_rate);	        
        if((p_para->state.full_time - full_time) > 30)
            p_para->state.full_time = full_time;        
    }	
    log_print("--[player_dec_init:%d]bit_rate=%d file_size=%lld file_type=%d stream_type=%d\n",__LINE__,p_para->pFormatCtx->bit_rate,p_para->file_size,p_para->file_type,p_para->stream_type);

    if(p_para->stream_type == STREAM_AUDIO)
        p_para->astream_num = 1;   
    else if(p_para->stream_type == STREAM_VIDEO)
        p_para->vstream_num = 1;   
    
    ret = set_decode_para(p_para);
    if(ret!=PLAYER_SUCCESS)
        goto init_fail;
    
    return PLAYER_SUCCESS;

init_fail:	
	if(p_para->pFormatCtx != NULL)
	{       
        av_close_input_file(p_para->pFormatCtx); 
		p_para->pFormatCtx=NULL;
	}
	return ret;
}

int player_decoder_init(play_para_t *p_para)
{
    int ret;
    const stream_decoder_t *decoder = NULL;
    decoder = find_stream_decoder(p_para->stream_type);       
	if(decoder== NULL)
	{
		log_print("[player_dec_init]can't find decoder!\n");
        ret = PLAYER_NO_DECODER;
		goto failed;
	}
	if(decoder->init(p_para) != PLAYER_SUCCESS)		
	{
		log_print("[player_dec_init] codec init failed!\n");
        ret = DECODER_INIT_FAILED;
		goto failed;
	}	
    if(p_para->pFormatCtx->iformat->flags & AVFMT_NOFILE) 
    {
        p_para->playctrl_info.raw_mode = 0;
    }
    if(p_para->playctrl_info.raw_mode)
	{
	    //log_print("*****data offset 0x%x\n", p_para->data_offset);
	    url_fseek(p_para->pFormatCtx->pb,p_para->data_offset, SEEK_SET);
    }   
	p_para->decoder=decoder;    
    p_para->check_end.end_count = CHECK_END_COUNT;  
    if(p_para->playctrl_info.raw_mode)
    {
        if(p_para->pFormatCtx->bit_rate > 0)
        {
            p_para->max_raw_size = (p_para->pFormatCtx->bit_rate >> 3)>> 2 ; //KB/s /4     
            if(p_para->max_raw_size < MIN_RAW_DATA_SIZE)
                p_para->max_raw_size = MIN_RAW_DATA_SIZE;
            if(p_para->max_raw_size > MAX_RAW_DATA_SIZE)
                p_para->max_raw_size = MAX_RAW_DATA_SIZE;
        }
        else
            p_para->max_raw_size = MAX_BURST_WRITE;
        log_print("====bitrate=%d max_raw_size=%d\n",p_para->pFormatCtx->bit_rate,p_para->max_raw_size);
    } 

    return PLAYER_SUCCESS;
failed:   
	if(p_para->pFormatCtx != NULL)
	{       
        av_close_input_file(p_para->pFormatCtx); 
		p_para->pFormatCtx=NULL;
	}
	return ret;
}

