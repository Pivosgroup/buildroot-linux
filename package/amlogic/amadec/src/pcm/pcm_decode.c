#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "adec.h"
#include "log.h"
#include "bytestream.h"

#define FORMAT_PATH \
"/sys/class/astream/format"
#define CHANNUM_PATH \
"/sys/class/astream/channum"
#define SAMPLE_RATE_PATH \
"/sys/class/astream/samplerate"


#define	pcm_buffer_size		(1024*1)

#if 0
typedef struct {
	unsigned char *data_buffer;
	int	data_size;
	int	double_chan;
	int	sample_conv;
}DestBuf;
static DestBuf dbuf = {0};
#endif

static short table[256];
static unsigned char pcm_buffer[pcm_buffer_size];
static adec_feeder_t *pcm_feeder = NULL;
//static int double_chan = 0;

#define	LOCAL	inline

#define	SIGN_BIT		(0x80)
#define	QUANT_MASK	(0xf)
#define	NSEGS			(8)
#define	SEG_SHIFT		(4)
#define	SEG_MASK		(0x70)

#define	BIAS			(0x84)

static int pcm_channels = 0;
static int pcm_samplerate = 0;
static int pcm_datewidth = 16;
static codec_type_t codec_id = CODEC_ID_NONE;
  
static LOCAL int read_byte(void)
{
	unsigned long tmp;
	pcm_feeder->get_bits(&tmp, 8);
	return tmp;
}

static LOCAL int read_2byte(void)
{
	unsigned long tmp;
	pcm_feeder->get_bits(&tmp, 16);
	return tmp;
}
static LOCAL int read_bytes(char *buf,int size)
{
	return pcm_feeder->get_bytes(buf, size);
}

static LOCAL int reset_bits(void)
{
	unsigned long tmp, bit;
	bit = pcm_feeder->bits_left() & (7);
	if (bit > 0)
		pcm_feeder->get_bits(&tmp, bit);
}



static LOCAL int pcm_read(unsigned char *buf, int size)
{
	reset_bits();
	return read_bytes(buf,size);
}

static int av_get_bits_per_sample(codec_type_t codec_id){
    	switch(codec_id){
    	case CODEC_ID_PCM_ALAW:
    	case CODEC_ID_PCM_MULAW:
    	//case CODEC_ID_PCM_S8:
    	case CODEC_ID_PCM_U8:
    	//case CODEC_ID_PCM_ZORK:
        return 8;
    	case CODEC_ID_PCM_S16BE:
    	case CODEC_ID_PCM_S16LE:
    	//case CODEC_ID_PCM_S16LE_PLANAR:
    	//case CODEC_ID_PCM_U16BE:
    	//case CODEC_ID_PCM_U16LE:
        	return 16;
    	//case CODEC_ID_PCM_S24DAUD:
    	//case CODEC_ID_PCM_S24BE:
    	//case CODEC_ID_PCM_S24LE:
    	//case CODEC_ID_PCM_U24BE:
    	//case CODEC_ID_PCM_U24LE:
       // 	return 24;
    	//case CODEC_ID_PCM_S32BE:
    	//case CODEC_ID_PCM_S32LE:
    	//case CODEC_ID_PCM_U32BE:
    	//case CODEC_ID_PCM_U32LE:
    	//case CODEC_ID_PCM_F32BE:
    	//case CODEC_ID_PCM_F32LE:
        //	return 32;
    	//case CODEC_ID_PCM_F64BE:
    	//case CODEC_ID_PCM_F64LE:
        //	return 64;
    	default:
        	return 0;
    	}
}

static codec_type_t get_audio_format()
{
    int fd;
    char format[21];
    int len;

    format[0] = 0;

    fd = open(FORMAT_PATH, O_RDONLY);
    if (fd < 0) {
        printf("amadec device not found");
        return CODEC_ID_NONE;
    }

    len = read(fd, format, 20);
    if (len > 0) {
        format[len] = 0;
    }

     if (strncmp(format, "amadec_pcm_s16be", 16) == 0) {
	  close(fd);
	  return CODEC_ID_PCM_S16BE;
    }

     if (strncmp(format, "amadec_pcm_u8", 13) == 0) {
        close(fd);
        return CODEC_ID_PCM_U8;
    }
	 
    if (strncmp(format, "amadec_pcm", 10) == 0) {
        close(fd);
        return CODEC_ID_PCM_S16LE;
    }
	 
     if (strncmp(format, "amadec_alaw", 11) == 0) {
        close(fd);
        return CODEC_ID_PCM_ALAW;
    }

     if (strncmp(format, "amadec_mulaw", 12) == 0) {
        close(fd);
        return CODEC_ID_PCM_MULAW;
    }
	  
    close(fd);

     return CODEC_ID_NONE;
}

static unsigned int get_audio_channel()
{
    int fd;
    char channels[5];
    int len;

    channels[0] = 0;

    fd = open(CHANNUM_PATH, O_RDONLY);
    if (fd < 0) {
        printf("amadec device not found");
        return AUDIO_FORMAT_UNKNOWN;
    }

    len = read(fd, channels, 5);
    if (len > 0) {
        channels[len] = 0;
    }
 
    printf("amadec channels: %s", channels);
    
    close(fd);

    return atoi(channels);
}

static unsigned int get_audio_samplerate()
{
    int fd;
    char samplerate[10];
    int len;

    samplerate[0] = 0;

    fd = open(SAMPLE_RATE_PATH, O_RDONLY);
    if (fd < 0) {
        printf("amadec device not found");
        return AUDIO_FORMAT_UNKNOWN;
    }

    len = read(fd, samplerate, 10);
    if (len > 0) {
        samplerate[len] = 0;
    }

    printf("amadec samplerate: %s", samplerate);

    close(fd);

    return atoi(samplerate);
}

static int alaw2linear(unsigned char a_val)
{
	int t;
       int seg;

       a_val ^= 0x55;

       t = a_val & QUANT_MASK;
       seg = ((unsigned)a_val & SEG_MASK) >> SEG_SHIFT;
       if(seg) 
	   	t= (t + t + 1 + 32) << (seg + 2);
       else    
	   	t= (t + t + 1) << 3;

       return (a_val & SIGN_BIT) ? t : -t;
}

static int ulaw2linear(unsigned char u_val)
{
	int t;

       u_val = ~u_val;

       t = ((u_val & QUANT_MASK) << 3) + BIAS;
       t <<= ((unsigned)u_val & SEG_MASK) >> SEG_SHIFT;

       return (u_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS);
}

#if 0
static void pcm_samplerate_check( int samplerate, DestBuf * buf)
{
/*
	switch (samplerate) {
		case 8000:
			buf->sample_conv = 4;
			pcm_samplerate = 32000;
			break;

		case 11024:
			buf->sample_conv = 4;
			pcm_samplerate = 44100;
			break;
			
		case 16000:
			buf->sample_conv = 2;
			pcm_samplerate = 32000;
			break;
			
		case 22050:
			buf->sample_conv = 2;
			pcm_samplerate = 44100;
			break;

		default:
			buf->sample_conv = 0;
			pcm_samplerate = samplerate;
			break;
	}
*/
	if(samplerate >= (24000 + 32000)/2){
		buf->sample_conv = 0;
		pcm_samplerate = samplerate;
	}else if(samplerate >= (22050 + 24000)/2){
		buf->sample_conv = 2;
		pcm_samplerate = 48000;
	}else if(samplerate >= (16000 + 22050)/2){
		buf->sample_conv = 2;
		pcm_samplerate = 44100;
	}else if(samplerate >= (11025 + 16000)/2){
		buf->sample_conv = 2;
		pcm_samplerate = 32000;
	}else if(samplerate >= (8000 + 11025)/2){
		buf->sample_conv = 4;
		pcm_samplerate = 44100;
	}else{
		buf->sample_conv = 4;
		pcm_samplerate = 32000;
	}

	return;
}

static int pcm_data_output(unsigned char *dst, DestBuf * buf)
{
	short *dst_pcm;
	short *src_pcm;
	int size = 0, i;

	dst_pcm = dst;
	src_pcm = buf->data_buffer;
	
	if(buf->double_chan == 0){
		switch(buf->sample_conv){
			case 0:
				memcpy(dst, buf->data_buffer, buf->data_size);
				size = buf->data_size;
				break;

			case 2:
				for(i = 0; i < buf->data_size; i += 2)
				{
					*dst_pcm = *src_pcm;
					*(dst_pcm + 2) = *src_pcm++;
					*(dst_pcm + 1) = *src_pcm;
					*(dst_pcm + 3) = *src_pcm++;
					dst_pcm += 4;
				}
				size = (buf->data_size)*2;
				break;

			case 4:
				for(i = 0; i < buf->data_size; i += 4)
				{
					*dst_pcm = *src_pcm;
					*(dst_pcm + 2) = *src_pcm;
					*(dst_pcm + 4) = *src_pcm;
					*(dst_pcm + 6) = *src_pcm++;
					*(dst_pcm + 1) = *src_pcm;
					*(dst_pcm + 3) = *src_pcm;
					*(dst_pcm + 5) = *src_pcm;
					*(dst_pcm + 7) = *src_pcm++;
					dst_pcm += 8;
				}
				size = (buf->data_size)*4;
				break;

			default:
				printf("pcm_data_output: This should never occur ! No Double Chanl !\n");
				break;
		}
	} else{
		switch(buf->sample_conv){
			case 0:
				for(i = 0; i < buf->data_size; i++)
				{
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm++;
				}
				size = (buf->data_size)*2;
				break;

			case 2:
				for(i = 0; i < buf->data_size; i++)
				{
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm++;
				}
				size = (buf->data_size)*4;
				break;

			case 4:
				for(i = 0; i < buf->data_size; i++)
				{
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm;
					*dst_pcm++ = *src_pcm++;
				}
				size = (buf->data_size)*8;
				break;

			default:
				printf("pcm_data_output: This should never occur ! Double Chanl !\n");
				break;
				
		}
	}

	return size;
}
#endif

/**
 * Read PCM samples macro
 * @param type Datatype of native machine format
 * @param endian bytestream_get_xxx() endian suffix
 * @param src Source pointer (variable name)
 * @param dst Destination pointer (variable name)
 * @param n Total number of samples (variable name)
 * @param shift Bitshift (bits)
 * @param offset Sample value offset
 */
#define DECODE(type, endian, src, dst, n, shift, offset) \
    dst_##type = (type*)dst; \
    for(;n>0;n--) { \
        register type v = bytestream_get_##endian(&src); \
        *dst_##type++ = (v - offset) << shift; \
    } \
    dst = (short*)dst_##type;
	
static int pcm_init(adec_feeder_t *feeder)
{
	int i;
//	int sample_rate;

	pcm_feeder = feeder;
	if(feeder == NULL)
		return -1;
#if 0
	dbuf.data_buffer = (unsigned char*)malloc(8*1024);
	if(dbuf.data_buffer == NULL){
		printf(" pcm_init failed: Not enough memory!\n");
		return -1;
	}
	dbuf.data_size = 0;
#endif

	codec_id = get_audio_format();
	pcm_samplerate = get_audio_samplerate();
//	sample_rate= get_audio_samplerate();
//	pcm_samplerate_check(sample_rate, &dbuf);
	pcm_channels = get_audio_channel();
#if 0
	if(pcm_channels == 1){
		dbuf.double_chan= 1;
		pcm_channels = 2;
	}
#endif

	switch(codec_id){
		case CODEC_ID_PCM_ALAW:
			for(i = 0; i < 256; i++){
				table[i] = alaw2linear(i);
			}
			break;
		case CODEC_ID_PCM_MULAW:
			for(i = 0; i < 256; i++){
				table[i] = ulaw2linear(i);
			}
			break;
		default:
			break;
	}

	feeder->channel_num = pcm_channels;
	feeder->sample_rate = pcm_samplerate;
	feeder->data_width = pcm_datewidth;	
	
	return 0;
}

static int pcm_decode_frame(unsigned char *buf, int len, struct frame_fmt *fmt)
{
	short *sample;
	unsigned char *src;
	int size, n, i;
	int sample_size;

	int16_t *dst_int16_t;
    	int32_t *dst_int32_t;
    	int64_t *dst_int64_t;
    	uint16_t *dst_uint16_t;
    	uint32_t *dst_uint32_t;

//	sample = dbuf.data_buffer;
	sample = buf;
	src = pcm_buffer;

	size = pcm_read(pcm_buffer, pcm_buffer_size);
	sample_size = av_get_bits_per_sample(codec_id)/8;
	n = size /sample_size;

	switch(codec_id){
		case CODEC_ID_PCM_ALAW:
		case CODEC_ID_PCM_MULAW:
				for(; n > 0; n--)
					*sample++ = table[*src++];
				//dbuf.data_size = size *2;
				size *=2;
			break;
				
		case CODEC_ID_PCM_S16LE:
				memcpy(sample, src, n*sample_size);
				//dbuf.data_size = size;

			break;

		case CODEC_ID_PCM_S16BE:
				DECODE(int16_t, be16, src, sample, n, 0, 0)
				//dbuf.data_size = size;
			break;

		case CODEC_ID_PCM_U8:
				for(;n>0;n--) {
          	 			*sample++ = ((int)*src++ - 128) << 8;
        			}
				//dbuf.data_size = size*2;
				size *=2;
			break;
			
		default:
			break;
	}

	//size = pcm_data_output(buf, &dbuf);
	
	return size;
}

static int pcm_decode_release(void)
{
#if 0
	if(dbuf.data_buffer)
		free(dbuf.data_buffer);
	memset(&dbuf, 0, sizeof(dbuf));
#endif	
	return 0;
}

#define PCM_DECODE(name, format, long_name)	\
am_codec_struct	name ## _decode={	\
	long_name,				\
	format,					\
	0,						\
	pcm_init,				\
	pcm_decode_release,	\
	pcm_decode_frame,		\
};

PCM_DECODE(pcm_s16le, AUDIO_FORMAT_PCM_S16LE, "PCM signed 16-bit little-endian");
PCM_DECODE(pcm_alaw, AUDIO_FORMAT_ALAW, "PCM A-law");
PCM_DECODE(pcm_mulaw, AUDIO_FORMAT_MULAW, "PCM mu-law");
PCM_DECODE(pcm_s16be, AUDIO_FORMAT_PCM_S16BE, "PCM signed 16-bit big-endian");
PCM_DECODE(pcm_u8, AUDIO_FORMAT_PCM_U8, "PCM unsigned 8-bit");

int register_pcm_codec(void)
{
	register_audio_codec(&pcm_s16le_decode);
	register_audio_codec(&pcm_alaw_decode);
	register_audio_codec(&pcm_mulaw_decode);
	register_audio_codec(&pcm_s16be_decode);
	register_audio_codec(&pcm_u8_decode);
	return 0;
}

