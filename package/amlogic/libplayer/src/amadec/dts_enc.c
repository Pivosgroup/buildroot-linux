#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <log-print.h>
#include <pthread.h>
#include "dts_enc.h"
#include "pcmenc_api.h"
#include "spdif_api.h"

typedef enum {
    IDLE,
    TERMINATED,
    STOPPED,
    INITTED,
    ACTIVE,
    PAUSED,
} dtsenc_state_t;

typedef struct{
    dtsenc_state_t  state;
    pthread_t       thread_pid;
    int raw_mode;
    int dts_flag;
}dtsenc_info_t;

    
static dtsenc_info_t dtsenc_info;
static void *dts_enc_loop();

#define DIGITAL_RAW_PATH             "sys/class/audiodsp/digital_raw"
#define FORMAT_PATH                        "/sys/class/astream/format"

static int get_dts_mode(void)
{
    int fd;
    int val = 0;
    char  bcmd[16];
    fd = open(DIGITAL_RAW_PATH, O_RDONLY);
    if (fd >= 0) {
        read(fd, bcmd, sizeof(bcmd));
        val = strtol(bcmd, NULL, 16);
        close(fd);
    }
    return val&0xf;
    
}

static int get_dts_format(void)
{
    int fd;
    char format[21];
    int len;

    format[0] = 0;

    fd = open(FORMAT_PATH, O_RDONLY);
    if (fd < 0) {
        adec_print("amadec device not found");
        return 0;
    }
    len = read(fd, format, 20);
    if (len > 0) {
        format[len] = 0;
    }
    if (strncmp(format, "NA", 2) == 0) {
        close(fd);
        return 0;
    }
    adec_print("amadec format: %s", format);
    if (strncmp(format, "amadec_dts", 10) == 0) {
        close(fd);
        return 1;
    }
    return 0;
}

int dtsenc_init()
{
    memset(&dtsenc_info,0,sizeof(dtsenc_info_t));
    /*dtsenc_info.dts_flag = get_dts_format();
    if(!dtsenc_info.dts_flag)
        return -1;
    dtsenc_info.raw_mode=get_dts_mode();
    if(!dtsenc_info.raw_mode)
        return -1;*/
   
    //drive init
    #if 0
    int ret=pcmenc_init();
    if(ret!=0)
    {
        adec_print("===pcmenc_init failed errno:%d \n",ret);
        return -1;
     }
    ret=iec958_init();
    if(ret!=0)
    {
        adec_print("===iec958_init failed errno:%d \n",ret);
        pcmenc_deinit();
        return -1;
    }
    #endif
    
    dtsenc_info.state=INITTED;
    adec_print("====dts_enc init success \n");
    return 0;
}
int dtsenc_start()
{
    if(dtsenc_info.state!=INITTED)
        return -1;
    pthread_t    tid;
    int ret = pthread_create(&tid, NULL, (void *)dts_enc_loop, NULL);
     if (ret != 0) {
        return ret;
    }
    dtsenc_info.thread_pid = tid;
    dtsenc_info.state=ACTIVE;
    adec_print("====dts_enc thread start success \n");
    return 0;
}
int dtsenc_pause()
{
    if(dtsenc_info.state!=ACTIVE)
        return -1;
    dtsenc_info.state=PAUSED;
    return 0;
}
int dtsenc_resume()
{
    if(dtsenc_info.state!=PAUSED)
        return -1;
    dtsenc_info.state=ACTIVE;
    return 0;
}
int dtsenc_stop()
{
    if(dtsenc_info.state<INITTED)
            return -1;
    dtsenc_info.state=STOPPED;
    //jone the thread
    if(dtsenc_info.thread_pid<=0)
        return -1;
    int ret = pthread_join(dtsenc_info.thread_pid, NULL);
    dtsenc_info.thread_pid=0;
    adec_print("====dts_enc stop ok\n");
    return 0;
}
int dtsenc_release()
{
    if(dtsenc_info.state!=STOPPED)
            return -1;
     //driver release 
     #if 0
     pcmenc_deinit();
     iec958_deinit();
     #endif
     dtsenc_info.state=TERMINATED;
     return 0;
}

static void *dts_enc_loop()
{
    while(1)
    {
        switch(dtsenc_info.state)
        {
            case INITTED:
               usleep(100000);
               continue;
            case ACTIVE:
                break;
            case PAUSED:
                usleep(100000);
                continue;
            case STOPPED:
                goto quit_loop;
            default:
                goto err;
          }
          //shaoshuai --non_block
          //enc_data();
          //write data();
          adec_print("====dts_enc thread is running \n");
          usleep(100000);
    }
 quit_loop:
    adec_print("====dts_enc thread exit success \n");
    pthread_exit(NULL);
    return 0;
 err:
 adec_print("====dts_enc thread exit success err\n");
    pthread_exit(NULL);
    return -1;
}


