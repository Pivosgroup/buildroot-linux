#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <pthread.h>

#include "include/log.h"
#include "include/adec.h"
#include "include/feeder.h"

#include "include/oss_out.h" 
#include "include/alsa_out.h"
#include "include/audiodsp_ctl.h"

//#define DUMP_ORIGINAL_DATA //
#define DUMP_TO_FILE //
#define TEST_PCM_IN
#define TSYNC_PCRSCR    "/sys/class/tsync/pts_pcrscr"
#define TSYNC_EVENT     "/sys/class/tsync/event"
#define TSYNC_APTS      "/sys/class/tsync/pts_audio"

#define abs(x) ({                               \
                long __x = (x);                 \
                (__x < 0) ? -__x : __x;         \
                })

#define SYSTIME_CORRECTION_THRESHOLD    (90000/10)
#define APTS_DISCONTINUE_THRESHOLD      (90000*3)
#define REFRESH_PTS_TIME_MS (1000/10)


static int adec_thread_start(void);
static void *adec_thread_run(void *args);

am_codec_struct *get_codec_by_fmt(audio_format_t fmt);

extern int audio_codec_init();
extern int default_outtype;

static unsigned long last_pts_valid;
static int thread_stop=0;
static int thread_running=0;
pthread_cond_t pthread_cond;
static pthread_t play_thread_id;
pthread_mutex_t pthread_mutex;

static adec_out_t *cur_out=NULL;
static am_codec_struct *cur_codec=NULL;

static unsigned long bufferend_len=0;
int pause_flag = 0;

static int first_audio_frame = 0;
static int adec_automute_cmd = 0;
static int adec_automute_stat = 0;

#ifdef DUMP_TO_FILE //
static int debug_file_fd=-1;
#endif

int adec_start()
{
    first_audio_frame = 1;
    if (adec_automute_cmd)
    {
        adec_automute_stat = 1;
        sound_mute_set("switch playback mute", 0, 1, NULL);
    }
    adec_thread_start();
    lp(LOG_INFO, "adec_started\n");     
    return 0;
};

void adec_thread_wait(int microseconds)
{
    struct timespec pthread_ts; 
    struct timeval now;
    gettimeofday (&now, NULL);
    pthread_ts.tv_sec = now.tv_sec + (now.tv_usec*1000 +microseconds*1000)/1000000000 ;
    pthread_ts.tv_nsec =(now.tv_usec*1000 +microseconds*1000)%1000000000 ;
    pthread_mutex_lock(&pthread_mutex);
    pthread_cond_timedwait(&pthread_cond, &pthread_mutex, &pthread_ts);
    pthread_mutex_unlock(&pthread_mutex);
    if(thread_stop)
        {
            printf("thread stoped\n");
            pthread_exit(0);
        }
}

static  int adec_thread_start(void)
{
    lp(LOG_INFO, "adec_thread_start\n");     
    thread_stop=0;  
    pthread_attr_t attr;
    //sched_param param;
    pthread_attr_init(&attr);
        //pthread_attr_getschedparam(&attr, &param);
        // param.sched_priority=15;
     pthread_attr_setschedpolicy (&attr, SCHED_RR);
     pthread_mutex_init(&pthread_mutex,NULL);
     pthread_cond_init(&pthread_cond,NULL);
     pthread_create(&play_thread_id, NULL, &adec_thread_run, (void*)get_default_adec_feeder());
}
static int adec_thread_stop(void)
{ 
    thread_stop=1;
    pthread_cond_signal(&pthread_cond);
    //pthread_cancel(play_thread_id);
    pthread_join(play_thread_id, NULL);
    pthread_mutex_destroy(&pthread_mutex);
    pthread_cond_destroy(&pthread_cond);


lp(LOG_INFO,"adec_thread_stop   thread_stop = %d\n",thread_stop);   
}

unsigned long adec_get_pts(void)
{
    adec_feeder_t  *pfeeder=get_default_adec_feeder(); 
    unsigned long pts,delay_pts;
    int fd;
    int tmp;
    if(pfeeder==NULL)
        return -1;
    pts=pfeeder->get_cur_pts(pfeeder);
    if(pts==-1) {
        //lp(LOG_INFO, "get get_cur_pts failed");
        return -1;
    }

    if((pfeeder->format == AUDIO_FORMAT_COOK)||(pfeeder->format == AUDIO_FORMAT_RAAC))
		return pts;

    if(cur_out==NULL || cur_out->get_delay==NULL)
        return -1;
    delay_pts=	cur_out->get_delay()*90/1000;/*us to 90KHZ*/
    tmp=bufferend_len/(pfeeder->channel_num*(pfeeder->data_width/8));
    delay_pts+=(tmp*90000)/(pfeeder->sample_rate);
    if(delay_pts<pts)
		pts-=delay_pts;
    else
		pts=0;
    return pts;
}
int adec_pts_start(void)
{
    unsigned long pts =0;
    char *file;
    int fd;
    char buf[64];

    memset(buf, 0, sizeof(buf));
    lp(LOG_INFO, "adec_pts_start");

    last_pts_valid = 0;
    pts=adec_get_pts();
    
    if(pts==-1) {

        lp(LOG_INFO, "pts==-1");

        file=TSYNC_APTS;
        if ((fd = open(file, O_RDONLY)) < 0) {
            lp(LOG_ERR, "unable to open file %s,err: %s",file, strerror(errno));
            return -1;
        }
        
        read(fd, buf, sizeof(buf));
        close(fd);
        if (sscanf(buf, "0x%x", &pts) < 1) {
            lp(LOG_ERR, "unable to get apts from: %s", buf);
            return -1;
        }
    }

    lp(LOG_INFO, "audio pts start from 0x%x", pts);

    file=TSYNC_EVENT;
       if ((fd = open(file, O_WRONLY)) < 0) {
            lp(LOG_ERR, "unable to open file %s,err: %s",file, strerror(errno));
            return -1;
        }
    sprintf(buf,"AUDIO_START:0x%x",pts);
    write(fd,buf,strlen(buf));
    close(fd);
    return 0;
}
int adec_pts_resume(void)
{     
    char *file;
    int fd;
    char buf[64];

    memset(buf, 0, sizeof(buf));
    lp(LOG_INFO, "adec_pts_resume");
    file=TSYNC_EVENT;
       if ((fd = open(file, O_WRONLY)) < 0) {
            lp(LOG_ERR, "unable to open file %s,err: %s",file, strerror(errno));
            return -1;
        }
    sprintf(buf,"AUDIO_RESUME");
    write(fd,buf,strlen(buf));
    close(fd);
    return 0;
}
int adec_pts_pause(void)
{
    char *file;
    int fd;
    char buf[64];
    file=TSYNC_EVENT;
       if ((fd = open(file, O_WRONLY)) < 0) {
            lp(LOG_ERR, "unable to open file %s,err: %s",file, strerror(errno));
            return -1;
        };
    sprintf(buf,"AUDIO_PAUSE");
    write(fd,buf,strlen(buf));
    close(fd);
    return 0;
}

int adec_refresh_pts(void)
{
    unsigned long pts;
    unsigned long systime;
    static unsigned long last_pts=-1;
    char *file;
    int fd; 
    char buf[64];
    if (pause_flag)
		return 0;
    /* get system time */
    memset(buf, 0, sizeof(buf));
    file=TSYNC_PCRSCR;
    if ((fd = open(file, O_RDWR)) < 0) {
        lp(LOG_ERR, "unable to open file %s,err: %s",file, strerror(errno));
        return -1;
    }
    
    read(fd,buf,sizeof(buf));
    close(fd);

    if (sscanf(buf,"0x%x",&systime) < 1) {
        lp(LOG_ERR, "unable to getsystime %s", buf);
        close(fd);
        return -1;
    }
    /* get audio time stamp */
	
    pts=adec_get_pts();
    if (pts==-1 || last_pts==pts)
    	{
	    	close(fd);
            if (!adec_automute_stat)
                return -1;
            else if (pts == -1)
                return -1;
	}

    if ((abs(pts-last_pts) > APTS_DISCONTINUE_THRESHOLD) && (last_pts_valid)) {
		/* report audio time interruption */
		lp(LOG_INFO, "pts=%x,last pts=%x\n",pts,last_pts);	
		file = TSYNC_EVENT;
		if ((fd = open(file, O_RDWR)) < 0) {
			lp(LOG_ERR, "unable to open file %s,err: %s", file, strerror(errno));
			return -1;
		}
		adec_feeder_t *feeder=get_default_adec_feeder();
		lp(LOG_INFO, "audio time interrupt: 0x%x->0x%x, 0x%x\n", last_pts, pts, abs(pts-last_pts));

		sprintf(buf, "AUDIO_TSTAMP_DISCONTINUITY:0x%x",pts);
		write(fd,buf,strlen(buf));
		close(fd);

		last_pts = pts;
		last_pts_valid = 1;

		return 0;
    }
    
    last_pts=pts;
    last_pts_valid = 1;
    
    if (abs(pts-systime) < SYSTIME_CORRECTION_THRESHOLD)
    {
        if (first_audio_frame)
        {
            first_audio_frame = 0;
            if (adec_automute_stat)
            {
                sound_mute_set("switch playback mute", 1, 1, NULL);
                adec_automute_stat = adec_automute_cmd;
            }
        }
        return 0;
    }

    if (adec_automute_stat &&first_audio_frame)
    {
        if (pts > systime)
        {
            return 1;
        }
        /*else
        {
            return 2;
        }*/
        sound_mute_set("switch playback mute", 1, 1, NULL);
    }
    
    /* report apts-system time difference */
    file = TSYNC_APTS;
    if ((fd = open(file, O_RDWR)) < 0) {
        lp(LOG_ERR, "unable to open file %s,err: %s", file, strerror(errno));
        return -1;
    }

    lp(LOG_INFO, "report apts as 0x%x,system pts=%x, different %d\n", 
        pts,systime, pts-systime);

    sprintf(buf,"0x%x",pts);
    write(fd,buf,strlen(buf));
    close(fd);

    return 0;
}

void adec_stop()
{
    first_audio_frame = 0;
  adec_thread_stop();
   if(cur_out && cur_out->uninit)
    {
            cur_out->uninit();
            cur_out = NULL;
    }
    if(cur_codec&& cur_codec->release)
    {
            cur_codec->release();
            cur_codec = NULL;
    }

    feeder_release();

    lp(LOG_INFO,"adec  stoped\n ");
};

void adec_reset()
{
	if(cur_out && cur_out->reset)
	{
		cur_out->reset();
	}
}

void adec_pause()
{
lp(LOG_INFO,"adec  pause\n ");
   if(cur_out && cur_out->pause)
    {
    	      pause_flag = 1;
             adec_pts_pause();
            cur_out->pause();
    }
};

void adec_resume()
{
   if(cur_out && cur_out->resume)
    {
    	 pause_flag = 0;
         cur_out->resume();
         //lp(LOG_INFO, "adec_pts_resume\n");
         adec_pts_resume();
         //adec_pts_start();
    }
};

void adec_auto_mute(int auto_mute)
{
    adec_automute_cmd = auto_mute;
}

#define BUFFER_SIZE (8*1024)
unsigned char decode_buffer[BUFFER_SIZE+64];
static void *adec_thread_run(void *args)
{
    unsigned char *buffer=(unsigned char *)(((unsigned long)decode_buffer+32)&(~0x1f));
    adec_feeder_t  *pfeeder=NULL; /*before init it is NULL*/
    adec_out_t *pcur_out=NULL;
    am_codec_struct *pcur_codec=NULL;
    int in_underrun_mode=0;

    lp(LOG_INFO,"adec_thread_run running \n");
    thread_running=1;
    
    /* get feeder ready, initialize audio DSP, decoder & output */
    while (!thread_stop) {
        pfeeder = feeder_init();

        if (pfeeder) {
            lp(LOG_INFO, "feeder_init ok ...\n");
            if (!pfeeder->dsp_on) {
                pcur_codec = get_codec_by_fmt(pfeeder->format);

                lp(LOG_INFO, "get_codec_by_fmt ok =%s\n", pcur_codec->name);

                if (pcur_codec == NULL) {
                    lp(LOG_ERR,"No codec for this format found (fmt:%d)\n",
                        pfeeder->format);
                }
                else if(pcur_codec->init!=NULL && pcur_codec->init(pfeeder)!=0) {
                    lp(LOG_ERR, "Codec init failed:name:%s,fmt:%d\n",
                        pcur_codec->name ,pcur_codec->format);
                    pcur_codec=NULL;
                }
                else {
                    pcur_codec->used=1;
                }
            }

            lp(LOG_INFO, "Audio Samplerate is %d\n",pfeeder->sample_rate);
		 
            if(default_outtype == 0) {
                pcur_out=get_alsa_device();
                lp(LOG_INFO, "Audio out type is alsa !\n");
            } else {
                pcur_out=get_oss_device();
                lp(LOG_INFO, "Audio out type is oss !\n");
            }

            if(pcur_out==NULL) {
                lp(LOG_ERR,"No oss device found\n");
                pcur_out=NULL;
            }
            else if( pcur_out->init(pfeeder)!=0) {
                lp(LOG_ERR, "Audio out device init failed\n");
                pcur_out=NULL; 
            }

            cur_out = pcur_out;
            cur_codec = pcur_codec;
            break;
        }

       adec_thread_wait(100000);/*100mS*/
    }
    
    lp(LOG_INFO, pfeeder->dsp_on ? "dsp is on" : "dsp is off");

    #ifdef  DUMP_ORIGINAL_DATA
    if(0)
    #else
    if(cur_out && (cur_codec ||pfeeder->dsp_on))
    #endif
        {//playing
            int len=0;
            int len2=0;
            int offset=0;
            lp(LOG_INFO,"start playing buffer=%x\n ",buffer);
            bufferend_len=len;
            if(pfeeder->dsp_on)
                {
                while(len<(128*2) && !thread_stop)
                    {
                    if(offset>0)
                        memcpy(buffer,buffer+offset,len);
                    len2=pfeeder->dsp_read(buffer+len,BUFFER_SIZE-len);
                    len=len+len2;
                    offset=0;
                    }
                len2=cur_out->play(buffer+offset,len);
                len-=len2;
                bufferend_len=len;
                /*start  the  the pts scr,...*/
                adec_pts_start();
                while(!thread_stop)
                        { 
                            while(len<(128*2) && !thread_stop)
                                {
                                if(offset>0)
                                    memcpy(buffer,buffer+offset,len);
                                len2=pfeeder->dsp_read(buffer+len,BUFFER_SIZE-len);
                                if( len2<=0 && !in_underrun_mode)/*enter underflow*/
                                    {
									if(cur_out->get_delay()<10000)
										{
										adec_pause();
										/*paused the  the pts scr,...*/
										in_underrun_mode=1;
										}
									len2=0;
                                    }
                                len=len+len2;
                                bufferend_len=len;
                                offset=0;
                                }
							while(pause_flag)
								adec_thread_wait(10000);

			                while (adec_refresh_pts() == 1) // audio ahead, wait 100ms
			                    adec_thread_wait(10000);

			                    len2=cur_out->play(buffer+offset,len);
                
                            if(len2>0 && in_underrun_mode)/*exit underflow*/
                                {
                                    adec_resume();
                                    /*resume the  the pts scr,...*/
                                    in_underrun_mode=0;
                                }
                            if(len2>0)
                                {
                                    len-=len2;
                                    offset+=len2;
                                    bufferend_len=len;
                                }
                            if (len < 0)
                                {
                                len = 0;
                                offset=0;
                                }
                        }
                }
            else
                {
                
				int refresh_pts_bytes=-1;
				unsigned long refresh_pts_bytes_max;
                bufferend_len=len;
                struct frame_fmt fmt;
				
				refresh_pts_bytes_max=(REFRESH_PTS_TIME_MS)*
										  (pfeeder->sample_rate*
										  pfeeder->channel_num*
										  pfeeder->data_width/8)/1000;
				memset(&fmt,0,sizeof(fmt));
                    len=cur_codec->decode_frame(buffer,BUFFER_SIZE,&fmt);
                    if(len>0)
                        {
                        if(fmt.channel_num>0)
                            cur_out->set_channel_num(fmt.channel_num);
                        if(fmt.sample_rate>0)
                            cur_out->set_sample_rate(fmt.sample_rate);
                        if(fmt.data_width>0)
                            cur_out->set_data_width(fmt.data_width);
						
                        }
					
					/*start  the  the pts scr,...*/
                	adec_pts_start();
                    while(!thread_stop)
					{ 
						
                            while(len<(128*2) && !thread_stop)
                                {
                                if(offset>0)
                                    memcpy(buffer,buffer+offset,len);
                                len2=cur_codec->decode_frame(buffer+len,BUFFER_SIZE-len,NULL);
                                if( len2<=0 && !in_underrun_mode)/*enter underflow*/
                                    {
									if(cur_out->get_delay()<10000)
										{
										adec_pause();
										adec_thread_wait(100000);
										/*paused the  the pts scr,...*/
										in_underrun_mode=1;
										refresh_pts_bytes=-1;
										}
									len2=0;
                                    }
                                len=len+len2;
                                bufferend_len=len;
                                offset=0;
                                }
							while(pause_flag)
								adec_thread_wait(100000);
							if(refresh_pts_bytes<0)
								{  
				                while (adec_refresh_pts() == 1) // audio ahead, wait 100ms
				                    adec_thread_wait(10000);
								refresh_pts_bytes=refresh_pts_bytes_max;
								}
			                len2=cur_out->play(buffer+offset,len);
                            if(len2>0 && in_underrun_mode)/*exit underflow*/
                                {
                                    adec_resume();
                                    /*resume the  the pts scr,...*/
                                    in_underrun_mode=0;
                                }
                            if(len2>0)
                                {
                                    len-=len2;
                                    offset+=len2;
                                    bufferend_len=len;
									refresh_pts_bytes-=len2;
                                }
                            if (len < 0)
                                {
                                len = 0;
                                offset=0;
                                }
                      }
                }
        }
    else
        {
        //pop data only,I think should do it in kernel ....
        //so the audio codec error can do no effect to 
        //video out;
        lp(LOG_INFO,"start throw data\n ");
        
            unsigned long tmp=0;
            while (!thread_stop)
            {
                pfeeder->get_bits(&tmp,8);
                *buffer++=tmp&0xff;
                
                if(buffer-decode_buffer>2000)
                    {
                    #ifdef  DUMP_TO_FILE
                    if(debug_file_fd>=0)
                        write(debug_file_fd, decode_buffer,buffer-decode_buffer);
                    buffer=decode_buffer;
                    #endif
                    }
                //lp("[%d]",tmp);
                //usleep(10);
            }
        }
    thread_running=0;
    printf("--thread_exit\n");
    pthread_exit(0);
    return 0;
}




int adec_init(void)
{
    audio_codec_init();
    return 0;
}



