/***************************************************
 * name	    : player_update.c
 * function : update player parameters, information, status etc. 
 * date		:  2010.3.2
 ***************************************************/
#include <fcntl.h> 
#include <stdlib.h>
#include <sys/time.h> 
#include <string.h>
#include <player_set_sys.h>

#include "player_update.h"
#include "thread_mgt.h"


void media_info_init(media_info_t *info)
{
    MEMSET(info, 0, sizeof(media_info_t));
    info->stream_info.filename = NULL;       
    info->stream_info.cur_audio_index   = -1;   
    info->stream_info.cur_sub_index     = -1;
    info->stream_info.cur_audio_index   = -1;
    info->stream_info.type              = UNKNOWN_FILE;
}
static int set_stream_info(play_para_t *p_para)
{
    mstream_info_t *info = &p_para->media_info.stream_info;
    AVFormatContext *ctx = p_para->pFormatCtx;
    if(!info || !ctx)
        return -1;
    info->bitrate   = ctx->bit_rate;
    info->duration  = (int)(ctx->duration/AV_TIME_BASE);
    info->filename  = p_para->file_name;
    info->file_size = p_para->file_size;
    info->type      = p_para->file_type;
    info->has_video = p_para->vstream_info.has_video;
    info->has_audio = p_para->astream_info.has_audio;
    info->has_sub   = p_para->sstream_info.has_sub;
    info->nb_streams        = ctx->nb_streams;
    info->total_video_num   = p_para->vstream_num;
    info->total_audio_num   = p_para->astream_num;
    info->total_sub_num     = p_para->sstream_num;
    if(info->total_video_num >= MAX_VIDEO_STREAMS)
    {
        log_error("[set_stream_info]too much video streams(%d)!\n ", info->total_video_num );
        return -2;
    }
    if(info->total_audio_num >= MAX_AUDIO_STREAMS)
    {
        log_error("[set_stream_info]too much audio streams(%d)!\n ", info->total_audio_num );
        return -3;
    }
    if(info->total_audio_num >= MAX_SUB_STREAMS)
    {
        log_error("[set_stream_info]too much sub streams(%d)!\n ", p_para->astream_num);
        return -4;
    }    
    info->cur_video_index   = p_para->vstream_info.video_index;
    info->cur_audio_index   = p_para->astream_info.audio_index;
    info->cur_sub_index     = p_para->sstream_info.sub_index;
    return 0;
}

static int set_vstream_info(play_para_t *p_para)
{   
    mstream_info_t *info = &p_para->media_info.stream_info;    
    mvideo_info_t *vinfo;
    AVFormatContext *pCtx = p_para->pFormatCtx;
    if(!info || !pCtx)
        return -1;
    if(info->has_video)
    {
        int i; 
        int vnum = 0;
        AVStream *pStream;        
        for(i = 0;i < pCtx->nb_streams;i ++)
        {           
            pStream = pCtx->streams[i];      
            if(pStream->codec->codec_type == CODEC_TYPE_VIDEO)
            {    
                vinfo = MALLOC(sizeof(mvideo_info_t));
                MEMSET(vinfo, 0, sizeof(mvideo_info_t));
                vinfo->id          = pStream->id;              
                vinfo->width       = pStream->codec->width;
                vinfo->height      = pStream->codec->height;
                vinfo->duartion    = (int)(pStream->duration*pStream->time_base.num/pStream->time_base.den);
                vinfo->bit_rate    = pStream->codec->bit_rate;
                vinfo->format      = p_para->vstream_info.video_format;
                vinfo->aspect_ratio_num = pStream->sample_aspect_ratio.num;
                vinfo->aspect_ratio_den = pStream->sample_aspect_ratio.den;
                vinfo->frame_rate_num   = pStream->r_frame_rate.num;
                vinfo->frame_rate_den   = pStream->r_frame_rate.den;
                p_para->media_info.video_info[vnum] = vinfo;
                vnum ++;
                if(vnum > p_para->media_info.stream_info.total_video_num)
                {
                    log_error("[set_vstream_info]video streams exceed!\n");
                    return -2;
                }
            } 
        }
    }    
    return 0;
}
static int metadata_set_string(AVFormatContext *s, const char *key,
                            char *buf, int buf_size)
{
    AVMetadataTag *tag;
    if ((tag = av_metadata_get(s->metadata, key, NULL, 0)))
        strncpy(buf, tag->value, buf_size);
    return !!tag;
}
static int get_id3v1_tag(AVFormatContext *s, audio_tag_info *audio_tag)
{
    AVMetadataTag *tag;
    int count = 0;

	if (!audio_tag->title[0])
	    count += metadata_set_string(s, "title",   audio_tag->title, 30);
	if (!audio_tag->author[0])
	    count += metadata_set_string(s, "author",  audio_tag->author, 30);
	if (!audio_tag->album[0])
    	count += metadata_set_string(s, "album",   audio_tag->album, 30);
	if (!audio_tag->year[0])
	    count += metadata_set_string(s, "year",    audio_tag->year,  4);
	if (!audio_tag->comment[0])
	    count += metadata_set_string(s, "comment", audio_tag->comment, 30);
	if (!audio_tag->genre[0])
	    count += metadata_set_string(s, "genre", audio_tag->genre, 32);
	
    if ((tag = av_metadata_get(s->metadata, "track", NULL, 0))) {
		if (!audio_tag->track)
	        audio_tag->track = atoi(tag->value);
        count++;
    }
    return count;
}
static int get_id3v2_tag(AVFormatContext *s,audio_tag_info *audio_tag)
{
    AVMetadataTag *tag;
    int count = 0;	
	
	if (!audio_tag->title[0])
	    count += metadata_set_string(s, "TIT2",   audio_tag->title, 512);
	if (!audio_tag->author[0])
    	count += metadata_set_string(s, "TPE1",  audio_tag->author, 512);
	if (!audio_tag->album[0])
    	count += metadata_set_string(s, "TALB",   audio_tag->album, 512);
	if (!audio_tag->year[0])
    	count += metadata_set_string(s, "TYER",    audio_tag->year,  4);
	if (!audio_tag->comment[0])
    	count += metadata_set_string(s, "COMM", audio_tag->comment, 512);
	if (!audio_tag->genre[0])
    	count += metadata_set_string(s, "TCON", audio_tag->genre, 32);
    	
    if ((tag = av_metadata_get(s->metadata, "TRCK", NULL, 0))) {
		if (!audio_tag->track)
        	audio_tag->track = atoi(tag->value);
        count++;
    }
    return count;
}
static void get_tag_from_metadata(AVFormatContext *s, audio_tag_info *tag)
{   

	get_id3v2_tag(s, tag);
	get_id3v1_tag(s, tag);
}
static int set_astream_info(play_para_t *p_para)
{   
    mstream_info_t *info = &p_para->media_info.stream_info;    
    maudio_info_t *ainfo;
    AVFormatContext *pCtx = p_para->pFormatCtx;
    if(!info || !pCtx)
        return -1;
    if(info->has_audio)
    {
        int i; 
        int anum = 0;
        AVStream *pStream;        
        for(i = 0;i < pCtx->nb_streams;i ++)
        {           
            pStream = pCtx->streams[i];      
            if(pStream->codec->codec_type == CODEC_TYPE_AUDIO)
            {    
                ainfo = MALLOC(sizeof(maudio_info_t));
                MEMSET(ainfo, 0, sizeof(maudio_info_t));
                ainfo->id           = pStream->id;              
                ainfo->channel      = p_para->astream_info.audio_channel;
                ainfo->sample_rate  = p_para->astream_info.audio_samplerate;
                ainfo->duration     = (int)(pStream->duration*pStream->time_base.num/pStream->time_base.den);
                ainfo->bit_rate     = pStream->codec->bit_rate;
                ainfo->aformat      = p_para->astream_info.audio_format;
                if(p_para->stream_type == STREAM_AUDIO)
                {
                    if(ainfo->bit_rate == 0)
                        ainfo->bit_rate = info->bitrate;
                    ainfo->audio_tag = MALLOC(sizeof(audio_tag_info));         
                    get_tag_from_metadata(pCtx, ainfo->audio_tag);

                }
                p_para->media_info.audio_info[anum] = ainfo;
                anum ++;
                if(anum > p_para->media_info.stream_info.total_audio_num)
                {
                    log_error("[set_vstream_info]video streams exceed!\n");
                    return -2;
                }
            } 
        }
    }    
    return 0;
}

static int set_sstream_info(play_para_t *p_para)
{   
    mstream_info_t *info = &p_para->media_info.stream_info;    
    msub_info_t *sinfo;
    AVFormatContext *pCtx = p_para->pFormatCtx;
    if(!info || !pCtx)
        return -1;
    if(info->has_sub)
    {
        int i; 
        int snum = 0;
        AVStream *pStream;    
        for(i = 0;i < pCtx->nb_streams;i ++)
        {           
            pStream = pCtx->streams[i];      
            if(pStream->codec->codec_type == CODEC_TYPE_SUBTITLE)
            {    
				AVMetadataTag *lang = av_metadata_get(pStream->metadata, "language", NULL, 0);
                sinfo = MALLOC(sizeof(msub_info_t));
                MEMSET(sinfo, 0, sizeof(msub_info_t));
                sinfo->id       = pStream->id;              
                sinfo->internal_external = 0; 
				sinfo->width	= pStream->codec->width;
			    sinfo->height	= pStream->codec->height;			   	
			    //sinfo->subtitle_size;  
			    if(lang)
			    	sinfo->sub_language = lang->value;   
                p_para->media_info.sub_info[snum] = sinfo;
                snum ++;
                if(snum > p_para->media_info.stream_info.total_sub_num)
                {
                    log_error("[set_sstream_info]sub streams exceed!\n");
                    return -2;
                }
            } 
        }
    }    
    return 0;
}

int set_media_info(play_para_t *p_para)
{
    int ret = -1;	
	media_info_init(&p_para->media_info);

    ret = set_stream_info(p_para);
    if(ret < 0)
        log_print("[set_media_info]set_stream_info failed!\n");
	
    ret = set_vstream_info(p_para);
    if(ret < 0)
        log_print("[set_media_info]set_vstream_info failed!\n");
    
    ret = set_astream_info(p_para);
    if(ret < 0)
        log_print("[set_media_info]set_astream_info failed ret=%d!\n", ret);
    
        
    ret = set_sstream_info(p_para);
    if(ret < 0)
        log_print("[set_media_info]set_sstream_info failed ret=%d!\n", ret);
    
	return 0;
}


static int check_vcodec_state(codec_para_t *codec, struct vdec_status *dec, struct buf_status *buf)
{
	int ret = 0;	
	
	ret = codec_get_vbuf_state(codec,  buf);
	if(ret != 0)	
		log_print("codec_get_vbuf_state error: %x\n", -ret);			
	
	ret = codec_get_vdec_state(codec, dec);
	if(ret != 0)
	{	
		log_print("codec_get_vdec_state error: %x\n", -ret);				
		ret = PLAYER_CHECK_CODEC_ERROR;
	}
	
	return ret;
}

static int check_acodec_state(codec_para_t *codec, struct adec_status *dec, struct buf_status *buf)
{
	int ret = PLAYER_SUCCESS;		

	ret = codec_get_abuf_state(codec,  buf);
	if(ret != 0)	
		log_print("codec_get_abuf_state error: %x\n", -ret);			
	
	ret = codec_get_adec_state(codec, dec);
	if(ret != 0)
	{
		log_print("codec_get_adec_state error: %x\n", -ret);		
		ret = PLAYER_FAILED;
	}
	return 0;
}
static int update_codec_info(play_para_t *p_para,
								struct buf_status *vbuf,
								struct buf_status *abuf,
								struct vdec_status *vdec,
								struct adec_status *adec)
{
	codec_para_t 	*vcodec = NULL;
    codec_para_t 	*acodec = NULL;
	if(p_para->codec)
	{
    	vcodec=p_para->codec;
    	acodec=p_para->codec;
	}
	else
	{
    	if(p_para->vcodec)
    		vcodec=p_para->vcodec;
    	if(p_para->acodec)
    		acodec=p_para->acodec;
	}
	if(vcodec && p_para->vstream_info.has_video)
	{     
		if ( p_para->vstream_info.video_format != VFORMAT_SW )
		{
	    	if(check_vcodec_state(vcodec, vdec, vbuf) != 0)
			{
				log_print("check_vcodec_state error!\n");
				return PLAYER_FAILED;
			}    
		}
	}
	if(acodec && p_para->astream_info.has_audio)
	{
    	if(check_acodec_state(acodec, adec, abuf) != 0)
		{
			log_print("check_acodec_state error!\n");
			return PLAYER_FAILED;
		}
	}
	return 0;
}
static unsigned int handle_current_time(play_para_t *para, unsigned int scr, unsigned int pts)
{       
    player_status sta = get_player_state(para);

    log_debug("[handle_current_time:sta=%d]scr=%u pts=%u\n",sta,scr,pts);
       
    if(sta == PLAYER_STOPED || sta == PLAYER_INITING)        
        return 0;
    
    if(!para->playctrl_info.pts_valid)
    {
        if(scr == pts || (scr-pts) <= PTS_FREQ)         
            para->playctrl_info.pts_valid = 1;    
    }
    
    if(para->playctrl_info.pts_valid)
        return scr;
    else
        return 0;
    
}

unsigned int get_pts_pcrscr()
{
    int handle;
    int size;
    char s[16];
    unsigned int value;
    
    handle = open("/sys/class/tsync/pts_pcrscr", O_RDONLY);      
    if(handle < 0)
    {
        log_print("[player_get_ctime]open pts_pcrscr error!\n");
        return -1;
    }
    size = read(handle, s, sizeof(s));         
    if(size > 0)
    {
        value = strtoul(s, NULL, 16);         
        log_debug("\npcrscr=%x(%d) ",value,value/PTS_FREQ);
    }    
    close(handle);  
    return value;
}

unsigned int get_pts_video()
{
    int handle;
    int size;
    char s[16];
    unsigned int value;
    handle = open("/sys/class/tsync/pts_video", O_RDONLY);      
    if(handle < 0)
    {
        log_print("[player_get_ctime]open pts_pcrscr error!\n");
        return -1;
    }
    size = read(handle, s, sizeof(s));         
    if(size > 0)
    {
        value = strtoul(s, NULL, 16);         
        log_debug("video=%x(%d)\n",value,value/PTS_FREQ);
    }    
    close(handle);
    return value;
}

static unsigned int get_pts_audio()
{
    int handle;
    int size;
    char s[16];
    unsigned int value;
    handle = open("/sys/class/tsync/pts_audio", O_RDONLY);    
    if(handle < 0)
    {
        log_print("[player_get_ctime]open pts_audio error!\n");
        return -1;
    }
    size = read(handle, s, sizeof(s));         
    if(size > 0)
    {
        value = strtoul(s, NULL, 16);         
        log_debug("audio=%x(%d)\n",value,value/PTS_FREQ);
    }    
    close(handle);
    return value;
}


static unsigned int get_current_time(play_para_t *p_para)
{        
    
    unsigned int pcr_scr = 0, vpts = 0, apts = 0; 
    unsigned int ctime = 0;
    
    #define REFRESH_CURTIME_INTERVAL    (100)  
    
    if(check_time_interrupt(&p_para->state.curtime_old_time,REFRESH_CURTIME_INTERVAL))
    {   
        if(p_para->vstream_info.has_video && p_para->astream_info.has_audio)
        {            
            pcr_scr = get_pts_pcrscr();
            apts = get_pts_audio();                
            ctime = handle_current_time(p_para, pcr_scr,apts);
            log_debug("***[get_current_time:%d]ctime=%u\n",__LINE__,ctime);
        }
        else if (!p_para->astream_info.has_audio)
        {
            vpts = get_pts_video();
            ctime = vpts;
        }
        else
        {
            pcr_scr = get_pts_pcrscr();
            ctime = pcr_scr;
        }
        p_para->state.last_pcr = ctime;
        log_debug("[get_current_time:%d]last_pcr=%u\n",__LINE__,ctime);
	}      
    else
    {       
        ctime = p_para->state.last_pcr;
    } 
    log_debug("===[get_current_time] current_time=%x (%d)\n",ctime, ctime/PTS_FREQ);
    return ctime;
}
static void update_current_time(play_para_t *p_para)
{
    unsigned int time = 0;  
    unsigned int start_time = 0;
	if(p_para->state.status!=PLAYER_PLAYEND)
    	p_para->state.last_sta = p_para->state.status;
	p_para->state.status=get_player_state(p_para);
    if(p_para->playctrl_info.f_step > 0)   
    {
        time = p_para->playctrl_info.time_point;
    }
    else
    {        
        time = get_current_time(p_para);  
        
        if(p_para->astream_info.start_time != INT64_0)
        {
            start_time = (unsigned int)p_para->astream_info.start_time;                  
        }           
        else if(p_para->vstream_info.start_time != INT64_0)
        {
            start_time = (unsigned int)p_para->vstream_info.start_time;                   
        }                
        //log_print("[update_current_time]time=%d astart_time=%d  vstart_time=%d\n",time/PTS_FREQ,start_time/PTS_FREQ,((unsigned int)p_para->vstream_info.start_time/PTS_FREQ));
       
        if(time > start_time) 
            time -= start_time;          

        //log_debug("[update_current_time]time=%d curtime=%d lasttime=%d\n",time/PTS_FREQ,p_para->state.current_time,p_para->state.last_time);

        time /= PTS_FREQ;        
    }
    p_para->state.current_time = (int)time;
    if(p_para->state.current_time > p_para->state.full_time)   
        p_para->state.current_time = p_para->state.full_time;  
    if(p_para->state.current_time < p_para->state.last_time)   
        p_para->state.current_time = p_para->state.last_time;  
}

static void update_dec_info(play_para_t *p_para,struct vdec_status *vdec,struct adec_status *adec)
{
    if(p_para->vstream_info.has_video)
    {
        if(p_para->vstream_info.video_width == 0)
        {
            p_para->vstream_info.video_width = vdec->width;
            p_para->vstream_info.video_height= vdec->height;
        }
        p_para->state.video_error_cnt = vdec->error_count;
    }   
    if(p_para->astream_info.has_audio)
        p_para->state.audio_error_cnt = adec->error_count;
}

static void check_avbuf_end(play_para_t *p_para, struct buf_status *vbuf,struct buf_status *abuf)
{
    if (p_para->vstream_info.has_video)
    {
        if((p_para->vstream_info.video_format == VFORMAT_MPEG4) && 
        (p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_H263) )
		{
    		if(vbuf->data_len<4*RESERVE_VIDEO_SIZE)
    			p_para->playctrl_info.video_end_flag = 1;
		}
		else
		{
    		if(vbuf->data_len<RESERVE_VIDEO_SIZE)
    			p_para->playctrl_info.video_end_flag = 1;
		}
	}
    else
    {
		p_para->playctrl_info.video_end_flag = 1;
    }
    
    if((p_para->astream_info.has_audio && abuf->data_len<RESERVE_AUDIO_SIZE) ||
        (!p_para->astream_info.has_audio))
    {
	    p_para->playctrl_info.audio_end_flag = 1;
    }
    
	if(p_para->playctrl_info.video_end_flag && 
       p_para->playctrl_info.audio_end_flag && 
       (!p_para->playctrl_info.end_flag))
    {            
        p_para->playctrl_info.end_flag = 1;  
        p_para->playctrl_info.search_flag = 0;
        if((p_para->state.full_time-p_para->state.current_time)<20)
              p_para->state.current_time = p_para->state.full_time;
        if(!p_para->playctrl_info.loop_flag)
        {
            set_player_state(p_para,PLAYER_PLAYEND); 
            p_para->state.status=get_player_state(p_para);            
        }
        log_print("pid[%d]::[update_playing_states]player playe end!\n",p_para->player_id); 
	}
}

static void check_force_end(play_para_t *p_para,struct buf_status *vbuf,struct buf_status *abuf)
{
    int check_flag;
    if(check_time_interrupt(&p_para->check_end.old_time_ms, p_para->check_end.interval))
    {
        log_print("v:%d vlen=%d a:%d alen=%d count=%d\n",p_para->vstream_info.has_video,vbuf->data_len, p_para->astream_info.has_audio,abuf->data_len,p_para->check_end.end_count);
        if(p_para->vstream_info.has_video)
        {
            if(vbuf->data_len!= p_para->vstream_info.vdec_buf_len)
            {
                p_para->check_end.end_count = CHECK_END_COUNT;
                p_para->vstream_info.vdec_buf_len = vbuf->data_len;
            }
            else
                check_flag = 1;
        }
        if(p_para->astream_info.has_audio)
        {
            if(abuf->data_len != p_para->astream_info.adec_buf_len)
            {
                p_para->check_end.end_count = CHECK_END_COUNT;
                p_para->astream_info.adec_buf_len = abuf->data_len; 
            }
            else
                check_flag = 1;
        } 
        
        if(check_flag)
        {
            p_para->check_end.end_count --;
            if(p_para->check_end.end_count <= 0)  
            {				
                if(!p_para->playctrl_info.video_end_flag)
                {
					if(!p_para->playctrl_info.loop_flag)
					{
	                    set_player_state(p_para,PLAYER_PLAYEND);            
	                    p_para->state.status=get_player_state(p_para);
						//player_clear_ctrl_flags(&p_para->playctrl_info);
	                    log_print("[check_force_end]video force end!v:%d vlen=%d count=%d\n",p_para->vstream_info.has_video,vbuf->data_len,p_para->check_end.end_count);
					}
					p_para->playctrl_info.video_end_flag = 1;

                }
                else if (!p_para->playctrl_info.audio_end_flag)
                {
                    if(!p_para->playctrl_info.loop_flag)
					{
	                    set_player_state(p_para,PLAYER_PLAYEND);            
	                    p_para->state.status=get_player_state(p_para);
						//player_clear_ctrl_flags(&p_para->playctrl_info);
					}
					p_para->playctrl_info.audio_end_flag = 1;
                    log_print("[check_force_end]audio force end!a:%d alen=%d count=%d\n",p_para->astream_info.has_audio,abuf->data_len,p_para->check_end.end_count);
                }
            }
        }           
    }
}

int update_playing_info(play_para_t *p_para)
{
	struct buf_status vbuf, abuf;
	struct vdec_status vdec;
	struct adec_status adec;    

    MEMSET(&vbuf, 0, sizeof(struct buf_status));
    MEMSET(&abuf, 0, sizeof(struct buf_status));

    if(get_player_state(p_para) == PLAYER_RUNNING)
	{
        if(update_codec_info(p_para,&vbuf,&abuf,&vdec,&adec)!=0)
		    return PLAYER_FAILED;
    
        update_dec_info(p_para,&vdec, &adec);
    }
    
	update_current_time(p_para);
   
	if(p_para->playctrl_info.read_end_flag)
	{        
		check_avbuf_end(p_para, &vbuf, &abuf);

        if(p_para->check_end.interval == 0)
            p_para->check_end.interval = CHECK_END_INTERVAL;
        
        check_force_end(p_para, &vbuf, &abuf);
	}
	return PLAYER_SUCCESS;
}

int check_time_interrupt(unsigned long *old_msecond, int interval_ms)
{
    int ret = 0;
    struct timeval  new_time;
    unsigned long new_time_mseconds;
	gettimeofday(&new_time, NULL);
	new_time_mseconds=(new_time.tv_usec/1000+ new_time.tv_sec*1000);
    if(new_time_mseconds > (*old_msecond + interval_ms) || (new_time_mseconds < *old_msecond))
    {
        ret = 1;
        *old_msecond = new_time_mseconds;
    }
    return ret;
}

