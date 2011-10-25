#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

#include "include/log.h"
#include "include/adec.h"

#include "include/oss_out.h" 
#include "include/alsa_out.h"
#include "include/audiodsp_ctl.h"


#define FORMAT_PATH \
"/sys/class/astream/format"
#define CHANNUM_PATH \
"/sys/class/astream/channum"
#define SAMPLE_RATE_PATH \
"/sys/class/astream/samplerate"
#define AUDIOPTS_PATH \
	"/sys/class/astream/pts"

#define DATAWIDTH_PATH \
"/sys/class/astream/datawidth"

	

#define AIFIFO_DEV \
"/dev/uio0"

#define AIFIFO_ADDR     \
	"/sys/class/astream/astream-dev/uio0/maps/map0/addr"
#define AIFIFO_SIZE     \
	"/sys/class/astream/astream-dev/uio0/maps/map0/size"
#define AIFIFO_OFFSET \
	"/sys/class/astream/astream-dev/uio0/maps/map0/offset"
	
static volatile unsigned *amport_regs = (unsigned *)MAP_FAILED;
static int amport_reg_offset;
static int amport_reg_size;
#define aififo_bits_ready(x) (((volatile)amport_regs[7] * 8) >= (x))
static int amport_fd;
#ifndef min
#define min(x,y) ((x)<(y)?(x):(y))
#endif
static unsigned long  get_num_infile(char *file)
{
    int fd;
    char buf[24]="";
    unsigned long num=0;
    if ((fd = open(file, O_RDONLY)) < 0) {
        lp(LOG_ERR, "unable to open file %s,err: %s",file, strerror(errno));
        return 0;
    }
    read(fd, buf, sizeof(buf));
    num = strtoul(buf, NULL, 0);
    close(fd);
   return num;
}


static audio_format_t get_audio_format()
{
    int fd;
    char format[21];
    int len;

    format[0] = 0;

    fd = open(FORMAT_PATH, O_RDONLY);
    if (fd < 0) {
        lp(LOG_INFO, "amadec device not found");
        return AUDIO_FORMAT_UNKNOWN;
    }

    len = read(fd, format, 20);
    if (len > 0) {
        format[len] = 0;
    }
     if (strncmp(format, "NA", 2) == 0) {
        close(fd);
        return AUDIO_FORMAT_UNKNOWN;
    }
	 
    lp(LOG_INFO, "amadec format: %s", format);

    if (strncmp(format, "amadec_mpeg", 11) == 0) {
        close(fd);
        return AUDIO_FORMAT_MPEG;
    }

    if (strncmp(format, "amadec_pcm_s16le", 16) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_PCM_S16LE;
    }

    if (strncmp(format, "amadec_pcm_s16be", 16) == 0) {
	   /*TODO: get format/channel numer/sample rate etc */
	   close(fd);
	   return AUDIO_FORMAT_PCM_S16BE;
    }

    if (strncmp(format, "amadec_pcm_u8", 13) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_PCM_U8;
    }

    if (strncmp(format, "amadec_adpcm", 12) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_ADPCM;
    }
	
    if (strncmp(format, "amadec_aac", 10) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_AAC;
    }

     if (strncmp(format, "amadec_ac3", 10) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_AC3;
    }

      if (strncmp(format, "amadec_alaw", 11) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_ALAW;
    }

      if (strncmp(format, "amadec_mulaw", 12) == 0) {
        /*TODO: get format/channel numer/sample rate etc */
        close(fd);
        return AUDIO_FORMAT_MULAW;
    }
	  
      if (strncmp(format, "amadec_dts", 10) == 0) {
	  /*TODO: get format/channel numer/sample rate etc */
	  close(fd);
	  return AUDIO_FORMAT_DTS;
    }

      if (strncmp(format, "amadec_flac", 11) == 0) {
	   /*TODO: get format/channel numer/sample rate etc */
	   close(fd);
	   return AUDIO_FORMAT_FLAC;
    }

	if(strncmp(format, "amadec_cook", 11) == 0) {
	    /*TODO: get format/channel numer/sample rate etc */
	    close(fd);
	    return AUDIO_FORMAT_COOK;
    }
	if(strncmp(format, "amadec_amr", 10) == 0) {
	    /*TODO: get format/channel numer/sample rate etc */
	    close(fd);
	    return AUDIO_FORMAT_AMR;
    }
	if(strncmp(format, "amadec_raac", 11) == 0) {
	    /*TODO: get format/channel numer/sample rate etc */
	    close(fd);
	    return AUDIO_FORMAT_RAAC;
    }
	if(strncmp(format, "amadec_wma", 10) == 0) {
	    /*TODO: get format/channel numer/sample rate etc */
	    close(fd);
	    return AUDIO_FORMAT_WMA;
    }	  
    close(fd);

    lp(LOG_ERR, "audio format unknow.");

    return AUDIO_FORMAT_UNKNOWN;
}


static int amport_init()
{
    void *amport_reg_addr;
    char buf[16];
    int fd;
    int pagesize = getpagesize();

    if ((fd = open(AIFIFO_DEV, O_RDWR)) < 0) {
        lp(LOG_ERR, "unable to open UIO device, %s", strerror(errno));
        return -4;
    	}

    amport_reg_addr = (void*)get_num_infile(AIFIFO_ADDR);
    amport_reg_size =(int)get_num_infile(AIFIFO_SIZE);
	amport_reg_offset=((unsigned long)amport_reg_addr) & ((pagesize-1));

    if(amport_reg_addr==NULL || amport_reg_size==0)
    	{
    	   lp(LOG_ERR,"get maped  addr and regs error=%x\n",amport_reg_addr);
  	   close(fd);
    	   return -1;
    	}
    lp(LOG_ERR,"reg offset=%d\n",amport_reg_offset);
    lp(LOG_INFO, "UIO mmap address: 0x%x, size: 0x%x, offset: 0x%x, pagesize: 0x%x",
    amport_reg_addr, amport_reg_size, amport_reg_offset, pagesize);

    amport_reg_size = (amport_reg_size + pagesize - 1) & ~(pagesize - 1);

    

    amport_regs = mmap(NULL, amport_reg_size,
                       PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (amport_regs == MAP_FAILED) {
        lp(LOG_ERR, "amports aififo mapping failed");
        close(fd);
        return -5;
    }
     lp(LOG_INFO, "maped reg to %x\n",amport_regs);
    amport_fd=fd;
    amport_regs = (unsigned *)((unsigned)amport_regs + amport_reg_offset);
    close(fd);

	 return 0;
}

static void amport_release(void)
{
    if (amport_fd>=0 && amport_regs != MAP_FAILED) {
        amport_regs = (unsigned *)((unsigned)amport_regs - amport_reg_offset);

        munmap((void *)amport_regs, amport_reg_size);
	//close(amport_fd);
        amport_regs = MAP_FAILED;
	 amport_fd=-1;
    }
}

static int inline  wait_bits_ready(int len)
{
	while(!aififo_bits_ready(len))
		{
		adec_thread_wait(1000);
		}
}

static int amport_get_bits(unsigned long *buf,int len)
{
	wait_bits_ready(len);
    amport_regs[2] = len;
	buf[0]=amport_regs[2];
	return 0;

}


static int amport_get_bytes(unsigned char *buffer,int size)
{
	int bytes;
	int len;
	unsigned char *p=buffer;
	int tmp;
	int space;
	int i;
	int wait_times=0;
	for(len=0;len<size;)
	{
					space=(size-len);
					bytes=amport_regs[7];//READ_MPEG_REG(AIU_MEM_AIFIFO_BYTES_AVAIL);
					wait_times=0;
					while(bytes==0)
					{
							wait_bits_ready((space>128)?128*8:(space*8));		/*wait 32 bytes,
																																			
									if the space is less than 32 bytes,wait the space bits
																																			
									*/
							bytes=amport_regs[7];//READ_MPEG_REG(AIU_MEM_AIFIFO_BYTES_AVAIL);
							wait_times++;
							if(wait_times>10)
									goto out;
					}
					bytes=min(space,bytes);
					for(i=0;i<bytes;i++)
					{
							//WRITE_MPEG_REG(MREG_AIU_AIFIFO_GBIT,8);
							//tmp=READ_MPEG_REG(MREG_AIU_AIFIFO_GBIT);
							amport_regs[2] = 8;
							tmp=amport_regs[2];
							*p++=tmp&0xff;
					}
					len+=bytes;
	}
	out:
	//stream_in_offset+=len;
	return len;
}


static int amport_reset_bits(void)
{
	amport_regs[2]=0x80;
}
static int amport_bits_left(void)
{
	return amport_regs[1]&0x1f;
}

static int amport_peek_bits(unsigned  long *buf,int len)
{
	unsigned long tmp;
	wait_bits_ready(len);
	amport_regs[2] = 0x40;
	tmp= amport_regs[2] ;
	buf[0]=((tmp&0xff)<<8|
			(tmp&0xff00)>>8)>>(16-len);
	return 0;
}
unsigned long get_cur_pts(adec_feeder_t *feeder)
{
	if(feeder->dsp_on)
		{
			return audiodsp_get_pts();
		}
	else
		{
		    int fd;
		    char buf[24]="";
		    unsigned long num=0;
		    if ((fd = open(AUDIOPTS_PATH, O_RDONLY)) < 0) {
		        lp(LOG_ERR, "unable to open file %s,err: %s",AUDIOPTS_PATH, strerror(errno));
		        return 0;
		    }
		    read(fd, buf, sizeof(buf));
			close(fd);
			if(buf[0]=='N' && buf[1]=='A')
				return -1;/*NA is no pts valid*/
		    num = strtoul(buf, NULL, 0);
		   // lp(LOG_INFO, "current audio pts= %x\n",num);
		   return num;
		}
}

static adec_feeder_t adec_feeder =
{
    .get_bits  = amport_get_bits,
	.get_bytes=amport_get_bytes,
    .peek_bits = amport_peek_bits,
    .reset_bits=amport_reset_bits,
    .bits_left=amport_bits_left,
    .get_cur_pts=get_cur_pts,
};

adec_feeder_t  *get_default_adec_feeder(void )
{
	return &adec_feeder;
}

adec_feeder_t * feeder_init(void)
{
	int ret=0;

	//adec_feeder.sample_rate=44100;
	//adec_feeder.channel_num=2;
	//adec_feeder.data_width=16;
	adec_feeder.format=get_audio_format();
	adec_feeder.channel_num=get_num_infile(CHANNUM_PATH);
	adec_feeder.data_width=get_num_infile(DATAWIDTH_PATH);
	adec_feeder.sample_rate=get_num_infile(SAMPLE_RATE_PATH);
	if(adec_feeder.format==AUDIO_FORMAT_UNKNOWN)
		{
		  return NULL;
		}
	audiodsp_init();
	if(audiodsp_start(&adec_feeder)==0)
		{
			adec_feeder.dsp_on=1;
			adec_feeder.dsp_read=audiodsp_stream_read;
		}
	else
		{
		audiodsp_release();
		adec_feeder.dsp_on=0;
		adec_feeder.dsp_read=NULL;
		ret=amport_init();
		}
	return ret==0?&adec_feeder:NULL;
}

int feeder_release(void)
{
	audiodsp_stop();
	audiodsp_release();
	amport_release();
	adec_feeder.dsp_on=0;
	return 0;
}
