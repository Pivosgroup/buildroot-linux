/*****************************************
 * name	: av_hwdec.c
 * function: decoder hardware relative functions
 *date		: 2010.2.4
 *****************************************/
#include <log_print.h>
#include "stream_decoder.h"
#include "player_priv.h"
#include "player_hwdec.h"

static int check_size_in_buffer(unsigned char *p, int len)
{
	unsigned int size;
	unsigned char *q = p;       
	while ( (q+4) < (p+len) )
	{
		size = (*q<<24)|(*(q+1)<<16)|(*(q+2)<<8)|(*(q+3));       
		if ( size & 0xff000000 )
			return 0;

		if ( q+size+4 == p+len )
			return 1;

		q += size+4;       
	}    
	return 0;
}

static int check_size_in_buffer3(unsigned char *p, int len)
{
	unsigned int size;
	unsigned char *q = p;       
	while ( (q+3) < (p+len) )
	{
		size = (*q<<16)|(*(q+1)<<8)|(*(q+2));

		if ( q+size+3 == p+len )
			return 1;

		q += size+3;       
	}    
	return 0;
}

static int check_size_in_buffer2(unsigned char *p, int len)
{
	unsigned int size;
	unsigned char *q = p;       
	while ( (q+2) < (p+len) )
	{
		size = (*q<<8)|(*(q+1));       

		if ( q+size+2 == p+len )
			return 1;

		q += size+2;       
	}    
	return 0;
}

/**********************************************************************
0: syncword 12 always: '111111111111' 
12: ID 1 0: MPEG-4, 1: MPEG-2 
13: layer 2 always: '00' 
15: protection_absent 1 
16: profile 2 
18: sampling_frequency_index 4 
22: private_bit 1 
23: channel_configuration 3 
26: original/copy 1 
27: home 1 
28: emphasis 2 only if ID == 0 

ADTS Variable header: these can change from frame to frame 
28: copyright_identification_bit 1 
29: copyright_identification_start 1 
30: aac_frame_length 13 length of the frame including header (in bytes) 
43: adts_buffer_fullness 11 0x7FF indicates VBR 
54: no_raw_data_blocks_in_frame 2 
ADTS Error check 
crc_check 16 only if protection_absent == 0 
}

**************************************************************************/
void extract_adts_header_info(uint8_t *extradata, int size, adts_header_t *hdr)
{
	uint8_t *p = extradata;	
	hdr->profile = (*p >> 3) -1;
	hdr->sample_freq_idx = (*p&0x7)<<1|(*(p+1) >> 7);
	hdr->channel_configuration = (*(p+1) & 0x7f) >> 3;
}

int adts_add_header(play_para_t *para, am_packet_t *pkt)
{	
    adts_header_t *adts_hdr = para->astream_info.adts_header;
    char* p= pkt->hdr->data;	
	adts_hdr->syncword = 0xfff;
	adts_hdr->id = 0;
	adts_hdr->layer = 0;
	adts_hdr->protection_absent = 1;
	adts_hdr->private_bit = 0;
	adts_hdr->original_copy = 0;
	adts_hdr->home = 0;  
	adts_hdr->copyright_identification_bit = 0; 
	adts_hdr->copyright_identification_start = 0;    
	adts_hdr->aac_frame_length = ADTS_HEADER_SIZE + pkt->data_size;
	adts_hdr->adts_buffer_fullness = 0x7ff;
	adts_hdr->number_of_raw_data_blocks_in_frame = 0;   
	p[0] = (char)(adts_hdr->syncword >>4);
	p[1] = (char)((adts_hdr->syncword & 0xf<<4) |
                  (adts_hdr->id << 3)|
                  (adts_hdr->layer << 1)|
                  adts_hdr->protection_absent);
	p[2] = (char)((adts_hdr->profile<<6)|
                  (adts_hdr->sample_freq_idx<<2)|
                  (adts_hdr->private_bit<<1)|
                  (adts_hdr->channel_configuration>>2));
	p[3] = (char)(((adts_hdr->channel_configuration&0x3)<<6)|
				    (adts_hdr->original_copy<<5)|
					(adts_hdr->home<<4)|
					(adts_hdr->copyright_identification_bit<<3)|
					(adts_hdr->copyright_identification_start<<2)|
					(adts_hdr->aac_frame_length)>>11);
	p[4] = (char)((adts_hdr->aac_frame_length>>3)&0xff);
	p[5] = (char)(((adts_hdr->aac_frame_length&0x7)<<5)|
                   (adts_hdr->adts_buffer_fullness>>6));
	p[6] = (char)(((adts_hdr->adts_buffer_fullness&0x3f)<<2)|
                    adts_hdr->number_of_raw_data_blocks_in_frame);   
    pkt->hdr->size = ADTS_HEADER_SIZE;  
    pkt->type = CODEC_AUDIO;
    return PLAYER_SUCCESS;
}

/*************************************************************************/
static int mjpeg_data_prefeeding(am_packet_t *pkt)
{
    const unsigned char mjpeg_addon_data[] = {
        0xff,0xd8,0xff,0xc4,0x01,0xa2,0x00,0x00,0x01,0x05,0x01,0x01,0x01,
        0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,
        0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x01,0x00,0x03,0x01,
        0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,
        0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x10,
        0x00,0x02,0x01,0x03,0x03,0x02,0x04,0x03,0x05,0x05,0x04,0x04,0x00,
        0x00,0x01,0x7d,0x01,0x02,0x03,0x00,0x04,0x11,0x05,0x12,0x21,0x31,
        0x41,0x06,0x13,0x51,0x61,0x07,0x22,0x71,0x14,0x32,0x81,0x91,0xa1,
        0x08,0x23,0x42,0xb1,0xc1,0x15,0x52,0xd1,0xf0,0x24,0x33,0x62,0x72,
        0x82,0x09,0x0a,0x16,0x17,0x18,0x19,0x1a,0x25,0x26,0x27,0x28,0x29,
        0x2a,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,
        0x48,0x49,0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,
        0x65,0x66,0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,
        0x7a,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x92,0x93,0x94,0x95,
        0x96,0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,
        0xaa,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,
        0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,
        0xd9,0xda,0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf1,
        0xf2,0xf3,0xf4,0xf5,0xf6,0xf7,0xf8,0xf9,0xfa,0x11,0x00,0x02,0x01,
        0x02,0x04,0x04,0x03,0x04,0x07,0x05,0x04,0x04,0x00,0x01,0x02,0x77,
        0x00,0x01,0x02,0x03,0x11,0x04,0x05,0x21,0x31,0x06,0x12,0x41,0x51,
        0x07,0x61,0x71,0x13,0x22,0x32,0x81,0x08,0x14,0x42,0x91,0xa1,0xb1,
        0xc1,0x09,0x23,0x33,0x52,0xf0,0x15,0x62,0x72,0xd1,0x0a,0x16,0x24,
        0x34,0xe1,0x25,0xf1,0x17,0x18,0x19,0x1a,0x26,0x27,0x28,0x29,0x2a,
        0x35,0x36,0x37,0x38,0x39,0x3a,0x43,0x44,0x45,0x46,0x47,0x48,0x49,
        0x4a,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0x63,0x64,0x65,0x66,
        0x67,0x68,0x69,0x6a,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x82,
        0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x92,0x93,0x94,0x95,0x96,
        0x97,0x98,0x99,0x9a,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,
        0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,0xb9,0xba,0xc2,0xc3,0xc4,0xc5,
        0xc6,0xc7,0xc8,0xc9,0xca,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,0xd9,
        0xda,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8,0xe9,0xea,0xf2,0xf3,0xf4,
        0xf5,0xf6,0xf7,0xf8,0xf9,0xfa
    };
   
	if(pkt->hdr->data)
    {
        MEMCPY(pkt->hdr->data, &mjpeg_addon_data, sizeof(mjpeg_addon_data));
	    pkt->hdr->size = sizeof(mjpeg_addon_data);
	}
	else
	{
		log_print("[mjpeg_data_prefeeding]No enough memory!\n");
		return PLAYER_FAILED;
	}
    return PLAYER_SUCCESS;
}
static int mjpeg_write_header(play_para_t *para, am_packet_t *pkt)
{
    mjpeg_data_prefeeding(pkt);
	if(para->vcodec)			
		pkt->codec = para->vcodec;
	else
	{
		log_print("[pre_header_feeding]invalid codec!");
		return PLAYER_EMPTY_P;
	}	
    pkt->avpkt_newflag = 1;
	write_av_packet(para, pkt);	
    return PLAYER_SUCCESS;
}
/*************************************************************************/
static int divx3_data_prefeeding(am_packet_t *pkt, unsigned w, unsigned h)
{
	unsigned i = (w << 12) | (h & 0xfff);
    unsigned char divx311_add[10] = {
        0x00, 0x00, 0x00, 0x01,
        0x20, 0x00, 0x00, 0x00,
        0x00, 0x00
    };
    divx311_add[5] = (i >> 16) & 0xff;
    divx311_add[6] = (i >> 8) & 0xff;
    divx311_add[7] = i & 0xff;

	if(pkt->hdr->data)
    {
        MEMCPY(pkt->hdr->data, divx311_add, sizeof(divx311_add));
	    pkt->hdr->size = sizeof(divx311_add);
	}
	else
	{
		log_print("[divx3_data_prefeeding]No enough memory!\n");
		return PLAYER_FAILED;
	}
    return PLAYER_SUCCESS;
}

static int divx3_write_header(play_para_t *para, am_packet_t *pkt)
{
    divx3_data_prefeeding(pkt, para->vstream_info.video_width, para->vstream_info.video_height);
    if(para->vcodec)			
    	pkt->codec = para->vcodec;
    else
    {
    	log_print("[pre_header_feeding]invalid codec!");
    	return PLAYER_EMPTY_P;
    }	
    pkt->avpkt_newflag = 1;
    write_av_packet(para, pkt);	
    return PLAYER_SUCCESS;
}
/*************************************************************************/
static int h264_add_header(unsigned char *buf, int size,  am_packet_t *pkt)
{
    char nal_start_code[]={0x0,0x0,0x0,0x1};
    int nalsize;
    unsigned char* p;
    int tmpi;   
    unsigned char* extradata = buf;	
    int header_len=0;
    char* buffer = pkt->hdr->data;	
    
    p=extradata;
    if ( size < 4 )        	
    	return PLAYER_FAILED;  
    
    if(size < 10)
    {
        log_print("avcC too short\n");
        return PLAYER_FAILED;
    }
    
    if(*p != 1)
    {
        log_print(" Unkonwn avcC version %d\n", *p);
        return PLAYER_FAILED;
    }
    
    int cnt= *(p+5)&0x1f; //number of sps  
   // printf("number of sps :%d\n", cnt);
    p+=6;
    for(tmpi=0; tmpi<cnt; tmpi++)
	{
        nalsize = (*p<<8)|(*(p+1));	        
        MEMCPY(&(buffer[header_len]), nal_start_code, 4);
        header_len+=4;
        MEMCPY(&(buffer[header_len]), p+2, nalsize);
        header_len+=nalsize;
        p += (nalsize+2);
    }   
    
    cnt = *(p++); //Number of pps 
   // printf("number of pps :%d\n", cnt);
    for(tmpi=0; tmpi<cnt; tmpi++)
	{
        nalsize = (*p<<8)|(*(p+1));		       
        MEMCPY(&(buffer[header_len]), nal_start_code, 4);
        header_len+=4;		
        MEMCPY(&(buffer[header_len]), p+2, nalsize);
        header_len+=nalsize;
        p += (nalsize+2);
    }   
    if(header_len>=HDR_BUF_SIZE)
	{
        log_print("header_len %d is larger than max length\n", header_len);        
       return PLAYER_FAILED;
    }  	
	pkt->hdr->size = header_len;
    pkt->type = CODEC_VIDEO;    
	return PLAYER_SUCCESS;	
}

static int h264_write_header(play_para_t *para, am_packet_t *pkt)
{    
	AVStream *pStream = NULL;
	AVCodecContext *avcodec;
    int ret = -1;
    int index = para->vstream_info.video_index;
    if(-1 == index)
        return PLAYER_ERROR_PARAM;
	
       
    pStream = para->pFormatCtx->streams[index];
    avcodec = pStream->codec;
    ret = h264_add_header(avcodec->extradata, avcodec->extradata_size, pkt);
	if(ret == PLAYER_SUCCESS)
	{
		if(para->vcodec)			
			pkt->codec = para->vcodec;
		else
		{
			log_print("[pre_header_feeding]invalid video codec!\n");
			return PLAYER_EMPTY_P;
		}	
     
        pkt->avpkt_newflag = 1;
		ret = write_av_packet(para, pkt);
	}
    return ret;
}
/*************************************************************************/
static int m4s2_dx50_mp4v_add_header(unsigned char *buf, int size,  am_packet_t *pkt)
{
	if (size > pkt->hdr->size)
	{
		FREE(pkt->hdr->data);
		pkt->hdr->size = 0;
		
		pkt->hdr->data = (char *)MALLOC(size);
        if(!pkt->hdr->data) {
				log_print("[m4s2_dx50_add_header] NOMEM!");
                return PLAYER_FAILED;
        }
	}

    pkt->hdr->size = size;
    pkt->type = CODEC_VIDEO;
    MEMCPY(pkt->hdr->data, buf, size);

	return PLAYER_SUCCESS;	
}

static int m4s2_dx50_mp4v_write_header(play_para_t *para, am_packet_t * pkt)
{    
	AVStream *pStream = NULL;
	AVCodecContext *avcodec;
    int ret = -1;
    int index = para->vstream_info.video_index;
    if(-1 == index)
        return PLAYER_ERROR_PARAM;
    		
	pStream = para->pFormatCtx->streams[index];
	avcodec = pStream->codec;

    ret = m4s2_dx50_mp4v_add_header(avcodec->extradata, avcodec->extradata_size, pkt);
	if(ret == PLAYER_SUCCESS)
	{
		if(para->vcodec)			
			pkt->codec = para->vcodec;
		else
		{
			log_print("[pre_header_feeding]invalid video codec!\n");
			return PLAYER_EMPTY_P;
		}
        pkt->avpkt_newflag = 1;
		ret = write_av_packet(para, pkt);
	}				
	return ret;		
}
/*************************************************************************/
static int avi_add_seqheader(AVStream *pStream, am_packet_t *pkt)
{
    AVIStream *avi_stream = pStream->priv_data;
    int seq_size = avi_stream->sequence_head_size;

    if (seq_size > pkt->hdr->size)
    {
        FREE(pkt->hdr->data);
        pkt->hdr->size = 0;

        pkt->hdr->data = (char *)MALLOC(seq_size);
        if(!pkt->hdr->data) 
        {
            log_print("[m4s2_dx50_add_header] NOMEM!");
            return PLAYER_FAILED;
        }
    }

    pkt->hdr->size = seq_size;
    pkt->type = CODEC_VIDEO;
    MEMCPY(pkt->hdr->data, avi_stream->sequence_head, seq_size);

    return PLAYER_SUCCESS;	
}

static int avi_write_header(play_para_t *para, am_packet_t *pkt)
{    
	AVStream *pStream = NULL;	
    int ret = -1;
    int index = para->vstream_info.video_index;
    if(-1 == index)
        return PLAYER_ERROR_PARAM;
    		
	pStream = para->pFormatCtx->streams[index];	
    ret = avi_add_seqheader(pStream, pkt);
    if (ret == PLAYER_SUCCESS)
    {
        if (para->vcodec)
            pkt->codec = para->vcodec;
        else
        {
            log_print("[pre_header_feeding]invalid video codec!\n");
			return PLAYER_EMPTY_P;
        }
        pkt->avpkt_newflag = 1;
        ret = write_av_packet(para, pkt);
    }
    return ret;
}
/*************************************************************************/
static int wmv3_write_header(play_para_t *para, am_packet_t *pkt)
{
    unsigned i, check_sum = 0;
    unsigned data_len = para->vstream_info.extradata_size + 4;
    int ret;
            
	pkt->hdr->data[0] = 0;
	pkt->hdr->data[1] = 0;
	pkt->hdr->data[2] = 1;
	pkt->hdr->data[3] = 0x10;
	
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
	pkt->hdr->size = para->vstream_info.extradata_size+26;
	if(para->vcodec)			
		pkt->codec = para->vcodec;
	else
	{
		log_print("[pre_header_feeding]invalid codec!");
		return PLAYER_EMPTY_P;
	}	
    pkt->avpkt_newflag = 1;
    pkt->type = CODEC_VIDEO;
	ret = write_av_packet(para, pkt);
    return ret;
}
/*************************************************************************/
static int wvc1_write_header(play_para_t *para, am_packet_t *pkt)
{
    int ret = -1;
    MEMCPY(pkt->hdr->data, para->vstream_info.extradata+1, para->vstream_info.extradata_size-1);
	pkt->hdr->size = para->vstream_info.extradata_size-1;
	if(para->vcodec)			
		pkt->codec = para->vcodec;
	else
	{
		log_print("[pre_header_feeding]invalid codec!");
		return PLAYER_EMPTY_P;
	}	
    pkt->avpkt_newflag = 1;
    pkt->type = CODEC_VIDEO;
	ret = write_av_packet(para, pkt);
    return ret;
}
/*************************************************************************/

int mpeg_check_sequence(play_para_t *para)
{   
#define MPEG_PROBE_SIZE     (4096)
#define MAX_MPEG_PROBE_SIZE (0x100000)      //1M
#define SEQ_START_CODE      (0x000001b3)
#define EXT_START_CODE      (0x000001b5)
    int code;
    int offset;
    int pos1 = 0,pos2 = 0;
    int i, j;
    int len;
    int read_size;
    int seq_size = 0;
    AVFormatContext *s = para->pFormatCtx;    
    unsigned char buf[MPEG_PROBE_SIZE];    
    offset = 0;
    len = 0;
    for(j = 0; j < (MAX_MPEG_PROBE_SIZE/MPEG_PROBE_SIZE); j++)
    {  
        url_fseek(s->pb, offset, SEEK_SET);
        read_size = get_buffer(s->pb, buf, MPEG_PROBE_SIZE); 
        if(read_size < 0)
        {
            log_error("[mpeg_check_sequence:%d] read error: %d\n",__LINE__,read_size);
            return read_size;
        }
        offset += read_size;    
        for(i=0; i<read_size; i++)
        {
            code = (code<<8) + buf[i];
            if ((code & 0xffffff00) == 0x100) 
            {     
                //log_print("[mpeg_check_sequence:%d]code=%08x\n",__LINE__, code);
                if(code == SEQ_START_CODE)
                {
                    pos1 = j*MPEG_PROBE_SIZE + i -3;
                }
                else if(code != EXT_START_CODE)
                {
                    pos2 = j*MPEG_PROBE_SIZE + i - 3;
                }
                if((pos2 > pos1) && (pos1 > 0))
                {
                    seq_size = pos2 - pos1;
                    //log_print("[mpeg_check_sequence:%d]pos1=%x pos2=%x seq_size=%d\n",__LINE__, pos1,pos2,seq_size);
                    break;
                }
            }              
        }
        if(seq_size > 0)
            break;
    }
    if(seq_size > 0)
    {
        url_fseek(s->pb, pos1, SEEK_SET);
        read_size = get_buffer(s->pb, buf, seq_size); 
        if(read_size < 0)
        {
            log_error("[mpeg_check_sequence:%d] read error: %d\n",__LINE__,read_size);
            return read_size;
        }
        #ifdef DEBUG_MPEG_SEARCH
        for(i = 0; i<seq_size;i++)
        {
            log_print("%02x ",buf[i]);                
            if(i%8==7)
                log_print("\n");
        }
        #endif
       
        para->vstream_info.extradata = MALLOC(seq_size);
        if(para->vstream_info.extradata)
        {           
            MEMCPY(para->vstream_info.extradata,buf,seq_size);                
            para->vstream_info.extradata_size = seq_size;
            #ifdef DEBUG_MPEG_SEARCH
            for(i = 0; i<seq_size;i++)
            {
                log_print("%02x ",para->vstream_info.extradata[i]);                
                if(i%8==7)
                    log_print("\n");
            }
            #endif
        }
        else
            log_error("[mpeg_check_sequece:%d] no enough memory !\n",__LINE__);
    }
    else
        log_error("[mpeg_check_sequence:%d]max probe size reached! not find sequence header!\n",__LINE__);
    return 0;
}
static int mpeg_add_header(play_para_t *para, am_packet_t *pkt)
{  
#define STUFF_BYTES_LENGTH     (256)
    int size;
    unsigned char packet_wrapper[] = {
        0x00, 0x00, 0x01, 0xe0,
        0x00, 0x00,                                /* pes packet length */
        0x81, 0xc0, 0x0d,
        0x20,0x00,0x00,0x00,0x00, /* PTS */
        0x1f,0xff,0xff,0xff,0xff, /* DTS */
        0xff,0xff,0xff,0xff,0xff,0xff
    };    
    size = para->vstream_info.extradata_size + sizeof(packet_wrapper);
    packet_wrapper[4] = size >> 8 ;
    packet_wrapper[5] = size & 0xff ;   
    MEMCPY(pkt->hdr->data, packet_wrapper, sizeof(packet_wrapper));
	size = sizeof(packet_wrapper);
    //log_print("[mpeg_add_header:%d]wrapper size=%d\n",__LINE__,size);
    MEMCPY(pkt->hdr->data+size, para->vstream_info.extradata, para->vstream_info.extradata_size);
    size += para->vstream_info.extradata_size;
    //log_print("[mpeg_add_header:%d]wrapper+seq size=%d\n",__LINE__,size);
    MEMSET(pkt->hdr->data+size, 0xff, STUFF_BYTES_LENGTH);
    size += STUFF_BYTES_LENGTH;
    pkt->hdr->size = size;
    //log_print("[mpeg_add_header:%d]hdr_size=%d\n",__LINE__,size);
	if(para->codec)			
		pkt->codec = para->codec;
	else
	{
		log_print("[pre_header_feeding]invalid codec!");
		return PLAYER_EMPTY_P;
	}
    #ifdef DEBUG_MPEG_SEARCH
    int i;
    for(i=0; i<pkt->hdr->size; i++)
    {
        if(i%16 == 0)
            log_print("\n");
        log_print("%02x ",pkt->hdr->data[i]);
    }
    #endif
    pkt->avpkt_newflag = 1;   
	return write_av_packet(para, pkt);    
}


int pre_header_feeding(play_para_t *para, am_packet_t *pkt)
{	
    int ret = -1;
    AVStream *pStream = NULL;
    AVCodecContext *avcodec;      
    int index = para->vstream_info.video_index;
    if(-1 == index)
        return PLAYER_ERROR_PARAM;   
    
    pStream = para->pFormatCtx->streams[index];
    avcodec = pStream->codec;    
    
	if(para->stream_type == STREAM_ES && para->vstream_info.has_video)
	{ 
		if(pkt->hdr == NULL)
		{
			pkt->hdr = MALLOC(sizeof(hdr_buf_t));
			pkt->hdr->data = (char *)MALLOC( HDR_BUF_SIZE);
            if(!pkt->hdr->data) {
				log_print("[pre_header_feeding] NOMEM!");
                return PLAYER_NOMEM;
            }
		}
		
		if(VFORMAT_MJPEG == para->vstream_info.video_format)
		{
			ret = mjpeg_write_header(para,pkt);				
            if(ret != PLAYER_SUCCESS)
                return ret;
		}
		else if ((VFORMAT_MPEG4 == para->vstream_info.video_format) &&
				 (VIDEO_DEC_FORMAT_MPEG4_3 == para->vstream_info.video_codec_type))
		{
			ret = divx3_write_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;
		}
		else if(VFORMAT_H264 == para->vstream_info.video_format)
		{
			ret = h264_write_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;
		}		
		else if ((CODEC_TAG_M4S2 == avcodec->codec_tag) ||
				 (CODEC_TAG_DX50 == avcodec->codec_tag) ||
				 (CODEC_TAG_mp4v == avcodec->codec_tag))
		{
            ret = m4s2_dx50_mp4v_write_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;				
		}
        else if ((AVI_FILE == para->file_type)
                && (VIDEO_DEC_FORMAT_MPEG4_3 != para->vstream_info.video_codec_type)
                && (VFORMAT_H264 != para->vstream_info.video_format)
                && (VFORMAT_VC1 != para->vstream_info.video_format))
        {
            ret = avi_write_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;
        }
        else if ( CODEC_TAG_WMV3 == avcodec->codec_tag )
        {
        	ret = wmv3_write_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;
        }
        else if ( (CODEC_TAG_WVC1 == avcodec->codec_tag) || (CODEC_TAG_WMVA == avcodec->codec_tag) )
        {
        	ret = wvc1_write_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;
        }         
		if(pkt->hdr)
		{
            if(pkt->hdr->data)
			{
                FREE(pkt->hdr->data);
			    pkt->hdr->data = NULL;
            }
			FREE(pkt->hdr);				
			pkt->hdr = NULL;				
		}
       
	}	
    else if(para->stream_type == STREAM_PS && para->vstream_info.has_video && para->playctrl_info.time_point > 0)
    {    
        if(pkt->hdr == NULL)
		{
			pkt->hdr = MALLOC(sizeof(hdr_buf_t));
			pkt->hdr->data = (char *)MALLOC( HDR_BUF_SIZE);
            if(!pkt->hdr->data) {
				log_print("[pre_header_feeding] NOMEM!");
                return PLAYER_NOMEM;
            }
		}
        if (CODEC_ID_MPEG1VIDEO == avcodec->codec_id )
        {           
        	ret = mpeg_add_header(para,pkt);
            if(ret != PLAYER_SUCCESS)
                return ret;
        }
        if(pkt->hdr)
		{
            if(pkt->hdr->data)
			{
                FREE(pkt->hdr->data);
			    pkt->hdr->data = NULL;
            }
			FREE(pkt->hdr);				
			pkt->hdr = NULL;				
		}
    }
	return PLAYER_SUCCESS;
}

int h264_update_frame_header(am_packet_t *pkt)
{
	int nalsize, size = pkt->data_size;
	unsigned char *data = pkt->data;
	unsigned char *p = data;	    
    if(p != NULL)
    {
    	if (check_size_in_buffer(p, size))
    	{		
    		while((p+4)<(data +size))
    		{
    			nalsize = (*p<<24)|(*(p+1)<<16)|(*(p+2)<<8)|(*(p+3));
    			*p=0;*(p+1)=0;*(p+2)=0;*(p+3)=1;
    			p+=(nalsize+4);
    		}
    		return PLAYER_SUCCESS;
    	}
    	else if ( check_size_in_buffer3(p, size) )
    	{
    		while((p+3)<(data +size))
    		{
    			nalsize = (*p<<16)|(*(p+1)<<8)|(*(p+2));
    			*p=0;*(p+1)=0;*(p+2)=1;
    			p+=(nalsize+3);
    		}
    		return PLAYER_SUCCESS;
    	}
    	else if ( check_size_in_buffer2(p, size) )
    	{
			unsigned char *new_data;
 			int new_len = 0;

			new_data = (unsigned char *)MALLOC(size+2*1024);
  			if ( !new_data )
  				return PLAYER_NOMEM;
                    
    		while((p+2)<(data +size))
    		{
    			nalsize = (*p<<8)|(*(p+1));
    			*(new_data+new_len)=0;*(new_data+new_len+1)=0;*(new_data+new_len+2)=0;*(new_data+new_len+3)=1;
    			memcpy(new_data+new_len+4, p+2, nalsize);
    			p+=(nalsize+2);
    			new_len += nalsize+4;
    		}

			FREE(pkt->buf);

    		pkt->buf = new_data;
    		pkt->buf_size = size+2*1024;
    		pkt->data = pkt->buf;
    		pkt->data_size = new_len;
    	}
    }
    else
    {   log_error("[%s]invalid pointer!\n",__FUNCTION__);
	    return PLAYER_FAILED;
    }
    return PLAYER_SUCCESS;
}

int divx3_prefix(am_packet_t *pkt)
{
	#define DIVX311_CHUNK_HEAD_SIZE 13
	const unsigned char divx311_chunk_prefix[DIVX311_CHUNK_HEAD_SIZE] =
	{
    	0x00, 0x00, 0x00, 0x01, 0xb6, 'D', 'I', 'V', 'X', '3', '.', '1', '1'
	};
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
			log_print("[divx3_prefix] NOMEM!");
            return PLAYER_FAILED;
        }
        
        pkt->hdr->data = NULL;
        pkt->hdr->size = 0;
	}

	pkt->hdr->data = MALLOC(DIVX311_CHUNK_HEAD_SIZE + 4);
	if (pkt->hdr->data == NULL)
	{
		log_print("[divx3_prefix] NOMEM!");
        return PLAYER_FAILED;
    }

    MEMCPY(pkt->hdr->data, divx311_chunk_prefix, DIVX311_CHUNK_HEAD_SIZE);
    
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 0] = (pkt->data_size >> 24) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 1] = (pkt->data_size >> 16) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 2] = (pkt->data_size >>  8) & 0xff;
    pkt->hdr->data[DIVX311_CHUNK_HEAD_SIZE + 3] = pkt->data_size & 0xff;

	pkt->hdr->size = DIVX311_CHUNK_HEAD_SIZE + 4;
    pkt->avpkt_newflag = 1;

	return PLAYER_SUCCESS;	
}


