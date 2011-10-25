#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "msgmngr_internal.h"
#include "mp_api.h"
#include "mp_utils.h"
#include "mp_types.h"
#include "mp_log.h"
#include "msgmngr.h"

static pGetRespMsgCallback prfunc = NULL;
static void* prObj = NULL;

#define SYNC_FUNC_TIMEO 1000*60    //so long time,1 min
#define SYNC_WAIT_FOREVER -1

int MP_PlayMediaSource(const char*url,int isRepeat,MP_MediaPlayMode plmode)
{
    MP_CommandMsg msg;
    MP_PlayFileMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];

    if(NULL == url && strlen(url) >FILE_URL_MAX&&strlen(url) ==0)
    {
        printf("url must not null and less than 1024\n");
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));
    msg.head.type = MP_CMD_PLAYFILE;

    body.url = strndup(url,FILE_URL_MAX);   
    body.isStart = 1;
    body.isRepeat = isRepeat;
    body.plmode = plmode;
    body.urlLength = strlen(url)+1;
    msg.auxDatLen = body.urlLength + 4*7;

    msg.auxDat = &body;
    msg.head.req_seq_no = cli_get_request_seq_no();

    if(msg.head.req_seq_no < 0)
    {
        log_err("play command failed\n");
        return -1;
    }
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret != 0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }
    cli_post_message(packet,packetLen);
    free(body.url);

    ret = cli_wait_bind_response(msg.head.req_seq_no,SYNC_FUNC_TIMEO);
    if(ret >0)        
        return msg.head.req_seq_no;
    else
        return -1;

}


int MP_CloseMediaID(int mediaID)
{
    int ret =-1;
    ret = cli_close_player_instance(mediaID);
    return ret;
    
}

int MP_CreateMPClient()
{
    int ret = -1;
    ret =  init_client_msg_mngr(NULL,5,NULL,NULL);
    log_con("Create mediaplayer client\n");
    return ret;    
}
int MP_ReleaseMPClient()
{
    int ret = -1;
    ret = uninit_client_msg_mngr();  
    log_con("Release mediaplayer client\n");
    return -1;    
}

int _unpack_response_msg(void* ptObj,void* msg,int len)
{
    MP_ResponseMsg* msgt = NULL;
    MP_ResponseType type;
    int ret = -1;
    if(NULL !=msg&&len >0)
    {        
        msgt = (MP_ResponseMsg*)malloc(sizeof(MP_ResponseMsg));
        if(NULL ==msgt)
        {
           log_err("failed to malloc memory for response msg\n");
           return -1;
        }
        ret = MP_unpack_response_msg((unsigned char*)msg,len,msgt,&type);
        if(ret !=0)
        {
            log_err("failed to unpack msg,type:%d\n",type);
            free(msgt);
            return -1;
        }
        log_info("succeed to unpack a msg,type:%d\n",type);        
              
        if(NULL !=prfunc)
        {
            prfunc(prObj,msgt);       
            return 0;
        }
        else
        {
            MP_free_response_msg(msgt);  
            return -1;
        }
        
           
    }
    log_err("receive invalid msg,size:%d\n,function:%s\n",len,__FUNCTION__);
    return -1;    
    
}

int MP_RegPlayerRespMsgListener(void* pt2Obj,pGetRespMsgCallback pf)
{
    int ret = -1;
     if(pf)
    {
        prfunc = pf;
        prObj= pt2Obj;

        pGetMsgCallback func = &_unpack_response_msg;  
        ret = register_get_msg_func(NULL,func);
        log_info("succeed to register function,%s\n",__FUNCTION__);
        return 0;
        
    }

    prfunc = NULL;
    prObj = NULL;

    register_get_msg_func(NULL,NULL);
        
    log_info("succeed to un-register function:%s\n",__FUNCTION__);
    return -1;
}

int MP_start(int media_id)
{
    MP_CommandMsg msg;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];

    int pid =-1;

    pid = cli_get_amplayer_task_id(media_id);
    if(pid>0)
    {

        memset((void*)&msg,0,sizeof(MP_CommandMsg));

        msg.head.type = MP_CMD_START;

        msg.auxDatLen = sizeof(int);
        msg.auxDat = &pid;

        ret = MP_pack_command_msg(&msg,packet,&packetLen);
        if(ret!=0)
        {
            log_err("failed to pack msg.%s\n",__FUNCTION__);
            return -1;
        }

        cli_post_message(packet,packetLen);
        return 0;

    }
    else
    {
        return -1;
    }
}


int MP_resume(int media_id)
{
    MP_CommandMsg msg;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;

    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }

    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_RESUME;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret != 0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_pause(int media_id)
{
    MP_CommandMsg msg;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_PAUSE;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;
}

int MP_fastforward(int media_id,int speed)
{
    MP_CommandMsg msg;
    MP_TrickPlayMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_FASTFORWARD;

    msg.auxDatLen = sizeof(MP_TrickPlayMsgBody);

    body.mediaID = pid;
    body.speed = speed;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);

    log_info("[%s]: start to fastforward.speed:%d\n",__FUNCTION__,speed);
    return 0;


}

int MP_rewind(int media_id,int speed)
{
    MP_CommandMsg msg;
    MP_TrickPlayMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_FASTREWIND;

    msg.auxDatLen = sizeof(MP_TrickPlayMsgBody);

    body.mediaID = pid;
    body.speed = speed;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);

    log_info("[%s]: start to rewind.speed:%d\n",__FUNCTION__,speed);
    return 0;
}

int MP_stop(int media_id)
{
    MP_CommandMsg msg;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_STOP;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    log_info("post a stop command,media id:%d\n",media_id);
    return 0;

}

int MP_seek(int media_id,int pos,int isStart)
{
    MP_CommandMsg msg;
    MP_SeekMsgBody body;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_SEEK;

    msg.auxDatLen = sizeof(MP_SeekMsgBody);
    body.isStart = isStart;
    body.mediaID = pid;
    body.searchPos = pos;
    msg.auxDat = &body;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;
}
int MP_mute(int media_id,int isMute)
{
    MP_CommandMsg msg;
    MP_MuteMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_MUTE;

    msg.auxDatLen = sizeof(MP_MuteMsgBody);

    body.mediaID = media_id;
    body.isMute = isMute;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_SetVolume(int media_id,int vol)
{
    MP_CommandMsg msg;
    MP_SetVolumeMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];

    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_SETVOLUME;

    msg.auxDatLen = sizeof(MP_SetVolumeMsgBody);

    body.mediaID = media_id;
    body.vol = vol;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_GetVolume(int media_id)
{
    MP_CommandMsg msg;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETVOLUME;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &media_id;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_GetVolumeSync(int media_id,int* vol)
{
    MP_CommandMsg msg;
    MP_ResponseMsg msgt;
    MP_ResponseType type;
    
    unsigned int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETVOLUME;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &media_id;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }
    
    ret = cli_send_message(packet,packetLen,packet,&packetLen,100);
    if(ret!=0)
    {
        log_err("failed to send msg.%s\n",__FUNCTION__);
        
        return -1;
    }

    ret = MP_unpack_response_msg(packet,packetLen,&msgt,&type);   
    if(ret ==0)
    {
        if(type == MP_RESP_VOLUME)
        {

               log_con("====== play volume:%d\n",*(int*)msgt.auxDat);    

              *vol = *(int*)msgt.auxDat;
            
              if( NULL != msgt.auxDat )
              {               
                free(msgt.auxDat);
              }   

              return 0;

        }
    }

    *vol = -1;
    
    return ret;
    
}

int MP_SetTone(int media_id,MP_Tone tone)
{
    MP_CommandMsg msg;
    MP_SetToneMsgBody body;
    unsigned int packetLen;
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    unsigned char packet[MESSAGE_SIZE_MAX];

    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_SETTONE;

    msg.auxDatLen = sizeof(MP_SetToneMsgBody);

    body.mediaID = pid;
    body.tone = tone;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_GetPosition(int media_id)
{
    MP_CommandMsg msg;
    unsigned int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETPOSITION;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);

    log_con("[%s]:get current play time\n",__FUNCTION__);
    return 0;

}
int MP_GetPositionSync(int media_id,int* pos)
{
    MP_CommandMsg msg;
    MP_ResponseMsg msgt;
    MP_ResponseType type;
    
    unsigned int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETPOSITION;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }
    
    ret = cli_send_message(packet,packetLen,packet,&packetLen,100);
    if(ret!=0)
    {
        log_err("failed to send msg.%s\n",__FUNCTION__);
        return -1;
    }

    ret = MP_unpack_response_msg(packet,packetLen,&msgt,&type);   
    if(ret ==0)
    {
        if(type == MP_RESP_TIME_CHANGED)
        {

              log_con("====== play current time:%d\n",*(int*)msgt.auxDat);    

              *pos = *(int*)msgt.auxDat;
            
              if( NULL != msgt.auxDat )
              {               
                free(msgt.auxDat);
              }   

              return 0;

        }
    }

    *pos = -1;
    
    return ret;
}
int MP_GetDuration(int media_id)
{
    MP_CommandMsg msg;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETDURATION;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);

    log_con("[%s]:get total play time\n",__FUNCTION__);
    return 0;

}

int MP_GetDurationSync(int media_id,int* dur)
{
    MP_CommandMsg msg;
    MP_ResponseMsg msgt;
    MP_ResponseType type;
    
    unsigned int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETDURATION;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    ret = cli_send_message(packet,packetLen,packet,&packetLen,100);
    if(ret!=0)
    {
        log_err("failed to send msg.%s\n",__FUNCTION__);
        return -1;
    }

    ret = MP_unpack_response_msg(packet,packetLen,&msgt,&type);   
    if(ret ==0)
    {
        if(type == MP_RESP_DURATION)
        {

              log_con("====== play total time:%d\n",*(int*)msgt.auxDat);    

              *dur = *(int*)msgt.auxDat;
            
              if( NULL != msgt.auxDat )
              {               
                free(msgt.auxDat);
              }   

              return 0;

        }
    }

    *dur = -1;    
    return ret;

}


int MP_GetStatus(int media_id)
{
    MP_CommandMsg msg;    
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETSTATUS;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    
    return 0;
    
}

int MP_GetStatusSync(int media_id,int* status,int* isMuteon,int* isRepeat)
{
    MP_CommandMsg msg;
    MP_ResponseMsg msgt;
    MP_ResponseType type;
    
    unsigned int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETSTATUS;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    ret = cli_send_message(packet,packetLen,packet,&packetLen,100);
    if(ret!=0)
    {
        log_err("failed to send msg.%s\n",__FUNCTION__);
        return -1;
    }

    ret = MP_unpack_response_msg(packet,packetLen,&msgt,&type);   
    if(ret ==0)
    {
         if(type == MP_RESP_STATE_CHANGED)
         {
            *status = ((MP_StateChangedRespBody*)msgt.auxDat)->state;
            *isMuteon =  ((MP_StateChangedRespBody*)msgt.auxDat)->isMuteOn;
            *isRepeat = ((MP_StateChangedRespBody*)msgt.auxDat)->isRepeat;
             if(NULL != ((MP_StateChangedRespBody*)msgt.auxDat)->stateEx&& (unsigned int)msgt.auxDatLen > sizeof(MP_StateChangedRespBody) -4)
            {
                log_err("====== erro msg:%s\n",((char*)((MP_StateChangedRespBody*)msgt.auxDat)->stateEx));
                free(((MP_StateChangedRespBody*)msgt.auxDat)->stateEx);
            }

            if(msgt.auxDat)
              free(msgt.auxDat);
         }

    }
    
    return ret;    
    
}

int MP_SetRepeat(int media_id,int isRepeat)
{
    MP_CommandMsg msg;
    MP_RepeatMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_REPEAT;

    msg.auxDatLen = sizeof(MP_RepeatMsgBody);

    body.mediaID = pid;
    body.isRepeat = isRepeat;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_SetVideoBlackOut(int media_id,int isBlackout)
{
    MP_CommandMsg msg;
    MP_BlackoutMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_BLACKOUT;

    msg.auxDatLen = sizeof(MP_BlackoutMsgBody);

    body.mediaID = pid;
    body.isBlackout= isBlackout;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;
    
}

int MP_SetSongSepctralOut(int media_id,int isStartPop, int interval)
{
    MP_CommandMsg msg;
    MP_SetSpectrumMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_SETSONGSPECTRUM;

    msg.auxDatLen = sizeof(MP_SetSpectrumMsgBody);

    body.mediaID = pid;
    body.isStartPop= isStartPop;
    body.interval = interval;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;
    
}

int MP_SetInternalSubtitleOut(int media_id,int sub_uid)
{
    MP_CommandMsg msg;
    MP_SetSubtitleMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_SETSUBTITLE;

    msg.auxDatLen = sizeof(MP_SetSubtitleMsgBody);

    body.mediaID = pid;
    body.sub_pid= sub_uid;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;

}

int MP_SetAudioTrack(int media_id,int track_uid)
{
    MP_CommandMsg msg;
    MP_SetAudioTrackMsgBody body;
    int packetLen;
    int ret = -1;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_SETAUDIOTRACK;

    msg.auxDatLen = sizeof(MP_SetAudioTrackMsgBody);

    body.mediaID = pid;
    body.track_pid= track_uid;
    msg.auxDat = &body;
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;
    
}

int MP_GetSubtitleInfo(const char*url,int media_id)
{
    MP_CommandMsg msg;    
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }    
    MP_GetInfoMsgBody body;
    if(NULL == url && media_id < 0)
    {
        log_err("Null file path and invalid media id\n");
        return -1;
    }
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETSUBTITLE;

    msg.auxDatLen = sizeof(int);
    body.mediaID = pid;
    
    if(NULL != url)
    {
        msg.auxDatLen +=strlen(url)+1;  
        body.url = strndup(url,FILE_URL_MAX);
    }
    else        
        body.url = NULL;    
    
    msg.auxDat = &body;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    
    return 0;

}

int MP_GetAudioTrackInfo(const char*url,int media_id)
{
    MP_CommandMsg msg;    
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =-1;
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }
    
    MP_GetInfoMsgBody body;
    if(NULL == url && media_id < 0)
    {
        log_err("Null file path and invalid media id\n");
        return -1;
    }
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETAUDIOTRICK;

    msg.auxDatLen = sizeof(int);

    body.mediaID = pid;
    
    if(NULL != url)
    {
        msg.auxDatLen +=strlen(url)+1;  
        body.url = strndup(url,FILE_URL_MAX);
    }
    else        
        body.url = NULL;    
    
    
    msg.auxDat = &body;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    
    return 0;

}

int MP_GetMediaInfo(const char*url,int media_id)
{
 
    MP_CommandMsg msg;    
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    MP_GetInfoMsgBody body;
    int pid =-1;
    if(NULL == url && media_id < 0)
    {
        log_err("Null file path and invalid media id\n");
        return -1;
    }    
    
    pid = cli_get_amplayer_task_id(media_id);
    if(pid<=0)
    {
        return -1;
    }    
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETMEDIAINFO;
    msg.auxDatLen = sizeof(int);
    body.mediaID = pid;
    
    if(NULL != url)
    {
        msg.auxDatLen +=strlen(url)+1;  
        body.url = strndup(url,FILE_URL_MAX);
    }
    else        
        body.url = NULL;    
    
    msg.auxDat = &body;
    
    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }
  
    ret = cli_post_message(packet,packetLen);
    
    return ret;
    
}

int MP_AddMediaSource(const char* url)
{
    MP_CommandMsg msg;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;    
     if(NULL == url && strlen(url) >FILE_URL_MAX&&strlen(url) ==0)
    {
        printf("url must not null and less than 1024\n");
        return -1;
    }
     
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_ADDSOURCE;
    msg.head.req_seq_no = cli_get_request_seq_no();
    if(msg.head.req_seq_no <=0)
    {
        return -1;
    }
    msg.auxDatLen = strlen(url)+1;
    msg.auxDat = strndup(url,FILE_URL_MAX);

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);

    free(msg.auxDat);
    ret = cli_wait_bind_response(msg.head.req_seq_no,SYNC_FUNC_TIMEO);
    if(ret >0)        
        return msg.head.req_seq_no;
    else
        return -1;
    
}

int MP_TerminateAmplayerProcess()
{
    MP_CommandMsg msg;
    int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    int ret = -1;
    int pid =28;
     memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_QUITPLAYER;

    msg.auxDatLen = sizeof(int);
    msg.auxDat = &pid;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }

    cli_post_message(packet,packetLen);
    return 0;
    
}

int MP_GetValidMediaIDSync(MP_ValidMediaIDInfo *mediaidpool)
{
    MP_CommandMsg msg;
    unsigned int packetLen;
    unsigned char packet[MESSAGE_SIZE_MAX];
    MP_ResponseMsg msgt;
    MP_ResponseType type;
    int ret = -1;
    int i;
    
    memset((void*)&msg,0,sizeof(MP_CommandMsg));

    msg.head.type = MP_CMD_GETALLMEDIAID;

    msg.auxDatLen = 0;
    msg.auxDat = NULL;

    ret = MP_pack_command_msg(&msg,packet,&packetLen);
    if(ret!=0)
    {
        log_err("failed to pack msg.%s\n",__FUNCTION__);
        return -1;
    }
    ret = cli_send_message(packet,packetLen,packet,&packetLen,100);
    if(ret!=0)
    {
        log_err("failed to send msg.%s\n",__FUNCTION__);
        return -1;
    }
    ret = MP_unpack_response_msg(packet,packetLen,&msgt,&type);   
    if(ret ==0)
    {
         if(type == MP_RESP_VALIDMEDIAID)
         {
             mediaidpool->num = ((MP_ValidMediaIDInfo*)msgt.auxDat)->num;
             log_con("total current active  id counts:%d\n",mediaidpool->num);
             for(i = 0;i<((MP_ValidMediaIDInfo*)msgt.auxDat)->num;i++)
             {
                int instid = -1;
                log_con("======||%d'st player task id:%d\n",i,((MP_ValidMediaIDInfo*)msgt.auxDat)->media_id[i]);
                instid = cli_get_mplayer_inst_id(((MP_ValidMediaIDInfo*)msgt.auxDat)->media_id[i]);
                if(instid < 0)
                {
                    instid =  cli_get_request_seq_no();
                    ret = cli_bind_inst_with_amplayer(instid,((MP_ValidMediaIDInfo*)msgt.auxDat)->media_id[i]);

                    log_con("======||%d'st mplayer instance id:%d\n",i,instid);
                    
                    mediaidpool->media_id[i] = instid;
                    
                }
                else
                {
                    printf("======||%d'st mplayer instance id:%d\n",i,instid);
                    mediaidpool->media_id[i] = instid;
                }
                
             }            
            

            if(msgt.auxDat)
              free(msgt.auxDat);
         }

    }
    
    
    return 0;
    
}

