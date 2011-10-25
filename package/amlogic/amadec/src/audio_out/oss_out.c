
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/soundcard.h>

#include <config.h>


#include "adec.h"
#include "log.h"
#define printf(fmt,args...) lp(0,"[OSS] "fmt,##args)

#ifndef PATH_DEV_DSP
#define PATH_DEV_DSP	"/dev/dsp"
#endif
//#include "../config.h"
#define MAX_OUTBURST 65536
static int audio_fd;
static int oss_init(adec_feeder_t * feed);
static int oss_uninit(void);
static int oss_play(char *data, unsigned len);
static  int oss_get_space(void);

static audio_buf_info zz;
static int buffer_size;
static int prepause_space;
static int channels;
static int samplerate;

static int oss_init(adec_feeder_t * feed)
{
	int r;
      audio_fd=open(PATH_DEV_DSP, O_WRONLY);
      if(audio_fd<0)
	  	{  
	  	printf("Can't open audio device\n");
		return -1; 
		}
	  
	channels = feed->channel_num;
	samplerate = feed->sample_rate;
	
	r=AFMT_S16_LE;
	ioctl (audio_fd, SNDCTL_DSP_SETFMT, &r);
	r=feed->channel_num-1;
	printf("feed->channel_num=%d\n",feed->channel_num);
	ioctl (audio_fd, SNDCTL_DSP_STEREO, &r);
	  r=feed->sample_rate;
	printf("feed->sample_rate=%d\n",feed->sample_rate);
	ioctl (audio_fd, SNDCTL_DSP_SPEED, &r);

	if(ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &zz)==-1){
		printf("oss_init:driver doesn't support SNDCTL_DSP_GETOSPACE :-(\n");
	}else{
		buffer_size = zz.bytes;
	}

	  return 0;
}

static int oss_set_datewidth(int width)
{
	int fmt;
	switch(width)
		{
		case 8:
			fmt=AFMT_S8;
			break;
		case 16:
			fmt=AFMT_S16_LE;
			break;
		default:
			return -1;
		}
	 printf("oss_set_datewidth=%d,fmt=%d\n",width,fmt);
	return ioctl (audio_fd, SNDCTL_DSP_SPEED, &fmt);
}
static int oss_set_channelnum(int num)
{
	 printf("oss_set_channelnum=%d\n",num);
	 num--;
	ioctl (audio_fd, SNDCTL_DSP_SPEED, &num);
}
static int oss_set_sample_rate(int sample_rate)
{	
	printf("oss_set_channelnum=%d\n",sample_rate);
 	ioctl (audio_fd, SNDCTL_DSP_SPEED, &sample_rate);
}
static int oss_uninit(void)
{
	printf("Calling oss uninit\n");
	if(audio_fd)
		{
		close(audio_fd);
		audio_fd=0;
		return 0;
		}
	return -1;
}

static int oss_play(char  *data, unsigned len)
{
 	return write(audio_fd,data,len);
}
static  int oss_get_space(void)
{
	audio_buf_info zz;
 	int playsize=0;
	 if(ioctl(audio_fd, SNDCTL_DSP_GETOSPACE, &zz)!=-1){
		      // calculate exact buffer space:
		      playsize = zz.fragments*zz.fragsize;
		      if (playsize > MAX_OUTBURST)
				playsize = (MAX_OUTBURST / zz.fragsize) * zz.fragsize;
	  }
	  return playsize; 
}

static void oss_reset(void)
{
	int r;

	ioctl(audio_fd, SNDCTL_DSP_RESET, NULL);
	oss_uninit();
	audio_fd=open(PATH_DEV_DSP, O_WRONLY);
      if(audio_fd<0){  
	  	printf("Can't open audio device\n");
		return -1; 
	}

	r = AFMT_S16_LE;
	ioctl (audio_fd, SNDCTL_DSP_SETFMT, &r);
	r = channels-1;
	ioctl (audio_fd, SNDCTL_DSP_STEREO, &r);
	r = samplerate;
	ioctl (audio_fd, SNDCTL_DSP_SPEED, &r);
}

static void oss_pause(void)
{
	prepause_space = oss_get_space();
	ioctl(audio_fd, SNDCTL_DSP_RESET, NULL);
	oss_uninit();
}

static void oss_resume(void)
{
	int fillcnt;
	oss_reset();
	fillcnt = oss_get_space() - prepause_space;
	if(fillcnt > 0){
		void *silence = calloc(fillcnt, 1);
      		oss_play(silence, fillcnt);
      		free(silence);
	}
}

static int oss_get_delay()
{
      	int r=0;
	int frame_num;
      	if(ioctl(audio_fd, SNDCTL_DSP_GETODELAY, &r)!=-1) {
			frame_num = r / (channels * 4);
			return (frame_num*(1000000/samplerate));
      	}
	frame_num = buffer_size;
	return (frame_num*(1000000/samplerate));
         	
}

static   adec_out_t oss_device=
{
	.init=oss_init,
	.uninit=oss_uninit,
	.play=oss_play,
	.get_space=oss_get_space,
	.set_data_width=oss_set_datewidth,
	.set_channel_num=oss_set_channelnum,
	.set_sample_rate=oss_set_sample_rate,
	.pause = oss_pause,
	.resume = oss_resume,
	.reset = oss_reset,
	.get_delay = oss_get_delay,
};


adec_out_t * get_oss_device(void)
{
	return &oss_device;
}

