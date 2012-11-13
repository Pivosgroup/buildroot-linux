/***************************************************
 * name     : player_update.c
 * function : update player parameters, information, status etc.
 * date     :  2010.3.2
 ***************************************************/
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <codec_type.h>
#include <player_set_sys.h>

#include "player_update.h"
#include "player_av.h"
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
    if (!info || !ctx) {
        return -1;
    }
    info->bitrate   = ctx->bit_rate;
    info->duration  = (int)(ctx->duration / AV_TIME_BASE);
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
    if ((p_para->file_type == AVI_FILE && !ctx->seekable) || 
		(p_para->file_type == MKV_FILE && !ctx->support_seek) ||
		(p_para->file_type == H264_FILE)) {
        info->seekable = 0;
    } else {
        info->seekable = 1;
    }
    if (info->total_video_num > MAX_VIDEO_STREAMS) {
        log_error("[set_stream_info]too much video streams(%d)!\n ", info->total_video_num);
        return -2;
    }
    if (info->total_audio_num > MAX_AUDIO_STREAMS) {
        log_error("[set_stream_info]too much audio streams(%d)!\n ", info->total_audio_num);
        return -3;
    }
    if (info->total_audio_num > MAX_SUB_STREAMS) {
        log_error("[set_stream_info]too much sub streams(%d)!\n ", p_para->astream_num);
        return -4;
    }
    info->cur_video_index   = p_para->vstream_info.video_index;
    info->cur_audio_index   = p_para->astream_info.audio_index;
    log_print("set stream info,current audio id:%d\n",p_para->media_info.stream_info.cur_audio_index);    	
    info->cur_sub_index     = p_para->sstream_info.sub_index;
    info->drm_check         = 0;//(p_para->pFormatCtx->drm.drm_check_value > 0) ? 1 : 0;
    return 0;
}

static int set_vstream_info(play_para_t *p_para)
{
    mstream_info_t *info = &p_para->media_info.stream_info;
    mvideo_info_t *vinfo;
    AVFormatContext *pCtx = p_para->pFormatCtx;
    if (!info || !pCtx) {
        return -1;
    }
    if (info->has_video) {
        unsigned int i;
        int vnum = 0;
        AVStream *pStream;
        for (i = 0; i < pCtx->nb_streams; i ++) {
            pStream = pCtx->streams[i];
            if (pStream->codec->codec_type == CODEC_TYPE_VIDEO) {
                vinfo = MALLOC(sizeof(mvideo_info_t));
                MEMSET(vinfo, 0, sizeof(mvideo_info_t));
		   vinfo->index 	   = i;
                vinfo->id          = pStream->id;
                vinfo->width       = pStream->codec->width;
                vinfo->height      = pStream->codec->height;
                vinfo->duartion    = (int)(pStream->duration * pStream->time_base.num / pStream->time_base.den);
                vinfo->bit_rate    = pStream->codec->bit_rate;
                vinfo->format      = p_para->vstream_info.video_format;
                vinfo->aspect_ratio_num = pStream->sample_aspect_ratio.num;
                vinfo->aspect_ratio_den = pStream->sample_aspect_ratio.den;
                vinfo->frame_rate_num   = pStream->r_frame_rate.num;
                vinfo->frame_rate_den   = pStream->r_frame_rate.den;
                vinfo->video_rotation_degree = pStream->rotation_degree;
                p_para->media_info.video_info[vnum] = vinfo;
                vnum ++;
                if (vnum > p_para->media_info.stream_info.total_video_num) {
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
    if ((tag = av_metadata_get(s->metadata, key, NULL, 0))) {
        strncpy(buf, tag->value, buf_size);
    }
    return !!tag;
}
static int get_id3v1_tag(AVFormatContext *s, audio_tag_info *audio_tag)
{
    AVMetadataTag *tag;
    int count = 0;

    if (!audio_tag->title[0]) {
        count += metadata_set_string(s, "title",   audio_tag->title, 30);
    }
    if (!audio_tag->author[0]) {
        count += metadata_set_string(s, "author",  audio_tag->author, 30);
    }
    if (!audio_tag->album[0]) {
        count += metadata_set_string(s, "album",   audio_tag->album, 30);
    }
    if (!audio_tag->year[0]) {
        count += metadata_set_string(s, "year",    audio_tag->year,  4);
    }
    if (!audio_tag->comment[0]) {
        count += metadata_set_string(s, "comment", audio_tag->comment, 30);
    }
    if (!audio_tag->genre[0]) {
        count += metadata_set_string(s, "genre", audio_tag->genre, 32);
    }

    if ((tag = av_metadata_get(s->metadata, "track", NULL, 0))) {
        if (!audio_tag->track) {
            audio_tag->track = atoi(tag->value);
        }
        count++;
    }
    return count;
}
static int get_id3v2_tag(AVFormatContext *s, audio_tag_info *audio_tag)
{
    AVMetadataTag *tag;
    int count = 0;

    if (!audio_tag->title[0]) {
        count += metadata_set_string(s, "TIT2",   audio_tag->title, 512);
    }
    if (!audio_tag->author[0]) {
        count += metadata_set_string(s, "TPE1",  audio_tag->author, 512);
    }
    if (!audio_tag->album[0]) {
        count += metadata_set_string(s, "TALB",   audio_tag->album, 512);
    }
    if (!audio_tag->year[0]) {
        count += metadata_set_string(s, "TYER",    audio_tag->year,  4);
    }
    if (!audio_tag->comment[0]) {
        count += metadata_set_string(s, "COMM", audio_tag->comment, 512);
    }
    if (!audio_tag->genre[0]) {
        count += metadata_set_string(s, "TCON", audio_tag->genre, 32);
    }

    if ((tag = av_metadata_get(s->metadata, "TRCK", NULL, 0))) {
        if (!audio_tag->track) {
            audio_tag->track = atoi(tag->value);
        }
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
    if (!info || !pCtx) {
        return -1;
    }
    if (info->has_audio) {
        unsigned int i, j;
		unsigned int new_flag = 1;
        int anum = 0;
        AVStream *pStream;
        for (i = 0; i < pCtx->nb_streams; i ++) {
            pStream = pCtx->streams[i];
            if (pStream->codec->codec_type == CODEC_TYPE_AUDIO) {
				for (j = 0; j < p_para->media_info.stream_info.total_audio_num; j ++) {						
					if (p_para->media_info.audio_info[j]) {
						if (pStream->id == p_para->media_info.audio_info[j]->id) {						
							new_flag = 0;							
							break;
						}
					} else {
						break;
					}
				}
				
				if (!new_flag){	
					log_print("[%s]i=%d j=%d exist stream_id: 0x%x, anum=%d\n", __FUNCTION__,i,j, pStream->id, p_para->media_info.stream_info.total_audio_num);
					new_flag = 1;
					continue;
            	}
				
                ainfo = MALLOC(sizeof(maudio_info_t));
                MEMSET(ainfo, 0, sizeof(maudio_info_t));
		   ainfo->index     = i;
                ainfo->id           = pStream->id;
                ainfo->channel      = pStream->codec->channels;
                ainfo->sample_rate  = pStream->codec->sample_rate;
                ainfo->duration     = (int)(pStream->duration * pStream->time_base.num / pStream->time_base.den);
                ainfo->bit_rate     = pStream->codec->bit_rate;
                ainfo->aformat      = audio_type_convert(pStream->codec->codec_id, p_para->file_type);
                if (pCtx->drmcontent) {
                    log_print("[%s:%d]DRM content found, not support yet.\n", __FUNCTION__, __LINE__);
                    ainfo->aformat = AFORMAT_UNSUPPORT;
                }
                if (p_para->stream_type == STREAM_AUDIO) {
                    if (ainfo->bit_rate == 0) {
                        ainfo->bit_rate = info->bitrate;
                    }
                    ainfo->audio_tag = MALLOC(sizeof(audio_tag_info));
                    get_tag_from_metadata(pCtx, ainfo->audio_tag);
                }
                p_para->media_info.audio_info[anum] = ainfo;                
				anum ++;
				if (anum > p_para->media_info.stream_info.total_audio_num) {
                    log_error("[set_astream_info]audio streams exceed!\n");
                    return -2;
                }
            }
        }
		p_para->media_info.stream_info.total_audio_num = anum;
    }
    return 0;
}

static int set_sstream_info(play_para_t *p_para)
{
    mstream_info_t *info = &p_para->media_info.stream_info;
    msub_info_t *sinfo;
    AVFormatContext *pCtx = p_para->pFormatCtx;
    if (!info || !pCtx) {
        return -1;
    }
    if (info->has_sub) {
        unsigned int i;
        int snum = 0;
        AVStream *pStream;
        for (i = 0; i < pCtx->nb_streams; i ++) {
            pStream = pCtx->streams[i];
            if (pStream->codec->codec_type == CODEC_TYPE_SUBTITLE) {
                AVMetadataTag *lang = av_metadata_get(pStream->metadata, "language", NULL, 0);
                sinfo = MALLOC(sizeof(msub_info_t));
                MEMSET(sinfo, 0, sizeof(msub_info_t));
		   sinfo->index = i;
                sinfo->id       = pStream->id;
                sinfo->internal_external = 0;
                sinfo->width    = pStream->codec->width;
                sinfo->height   = pStream->codec->height;
                sinfo->sub_type = pStream->codec->codec_id;
                //sinfo->subtitle_size;
                if (lang) {
                    sinfo->sub_language = lang->value;
                }
                p_para->media_info.sub_info[snum] = sinfo;
                snum ++;
                if (snum > p_para->media_info.stream_info.total_sub_num) {
                    log_error("[set_sstream_info]sub streams exceed!\n");
                    return -2;
                }
            }
        }
    }
    return 0;
}

static int set_chapter_info(play_para_t *p_para)
{
    mstream_info_t *info = &p_para->media_info.stream_info;
    mchapter_info_t *cinfo;
    AVFormatContext *pCtx = p_para->pFormatCtx;
    if (!info || !pCtx) {
        return -1;
    }

    unsigned int i;
    int snum = 0;
    info->total_chapter_num   = (pCtx->nb_chapters > MAX_CHAPTERS) ? MAX_CHAPTERS : pCtx->nb_chapters;

    for (i = 0; i < info->total_chapter_num; i ++) {

      AVChapter *chapter = pCtx->chapters[i];
      if(!chapter)
        continue;

      cinfo = MALLOC(sizeof(mchapter_info_t));
      MEMSET(cinfo, 0, sizeof(mchapter_info_t));

      cinfo->seekto_ms = chapter->start / AV_TIME_BASE;

      AVMetadataTag *tag = av_metadata_get(pCtx->chapters[i]->metadata,"title", NULL, 0);
      if(tag) {
        cinfo->name = tag->value;
      }
      p_para->media_info.chapter_info[snum] = cinfo;
      snum++;
    }
    info->total_chapter_num = snum;

    return 0;
}

int set_media_info(play_para_t *p_para)
{
    int ret = -1;
    media_info_init(&p_para->media_info);

    ret = set_stream_info(p_para);
    if (ret < 0) {
        log_error("[set_media_info]set_stream_info failed! ret=%d\n", ret);
    }

    ret = set_vstream_info(p_para);
    if (ret < 0) {
        log_error("[set_media_info]set_vstream_info failed! ret=%d\n", ret);
    }

    ret = set_astream_info(p_para);
    if (ret < 0) {
        log_error("[set_media_info]set_astream_info failed ret=%d!\n", ret);
    }


    ret = set_sstream_info(p_para);
    if (ret < 0) {
        log_error("[set_media_info]set_sstream_info failed ret=%d!\n", ret);
    }
	
    ret = set_chapter_info(p_para);
    if (ret < 0) {
        log_error("[set_media_info]set_chapter_info failed ret=%d!\n", ret);
    }
	
    return 0;
}

int set_ps_subtitle_info(play_para_t *p_para, subtitle_info_t *sub_info, int sub_num)
{
	mstream_info_t *info = &p_para->media_info.stream_info;
    msub_info_t *sinfo;
	int i;	

	if (!info){
		log_error("[%s]invalid parameters!\n", __FUNCTION__);
		return PLAYER_EMPTY_P;
	}
	
	log_print("[%s]total_sub_num=%d new_sub_num=%d\n", __FUNCTION__, info->total_sub_num, sub_num);

	for (i = info->total_sub_num; i < sub_num; i ++ ) {
		sinfo = MALLOC(sizeof(msub_info_t));
		if (sinfo) {
	        MEMSET(sinfo, 0, sizeof(msub_info_t));
	        sinfo->id       = sub_info[i].id;
			p_para->media_info.sub_info[i] = sinfo;            
			log_print("[%s]sub[%d].id=0x%x\n", __FUNCTION__,i, sinfo->id);
		} else {
			log_error("[%s]malloc [%d] failed!\n", __FUNCTION__, i);
			return PLAYER_NOMEM;
		}
	}
	if (sub_num > 0) {
		info->has_sub = 1;
		info->total_sub_num = sub_num;
        p_para->sstream_info.has_sub = 1;
        p_para->sstream_num = sub_num;
	}
	return PLAYER_SUCCESS;
}


static int check_vcodec_state(codec_para_t *codec, struct vdec_status *dec, struct buf_status *buf)
{
    int ret = 0;

    ret = codec_get_vbuf_state(codec,  buf);
    if (ret != 0) {
        log_error("codec_get_vbuf_state error: %x\n", -ret);
    }

    ret = codec_get_vdec_state(codec, dec);
    if (ret != 0) {
        log_error("codec_get_vdec_state error: %x\n", -ret);
        ret = PLAYER_CHECK_CODEC_ERROR;
    }

    return ret;
}

static int check_acodec_state(codec_para_t *codec, struct adec_status *dec, struct buf_status *buf)
{
    int ret = PLAYER_SUCCESS;

    ret = codec_get_abuf_state(codec,  buf);
    if (ret != 0) {
        log_error("codec_get_abuf_state error: %x\n", -ret);
    }

    ret = codec_get_adec_state(codec, dec);
    if (ret != 0) {
        log_error("codec_get_adec_state error: %x\n", -ret);
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
    codec_para_t    *vcodec = NULL;
    codec_para_t    *acodec = NULL;
    if ((p_para->stream_type == STREAM_ES)
        || (p_para->stream_type == STREAM_AUDIO)
        || (p_para->stream_type == STREAM_VIDEO)) {
        if (p_para->astream_info.has_audio && p_para->acodec) {
            acodec = p_para->acodec;
        }
        if (p_para->vstream_info.has_video && p_para->vcodec) {
            vcodec = p_para->vcodec;
        }
    } else if (p_para->codec) {
        vcodec = p_para->codec;
        acodec = p_para->codec;
    }
    if (vcodec && p_para->vstream_info.has_video) {
        if (check_vcodec_state(vcodec, vdec, vbuf) != 0) {
            log_error("check_vcodec_state error!\n");
            return PLAYER_FAILED;
        }
    }
    if (acodec && p_para->astream_info.has_audio) {
        if (check_acodec_state(acodec, adec, abuf) != 0) {
            log_error("check_acodec_state error!\n");
            return PLAYER_FAILED;
        }
    }
    return 0;
}
static unsigned int handle_current_time(play_para_t *para, unsigned int scr, unsigned int pts)
{
    player_status sta = get_player_state(para);

    if (!para->playctrl_info.pts_valid) {
        log_debug("[handle_current_time:sta=0x%x]scr=0x%x pts=0x%x\n", sta, scr, pts);
    }

    //if(sta == PLAYER_STOPED || sta == PLAYER_INITING)
    if (sta < PLAYER_RUNNING) {
        return 0;
    }
    if (pts == 0xffffffff) {
        return 0;
    }
    if (!para->playctrl_info.pts_valid) {
        if (scr > 0 && abs(scr - pts) <= PTS_FREQ) { //in tsync_avevent, pts as u32
            para->playctrl_info.pts_valid = 1;
            log_print("[%s:%d]scr=0x%x pts=0x%x diff=0x%x \n", __FUNCTION__, __LINE__, scr, pts, (scr - pts));
        }
    }

    if (para->playctrl_info.pts_valid) {
        return scr;
    } else {
        return 0;
    }

}

unsigned int get_pts_pcrscr(play_para_t *p_para)
{
    int handle;
    int size;
    char s[16];
    unsigned int value = 0;
    codec_para_t *pcodec;
#if 0
    handle = open("/sys/class/tsync/pts_pcrscr", O_RDONLY);
    if (handle < 0) {
        log_error("[player_get_ctime]open pts_pcrscr error!\n");
        return -1;
    }
    size = read(handle, s, sizeof(s));
    if (size > 0) {
        value = strtoul(s, NULL, 16);
        log_debug("\npcrscr=%x(%d) ", value, value / PTS_FREQ);
    }
    close(handle);
#else
    if (p_para->codec) {
        pcodec = p_para->codec;
    } else if (p_para->vcodec) {
        pcodec = p_para->vcodec;
    } else if (p_para->acodec) {
        pcodec = p_para->acodec;
    } else {
        log_print("[%s]No codec handler\n", __FUNCTION__);
        return -1;
    }

    value = codec_get_pcrscr(pcodec);
#endif
    return value;
}

unsigned int get_pts_video(play_para_t *p_para)
{
    int handle;
    int size;
    char s[16];
    unsigned int value = 0;
    codec_para_t *pcodec;

#if 0
    handle = open("/sys/class/tsync/pts_video", O_RDONLY);
    if (handle < 0) {
        log_print("[player_get_ctime]open pts_pcrscr error!\n");
        return -1;
    }
    size = read(handle, s, sizeof(s));
    if (size > 0) {
        value = strtoul(s, NULL, 16);
        log_debug("video=%x(%d)\n", value, value / PTS_FREQ);
    }
    close(handle);
#else
    if (p_para->codec) {
        pcodec = p_para->codec;
    } else if (p_para->vcodec) {
        pcodec = p_para->vcodec;
    } else {
        log_print("[%s]No codec handler\n", __FUNCTION__);
        return -1;
    }

    value = codec_get_vpts(pcodec);
#endif
    return value;
}

static unsigned int get_pts_audio(play_para_t *p_para)
{
    int handle;
    int size;
    char s[16];
    unsigned int value;
    codec_para_t *pcodec;

#if 0
    handle = open("/sys/class/tsync/pts_audio", O_RDONLY);
    if (handle < 0) {
        log_error("[player_get_ctime]open pts_audio error!\n");
        return -1;
    }
    size = read(handle, s, sizeof(s));
    if (size > 0) {
        value = strtoul(s, NULL, 16);
        log_debug("audio=%x(%d)\n", value, value / PTS_FREQ);
    }
    close(handle);
#else
    if (p_para->codec) {
        pcodec = p_para->codec;
    } else if (p_para->acodec) {
        pcodec = p_para->acodec;
    } else {
        log_print("[%s]No codec handler\n", __FUNCTION__);
        return -1;
    }

    value = codec_get_apts(pcodec);
#endif
    return value;
}

static int match_ext(const char *filename, const char *extensions)//get file type, .vob,.mp4,.ts...
{
    const char *ext, *p;
    char ext1[32], *q;

    if(!filename)
        return 0;

    ext = strrchr(filename, '.');
    if (ext) {
        ext++;
        p = extensions;
        for(;;) {
            q = ext1;
            while (*p != '\0' && *p != ',' && q-ext1<sizeof(ext1)-1)
                *q++ = *p++;
            *q = '\0';
            if (!strcasecmp(ext1, ext))
                return 1;
            if (*p == '\0')
                break;
            p++;
        }
    }
    return 0;
}

static unsigned int is_chapter_discontinue(play_para_t *p_para)
{
	char *extensions[4] = {"vob", "VOB", "iso", "ISO"};
	int i = 0;
	
	if (p_para->pFormatCtx && p_para->pFormatCtx->pb && 
          url_support_time_seek(p_para->pFormatCtx->pb)) {
		  return 1;
	}
	for (i = 0; i < 4; i ++) {
		//log_print("[%s]file_name=%s ext=%s\n", __FUNCTION__, p_para->file_name, extensions[i]);

		if (match_ext(p_para->file_name, extensions[i])) {
			log_print("[%s]return true\n", __FUNCTION__);
			return 1;
		}
	}
	return 0;
}

static unsigned int get_current_time(play_para_t *p_para)
{
    unsigned int pcr_scr = 0, vpts = 0, apts = 0;
    unsigned int ctime = 0;
	int set_discontinue = 0;
	int audio_pts_discontinue = 0, video_pts_discontinue = 0;
	codec_para_t *codec = NULL;

	if(p_para->vstream_info.has_video) {
        	if(p_para->vcodec){
        	    codec = p_para->vcodec;
        	   }
        	else if(p_para->codec){
        	     codec = p_para->codec;
        	     }
	}
	if(codec)
	{
		audio_pts_discontinue = codec_get_sync_audio_discont(codec);
		video_pts_discontinue = codec_get_sync_video_discont(codec);
	}

    if (video_pts_discontinue > 0)
	{		
		log_info("video pts discontinue!!!\n");		
		if(!set_discontinue && is_chapter_discontinue(p_para))
		{
		    p_para->discontinue_last_point = p_para->discontinue_point;
        	    p_para->discontinue_point = p_para->state.current_time;        	
		    set_discontinue = 1;
        	    log_info("vpts discontinue, point=%d ldpoint=%d\n", p_para->discontinue_point,p_para->discontinue_last_point);
		}
		if(codec)
		    codec_set_sync_video_discont(codec, 0);	
		log_info("vpts discontinue, vpts=0x%x scr=0x%x apts=0x%x\n", get_pts_video(p_para),get_pts_pcrscr(p_para),get_pts_audio(p_para));
    }

	if (audio_pts_discontinue > 0)
	{
		log_info("audio pts discontinue, curtime=%d lasttime=%d\n",p_para->state.current_time,p_para->state.last_time);
		if(!set_discontinue && is_chapter_discontinue(p_para) &&  
			(p_para->state.current_time < p_para->state.last_time))
		{
		    p_para->discontinue_last_point = p_para->discontinue_point;
        	    p_para->discontinue_point = p_para->state.current_time;   
		    set_discontinue = 1;
        	    log_info("apts discontinue, point=%d ldpoint=%d\n", p_para->discontinue_point,p_para->discontinue_last_point);			
		}
		if(codec)
		    codec_set_sync_audio_discont(codec, 0);		
		log_info("apts discontinue, vpts=0x%x scr=0x%x apts=0x%x\n", get_pts_video(p_para),get_pts_pcrscr(p_para),get_pts_audio(p_para));
	}

    if (p_para->vstream_info.has_video && p_para->astream_info.has_audio) {
        pcr_scr = get_pts_pcrscr(p_para);
        apts = get_pts_audio(p_para);
        vpts = get_pts_video(p_para);
        ctime = handle_current_time(p_para, pcr_scr, apts);
        log_debug("***[get_current_time:%d]ctime=0x%x\n", __LINE__, ctime);
    } else if (p_para->astream_info.has_audio)/* &&
            (p_para->stream_type == STREAM_ES) &&
            (p_para->astream_info.audio_format != AFORMAT_WMA)) */{
        apts = get_pts_audio(p_para);
        ctime = apts;
    } else {
        pcr_scr = get_pts_pcrscr(p_para);
        ctime = pcr_scr;
    }
    if (ctime == 0) {
        log_print("[get_current_time] curtime=0x%x pcr=0x%x apts=0x%x vpts=0x%x\n", ctime, pcr_scr, apts, vpts);
    }
    log_debug("===[get_current_time] current_time=0x%x (%d)\n", ctime, ctime / PTS_FREQ);
    return ctime;
}

static void update_current_time(play_para_t *p_para)
{
	#define REFRESH_CURTIME_INTERVAL    (100)
    unsigned int time = p_para->state.current_time;
	if (check_time_interrupt(&p_para->state.curtime_old_time, REFRESH_CURTIME_INTERVAL) || 
		!p_para->playctrl_info.pts_valid || 
		(p_para->playctrl_info.end_flag && !p_para->playctrl_info.search_flag)) {
	    if (p_para->playctrl_info.f_step > 0) {
	        time = (unsigned int)p_para->playctrl_info.time_point;
	        p_para->state.current_time = time;
	        p_para->state.current_ms = (unsigned int)(p_para->playctrl_info.time_point * 1000);
	        log_print("[update_current_time]ff/fb:time=%d\n", time);
#ifdef DEBUG_VARIABLE_DUR
	        if (p_para->playctrl_info.info_variable) {
	            update_variable_info(p_para);
	        }
#endif
	    } else  if (!p_para->playctrl_info.end_flag) {
	        time = get_current_time(p_para);
		  p_para->state.current_pts = time;
	        if (p_para->state.start_time == -1) {
	            if (p_para->astream_info.start_time != -1) {
	                p_para->state.start_time = p_para->astream_info.start_time;
	            } else if (p_para->vstream_info.start_time != -1) {
	                p_para->state.start_time = p_para->vstream_info.start_time;
	            }
	        }

	        log_debug("[update_current_time]time=%d astart_time=%d  vstart_time=%d last_time=%d\n", time / PTS_FREQ, ((unsigned int)p_para->astream_info.start_time / PTS_FREQ), ((unsigned int)p_para->vstream_info.start_time / PTS_FREQ), p_para->state.last_time);
			if (p_para->state.first_time == 0) {
				p_para->state.first_time = time;
			}

			if ((unsigned int)p_para->state.first_time > 0) {
				if ((unsigned int)p_para->state.first_time < (unsigned int)p_para->state.start_time) {
	                log_print("[update_current_time:%d]time=0x%x start_time=0x%x\n", __LINE__, time, ((unsigned int)p_para->astream_info.start_time));
	                p_para->state.start_time = p_para->state.first_time;
	            } else if (((unsigned int)p_para->state.first_time - (unsigned int)p_para->state.start_time) >  0 &&
	                       (p_para->state.start_time == 0) && p_para->playctrl_info.time_point == 0) {
	                p_para->state.start_time = p_para->state.first_time;
	                log_print("[update_current_time:%d]reset start_time=0x%x time=0x%x\n", __LINE__, p_para->state.start_time, time);
	            } 		
			}
			
	        if ((unsigned int)time > 0 && (unsigned int)p_para->state.start_time > 0) {				 
	            if ((unsigned int)p_para->state.start_time < (unsigned int)time) {
	                log_debug("[update_current_time:%d]time=0x%x start_time=0x%x\n", __LINE__, time, p_para->state.start_time);
	                time -= p_para->state.start_time;
	                log_debug("[update_current_time:%d]time=0x%x (%d)\n", __LINE__, time, time / PTS_FREQ);

	            }
	        }

	        log_debug("[update_current_time:%d]time=%d discontinue=%d\n", __LINE__, time / PTS_FREQ, p_para->discontinue_point);
	        if (p_para->discontinue_point > 0) 
			{
            	log_debug("[update_current_time:%d]time=%d dpoint=%d  ldpoint=%d\n", 
						__LINE__, time / PTS_FREQ, p_para->discontinue_point, p_para->discontinue_last_point);
	            if (p_para->pFormatCtx && p_para->pFormatCtx->pb && 
					url_support_time_seek(p_para->pFormatCtx->pb) && 
					(time / PTS_FREQ > 0) && (!p_para->discontinue_flag))					
				{
					if ((time / PTS_FREQ) < (p_para->discontinue_point - p_para->discontinue_last_point))  				 
					{
		                p_para->discontinue_point = p_para->discontinue_point - time / PTS_FREQ;
		                log_print("[update_current_time:%d]time<dpoint dpoint=%d\n", __LINE__, p_para->discontinue_point);
					} else if (time > p_para->discontinue_point * PTS_FREQ)
					{
						p_para->discontinue_point = 0;
		                log_print("[update_current_time:%d]time>dpoint dpoint=%d\n", __LINE__, p_para->discontinue_point);
					}
					p_para->discontinue_flag = 1;
	            }			
                time += p_para->discontinue_point * PTS_FREQ;
	        }
	        log_debug("[update_current_time]time=%d curtime=%d lasttime=%d\n", time / PTS_FREQ, p_para->state.current_time, p_para->state.last_time);
	        p_para->state.current_ms = time / PTS_FREQ_MS;
	      
	        time /= PTS_FREQ;
	    } else if (!p_para->playctrl_info.reset_flag && !p_para->playctrl_info.search_flag){
	    	time = p_para->state.full_time;
	        log_print("[update_current_time:%d]play end, curtime: %d\n", __LINE__, time);
	    }

	    if (p_para->state.current_time != p_para->state.last_time) {
	        p_para->state.last_time = p_para->state.current_time;
	    }
	    p_para->state.current_time = (int)time;

	    log_debug("[update_current_time:%d]curtime=%d lasttime=%d tpos=%d full_time=%d\n", __LINE__, p_para->state.current_time, p_para->state.last_time, p_para->playctrl_info.time_point, p_para->state.full_time);

	    if (p_para->state.current_time == 0 && p_para->playctrl_info.time_point > 0) {
	        p_para->state.current_time = p_para->playctrl_info.time_point;
	        p_para->state.current_ms = p_para->playctrl_info.time_point * 1000;
	        log_print("[update_current_time:%d]curtime: 0->%d\n", __LINE__, p_para->playctrl_info.time_point);
	    }
	    if ((p_para->state.current_time > p_para->state.full_time) && (p_para->state.full_time > 0)) {
	        //log_print("[update_current_time:%d]time=%d fulltime=%d\n", __LINE__, time, p_para->state.full_time);
	        if (p_para->state.current_time > p_para->state.full_time) {
	            p_para->state.current_time = p_para->state.full_time;
	            p_para->state.current_ms = p_para->state.current_time * 1000;
	        }
	    }
	    log_debug("[update_current_time:%d]time=%d last_time=%d time_point=%d\n", __LINE__, p_para->state.current_time, p_para->state.last_time, p_para->playctrl_info.time_point);

#ifdef DEBUG_VARIABLE_DUR
	    if (p_para->playctrl_info.info_variable) {
	        update_variable_info(p_para);
	    }
#endif
	}
}

static void update_dec_info(play_para_t *p_para, 
							struct vdec_status *vdec, 
							struct adec_status *adec,
							struct buf_status *vbuf, 
							struct buf_status *abuf)
{
    if (p_para->vstream_info.has_video) {
        if (p_para->vstream_info.video_width == 0) {
            p_para->vstream_info.video_width = vdec->width;
            p_para->vstream_info.video_height = vdec->height;
        }
        p_para->state.video_error_cnt = vdec->error_count;		    	
    }
    if (p_para->astream_info.has_audio) {
        p_para->state.audio_error_cnt = adec->error_count;		
    }
}

static void check_avbuf_end(play_para_t *p_para, struct buf_status *vbuf, struct buf_status *abuf)
{
    int vlimit = 0;
	
    if (p_para->vstream_info.has_video) {
        if ((p_para->vstream_info.video_format == VFORMAT_MPEG4) &&
            (p_para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_H263)) {
            vlimit = RESERVE_VIDEO_SIZE << 2;
        } else if (p_para->vstream_info.video_format == VFORMAT_MJPEG) {
            vlimit = RESERVE_VIDEO_SIZE >> 2;
        } else {
            vlimit = RESERVE_VIDEO_SIZE;
        }
        if (vbuf->data_len < vlimit) {
            log_print("[%s:%d]vbuf=0x%x	(limit=0x%x) video_low_buffer\n", __FUNCTION__, __LINE__, vbuf->data_len, vlimit);
            p_para->playctrl_info.video_low_buffer = 1;
        }
    } else {
        p_para->playctrl_info.video_end_flag = 1;
    }

    if ((p_para->astream_info.has_audio && abuf->data_len < RESERVE_AUDIO_SIZE) ||
        (!p_para->astream_info.has_audio)) {
        if (p_para->astream_info.has_audio){
        	log_print("[%s:%d]abuf=0x%x	(limit=0x%x) audio_low_buffer\n", __FUNCTION__, __LINE__, abuf->data_len, RESERVE_AUDIO_SIZE);
        }
        p_para->playctrl_info.audio_low_buffer = 1;
    }
	else if(p_para->astream_info.has_audio&&p_para->astream_info.audio_format==AFORMAT_WMAPRO)
	{
		int frame_size=p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->block_align;
		if(abuf->data_len<frame_size)
			p_para->playctrl_info.audio_low_buffer = 1;
	}
    //log_print("[%s:%d]abuf=0x%x   vbuf=0x%x\n", __FUNCTION__, __LINE__, abuf->data_len, abuf->data_len);

    if ((p_para->playctrl_info.video_low_buffer ||
        p_para->playctrl_info.audio_low_buffer) &&
        (!p_para->playctrl_info.end_flag)) {
        //p_para->state.vdec_buf_rp = vbuf->read_pointer;
        //p_para->state.adec_buf_rp = abuf->read_pointer;
        if (p_para->playctrl_info.avsync_enable) {
            set_tsync_enable(0);
            p_para->playctrl_info.avsync_enable = 0;
            log_print("[%s:%d]audio or video low buffer ,close av sync!\n", __FUNCTION__, __LINE__);
        }

		if (p_para->playctrl_info.video_low_buffer && p_para->playctrl_info.audio_low_buffer){
			p_para->playctrl_info.end_flag = 1;   
			player_thread_wait(p_para, 1000 * 1000);  // wait 1s for display out all frame
            if ((p_para->state.full_time - p_para->state.current_time) < 20) {
                p_para->state.current_time = p_para->state.full_time;
            }
			if (!p_para->playctrl_info.loop_flag) {
                set_player_state(p_para, PLAYER_PLAYEND);
				update_playing_info(p_para);
                update_player_states(p_para, 1);
                p_para->state.status = get_player_state(p_para);
                player_clear_ctrl_flags(&p_para->playctrl_info);
                set_black_policy(p_para->playctrl_info.black_out);
                log_print("[%s]low buffer, black=%d\n", __FUNCTION__, p_para->playctrl_info.black_out);
            }
		}
    }
}

static void check_force_end(play_para_t *p_para, struct buf_status *vbuf, struct buf_status *abuf)
{
    int check_flag = 0;
	//log_print("[%s]vlevel=%.03f alevel=%.03f count=%d\n", __FUNCTION__, p_para->state.video_bufferlevel,p_para->state.audio_bufferlevel, p_para->check_end.end_count);
    //if (check_time_interrupt(&p_para->check_end.old_time_ms, p_para->check_end.interval)) {
    if (!p_para->playctrl_info.end_flag && (
		(p_para->vstream_info.has_video && (p_para->state.video_bufferlevel < 0.04)) ||
		(p_para->astream_info.has_audio && (p_para->state.audio_bufferlevel < 0.04)) ||
		(p_para->astream_info.has_audio&&p_para->astream_info.audio_format==AFORMAT_WMAPRO&&abuf->data_len<p_para->pFormatCtx->streams[p_para->astream_info.audio_index]->codec->block_align)
		)){
        //log_print("v:%d vlen=0x%x a:%d alen=0x%x count=%d, vrp 0x%x, arp 0x%x\n",
        //    p_para->vstream_info.has_video,vbuf->data_len, p_para->astream_info.has_audio,abuf->data_len,p_para->check_end.end_count,vbuf->read_pointer,abuf->read_pointer);
        if (p_para->vstream_info.has_video) {
            if (p_para->vbuffer.rp_is_changed) {
                p_para->check_end.end_count = CHECK_END_COUNT;               
            } else {
                check_flag = 1;
				//log_print("[%s]vrp not move,vrp=vbufrp=0x%x,vlevel=%.03f cnt=%d\n", __FUNCTION__, vbuf->read_pointer,p_para->state.video_bufferlevel, p_para->check_end.end_count);
            }
        }
        if (p_para->astream_info.has_audio) {
	   if(p_para->astream_info.audio_format==AFORMAT_AMR||AFORMAT_PCM_S16LE==p_para->astream_info.audio_format)
           	if(p_para->state.current_time<p_para->state.full_time)
                	p_para->check_end.end_count = CHECK_END_COUNT; 
		 
            if (p_para->abuffer.rp_is_changed) {
                p_para->check_end.end_count = CHECK_END_COUNT;                
            } else {
                check_flag = 1;
				//log_print("[%s]arp not move,arp=abufrp=0x%x alevel=%.03f cnt=%d\n", __FUNCTION__, abuf->read_pointer, p_para->state.audio_bufferlevel, p_para->check_end.end_count);
            }
        }

        if (check_flag) {
			int dec_unit=1;
			float total_level=(p_para->state.video_bufferlevel+p_para->state.audio_bufferlevel)+0.000001;
			while(total_level*dec_unit<0.02 && dec_unit<9)
				dec_unit*=2;
            p_para->check_end.end_count -=dec_unit;
			if	(!p_para->playctrl_info.reset_flag){
				player_thread_wait(p_para, 100 * 1000);	//40ms
			}
            if (p_para->check_end.end_count <= 0) {
                if (!p_para->playctrl_info.video_end_flag) {
                    p_para->playctrl_info.video_end_flag = 1;
                    //log_print("[check_force_end]video force end!v:%d vlen=%d count=%d\n", p_para->vstream_info.has_video, vbuf->data_len, p_para->check_end.end_count);
                } else if (!p_para->playctrl_info.audio_end_flag) {
                    p_para->playctrl_info.audio_end_flag = 1;
                    //log_print("[check_force_end]audio force end!a:%d alen=%d count=%d\n", p_para->astream_info.has_audio, abuf->data_len, p_para->check_end.end_count);
                }
                if (p_para->playctrl_info.video_end_flag && p_para->playctrl_info.audio_end_flag) {
                    p_para->playctrl_info.end_flag = 1;
                    p_para->playctrl_info.search_flag = 0;
                    if ((p_para->state.full_time - p_para->state.current_time) < 20) {
                        p_para->state.current_time = p_para->state.full_time;
                    }
                    if (!p_para->playctrl_info.loop_flag) {
                        set_player_state(p_para, PLAYER_PLAYEND);
						update_playing_info(p_para);
                		update_player_states(p_para, 1);
                        p_para->state.status = get_player_state(p_para);
                        player_clear_ctrl_flags(&p_para->playctrl_info);
                        set_black_policy(p_para->playctrl_info.black_out);
                        log_print("[%s]force end, black=%d\n", __FUNCTION__, p_para->playctrl_info.black_out);
                    }
                }
            }
        }
    }
}

static int  update_buffering_states(play_para_t *p_para,
                                    struct buf_status *vbuf,
                                    struct buf_status *abuf)
{
    float alevel, vlevel;
    float minlevel, maxlevel;

	if(abuf->size>0)
		alevel = (float)abuf->data_len / abuf->size;
	else
		alevel=0;
	if(vbuf->size>0)
    	vlevel = (float)vbuf->data_len / vbuf->size;
	else
		vlevel=0;

    if(p_para->buffering_enable && p_para->buffering_start_time_s>0){
		/*reset  buffering parameters for  frame rate*/
		int bitrate=0;
		int buftime=p_para->buffering_start_time_s;
		
		if(p_para->pFormatCtx){

			bitrate=p_para->pFormatCtx->bit_rate;
			if(bitrate<=0 && p_para->pFormatCtx->file_size>0 && p_para->pFormatCtx->duration>0)/*caculate is better */
				bitrate=(p_para->pFormatCtx->file_size*8)/(p_para->pFormatCtx->duration / AV_TIME_BASE);
		}
		if(bitrate>0 && (vbuf->size>0 || abuf->size>0)){
			/*===time * bitrate/8/bufsize=buflevel =====*/
			p_para->buffering_threshhold_middle=((float)buftime*bitrate/8)/(vbuf->size+abuf->size);//for tmp start.we will reset after playing	
			if(p_para->buffering_threshhold_middle<=p_para->buffering_threshhold_min*2)
				p_para->buffering_threshhold_middle=p_para->buffering_threshhold_min*2;
			if(p_para->buffering_threshhold_middle>=p_para->buffering_threshhold_max*9/10)
				p_para->buffering_threshhold_middle=p_para->buffering_threshhold_max*9/10;
		}else{
			p_para->buffering_threshhold_middle=0.02;/*used default value*/
		}
		log_print("buffering threadhold  changed by bufering time(must min=%f<middle=%f<max=%f),buftime=%dS,bitrate=%d\n",
                      p_para->buffering_threshhold_min,
                      p_para->buffering_threshhold_middle,
                      p_para->buffering_threshhold_max,
                      buftime,
			 bitrate);
		/*seted,clear the value,we can changed later,and reset here again*/
		p_para->buffering_start_time_s=0;
    }			
	
    p_para->state.audio_bufferlevel = alevel;
    p_para->state.video_bufferlevel = vlevel;
	if (p_para->pFormatCtx && p_para->pFormatCtx->pb) {
		int buftime=-1;;
        	p_para->state.bufed_pos=url_buffed_pos(p_para->pFormatCtx->pb);
		buftime=(int)url_fbuffered_time(p_para->pFormatCtx->pb);
		if(buftime<0)
			buftime=(int)av_buffering_data(p_para->pFormatCtx,-1);
		p_para->state.bufed_time=(int)buftime;
		}
	else{
		p_para->state.bufed_pos=0;
		p_para->state.bufed_time=0;
		}
	
    if (p_para->astream_info.has_audio && 0)
        log_print("update_buffering_states,alevel=%d,asize=%d,level=%f,status=%d\n",
                  abuf->data_len, abuf->size, alevel, get_player_state(p_para));
    if (p_para->vstream_info.has_video && 0)
        log_print("update_buffering_states,vlevel=%d,vsize=%d,level=%f,status=%d\n",
                  vbuf->data_len, vbuf->size, vlevel, get_player_state(p_para));
	
	if(p_para->buffering_force_delay_s>0){
		if(p_para->buffering_check_point ==0){
			check_time_interrupt(&p_para->buffering_check_point,-1);
			return 0;
		}

		//log_print("buffering second check point:%ld,%f\n",p_para->buffering_check_point,p_para->buffering_force_delay_s);
		if(!check_time_interrupt(&p_para->buffering_check_point,(int)(p_para->buffering_force_delay_s*1000)))
		{
			//delay buffering  
			return 0;
		}
		p_para->buffering_force_delay_s=0;
	}

	//if (!p_para->playctrl_info.read_end_flag){
	    if (p_para->buffering_enable && get_player_state(p_para) != PLAYER_PAUSE) {
	        if (p_para->astream_info.has_audio && p_para->vstream_info.has_video) {
	            minlevel = MIN(alevel, vlevel);
	            maxlevel = MAX(alevel, vlevel);
	        } else if (p_para->astream_info.has_audio) {
	            minlevel = alevel;
	            maxlevel = alevel;
	        } else {
	            minlevel = vlevel;
	            maxlevel = vlevel;
	        }

	        if ((get_player_state(p_para) == PLAYER_RUNNING) &&
	            (minlevel < p_para->buffering_threshhold_min)  &&
	            (maxlevel < p_para->buffering_threshhold_max*3/4) &&
	            !p_para->playctrl_info.read_end_flag) {
	            codec_pause(p_para->codec);
	            set_player_state(p_para, PLAYER_BUFFERING);
	            update_player_states(p_para, 1);
	            log_print("enter buffering!!!\n");
	        }

	        else if ((get_player_state(p_para) == PLAYER_BUFFERING) &&
	                 ((minlevel > p_para->buffering_threshhold_middle)  ||
	                  (maxlevel > p_para->buffering_threshhold_max) ||
	                  p_para->playctrl_info.read_end_flag)) {
	            codec_resume(p_para->codec);
	            set_player_state(p_para, PLAYER_BUFFER_OK);
	            update_player_states(p_para, 1);
	            log_print("leave buffering!!!\n");
				set_player_state(p_para, PLAYER_RUNNING);
	            update_player_states(p_para, 1);
	        }
    	}
    //}
    return 0;
}

static void update_decbuf_states(play_para_t *p_para, struct buf_status *vbuf, struct buf_status *abuf)
{
	if(p_para->astream_info.has_audio){
		if (p_para->abuffer.buffer_size == 0){
			p_para->abuffer.buffer_size = abuf->size;
		}

		p_para->abuffer.data_level = abuf->data_len;
		
		if (abuf->read_pointer != p_para->abuffer.buffer_rp) {
			p_para->abuffer.rp_is_changed = 1;			
	        p_para->abuffer.buffer_rp = abuf->read_pointer;	      
	    }else{
	    	p_para->abuffer.rp_is_changed = 0;	
	    }
	}
	
	if(p_para->vstream_info.has_video){
		if (p_para->vbuffer.buffer_size == 0){
			p_para->vbuffer.buffer_size = vbuf->size;
		}
		
		p_para->vbuffer.data_level = vbuf->data_len;
		
		if (vbuf->read_pointer != p_para->vbuffer.buffer_rp) {
			p_para->vbuffer.rp_is_changed = 1;			
	        p_para->vbuffer.buffer_rp = vbuf->read_pointer;	      
	    }else{
	    	p_para->vbuffer.rp_is_changed = 0;	
	    }
	}	
}

static void update_av_sync_for_audio(play_para_t *p_para)
{
    if(!p_para->abuffer.rp_is_changed && !check_time_interrupt(&p_para->playctrl_info.avsync_check_old_time,20))
        return ;//no changed and time is no changed.do count---,1S no changesd..20*50
    if (p_para->playctrl_info.audio_ready && 
		p_para->vstream_info.has_video && 
		p_para->astream_info.has_audio &&
		get_player_state(p_para) == PLAYER_RUNNING) {
        if (!p_para->abuffer.rp_is_changed) {
            p_para->abuffer.check_rp_change_cnt --;
			if (!p_para->playctrl_info.pts_valid && p_para->abuffer.data_level == 0) {
				p_para->abuffer.buf_empty ++;
			}
			//log_print("[%s:%d]arp not change, cnt=%d! empty=%d\n", __FUNCTION__, __LINE__, p_para->abuffer.check_rp_change_cnt, p_para->abuffer.buf_empty);
        } else {
            p_para->abuffer.check_rp_change_cnt = CHECK_AUDIO_HALT_CNT;
            if (!p_para->playctrl_info.avsync_enable) {
                set_tsync_enable(1);
                p_para->playctrl_info.avsync_enable = 1;
                log_print("[%s:%d]arp alived, enable sync!\n", __FUNCTION__, __LINE__);
            }
        }
        if (p_para->playctrl_info.avsync_enable &&
            p_para->abuffer.check_rp_change_cnt <= 0) {
            set_tsync_enable(0);
            p_para->playctrl_info.avsync_enable = 0;
			if (p_para->abuffer.buf_empty == CHECK_AUDIO_HALT_CNT && p_para->playctrl_info.pts_valid == 0) {
				p_para->playctrl_info.pts_valid = 1;
				p_para->abuffer.buf_empty = 0;
			}
            log_print("[%s:%d]arp not alived, disable sync\n", __FUNCTION__, __LINE__);
            p_para->abuffer.check_rp_change_cnt = CHECK_AUDIO_HALT_CNT;
        }
    }
}

int update_playing_info(play_para_t *p_para)
{
    struct buf_status vbuf, abuf;
    struct vdec_status vdec;
    struct adec_status adec;
	player_status sta;
	int ret;

    MEMSET(&vbuf, 0, sizeof(struct buf_status));
    MEMSET(&abuf, 0, sizeof(struct buf_status));

	sta = get_player_state(p_para);	
    if (sta > PLAYER_INITOK) {
		if (sta != PLAYER_SEARCHING) {
			ret = update_codec_info(p_para, &vbuf, &abuf, &vdec, &adec);
	        if (ret != 0) {
	            return PLAYER_FAILED;
	        }
		}
        update_dec_info(p_para, &vdec, &adec, &vbuf, &abuf);

		update_decbuf_states(p_para, &vbuf, &abuf);

		update_buffering_states(p_para, &vbuf, &abuf);

		update_av_sync_for_audio(p_para);

		if (sta > PLAYER_INITOK && sta < PLAYER_ERROR) {
		    if (p_para->playctrl_info.audio_ready != 1){
    			p_para->playctrl_info.audio_ready  = codec_audio_isready(p_para->codec);
    			if(p_para->playctrl_info.audio_ready)
                	log_print("[%s:%d]audio_ready=%d\n", __FUNCTION__, __LINE__, p_para->playctrl_info.audio_ready);
             }		
		
            if (p_para->astream_info.has_audio 	&& (p_para->playctrl_info.audio_ready != 1)) {
    			if (check_audiodsp_fatal_err() == AUDIO_DSP_INIT_ERROR){
    				p_para->playctrl_info.audio_ready = 1;
    	            log_print("[%s]dsp init failed, set audio_ready for time update\n", __FUNCTION__);
    			} else if (0 == p_para->abuffer.data_level) {
    			    if (check_audio_ready_time(&p_para->playctrl_info.check_audio_ready_ms)) {
                        p_para->playctrl_info.audio_ready = 1;
    	                log_print("[%s]no audio data, set audio_ready for time update\n", __FUNCTION__);
    		        }
    		    }
            }
        }
		
        if (p_para->playctrl_info.audio_ready == 1 || 
			p_para->playctrl_info.search_flag ||
            p_para->playctrl_info.fast_backward ||
            p_para->playctrl_info.fast_forward)
        {
        	update_current_time(p_para);
        }
		
		p_para->state.pts_video = get_pts_video(p_para);	    
    }

    if (p_para->playctrl_info.read_end_flag && (get_player_state(p_para) != PLAYER_PAUSE)){
		
        check_avbuf_end(p_para, &vbuf, &abuf);       
		
        check_force_end(p_para, &vbuf, &abuf);       
    }    	
	
    return PLAYER_SUCCESS;
}

int check_time_interrupt(long *old_msecond, int interval_ms)
{
    int ret = 0;
    struct timeval  new_time;
    long new_time_mseconds;
    gettimeofday(&new_time, NULL);
    new_time_mseconds = (new_time.tv_usec / 1000 + new_time.tv_sec * 1000);
    if (new_time_mseconds > (*old_msecond + interval_ms) || (new_time_mseconds < *old_msecond)) {
        ret = 1;
        *old_msecond = new_time_mseconds;
    }
    return ret;
}

void set_drm_rental(play_para_t *p_para, unsigned int rental_value)
{
    p_para->state.drm_rental = rental_value;

    return;
}

int check_audio_ready_time(int *first_time)
{
    struct timeval  new_time;
    long new_time_mseconds;
    gettimeofday(&new_time, NULL);
    new_time_mseconds = (new_time.tv_usec / 1000 + new_time.tv_sec * 1000);

    if (*first_time == 0) {
        *first_time = new_time_mseconds;
    }

    if (new_time_mseconds - *first_time > 3000) {// 3s watchdog
        return 1;
    }

    return 0;
}

