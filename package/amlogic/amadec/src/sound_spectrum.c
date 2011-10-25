#include <stdio.h>

#include "sound_spectrum.h"
#include "sound_ctrl.h"
#include "taskmngr.h"
#include "comm_socket_udp.h"

#define SPECTRUM_UDP_PATH   "/tmp/spectrum_udp_path"
#define CLIENT_UDP_PATH     "/tmp/.spectrum_daemon_path"

static int socket_fd = -1;

static int isStartPopRawDat = -1;

static int task_id = -1;

#define FAILED_NOTIFY_RETRY 10
void* spectrum_notify_task(void* interval)
{
    int flag = 1;
    int ret = -1;
    MP_AudioSpectrum dat;   
    int delay = 150000;
    int retry = 0;
    if(*(int*)interval > 50)
        delay = *(int*)interval *1000;   
    while(flag&&(retry < FAILED_NOTIFY_RETRY))
    {
        ret = get_audioSpectrum(&dat);
        if(ret != 0)
        {
            printf("failed to get spectrum data\n");            
            usleep(delay);          
            continue;
        }        
        ret = Comm_SocketIPC_FlushSend(socket_fd,SPECTRUM_UDP_PATH,&dat,sizeof(MP_AudioSpectrum));
         if(ret != 0)
        {
            printf("failed to send spectrum data\n");       
            retry ++;
        }
        usleep(delay);     
    }
     if(socket_fd >0)
    {
        Comm_SocketIPCClose(socket_fd);
        printf("Exit to notify spectrum task after retry 10\n");
        isStartPopRawDat = -1;
        return;
    }

    isStartPopRawDat = -1;
        
}

int start_spectrum_notify_task(int interval)
{  
     if(isStartPopRawDat >0)
    {
        printf("Just has started pop task.disable to retry!!!\n");    
        return -1;
    }
    socket_fd = Comm_SocketIPCCreate(CLIENT_UDP_PATH);
    if(socket_fd <= 0)
    {
        printf("failed to create udp path for spectrum producer\n");
        return -1;
    }
   
    MP_taskBlockSignalPE();
     task_id = MP_taskCreate("spectrum_notify_task",1, 409600,spectrum_notify_task,(void*)&interval);
     if(task_id <= 0)
    {
        Comm_SocketIPCClose(socket_fd); 
        return -1;   

     }
    printf("start notify spectrum task,default interval:%s,current interval %dms\n","150ms",interval);
     isStartPopRawDat = 1;
     return 0;  
    
}

int stop_spectrum_notify_task()
{
    if(isStartPopRawDat <0)
    {
        printf("Spectral data pop stoped,please start it firstly!\n");
        return -1;
    }
    MP_taskDelete(task_id);
    usleep(100);
    if(socket_fd >0)
    {
        Comm_SocketIPCClose(socket_fd);
        printf("Stop notify spectrum task\n");
        isStartPopRawDat = -1;
        return 0;
    }

    isStartPopRawDat = -1;
    return -1;
}

