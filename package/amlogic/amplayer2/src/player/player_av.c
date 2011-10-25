/************************************************
 * name	:av_decoder.c
 * function	:decoder relative functions
 * data		:2010.2.4
 *************************************************/
 //header file
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <player_error.h>

#include "player_priv.h"
#include "player_av.h"
#include "player_hwdec.h"
#include "h263vld.h"
#include "thread_mgt.h"

#define DUMP_WRITE   (0)
#define DUMP_READ    (0)
#if (DUMP_WRITE || DUMP_READ)
#include <fcntl.h>
int fdw=-1,fdr=-1;
#endif

static const media_type media_array[] = {    
    {"mpegts", MPEG_FILE, STREAM_TS},
    {"mpeg", MPEG_FILE, STREAM_PS},
    {"rm", RM_FILE, STREAM_RM},
    {"avi", AVI_FILE, STREAM_ES},
    {"mkv", MKV_FILE, STREAM_ES},	
    {"matroska", MKV_FILE, STREAM_ES},
    {"mov", MOV_FILE, STREAM_ES},
    {"mp4", MP4_FILE, STREAM_ES},
    {"flv", FLV_FILE, STREAM_ES},
    {"aac", AAC_FILE, STREAM_AUDIO},
    {"ac3", AC3_FILE, STREAM_AUDIO},
    {"mp3", MP3_FILE, STREAM_AUDIO},
    {"wav", WAV_FILE, STREAM_AUDIO},
    {"dts", DTS_FILE, STREAM_AUDIO},
    {"flac", FLAC_FILE, STREAM_AUDIO},  
    {"h264", H264_FILE, STREAM_VIDEO},
    {"mpegvideo", M2V_FILE, STREAM_VIDEO},	
    {"p2p", P2P_FILE, STREAM_ES},
    {"asf", ASF_FILE, STREAM_ES},   
};

aformat_t audio_type_convert(enum CodecID id,pfile_type File_type)
{
    aformat_t format = -1;    
    switch (id)
    {
        case CODEC_ID_PCM_MULAW:
        	format = AFORMAT_MULAW;
            break;
        	
        case CODEC_ID_PCM_ALAW:
        	format = AFORMAT_ALAW;    
            break;
    	
        case CODEC_ID_MP2:
        case CODEC_ID_MP3:
        	format = AFORMAT_MPEG;
            break;
        	
        case CODEC_ID_AAC:
        	if(File_type==RM_FILE)
        		format=AFORMAT_RAAC;
        	else
        		format = AFORMAT_AAC;
            break;
    	
        case CODEC_ID_AC3:
        	format = AFORMAT_AC3;
            break;
        	
        case CODEC_ID_DTS:
        	format = AFORMAT_DTS;
            break;
        	
        case CODEC_ID_PCM_S16BE:
        	format = AFORMAT_PCM_S16BE;
            break;
        	
        case CODEC_ID_PCM_S16LE:
        	format = AFORMAT_PCM_S16LE;
            break;

        case CODEC_ID_PCM_U8:
        	format = AFORMAT_PCM_U8;
            break;

        case CODEC_ID_COOK:
        	format = AFORMAT_COOK;
            break;

        case CODEC_ID_ADPCM_IMA_WAV:
        case CODEC_ID_ADPCM_MS:
        	format = AFORMAT_ADPCM;
            break;
        case CODEC_ID_AMR_NB:
        case CODEC_ID_AMR_WB:
            format=  AFORMAT_AMR;
            break;
		case CODEC_ID_WMAV1:
		case CODEC_ID_WMAV2:
			format=  AFORMAT_WMA;
			break;
        case CODEC_ID_FLAC:
            format = AFORMAT_FLAC;
            break;
            
        default:
        	format = -1;
            log_print("audio codec_id=0x%x\n", id);
    }
    log_print("[audio_type_convert]audio codec_id=0x%x format=%d\n", id, format);
    return format;
}


vformat_t video_type_convert(enum CodecID id)
{
    vformat_t format;
	switch (id)
    {
    	case CODEC_ID_MPEG1VIDEO:	 
    	case CODEC_ID_MPEG2VIDEO:
    	case CODEC_ID_MPEG2VIDEO_XVMC:
    		format = VFORMAT_MPEG12;
            break;
    		
    	case CODEC_ID_H263:
    	case CODEC_ID_MPEG4:
    	case CODEC_ID_H263P:
    	case CODEC_ID_H263I:
    	case CODEC_ID_XVID:
        case CODEC_ID_MSMPEG4V2:
        case CODEC_ID_MSMPEG4V3:
        case CODEC_ID_FLV1:      
    		format = VFORMAT_MPEG4;
            break;
    		
    	case CODEC_ID_RV10:
    	case CODEC_ID_RV20:
    	case CODEC_ID_RV30:
    	case CODEC_ID_RV40:
    		format = VFORMAT_REAL;
            break;
    		
    	//case CODEC_ID_AVC1:
    	case CODEC_ID_H264:
    		format = VFORMAT_H264;
            break;
    		
    	case CODEC_ID_MJPEG:
    		format = VFORMAT_MJPEG;
            break;

    	case CODEC_ID_VC1:        
    	case CODEC_ID_WMV3:
        //case CODEC_ID_WMV1:           //not support
        //case CODEC_ID_WMV2:           //not support
    		format = VFORMAT_VC1;
            break;

    	case CODEC_ID_VP6F:
    		format = VFORMAT_SW;
    		break;
    		
    	default:
    	    format = -1;
            log_print("video_type_convert failed:video codec_id=0x%x\n", id);
    } 
    log_print("[video_type_convert]video codec_id=0x%x format=%d\n", id, format);
    return format;
}

vdec_type_t video_codec_type_convert(unsigned int id)
{
	vdec_type_t dec_type;
	log_print("[video_codec_type_convert]id=(0x%x) ", id);
	 switch (id)
    {
        case CODEC_TAG_MJPEG:
        case CODEC_TAG_mjpeg:
        case CODEC_TAG_jpeg:
        case CODEC_TAG_mjpa:
            log_print("VIDEO_TYPE_MJPEG\n");
            dec_type = VIDEO_DEC_FORMAT_MJPEG;
            break;

        // xvid
        case CODEC_TAG_XVID:
        case CODEC_TAG_xvid:
        case CODEC_TAG_XVIX:
            log_print("VIDEO_TYPE_XVID\n");
            dec_type =VIDEO_DEC_FORMAT_MPEG4_5;
            break;

        //divx3.11
        case CODEC_TAG_COL1:
        case CODEC_TAG_DIV3:
        case CODEC_TAG_MP43:        
            log_print("VIDEO_TYPE_DIVX311\n");
            dec_type = VIDEO_DEC_FORMAT_MPEG4_3;
            break;
                
        // divx4
        case CODEC_TAG_DIV4:
        case CODEC_TAG_DIVX:        
            log_print("VIDEO_TYPE_DIVX4\n");
            dec_type = VIDEO_DEC_FORMAT_MPEG4_4;
            break;
                
        // divx5
        case CODEC_TAG_DIV5:
        case CODEC_TAG_DX50:  
        case CODEC_TAG_M4S2:
            log_print("VIDEO_TYPE_DIVX 5\n");
            dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
            break;
                
        // divx6
        case CODEC_TAG_DIV6:
            log_print("VIDEO_TYPE_DIVX 6\n");
            dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
            break;
        
        // mp4
        case CODEC_TAG_MP4V:
        case CODEC_TAG_RMP4:         
        case CODEC_TAG_MPG4:
		case CODEC_TAG_mp4v:
            log_print("VIDEO_DEC_FORMAT_MPEG4_5\n");
            dec_type = VIDEO_DEC_FORMAT_MPEG4_5;
            break;

		// h263
		case CODEC_TAG_H263:
		case CODEC_TAG_h263:
		case CODEC_TAG_s263:
		case CODEC_TAG_F263:         
            log_print("VIDEO_DEC_FORMAT_H263\n");
            dec_type = VIDEO_DEC_FORMAT_H263;
            break;

		//avc1
		case CODEC_TAG_AVC1:
		case CODEC_TAG_avc1:
        // h264
        case CODEC_TAG_H264:
        case CODEC_TAG_h264:
            log_print("VIDEO_TYPE_H264\n");
            dec_type = VIDEO_DEC_FORMAT_H264;
            break;
						
		case CODEC_ID_RV30:
            log_print("[video_codec_type_convert]VIDEO_DEC_FORMAT_REAL_8(0x%x)\n", id);
            dec_type = VIDEO_DEC_FORMAT_REAL_8;
            break;
            
        case CODEC_ID_RV40:
            log_print("[video_codec_type_convert]VIDEO_DEC_FORMAT_REAL_9(0x%x)\n", id);
            dec_type = VIDEO_DEC_FORMAT_REAL_9;
            break;

		case CODEC_ID_H264:
            log_print("[video_codec_type_convert]VIDEO_DEC_FORMAT_H264(0x%x)\n", id);
			dec_type = VIDEO_DEC_FORMAT_H264;
			break;

        //case CODEC_TAG_WMV1:          //not support
        //case CODEC_TAG_WMV2:          //not support
		case CODEC_TAG_WMV3:      
            log_print("[video_codec_type_convert]VIDEO_DEC_FORMAT_WMV3(0x%x)\n", id);
			dec_type = VIDEO_DEC_FORMAT_WMV3;
			break;
        
		case CODEC_TAG_WVC1:
        case CODEC_ID_VC1:          
		case CODEC_TAG_WMVA:
            log_print("[video_codec_type_convert]VIDEO_DEC_FORMAT_WVC1(0x%x)\n", id);
			dec_type = VIDEO_DEC_FORMAT_WVC1;
			break; 
        
    	case CODEC_ID_VP6F:
            log_print("[video_codec_type_convert]VIDEO_DEC_FORMAT_SW(0x%x)\n", id);
    		dec_type = VIDEO_DEC_FORMAT_SW;
    		break;

        default: 
            log_print("[video_codec_type_convert]error:VIDEO_TYPE_UNKNOW  id = 0x%x\n", id);
            dec_type = VIDEO_DEC_FORMAT_UNKNOW;
            break;
    }
	return dec_type;
}

stream_type_t stream_type_convert(pstream_type type, char vflag, char aflag)
{
	switch(type)
	{
		case STREAM_TS:
			return STREAM_TYPE_TS;

		case STREAM_PS:
			return STREAM_TYPE_PS;

        case STREAM_RM:
            return STREAM_TYPE_RM;

		case STREAM_ES:
			if(vflag == 1)
				return STREAM_TYPE_ES_VIDEO;
			if(aflag == 1)
				return STREAM_TYPE_ES_AUDIO;
			else
				return STREAM_TYPE_UNKNOW;
            
		case STREAM_AUDIO:
			return STREAM_TYPE_ES_AUDIO;
            
		case STREAM_VIDEO:
			return STREAM_TYPE_ES_VIDEO;
            
		case STREAM_UNKNOWN:
		default:
			return STREAM_TYPE_UNKNOW;	
	}
}
int set_file_type(const char *name, pfile_type *ftype, pstream_type *stype)
{	
	int i, j ;    
    j = sizeof(media_array)/sizeof(media_type);
    for (i=0; i<j; i++)
    {		
    	//log_print("[set_file_type:0]name = %s  mname=%s\n",name ,media_array[i].file_ext);
		if(strcmp(name,media_array[i].file_ext) == 0)			
			break;			
    }    
    if (i == j)
    {
        for (i=0; i<j; i++)
        {		
        	//log_print("[set_file_type:1]name = %s  mname=%s\n",name ,media_array[i].file_ext);
    		if(strstr(name,media_array[i].file_ext) != NULL)			
    			break;	
            if(i == j)
            {
                log_print("Unsupport file type %s\n", name);        
                return PLAYER_UNSUPPORT;
            }
        }        
    }
    *ftype = media_array[i].file_type;   
	*stype = media_array[i].stream_type;
    log_info("[set_file_type]file_type=%d stream_type=%d\n",*ftype, *stype);
	return PLAYER_SUCCESS;
}

#define MAX_TRY_READ_COUNT  (50)
static int raw_read(play_para_t *para, am_packet_t *pkt)
{
    int rev_byte = -1; 			
    ByteIOContext *pb = para->pFormatCtx->pb;
    unsigned char *pbuf ;    
    static int try_count = MAX_TRY_READ_COUNT;

    if(pkt->data_size > 0)
    {
        player_thread_wait(para,RW_WAIT_TIME);        
        return PLAYER_SUCCESS;	
    }
    
	if(pkt->buf == NULL)
	{
		pkt->buf_size = para->max_raw_size + 16;
		pkt->buf = MALLOC(pkt->buf_size);	
		if(pkt->buf == NULL)
        {
            log_print("not enough memory,please fre memory\n");
            return PLAYER_RD_EMPTYP;
        }	
		pkt->data=pkt->buf;
	}    
    pbuf = pkt->data;
    if(!para->playctrl_info.read_end_flag && (0 == pkt->data_size))
    {        
    	rev_byte = get_buffer(pb,pbuf,para->max_raw_size);
        if(rev_byte > 0)
    	{	
            try_count = MAX_TRY_READ_COUNT;
    		pkt->data_size = rev_byte;		
    		para->read_size.total_bytes += rev_byte;
            pkt->avpkt_newflag = 1;
            pkt->avpkt_isvalid = 1;    
    	} 
        else if(rev_byte == AVERROR_EOF)//if(rev_byte != AVERROR(EAGAIN))
        {/*if the return is EAGAIN,we need to try more times*/
            para->playctrl_info.read_end_flag = 1;
            log_print("raw read: read end!\n");
        }
    	else 					   
    	{            
            if(rev_byte != AVERROR(EAGAIN))
            {
               log_print("raw_read buffer error!\n");	            
               return PLAYER_RD_FAILED;
            } 
            else
            {
                try_count --;
                if(! try_count)
                {
                    log_print("raw_read buffer try too more counts,exit!\n");              
                    return PLAYER_RD_TIMEOUT;
                }
                else
                    return PLAYER_RD_AGAIN;
            }
                
    	}    	    			
    }
	if(para->stream_type == STREAM_TS)
	{
		pkt->codec = para->codec;	
		pkt->type = CODEC_COMPLEX;
	}
	else if(para->stream_type == STREAM_PS)
	{
		pkt->codec = para->codec;	
		pkt->type = CODEC_COMPLEX;
	}
    else if(para->stream_type == STREAM_RM)
    {
        pkt->codec = para->codec;	
		pkt->type = CODEC_COMPLEX;
    }
	else if(para->stream_type == STREAM_AUDIO)
	{
		pkt->codec = para->acodec;	
		pkt->type = CODEC_AUDIO;
	}
	else if(para->stream_type == STREAM_VIDEO)
	{
		pkt->codec = para->vcodec;	
		pkt->type = CODEC_VIDEO;
	}		
	return PLAYER_SUCCESS;	  
}

static int non_raw_read(play_para_t *para, am_packet_t *pkt)
{		
    static int try_count = MAX_TRY_READ_COUNT;
	signed short video_idx = para->vstream_info.video_index;
	signed short audio_idx = para->astream_info.audio_index;
    signed short sub_idx = para->sstream_info.sub_index;
    int has_video = para->vstream_info.has_video;
    int has_audio = para->astream_info.has_audio;
    int has_sub = para->sstream_info.has_sub;
    
    if(pkt->data_size > 0)
    {
        player_thread_wait(para,RW_WAIT_TIME);   
        //log_print("[%s:%d]wait---data_size=%d!\n",__FUNCTION__, __LINE__,pkt->data_size);             
        return PLAYER_SUCCESS;	
    }
    
	if(para->vstream_info.has_video && !para->vcodec)
	{
		log_print("[non_raw_read]video codec invalid!\n");
		return PLAYER_RD_EMPTYP;
	}
	
	if(para->astream_info.has_audio && !para->acodec)
	{
		log_print("[non_raw_read]audio codec invalid!\n");
		return PLAYER_RD_EMPTYP;
	}	
	
	if(pkt->avpkt == NULL)
	{        
		log_print("non_raw_read error:avpkt pointer NULL!\n");
		return PLAYER_RD_EMPTYP;
	}
   
	while(!para->playctrl_info.read_end_flag &&(0 == pkt->data_size))
	{   
		int ret;   
		ret=av_read_frame(para->pFormatCtx, pkt->avpkt);
         
		if(ret< 0)
		{                
			if(AVERROR(EAGAIN)!= ret)
			{/*if the return is EAGAIN,we need to try more times*/
			    log_error("[%s:%d]av_read_frame return (%d)\n",__FUNCTION__,__LINE__,ret);
                
                if(AVERROR_EOF != ret)
                    return PLAYER_RD_FAILED;   
                else
                {
                    para->playctrl_info.read_end_flag = 1;  
                #if DUMP_READ                
                    if(fdr > 0)
                        close(fdr);
                #endif
                }
			}
            else
            {
                try_count --;
                if(!try_count)
                {
                    log_print("try %d counts, can't get packet,exit!\n",MAX_TRY_READ_COUNT);
                    return PLAYER_RD_TIMEOUT;
                }
                else
                {
                    log_print("[non_raw_read]EAGAIN, try count=%d\n",try_count);
                    return PLAYER_RD_AGAIN;
                }
            }
		}
        else //read success
        {            
            try_count = MAX_TRY_READ_COUNT;              
        	#if DUMP_READ
            if(fdr==-1)
            {
                fdr=open("./dump_read.dat",O_CREAT|O_RDWR);
                if(fdr < 0)
                    log_print("creat dump file failed!fd=%d\n",fdr);
            }           
            #endif       

            //log_print("av_read_frame return (%d) pkt->avpkt=%p pkt->avpkt->data=%p\r",ret,pkt->avpkt,pkt->avpkt->data);
        
        }
       
		if(pkt->avpkt->size >= MAX_PACKET_SIZE)
		{ 
			log_print("non_raw_read error:packet size exceed malloc memory! size %d\n", pkt->avpkt->size );
			av_free_packet(pkt->avpkt);
			return PLAYER_RD_FAILED;
		}       
		if(para->stream_type == STREAM_ES && !para->playctrl_info.read_end_flag)
		{            
			if(has_video && video_idx == pkt->avpkt->stream_index)		
			{
				pkt->codec = para->vcodec;			
				pkt->type = CODEC_VIDEO;
				para->read_size.vpkt_num ++;                
			}
			else if (has_audio && audio_idx == pkt->avpkt->stream_index)		
			{
				pkt->codec = para->acodec;			
				pkt->type = CODEC_AUDIO;
				para->read_size.apkt_num ++;
			}
            else if (has_sub && sub_idx == pkt->avpkt->stream_index)
            {
                /* here we get the subtitle data, something should to be done */
                para->read_size.spkt_num ++;
                pkt->type = CODEC_SUBTITLE;
                pkt->codec = para->scodec;
            }
			else		
			{
				pkt->codec = NULL;			
				pkt->type = CODEC_UNKNOW;
                av_free_packet(pkt->avpkt);
                continue;
			}

            if(para->first_index == -1)
                para->first_index = pkt->avpkt->stream_index;
		}
        
        if(ret == 0)
        {
            pkt->data = pkt->avpkt->data;
    		pkt->data_size = pkt->avpkt->size;	            
            #if DUMP_READ           
            if(fdr>0)
            {     
                int dsize;
                dsize = write(fdr,  pkt->data, pkt->data_size);
                if(dsize != pkt->data_size)
                    log_print("dump data write failed!size=%d len=%d\n",dsize,pkt->data_size);
            }  
            #endif
            pkt->avpkt_newflag = 1;
            pkt->avpkt_isvalid = 1;  
			//log_print("[%s:%d]read finish-data_size=%d!\r",__FUNCTION__, __LINE__,pkt->data_size); 
        }
        break;
	}	
	return PLAYER_SUCCESS;
}

int read_av_packet(play_para_t *para, am_packet_t *pkt)
{		
	char raw_mode = para->playctrl_info.raw_mode;		
	int ret = PLAYER_SUCCESS;
    
	if(para == NULL || pkt == NULL)
		return PLAYER_RD_EMPTYP;

	if(raw_mode == 1)
	{			
        ret = raw_read(para, pkt);
		if(ret != PLAYER_SUCCESS && ret != PLAYER_RD_AGAIN)
		{
			log_print("raw read failed!\n");
			return ret;
		}
	}
	else if(raw_mode == 0)
	{	
        ret = non_raw_read(para, pkt);
		if(ret != PLAYER_SUCCESS && ret != PLAYER_RD_AGAIN)
		{
			log_print("non raw read failed!\n");
			return ret;
		}
	}
    if(ret == PLAYER_RD_AGAIN)
    {                    
        para->playctrl_info.need_reset = 1;
    }
	return ret;
}

static int write_header(play_para_t *para, am_packet_t *pkt)
{
	int write_bytes = 0, len = 0;    
    
	if(pkt->hdr && pkt->hdr->size > 0)
	{
        if((NULL == pkt->codec) ||(NULL == pkt->hdr->data))
        {
            log_error("[write_header]codec null!\n");
            return PLAYER_EMPTY_P;
        }        
		while(1)
		{
			write_bytes = codec_write(pkt->codec, pkt->hdr->data+len, pkt->hdr->size-len);
			if(write_bytes < 0 ||write_bytes > (pkt->hdr->size-len))
			{
                if(-errno != AVERROR(EAGAIN))
                {
    				log_print("ERROR:write header failed!\n");		
    				return PLAYER_WR_FAILED;
                }
                else
                    continue;
			}
			else 
			{
                #if DUMP_WRITE
                int size;
                //if(fd > 0 && pkt->type == CODEC_VIDEO)
                if(fdw>0)
                {
                    size = write(fdw, pkt->hdr->data+len, write_bytes);
                    if(size != write_bytes)
                        log_print("dump data write failed!size=%d bytes=%d\n",size,write_bytes);
                }                
                #endif
				len += write_bytes;
				if(len == pkt->hdr->size)				
					break;				
			}					
		}			
	}
	return PLAYER_SUCCESS;
}

static int check_write_finish(play_para_t *para, am_packet_t *pkt)
{     
    if(para->playctrl_info.read_end_flag)
    {
    	if(para->playctrl_info.raw_mode 
            && (para->write_size.total_bytes == para->read_size.total_bytes))
            return PLAYER_WR_FINISH;
    		
    	if(!para->playctrl_info.raw_mode   
    		&&(para->write_size.vpkt_num == para->read_size.vpkt_num) 
    		&&(para->write_size.apkt_num == para->read_size.apkt_num))	   	
            return PLAYER_WR_FINISH;       
    }
	return PLAYER_WR_FAILED;
}

static unsigned int rm_offset_search_pts(AVStream *pStream, unsigned int timepoint)
{
    int64_t wanted_pts = timepoint * 1000;
    int index_entry, index_entry_f, index_entry_b;
    int64_t pts_f, pts_b;

    index_entry_f = av_index_search_timestamp(pStream, wanted_pts, 0);
    index_entry_b = av_index_search_timestamp(pStream, wanted_pts, AVSEEK_FLAG_BACKWARD);

    if (index_entry_f < 0)
    {
        if (index_entry_b < 0)
            return 0;
        else
            return pStream->index_entries[index_entry_b].pos;
    }
    if (index_entry_b < 0)
    {
        if (index_entry_f < 0)
            return 0;
        else
            return pStream->index_entries[index_entry_f].pos;
    }
    
    pts_f = pStream->index_entries[index_entry_f].timestamp;
    pts_b = pStream->index_entries[index_entry_b].timestamp;

    if ((wanted_pts - pts_b) < (pts_f - wanted_pts))
        index_entry = index_entry_b;
    else
        index_entry = index_entry_f;
    
    return pStream->index_entries[index_entry].pos;
}

static unsigned int rm_offset_search(play_para_t *am_p, int64_t offset, unsigned int time_point)
{    
    int read_length = 0;
    unsigned char *data;
    unsigned short video_id = am_p->vstream_info.video_pid;
	unsigned short audio_id = am_p->astream_info.audio_pid;
	unsigned skip_byte = 0;
	unsigned char *pkt;
	const unsigned int read_size = 16*1024;
	int64_t cur_offset=0;
	unsigned short sync_flag = 0; 
    int i = 0;
    AVStream *pStream;

    AVFormatContext *s = am_p->pFormatCtx;

    /* first check the video stream index table */
    for (i=0; i < s->nb_streams; i++)
    {
        pStream = s->streams[i];
        if (pStream->index == am_p->vstream_info.video_index)
            break;
    }
    
    if (i < s->nb_streams)
    {
        if (pStream->nb_index_entries > 1)
            return rm_offset_search_pts(pStream, time_point);
    }

    /* no index, then search byte by byte */
    data = MALLOC(read_size+12);
    if (!data){
        return 0;
    }        
    cur_offset = offset;    
    while (1)
    {        
        url_fseek(s->pb, offset, SEEK_SET);
        read_length = get_buffer(s->pb, data+12, read_size);          
        if (read_length<=0)        
        {
			FREE(data);
			return 0;        
        }
        else 
        {
            pkt = data+12;
            while (1)
            {
                for (i=0;i<read_size;i++)
                {
                    if (skip_byte>0)
                    {
                        skip_byte--;
                    	if (skip_byte==0){
                            //media_packet_header
                            unsigned short version = (pkt[0]<<8)|pkt[1];
                            unsigned short size = (pkt[2]<<8)|pkt[3];
                            unsigned short streamid = (pkt[4]<<8)|pkt[5];
                            unsigned char flag = pkt[11];
                            unsigned int timestamp;
                           
                            if (((version==0)||(version==1))
                                &&(size>=12)
                                &&((streamid==video_id)||(streamid==audio_id)))
                            {                                
                                if ((flag&2)&&(streamid==video_id)){
                                    timestamp = (pkt[6]<<24)|(pkt[7]<<16)|(pkt[8]<<8)|pkt[9];
                                    cur_offset += pkt - (data+12);
                                    FREE(data);
                                    return cur_offset;
                                }
                                else 
                                {
                                    skip_byte = size;                                    
                                }
                                sync_flag = 0;                                 
                            }                               
                        }                        
                    }
                    else
                    {
                        unsigned short version = (pkt[0]<<8)|pkt[1];
                        unsigned short size = (pkt[2]<<8)|pkt[3];
                        unsigned short streamid = (pkt[4]<<8)|pkt[5];                        
                    	if (((version==0)||(version==1))
                    	    &&(size>=12)
                    	    &&((streamid==video_id)||(streamid==audio_id))){
                            skip_byte = size;
                            sync_flag = 0;                            
                    	}
                    }
                    pkt++;
                }               
                sync_flag++;
                MEMCPY(data, data+read_size, 12);                
                cur_offset += read_size;     
                //log_print("[%s:%d]cur_offset=%x file_size=%x\n",__FUNCTION__, __LINE__,cur_offset,s->file_size);
                if (cur_offset < s->file_size)
                    url_fseek(s->pb, cur_offset, SEEK_SET);
                read_length = get_buffer(s->pb, data+12, read_size);                    
                if ((read_length<=0)||(sync_flag==1024))
                {
                   FREE(data);
                   return 0;
                }
                pkt = data;
            }
        }       
    } 
    FREE(data);
    return 0;
}


int time_search(play_para_t *am_p)
{	
    AVFormatContext *s = am_p->pFormatCtx;
    unsigned int time_point = am_p->playctrl_info.time_point;
	int64_t timestamp = 0;
	int64_t offset = 0;
	unsigned int temp = 0;
	int stream_index= -1;
	int ret;
    int seek_flags = AVSEEK_FLAG_BACKWARD;

	temp = (unsigned int)(s->duration/AV_TIME_BASE);
    log_info("[time_search:%d]time_point =%d temp=%d duration= %lld\n",__LINE__,time_point,temp,s->duration);	
    /* if seeking requested, we execute it */ 
    if (time_point >= 0 &&(time_point <temp))   
	{		
		if(am_p->file_type == AVI_FILE ||
           am_p->file_type == MP4_FILE ||
           am_p->file_type == MKV_FILE ||
           am_p->file_type == FLV_FILE ||
           am_p->file_type == MOV_FILE ||
           am_p->file_type == P2P_FILE)
        {            
        	timestamp = (int64_t)time_point * AV_TIME_BASE;					
		    /* add the stream start time */
		    if (s->start_time != AV_NOPTS_VALUE)
		        timestamp += s->start_time;
            if (timestamp == s->start_time)
            {
                if (am_p->file_type == AVI_FILE)
                {
                    stream_index = am_p->first_index;
                }
                seek_flags |= AVSEEK_FLAG_ANY;
            }
         	log_info("[time_search:%d] stream_index %d, time_point=%d timestamp=%lld start_time=%lld\n",
              __LINE__,stream_index, time_point, timestamp, s->start_time);
            
	        ret = av_seek_frame(s, stream_index, timestamp, seek_flags);
	        if (ret < 0) 
			{
	            log_info("%s: could not seek to position %0.3f s\n",s->filename, (double)timestamp / AV_TIME_BASE);		
				return -1;
	        }		
		}
		else
		{           
			offset = ((int64_t)time_point * (s->bit_rate >> 3));				
			log_info("time_point = %d  bit_rate=%x offset=0x%llx\n",time_point, s->bit_rate, offset);
                  
            if(am_p->file_type == RM_FILE)
    		{                
                if(offset > 0)
                    offset = rm_offset_search(am_p,am_p->data_offset+offset,time_point);
                else
                    offset = am_p->data_offset;
            }            
            log_info("time_point = %d  offset=%llx \n",time_point, offset);   
            ret = url_fseek(s->pb, offset, SEEK_SET);
		    if (ret < 0)
            {
                log_info("%s: could not seek to position %llx\n",s->filename, offset);
			    return PLAYER_FAILED;
		    }    
		 }
            
        /* reset seek info */
        //time_point = 0;
    }
	return PLAYER_SUCCESS;
}

int write_av_packet(play_para_t *para, am_packet_t *pkt)
{	
	int write_bytes = 0, len = 0, ret;	
    unsigned char *buf;
    int size ;
	#if DUMP_WRITE
    if(fdw==-1)
    {
        fdw=open("./dump_write.dat",O_CREAT|O_RDWR);
        if(fdw < 0)
            log_print("creat dump file failed!fd=%d\n",fdw);
    }
    #endif       
       
	if(pkt->avpkt_newflag)
    {
        if (pkt->type != CODEC_SUBTITLE)
        {
            if (pkt->avpkt_isvalid)
        	{
               	ret = check_in_pts(para, pkt);	
                if(ret != PLAYER_SUCCESS)
                {
                    log_error("check in pts failed\n");
                    return PLAYER_WR_FAILED;
                }
            }
            if(write_header(para, pkt) == PLAYER_WR_FAILED)
    		{
                log_error("[%s]write header failed!\n", __FUNCTION__);
                return PLAYER_WR_FAILED;
            }
        }
        else
        {
            process_es_subtitle(para, pkt);
        }
        pkt->avpkt_newflag = 0;
	}
    buf = pkt->data;
    size = pkt->data_size ;
    while(size>0 && pkt->avpkt_isvalid)
	{   
        write_bytes = codec_write(pkt->codec, (char *)buf, size);       
		if(write_bytes < 0 ||write_bytes > size)
		{            
            if(-errno != AVERROR(EAGAIN))
			{
                log_print("write codec data failed!\n");
    			return PLAYER_WR_FAILED;
            }       
            else
            {
                pkt->data += len;
                pkt->data_size-=len;
                player_thread_wait(para,RW_WAIT_TIME);   
                //log_print("[%s:%d]eagain---data_size=%d!type=%d\n",__FUNCTION__, __LINE__,pkt->data_size,pkt->type);             
                return PLAYER_SUCCESS;
            }
		}	
		else
		{            
         #if DUMP_WRITE 
            int dsize;
            //if(fd > 0 && debug && pkt->type == CODEC_VIDEO)
            if(fdw>0)
            {                   
                dsize = write(fdw, buf, write_bytes);
                if(dsize != write_bytes)
                    log_print("dump data write failed!size=%d len=%d\n",size,len);
            }                
          #endif            
			len += write_bytes;            
			if(len == pkt->data_size)
			{
				if((pkt->type == CODEC_VIDEO) && (!para->playctrl_info.raw_mode))
					para->write_size.vpkt_num ++;
				else if((pkt->type == CODEC_AUDIO) && (!para->playctrl_info.raw_mode))
					para->write_size.apkt_num ++;
                if(para->playctrl_info.raw_mode)
				    para->write_size.total_bytes += len;      
                if(pkt->avpkt)
                {                    
                    av_free_packet(pkt->avpkt);
                }
				pkt->avpkt_isvalid=0;                
                pkt->data_size = 0;
				//log_print("[%s:%d]write finish pkt->data_size=%d\r",__FUNCTION__, __LINE__,pkt->data_size);   
                /*if(para->playctrl_info.fast_backward)
                {
                    if(para->playctrl_info.raw_mode)                    
                        log_print("[write_av_packet:%d]twrite=%lld(tread=%lld)\n",__LINE__,
                        para->write_size.total_bytes,para->read_size.total_bytes);
                    else
                        printf("[write_av_packet:%d]vpkt=%d(%d) apkt=%d(%d)\n",__LINE__,
                        para->write_size.vpkt_num, para->read_size.vpkt_num,
                        para->write_size.apkt_num, para->read_size.apkt_num);
                }*/
				break;
			}
            else if(len < pkt->data_size)
            {
                buf += write_bytes;
                size -= write_bytes;
            }
            else
                return PLAYER_WR_FAILED;
                
		}        
	}	    
	if(check_write_finish(para, pkt) == PLAYER_WR_FINISH)        
    {
        #if DUMP_WRITE
        if(fdw>0)
            close(fdw);
        #endif
        return PLAYER_WR_FINISH;
	}    
	return PLAYER_SUCCESS;	
}

int check_in_pts(play_para_t *para, am_packet_t *pkt)
{		
	int last_duration = 0;
    static int last_v_duration=0, last_a_duration=0;
	int64_t pts;
	float duration;	
	long long start_time;  

	if(pkt->type == CODEC_AUDIO)
	{   
		duration = para->astream_info.audio_duration;		
		start_time = para->astream_info.start_time;
        last_duration = last_a_duration;
	}
	else if(pkt->type == CODEC_VIDEO)
	{      
		duration = para->vstream_info.video_pts;
		start_time = para->vstream_info.start_time;
        last_duration = last_v_duration;
	}  
    
	if(para->stream_type == STREAM_ES)
	{
		if (INT64_0 != pkt->avpkt->pts)
	    {           
	        pts = pkt->avpkt->pts * duration;
	    	if(pts < start_time)
			 	pts = pts * last_duration;  

	    	if(codec_checkin_pts(pkt->codec, pts) != 0)
			{
				log_error("ERROR pid[%d]: check in pts error!\n",para->player_id);
				return PLAYER_PTS_ERROR;
	    	}
            //log_print("[check_in_pts:%d]type=%d pkt->pts=%llx pts=%llx start_time=%llx duration=%.2f\n",__LINE__,pkt->type,pkt->avpkt->pts,pts, start_time, duration);
		
		}
		else if (INT64_0 != pkt->avpkt->dts)
		{	
			pts = pkt->avpkt->dts * duration* last_duration;   
            //log_print("[check_in_pts:%d]type=%d pkt->dts=%llx pts=%llx\n",__LINE__,pkt->type,pkt->avpkt->dts,pts);

			if(codec_checkin_pts(pkt->codec, pts)!=0)
			{
				log_error("ERROR pid[%d]: check in dts error!\n",para->player_id);
				return PLAYER_PTS_ERROR;
			}    
            if (pkt->type == CODEC_AUDIO)        
            {                
                last_a_duration = pkt->avpkt->duration;        
            }
            else if (pkt->type == CODEC_VIDEO)        
            {                
                last_v_duration = pkt->avpkt->duration;
            }
		}
		else
		{	
			if(!para->astream_info.check_first_pts && pkt->type == CODEC_AUDIO)
			{
                if(codec_checkin_pts(pkt->codec, 0)!= 0)
				{
					log_print("ERROR pid[%d]: check in 0 to audio pts error!\n",para->player_id);
					return PLAYER_PTS_ERROR;
				}
			}
            if(!para->vstream_info.check_first_pts && pkt->type == CODEC_VIDEO)
			{
                if(codec_checkin_pts(pkt->codec, 0)!= 0)
				{
					log_print("ERROR pid[%d]: check in 0 to audio pts error!\n",para->player_id);
					return PLAYER_PTS_ERROR;
				}
			}
		}        
		if(pkt->type == CODEC_AUDIO && !para->astream_info.check_first_pts)
		{
			para->astream_info.check_first_pts = 1;
		}
        else if(pkt->type == CODEC_VIDEO && !para->vstream_info.check_first_pts)
		{
			para->vstream_info.check_first_pts = 1;
		} 
	}
    else if(para->stream_type == STREAM_AUDIO)
    {        
        if(!para->astream_info.check_first_pts)
        {
            pts = para->playctrl_info.time_point * PTS_FREQ;
            if(codec_checkin_pts(pkt->codec, pts)!= 0)
    		{
    			log_print("ERROR pid[%d]: check in 0 to audio pts error!\n",para->player_id);
    			return PLAYER_PTS_ERROR;
    		}
            para->astream_info.check_first_pts = 1;
        }
    }
	return PLAYER_SUCCESS;
}

int set_header_info(play_para_t *para, am_packet_t *pkt)
{
    int ret;
	if(pkt->avpkt_newflag)
    {
        if(pkt->hdr)
            pkt->hdr->size = 0;
       
        if(pkt->type == CODEC_VIDEO)
        {
            if((para->vstream_info.video_format == VFORMAT_H264) && 
               (para->file_type != AVI_FILE))
            {           
                ret = h264_update_frame_header(pkt);
                if(ret != PLAYER_SUCCESS)
                    return ret;
            }            
    		else if (para->vstream_info.video_format == VFORMAT_MPEG4) 
    		{
    			if (para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_MPEG4_3)
                	return divx3_prefix(pkt);
    			else if (para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_H263)
    			{
    				unsigned char *vld_buf;
    				int vld_len, vld_buf_size = para->vstream_info.video_width*para->vstream_info.video_height*2;

    				if ( !pkt->data_size )
    					return PLAYER_SUCCESS;

    				if ( (pkt->data[0] == 0) && (pkt->data[1] == 0) && (pkt->data[2] == 1) && (pkt->data[3] == 0xb6) )
    					return PLAYER_SUCCESS;

    				vld_buf = (unsigned char *)MALLOC(vld_buf_size);
    				if ( !vld_buf )
    					return PLAYER_NOMEM;
                    
    				if ( para->vstream_info.flv_flag )
    					vld_len = h263vld(pkt->data, vld_buf, pkt->data_size, 1);
    				else
    					vld_len = h263vld(pkt->data, vld_buf, pkt->data_size, 0);

    				//printf("###%02x %02x %02x %02x %02x %02x %02x %02x###\n", pkt->data[0], pkt->data[1], pkt->data[2], pkt->data[3], pkt->data[4], pkt->data[5], pkt->data[6], pkt->data[7]);
    				//printf("###pkt->data_size = %d, vld_buf_size = %d, vld_len = %d###\n", pkt->data_size, vld_buf_size, vld_len);

    				if ( vld_len > 0 )
    				{
    					if ( pkt->buf )
    						FREE(pkt->buf);
    					pkt->buf = vld_buf;
    					pkt->buf_size = vld_buf_size;
    					pkt->data = pkt->buf;
    					pkt->data_size = vld_len;
    				}
    				else
    				{
    					FREE(vld_buf);
    					pkt->data_size = 0;
    				}
    			}
    		}
    		else if (para->vstream_info.video_format == VFORMAT_VC1)
    		{
    			if ( para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WMV3 )
    			{
    				unsigned i, check_sum = 0, data_len = 0;
    				
    				if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL))
    				{
    					FREE(pkt->hdr->data);
    					pkt->hdr->data = NULL;
    				}

    				if (pkt->hdr == NULL)
    				{
    					pkt->hdr = MALLOC(sizeof(hdr_buf_t));
    					if (!pkt->hdr)
    					{
    						log_print("[wmv3_prefix]pid=%d NOMEM!",para->player_id);
    			            return PLAYER_FAILED;
    			        }
    			        
    			        pkt->hdr->data = NULL;
    			        pkt->hdr->size = 0;
    				}

					if ( pkt->avpkt->flags )
					{
	    				pkt->hdr->data = MALLOC(para->vstream_info.extradata_size+26+22);
	    				if (pkt->hdr->data == NULL)
	    				{
	    					log_print("[wmv3_prefix]pid=%d NOMEM!",para->player_id);
	    			        return PLAYER_FAILED;
	    			    }

						pkt->hdr->data[0] = 0;
						pkt->hdr->data[1] = 0;
						pkt->hdr->data[2] = 1;
						pkt->hdr->data[3] = 0x10;
						
    					data_len = para->vstream_info.extradata_size + 4;
						pkt->hdr->data[4] = 0;
						pkt->hdr->data[5] = (data_len >> 16) & 0xff;
						pkt->hdr->data[6] = 0x88;
						pkt->hdr->data[7] = (data_len >> 8) & 0xff;
						pkt->hdr->data[8] = data_len & 0xff;
						pkt->hdr->data[9] = 0x88;

						pkt->hdr->data[10] = 0xff;
						pkt->hdr->data[11] = 0xff;
						pkt->hdr->data[12] = 0x88;
						pkt->hdr->data[13] = 0xff;
						pkt->hdr->data[14] = 0xff;
						pkt->hdr->data[15] = 0x88;

						for ( i = 4 ; i < 16 ; i++ )
							check_sum += pkt->hdr->data[i];

						pkt->hdr->data[16] = (check_sum >> 8) & 0xff;
						pkt->hdr->data[17] = check_sum & 0xff;
						pkt->hdr->data[18] = 0x88;
						pkt->hdr->data[19] = (check_sum >> 8) & 0xff;
						pkt->hdr->data[20] = check_sum & 0xff;
						pkt->hdr->data[21] = 0x88;

						pkt->hdr->data[22] = (para->vstream_info.video_width >> 8) & 0xff;
						pkt->hdr->data[23] = para->vstream_info.video_width & 0xff;
						pkt->hdr->data[24] = (para->vstream_info.video_height >> 8) & 0xff;
						pkt->hdr->data[25] = para->vstream_info.video_height & 0xff;

						MEMCPY(pkt->hdr->data+26, para->vstream_info.extradata, para->vstream_info.extradata_size);

						check_sum = 0;
						data_len = para->vstream_info.extradata_size+26;
					}
					else
					{
	    				pkt->hdr->data = MALLOC(22);
	    				if (pkt->hdr->data == NULL)
	    				{
	    					log_print("[wmv3_prefix]pid=%d NOMEM!",para->player_id);
	    			        return PLAYER_FAILED;
	    			    }
					}

    				pkt->hdr->data[data_len+0] = 0;
    				pkt->hdr->data[data_len+1] = 0;
    				pkt->hdr->data[data_len+2] = 1;
    				pkt->hdr->data[data_len+3] = 0xd;

    	        	pkt->hdr->data[data_len+4] = 0;
    	        	pkt->hdr->data[data_len+5] = (pkt->data_size >> 16) & 0xff;
    	        	pkt->hdr->data[data_len+6] = 0x88;
    	        	pkt->hdr->data[data_len+7] = (pkt->data_size >> 8) & 0xff;
    	        	pkt->hdr->data[data_len+8] = pkt->data_size & 0xff;
    	        	pkt->hdr->data[data_len+9] = 0x88;

    	        	pkt->hdr->data[data_len+10] = 0xff;
    	        	pkt->hdr->data[data_len+11] = 0xff;
    	        	pkt->hdr->data[data_len+12] = 0x88;
    	        	pkt->hdr->data[data_len+13] = 0xff;
    	        	pkt->hdr->data[data_len+14] = 0xff;
    	        	pkt->hdr->data[data_len+15] = 0x88;

    	        	for ( i = data_len+4 ; i < data_len+16 ; i++ )
    	        		check_sum += pkt->hdr->data[i];

    	        	pkt->hdr->data[data_len+16] = (check_sum >> 8) & 0xff;
    	        	pkt->hdr->data[data_len+17] = check_sum & 0xff;
    	        	pkt->hdr->data[data_len+18] = 0x88;
    	        	pkt->hdr->data[data_len+19] = (check_sum >> 8) & 0xff;
    	        	pkt->hdr->data[data_len+20] = check_sum & 0xff;
    	        	pkt->hdr->data[data_len+21] = 0x88;

    				pkt->hdr->size = data_len+22;
    			}
    			else if ( para->vstream_info.video_codec_type == VIDEO_DEC_FORMAT_WVC1 )
    			{
    				if ((pkt->hdr != NULL) && (pkt->hdr->data != NULL))
    				{
    					FREE(pkt->hdr->data);
    					pkt->hdr->data = NULL;
    				}

    				if (pkt->hdr == NULL)
    				{
    					pkt->hdr = MALLOC(sizeof(hdr_buf_t));
    					if (!pkt->hdr)
    					{
    						log_print("[wvc1_prefix] NOMEM!");
    			            return PLAYER_FAILED;
    			        }
    			        
    			        pkt->hdr->data = NULL;
    			        pkt->hdr->size = 0;
    				}

    				pkt->hdr->data = MALLOC(4);
    				if (pkt->hdr->data == NULL)
    				{
    					log_print("[wvc1_prefix] NOMEM!");
    			        return PLAYER_FAILED;
    			    }

    				pkt->hdr->data[0] = 0;
    				pkt->hdr->data[1] = 0;
    				pkt->hdr->data[2] = 1;
    				pkt->hdr->data[3] = 0xd;
    				pkt->hdr->size = 4;
    			}
    		}
        }
        else if(pkt->type == CODEC_AUDIO)
        {        
            if(!para->playctrl_info.raw_mode && (para->astream_info.audio_format == AFORMAT_AAC))
            {           
               if(pkt->hdr == NULL)
        		{ 
        			pkt->hdr = MALLOC(sizeof(hdr_buf_t));
                    if(!pkt->hdr)
                        return PLAYER_NOMEM;
        			pkt->hdr->data = (char *)MALLOC(ADTS_HEADER_SIZE);                
                    if(!pkt->hdr->data)
                        return PLAYER_NOMEM;
        		}           
                adts_add_header(para,pkt);
            }
        }	    
    }
	return PLAYER_SUCCESS;
}

void av_packet_release(am_packet_t *pkt)
{   
	if(pkt->avpkt_isvalid)
	{       
		av_free_packet(pkt->avpkt);
		pkt->avpkt_isvalid=0;
	}   
	if(pkt->buf!= NULL)
	{       
		FREE(pkt->buf);
		pkt->buf= NULL;
	}   
	if(pkt->hdr != NULL)
	{        
		FREE(pkt->hdr->data);
        pkt->hdr->data = NULL;        
        FREE(pkt->hdr);
		pkt->hdr = NULL;
	}    
	pkt->codec = NULL;	
}

int poll_sub(am_packet_t *pkt)
{
    if (pkt->codec)
    {
        return codec_poll_sub(pkt->codec);
    }
    else
        return 0;
}

int get_sub_size(am_packet_t *pkt)
{
    if (pkt->codec)
    {
        return codec_get_sub_size(pkt->codec);
    }
    else
        return 0;
}

int read_sub_data(am_packet_t *pkt, char *buf, unsigned int length)
{
    if (pkt->codec)
    {
        return codec_read_sub_data(pkt->codec, buf, length);
    }
    else
        return 0;
}

int write_sub_data(am_packet_t *pkt, char *buf, unsigned int length)
{
    int write_bytes, size, len;

    if (pkt->codec == NULL)
        return 0;

    size = length;
    while (size > 0)
    {
        write_bytes = codec_write_sub_data(pkt->codec, buf, size);
        if(write_bytes < 0)
		{            
            if(-errno != AVERROR(EAGAIN))
			{
                log_print("[%s:%d]write sub data failed!\n", __FUNCTION__, __LINE__);
    			return PLAYER_WR_FAILED;
            }       
            else
            {
                continue;
            }
        }	
        else
        {            
            len += write_bytes;            
            if(len == length)
            {
                break;
			}
            size -= write_bytes;
		}        
    }

    return PLAYER_SUCCESS;
}

int process_es_subtitle(play_para_t *para, am_packet_t *pkt)
{
    unsigned char sub_header[16] = {0x41, 0x4d, 0x4c, 0x55, 0xaa, 0};
    unsigned int sub_type;
    int64_t sub_pts;
    static int last_duration = 0;
    float duration = para->sstream_info.sub_duration;
    long long start_time = para->sstream_info.start_time;
    int data_size = pkt->avpkt->size;

    /* get pkt pts */
	if (INT64_0 != pkt->avpkt->pts)
    {
        sub_pts = pkt->avpkt->pts * duration;
        if(sub_pts < start_time)
            sub_pts = sub_pts * last_duration;  
	}
	else if (INT64_0 != pkt->avpkt->dts)
	{	
        sub_pts = pkt->avpkt->dts * duration * last_duration;
        last_duration = pkt->avpkt->duration;
	}
	else
	{	
        if(!para->sstream_info.check_first_pts)
            sub_pts = 0;
	} 
    if (!para->sstream_info.check_first_pts)
        para->sstream_info.check_first_pts = 1;

    /* first write the header */
    sub_type = para->sstream_info.sub_type;
    sub_header[5] = (sub_type >> 16) & 0xff;
    sub_header[6] = (sub_type >> 8) & 0xff;
    sub_header[7] = sub_type & 0xff;
    sub_header[8] = (data_size >> 24) & 0xff;
    sub_header[9] = (data_size >> 16) & 0xff;
    sub_header[10] = (data_size >> 8) & 0xff;
    sub_header[11] = data_size & 0xff;
    sub_header[12] = (sub_pts >> 24) & 0xff;
    sub_header[13] = (sub_pts >> 16) & 0xff;
    sub_header[14] = (sub_pts >> 8) & 0xff;
    sub_header[15] = sub_pts & 0xff;

    if (write_sub_data(pkt, (char *)&sub_header, sizeof(sub_header)))
    {
        log_print("[%s:%d]write sub header failed\n", __FUNCTION__, __LINE__);
    }   

    return PLAYER_SUCCESS;
}

int poll_cntl(am_packet_t *pkt)
{
    if (pkt->codec)
    {
        return codec_poll_cntl(pkt->codec);
    }
    else
        return 0;
}

int get_cntl_state(am_packet_t *pkt)
{
    if (pkt->codec)
    {
        return codec_get_cntl_state(pkt->codec);
    }
    else
        return 0;
}

int set_cntl_mode(play_para_t *para, unsigned int mode)
{
    if (para->vstream_info.has_video == 0)
        return 0;
	
    if (para->vcodec)
        return codec_set_cntl_mode(para->vcodec, mode);
    else if(para->codec)
        return codec_set_cntl_mode(para->codec, mode);
    return 0;
}

int set_cntl_avthresh(play_para_t *para, unsigned int avthresh)
{
    if (para->vstream_info.has_video == 0)
        return 0;
	
    if (para->vcodec)
        return codec_set_cntl_avthresh(para->vcodec, avthresh);
    else
        return codec_set_cntl_avthresh(para->codec, avthresh);
}

int set_cntl_syncthresh(play_para_t *para)
{
    if (para->vstream_info.has_video == 0)
        return 0;

    if (para->vcodec)
        return codec_set_cntl_syncthresh(para->vcodec, para->astream_info.has_audio);
    else
        return codec_set_cntl_syncthresh(para->codec, para->astream_info.has_audio);
}

void player_switch_audio(play_para_t *para)
{
    codec_para_t *pcodec;
    AVStream *pstream;
    int i;
    short audio_index;
    AVCodecContext  *pCodecCtx;
    AVFormatContext *pFCtx = para->pFormatCtx;
    
    /* check if it has audio */
    if (para->astream_info.has_audio == 0)
        return;

    /* find stream for new id */
    for (i=0; i<pFCtx->nb_streams; i++)
    {
        pstream = pFCtx->streams[i];
        if (pstream->id == para->playctrl_info.switch_audio_id)
            break;
    }

    if (i == pFCtx->nb_streams)
    {
        log_print("[%s:%d]no stream found for new aid\n", __FUNCTION__, __LINE__);
        return;
    }
    
    if (para->acodec)
        pcodec = para->acodec;
    else
        pcodec = para->codec;

    /* automute */
    codec_audio_automute(1);

    /* close audio */
    codec_close_audio(pcodec);

    /* first set an invalid audio id */
    pcodec->audio_pid = 0xffff;
	para->astream_info.audio_index=-1;
    if (codec_set_audio_pid(pcodec))
    {
        log_print("[%s:%d]set invalid audio pid failed\n", __FUNCTION__, __LINE__);
        return;
    }

    /* get new information */
    audio_index = pstream->index;
    log_print("[%s:%d]audio_index %d, i %d\n", __FUNCTION__, __LINE__, audio_index, i);
    if (audio_index == -1)
    {
        log_print("[%s:%d]no index found\n", __FUNCTION__, __LINE__);
        return;
    }
    else
    {
        pCodecCtx= pFCtx->streams[audio_index]->codec;
    }

    para->astream_info.audio_format = audio_type_convert(pCodecCtx->codec_id,para->file_type);
    if(para->astream_info.audio_format < 0 || para->astream_info.audio_format >= AFORMAT_MAX)
    {
        log_print("[%s:%d]unkown audio format\n", __FUNCTION__, __LINE__);
        para->astream_info.has_audio = 0;
        return;
    }
    if(0!=pstream->time_base.den)
    {
        para->astream_info.audio_duration = PTS_FREQ * ((float)pstream->time_base.num / pstream->time_base.den);           
        para->astream_info.start_time = pstream->start_time * pstream->time_base.num * PTS_FREQ/ pstream->time_base.den;
    }

    para->astream_info.audio_channel = pCodecCtx->channels;    
    para->astream_info.audio_samplerate = pCodecCtx->sample_rate;
        
    if (!para->playctrl_info.raw_mode
        && para->astream_info.audio_format == AFORMAT_AAC)
    {
        adts_header_t *adts_hdr;

        if (para->astream_info.adts_header)
            adts_hdr = para->astream_info.adts_header;
        else
        {
            adts_hdr = MALLOC(sizeof(adts_header_t));      
            if(adts_hdr == NULL)
            {
                log_print("[%s:%d]no memory for adts_hdr\n", __FUNCTION__, __LINE__);
                return;
            }
        }
        extract_adts_header_info(pCodecCtx->extradata, pCodecCtx->extradata_size,adts_hdr);                        
        para->astream_info.adts_header=adts_hdr;           
    }       
    para->astream_info.audio_index = audio_index;

    /* reinit audio info */
    pcodec->has_audio = 1;
    pcodec->audio_type = para->astream_info.audio_format;
    pcodec->audio_pid = pstream->id;		
    pcodec->audio_channels = para->astream_info.audio_channel;
    pcodec->audio_samplerate = para->astream_info.audio_samplerate;

    if (codec_audio_reinit(pcodec))
    {
        log_print("[%s:%d]audio reinit failed\n", __FUNCTION__, __LINE__);
        return;
    }

    /* reset audio */
    if (codec_reset_audio(pcodec))
    {
        log_print("[%s:%d]reset audio failed\n", __FUNCTION__, __LINE__);
        return;
    }

    /* resume audio */
    codec_resume_audio(pcodec, para->astream_info.resume_audio);

    /* unmute*/
    codec_audio_automute(0);

    return;
}
void player_switch_sub(play_para_t *para)
{
    codec_para_t *pcodec;
    AVStream *pstream;
    int i;    
    AVFormatContext *pFCtx = para->pFormatCtx;
    
    /* check if it has audio */
    if (para->sstream_info.has_sub == 0)
        return;

    /* find stream for new id */
    for (i=0; i<pFCtx->nb_streams; i++)
    {
        pstream = pFCtx->streams[i];
        if (pstream->id == para->playctrl_info.switch_sub_id)
            break;
    }

    if (i == pFCtx->nb_streams)
    {
        log_print("[%s:%d]no stream found for new sid\n", __FUNCTION__, __LINE__);
        return;
    }

    /* only ps and ts stream */
    if (para->codec == NULL)
    {
        para->sstream_info.sub_index = i;
        if (codec_reset_subtile(para->scodec))
        {
            log_print("[%s:%d]reset subtile failed\n", __FUNCTION__, __LINE__);
        }
        return;
    }
    else
        pcodec = para->codec;

    /* first set an invalid sub id */
    pcodec->sub_pid = 0xffff;
    if (codec_set_sub_id(pcodec))
    {
        log_print("[%s:%d]set invalid sub pid failed\n", __FUNCTION__, __LINE__);
        return;
    }

    /* reset sub */
    pcodec->sub_pid = pstream->id;
    if (codec_set_sub_id(pcodec))
    {
        log_print("[%s:%d]set invalid sub pid failed\n", __FUNCTION__, __LINE__);
        return;
    }
    if (codec_reset_subtile(pcodec))
    {
        log_print("[%s:%d]reset subtile failed\n", __FUNCTION__, __LINE__);
    }

    return;
}
void av_packet_init(am_packet_t *pkt)
{
	pkt->avpkt 	= NULL;
	pkt->avpkt_isvalid=0;
    pkt->avpkt_newflag=0;
	pkt->codec 	= NULL;
	pkt->hdr	= NULL;
	pkt->buf    = NULL;
	pkt->buf_size = 0;
	pkt->data 	= NULL;
	pkt->data_size 	= 0;   
}

static void av_packet_reset(am_packet_t *pkt)
{	
	pkt->avpkt_isvalid=0;
    pkt->avpkt_newflag = 0;	
    pkt->data_size 	= 0;
}

int player_reset(play_para_t *p_para,am_packet_t *pkt)
{   
    int ret = PLAYER_SUCCESS;
	player_para_reset(p_para);  
    av_packet_reset(pkt);    
    ret = player_dec_reset(p_para);     
    return ret;    
}

