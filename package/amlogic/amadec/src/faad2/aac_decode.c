#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <math.h>

#include "adec.h"
#include "log.h"
#include "neaacdec.h"
#include "audio.h"

#define	LOCAL	inline

#define	MAX_CHANNELS	6

static unsigned char frame_buffer[FAAD_MIN_STREAMSIZE * MAX_CHANNELS];
static adec_feeder_t *a_feeder=NULL;
static aac_buffer b;

static NeAACDecHandle hDecoder;
static NeAACDecFrameInfo frameInfo;
static NeAACDecConfigurationPtr config;

static int first_time = 1;

static LOCAL int  aac_read_byte(void)
{
	unsigned long tmp;
	a_feeder->get_bits(&tmp,8);
	return tmp;
}
static LOCAL int  aac_read_2byte(void)
{
	unsigned long tmp;
	a_feeder->get_bits(&tmp,16);
	return tmp;
}

static LOCAL int  aac_reset_bits(void)
{
	unsigned long tmp,bit;
	bit=a_feeder->bits_left()&(7);
	if(bit>0)
		a_feeder->get_bits(&tmp,bit);
}

static LOCAL int faad_read(char *buf,int size)
{
  int i;
  aac_reset_bits();
  for(i=0;i<size;i++)
  	{
	buf[i]=aac_read_byte();
	//tmp=read_2byte();
	//buf[2*i]=(tmp&0xff00)>>8;
	//buf[2*i+1]=tmp&0xff;

  	}
  return i;
}

static int fill_buffer(aac_buffer *b)
{
	int bread;

	if (b->bytes_consumed > 0)
	{
		if (b->bytes_into_buffer)
		{
			memmove((void*)b->buffer, (void*)(b->buffer + b->bytes_consumed),
					b->bytes_into_buffer*sizeof(unsigned char));
		}

		if ( !b->at_eof)
		{
			bread = faad_read((b->buffer + b->bytes_into_buffer), b->bytes_consumed);
			
			if (bread != b->bytes_consumed)
				b->at_eof = 1;
			
			b->bytes_into_buffer += bread;
		}

		b->bytes_consumed = 0;

		if (b->bytes_into_buffer > 3)
       	{
          	 	if (memcmp(b->buffer, "TAG", 3) == 0)
                	b->bytes_into_buffer = 0;
        	}
      		if (b->bytes_into_buffer > 11)
        	{
            		if (memcmp(b->buffer, "LYRICSBEGIN", 11) == 0)
                	b->bytes_into_buffer = 0;
        	}
        	if (b->bytes_into_buffer > 8)
        	{
            		if (memcmp(b->buffer, "APETAGEX", 8) == 0)
                	b->bytes_into_buffer = 0;
        	}
			
	}

	return 1;
}

static void advance_buffer(aac_buffer *b, int bytes)
{
	b->file_offset += bytes;
	b->bytes_consumed = bytes;
	b->bytes_into_buffer -= bytes;

	if (b->bytes_into_buffer < 0)
		b->bytes_into_buffer = 0;
}

static int faad_init(adec_feeder_t * feeder)
{
	unsigned long samplerate;
	unsigned char channels;
 	int bread;
	int tagsize;
	int header_type;
	
	a_feeder = feeder;
	if (a_feeder == NULL)
		return -1;

	memset(&b, 0, sizeof(aac_buffer));
	b.buffer = frame_buffer;
	
	bread = faad_read(b.buffer, FAAD_MIN_STREAMSIZE*MAX_CHANNELS);
	b.bytes_into_buffer = bread;
	b.bytes_consumed =0;
	b.file_offset = 0;

	if (bread != FAAD_MIN_STREAMSIZE*MAX_CHANNELS)
		b.at_eof = 1;

	tagsize = 0;
	if (!memcmp(b.buffer, "ID3", 3))
	{
		tagsize = (b.buffer[6] << 21) | (b.buffer[7] << 14) |
			(b.buffer[8] << 7) | (b.buffer[9] <<0);
		tagsize += 10;

		advance_buffer(&b, tagsize);
		fill_buffer(&b);
	}

	hDecoder = NeAACDecOpen();
	config = NeAACDecGetCurrentConfiguration(hDecoder);
	config->defObjectType = LC;
    	config->outputFormat = FAAD_FMT_16BIT;
    	config->downMatrix = 0;
    	config->useOldADTSFormat = 0;
    	//config->dontUpSampleImplicitSBR = 1;
    	NeAACDecSetConfiguration(hDecoder, config);


	if ((bread = NeAACDecInit(hDecoder, b.buffer,
        b.bytes_into_buffer, &samplerate, &channels)) < 0)
    	{
        /* If some error initializing occured, skip the file */
        printf("Error initializing decoder library.\n");
        NeAACDecClose(hDecoder);
        return 1;
    	}

	a_feeder->sample_rate = samplerate;
	a_feeder->channel_num = channels;
	
	advance_buffer(&b, bread);
    	fill_buffer(&b);
		
	return 0;
}

static void * sample_buffer;
static int faad_decode_frame(char *buf,int len,struct frame_fmt *fmt)
{ 
	int res, bread;
	
	sample_buffer = NeAACDecDecode(hDecoder, &frameInfo,
            b.buffer, b.bytes_into_buffer);
         
	res = frameInfo.samples * 2;
	memcpy(buf, (char*)sample_buffer, res);

	advance_buffer(&b, frameInfo.bytesconsumed);

	

	 if (frameInfo.error > 0)
        {
           	 printf( "Error: %s\n",
                NeAACDecGetErrorMessage(frameInfo.error));
        }

	 if (first_time && !frameInfo.error)
	 {
	 	fmt->channel_num = frameInfo.channels;
		fmt->sample_rate = frameInfo.samplerate;

		first_time = 0;
	 }

	 fill_buffer(& b);
	
	 return res;
}

static int faad_decode_release(void)
{
	if(hDecoder)
	{
		NeAACDecClose(hDecoder);
		hDecoder = NULL;
		sample_buffer = NULL;
		memset(&frameInfo, 0, sizeof(frameInfo));
		memset(&config, 0, sizeof(config));
		
		return 0;
	}
	
	return 1;
}

static am_codec_struct faad_codec=
{
  .name="faad2",
  .format=AUDIO_FORMAT_AAC,
  .used=0,
  .init=faad_init,
  .release=faad_decode_release,
  .decode_frame=faad_decode_frame,
};


int register_faad_codec(void)
{
	return register_audio_codec(&faad_codec);
}


