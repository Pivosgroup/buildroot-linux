#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include "adec.h"
#include "log.h"
#include "bytestream.h"

#define LOCAL inline
#define adpcm_buffer_size	(1024*2)

/* step_table[] and index_table[] are from the ADPCM reference source */
/* This is the index table: */
static const int index_table[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8,
};

/**
 * This is the step table. Note that many programs use slight deviations from
 * this table, but such deviations are negligible:
 */
static const int step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

/* These are for MS-ADPCM */
/* AdaptationTable[], AdaptCoeff1[], and AdaptCoeff2[] are from libsndfile */
static const int AdaptationTable[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
};

/** Divided by 4 to fit in 8-bit integers */
static const uint8_t AdaptCoeff1[] = {
        64, 128, 0, 48, 60, 115, 98
};

/** Divided by 4 to fit in 8-bit integers */
static const int8_t AdaptCoeff2[] = {
        0, -64, 0, 16, 0, -52, -58
};

/* These are for CD-ROM XA ADPCM */
static const int xa_adpcm_table[5][2] = {
   {   0,   0 },
   {  60,   0 },
   { 115, -52 },
   {  98, -55 },
   { 122, -60 }
};

static const int ea_adpcm_table[] = {
    0, 240, 460, 392, 0, 0, -208, -220, 0, 1,
    3, 4, 7, 8, 10, 11, 0, -1, -3, -4
};

// padded to zero where table size is less then 16
static const int swf_index_tables[4][16] = {
    /*2*/ { -1, 2 },
    /*3*/ { -1, -1, 2, 4 },
    /*4*/ { -1, -1, -1, -1, 2, 4, 6, 8 },
    /*5*/ { -1, -1, -1, -1, -1, -1, -1, -1, 1, 2, 4, 6, 8, 10, 13, 16 }
};

static const int yamaha_indexscale[] = {
    230, 230, 230, 230, 307, 409, 512, 614,
    230, 230, 230, 230, 307, 409, 512, 614
};

static const int yamaha_difflookup[] = {
    1, 3, 5, 7, 9, 11, 13, 15,
    -1, -3, -5, -7, -9, -11, -13, -15
};

typedef struct OutData {
    //uint8_t outb[4*adpcm_buffer_size+32];
    uint8_t* outb;
    int byte;
    int index;
} OutData;

typedef struct _ADPCMInfo {
	int codec_id;
	int sample_rate;
	int channels;
	int block_align;
	int extradata_size;
	char *extradata;
} ADPCMInfo;

typedef struct _ADPCMChannelStatus {
    	int predictor;
    	short int step_index;
    	int step;
    	/* for encoding */
    	int prev_sample;

    	/* MS version */
    	short sample1;
    	short sample2;
    	int coeff1;
    	int coeff2;
    	int idelta;
} ADPCMChannelStatus;

typedef struct _ADPCMContext {
    	ADPCMChannelStatus status[6];
} ADPCMContext;

static ADPCMContext adpcmtext;
static ADPCMInfo adpcm_info;
static adec_feeder_t *adpcm_feeder = NULL;
//static unsigned char adpcm_buffer[adpcm_buffer_size];
static unsigned char* adpcm_buffer;
static OutData outbuffer;

static LOCAL int read_byte(void)
{
	unsigned long tmp;
	adpcm_feeder->get_bits(&tmp, 8);
	return tmp;
}

static LOCAL int read_2byte(void)
{
	unsigned long tmp;
	adpcm_feeder->get_bits(&tmp, 16);
	return tmp;
}
static LOCAL int read_bytes(char *buf,int size)
{
	return adpcm_feeder->get_bytes(buf, size);
}

static LOCAL int reset_bits(void)
{
	unsigned long tmp, bit;
	bit = adpcm_feeder->bits_left() & (7);
	if (bit > 0)
		adpcm_feeder->get_bits(&tmp, bit);
}



static LOCAL int adpcm_read(unsigned char *buf, int size)
{
	//reset_bits();
	return read_bytes(buf,size);
}

static void mem_share_get(ADPCMInfo * info)
{
	key_t shmkey;
	int shmid;
	char *ptr;
	int *tmp;

	shmkey = ftok( "/dev/null" , 'a' );
	if(shmkey == -1)
	{
		printf("Creat shmkey failed!\n");
		return;
	}
	
	shmid = shmget(shmkey, 2048, IPC_CREAT|0666);
	if(shmid == -1)
	{
		printf("Get share memory failed!\n");
		return;
	}
	
	ptr = shmat(shmid, (void *)0, 0);
	if(ptr == (void *)-1)
	{
		printf("shmat failed!\n");
		return;
	}

	tmp = (int *)ptr;
	info->codec_id = *tmp++;
	info->sample_rate = *tmp++;
	info->channels = *tmp++;
	info->block_align = *tmp++;
	info->extradata_size = *tmp++;

	if(info->extradata_size > 0 && info->extradata_size <2048)
	{
		info->extradata = (char *)malloc(info->extradata_size);
		if(info->extradata == NULL)
		{
			printf("malloc memory failed!\n");
			return;
		}
		memcpy(info->extradata, (char *)tmp, info->extradata_size);
	}

	if(shmdt(ptr) == -1)
	{
		printf("shmdt failed!\n");
		return;
	}
	
	if(shmctl(shmid, IPC_RMID, 0) == -1)
	{
		printf("shmctl failed!\n");
		return;
	}
}

static inline short adpcm_ima_expand_nibble(ADPCMChannelStatus *c, char nibble, int shift)
{
    	int step_index;
    	int predictor;
    	int sign, delta, diff, step;

    	step = step_table[c->step_index];
    	step_index = c->step_index + index_table[(unsigned)nibble];
    	if (step_index < 0)
		step_index = 0;
    	else if (step_index > 88) 
		step_index = 88;

    	sign = nibble & 8;
    	delta = nibble & 7;
    	/* perform direct multiplication instead of series of jumps proposed by
     	* the reference ADPCM implementation since modern CPUs can do the mults
     	* quickly enough */
    	diff = ((2 * delta + 1) * step) >> shift;
    	predictor = c->predictor;
    	if (sign) 
		predictor -= diff;
    	else 
		predictor += diff;

    	c->predictor = av_clip_int16(predictor);
    	c->step_index = step_index;

    	return (short)c->predictor;
}


static inline short adpcm_ms_expand_nibble(ADPCMChannelStatus *c, char nibble)
{
    	int predictor;

    	predictor = (((c->sample1) * (c->coeff1)) + ((c->sample2) * (c->coeff2))) / 64;
    	predictor += (signed)((nibble & 0x08)?(nibble - 0x10):(nibble)) * c->idelta;

    	c->sample2 = c->sample1;
	c->sample1 = av_clip_int16(predictor);
    	c->idelta = (AdaptationTable[(int)nibble] * c->idelta) >> 8;
    	if (c->idelta < 16) 
		c->idelta = 16;

    	return c->sample1;
}

static int adpcm_init(adec_feeder_t *feeder)
{
	unsigned int max_channels = 2;

	adpcm_feeder = feeder;
	if(feeder == NULL)
		return -1;

	memset(&outbuffer, 0, sizeof(outbuffer));
	mem_share_get(&adpcm_info);

	adpcm_buffer = (unsigned char*)malloc(adpcm_info.block_align);
	if(adpcm_buffer == NULL)
	{
		printf("Error: malloc adpcm buffer failed!\n");
		return -1;
	}

	outbuffer.outb = (unsigned char*)malloc(4 * adpcm_info.block_align + 32);
	if(outbuffer.outb == NULL)
	{
		free(adpcm_buffer);
		printf("Error: malloc adpcm out buffer failed!\n");
		return -1;
	}
	
	switch(adpcm_info.codec_id) 
	{
		case CODEC_ID_ADPCM_EA_R1:
    		case CODEC_ID_ADPCM_EA_R2:
    		case CODEC_ID_ADPCM_EA_R3:
			max_channels = 6;
			break;
	}

	if(adpcm_info.channels > max_channels)
	{
       	return -1;
    	}

	switch(adpcm_info.codec_id) 
	{
		case CODEC_ID_ADPCM_CT:
       		adpcmtext.status[0].step = adpcmtext.status[1].step = 511;
			break;
		case CODEC_ID_ADPCM_IMA_WS:
			if (adpcm_info.extradata && adpcm_info.extradata_size == 2 * 4) 
			{
            			adpcmtext.status[0].predictor = AV_RL32(adpcm_info.extradata);
            			adpcmtext.status[1].predictor = AV_RL32(adpcm_info.extradata + 4);
        		}
			break;
		default:
			break;
	}

	feeder->channel_num = adpcm_info.channels;
	feeder->sample_rate = adpcm_info.sample_rate;
	feeder->data_width = 16;

	return 0;
}

static int adpcm_decode_frame(unsigned char *buf, int len, struct frame_fmt *fmt)
{
	int buf_size;
	ADPCMContext *c = &adpcmtext;
    	ADPCMChannelStatus *cs;
	int n, m, channel, i;
    	int block_predictor[2];
    	short *samples;
    	short *samples_end;
    	const uint8_t *src;
    	int st; /* stereo */

	unsigned char last_byte = 0;
    	unsigned char nibble;
    	int decode_top_nibble_next = 0;
    	int diff_channel;

	/* EA ADPCM state variables */
    	uint32_t samples_in_chunk;
    	int32_t previous_left_sample, previous_right_sample;
    	int32_t current_left_sample, current_right_sample;
    	int32_t next_left_sample, next_right_sample;
    	int32_t coeff1l, coeff2l, coeff1r, coeff2r;
    	uint8_t shift_left, shift_right;
    	int count1, count2;
    	int coeff[2][2], shift[2];//used in EA MAXIS ADPCM
	int output_size;

	if(outbuffer.byte)
		goto end;

	buf_size = adpcm_read(adpcm_buffer, adpcm_info.block_align);
	if(!buf_size)
		return 0;

	//if(sizeof(outbuffer.outb)/4 < buf_size + 8)
	//	return -1;

	samples = (short *)outbuffer.outb;
	samples_end = samples + (4*adpcm_info.block_align + 32)/2;
	src = adpcm_buffer;
	st = adpcm_info.channels == 2 ? 1 : 0;

	switch(adpcm_info.codec_id)
	{
		case CODEC_ID_ADPCM_IMA_WAV:
			if (adpcm_info.block_align != 0 && buf_size > adpcm_info.block_align)
            			buf_size = adpcm_info.block_align;
			for(i=0; i<adpcm_info.channels; i++)
			{
            			cs = &(c->status[i]);
            			cs->predictor = *samples++ = (int16_t)bytestream_get_le16(&src);

            			cs->step_index = *src++;
            			if (cs->step_index > 88)
				{
                			printf("ERROR: step_index = %i\n", cs->step_index);
                			cs->step_index = 88;
            			}
            			if (*src++) 
						printf( "unused byte should be null but is %d!!\n", src[-1]); /* unused */
        		}

        		while(src < adpcm_buffer + buf_size)
			{
            			for(m=0; m<4; m++)
				{
                			for(i=0; i<=st; i++)
                    				*samples++ = adpcm_ima_expand_nibble(&c->status[i], src[4*i] & 0x0F, 3);
                			for(i=0; i<=st; i++)
                    				*samples++ = adpcm_ima_expand_nibble(&c->status[i], src[4*i] >> 4  , 3);
                			src++;
            			}
            			src += 4*st;
        		}
        		break;
			
		case CODEC_ID_ADPCM_MS:
			if (adpcm_info.block_align != 0 && buf_size > adpcm_info.block_align)
            			buf_size = adpcm_info.block_align;
			n = buf_size - 7 * adpcm_info.channels;
			if(n < 0)
				return -1;
			block_predictor[0] = av_clip(*src++, 0, 6);
			block_predictor[1] = 0;
			if (st)
            			block_predictor[1] = av_clip(*src++, 0, 6);
			c->status[0].idelta = (int16_t)bytestream_get_le16(&src);
			if (st)
			{
            			c->status[1].idelta = (int16_t)bytestream_get_le16(&src);
        		}
			c->status[0].coeff1 = AdaptCoeff1[block_predictor[0]];
        		c->status[0].coeff2 = AdaptCoeff2[block_predictor[0]];
        		c->status[1].coeff1 = AdaptCoeff1[block_predictor[1]];
        		c->status[1].coeff2 = AdaptCoeff2[block_predictor[1]];

			c->status[0].sample1 = bytestream_get_le16(&src);
        		if (st) 
				c->status[1].sample1 = bytestream_get_le16(&src);
        		c->status[0].sample2 = bytestream_get_le16(&src);
        		if (st) 
				c->status[1].sample2 = bytestream_get_le16(&src);

        		*samples++ = c->status[0].sample2;
        		if (st) 
				*samples++ = c->status[1].sample2;
        		*samples++ = c->status[0].sample1;
        		if (st) 
				*samples++ = c->status[1].sample1;
        		for(;n>0;n--) 
			{
            			*samples++ = adpcm_ms_expand_nibble(&c->status[0 ], src[0] >> 4  );
            			*samples++ = adpcm_ms_expand_nibble(&c->status[st], src[0] & 0x0F);
            			src ++;
        		}
		
        		break;

		default:
			return -1;
	}

	outbuffer.byte = (unsigned char *)samples - (unsigned char *)outbuffer.outb;

end:
	if (outbuffer.outb) {
		if(outbuffer.byte > len) {
			memcpy(buf, outbuffer.outb + outbuffer.index, len);
			outbuffer.byte-= len;
			outbuffer.index += len;
			output_size = len;
		} else {
			memcpy(buf, outbuffer.outb + outbuffer.index, outbuffer.byte);
			output_size = outbuffer.byte;
			outbuffer.byte = 0;
			outbuffer.index = 0;
		}
	}

	return output_size;
}

static int adpcm_decode_release(void)
{
	if(adpcm_buffer)
		free(adpcm_buffer);
	if(outbuffer.outb)
		free(outbuffer.outb);

	return 0;
}

#define ADPCM_DECODE(name, format, long_name)	\
am_codec_struct	name ## _decode={	\
	long_name,				\
	format,					\
	0,						\
	adpcm_init,				\
	adpcm_decode_release,	\
	adpcm_decode_frame,		\
};

ADPCM_DECODE(adpcm, AUDIO_FORMAT_ADPCM, "ADPCM");

int register_adpcm_codec(void)
{
	register_audio_codec(&adpcm_decode);
	return 0;
}
