
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "player_ctrl.h"
#include "player_error.h"
#include "controler.h"

#include "msgmngr_svr.h"
#include "mp_types.h"
#include "mp_utils.h"

#include "mp_log.h"
#include "mp_events.h"

#define MSG_LEN_MAX 1024*4          // 4k

#define PID_RANDOM_NUMBER 28                 //a random number for internal pid.

#define ERRMSG_LEN_MAX 128

#define PLAYER_HELPER_PID 0xff
#define VOL_HELPER_PID -100

#define LISTPID_HELPER_PID -101

#define PLAYER_ERROR_PID -3

static int socket2_controler_init(global_ctrl_para_t *ctrl)
{
	
    init_svr_msg_mngr(NULL);
    
    log_con("[%s] Socket2 controler init\n",__FUNCTION__);
    
    return 0;
}


static MP_PlayBackState _map_internal__status_to_socket2(player_status inStatus)
{
    MP_PlayBackState state;
    switch(inStatus)
    {
         case PLAYER_STOPED:
                state = MP_STATE_STOPED;                     
                break;
    	    case PLAYER_RUNNING:
                state = MP_STATE_PLAYING;               
                break;
            case PLAYER_PAUSE:
                state = MP_STATE_PAUSED;               
                break;
            case PLAYER_SEARCHING:
                state = MP_STATE_SEARCHING;                
                break;      
            case PLAYER_INITING:
                state = MP_STATE_INITING;               
                break;
            case PLAYER_ERROR: 
                state = MP_STATE_NORMALERROR;                    
                break; 
            case PLAYER_PLAYEND:
                state = MP_STATE_FINISHED;                
                break;
            case PLAYER_START:
                state = MP_STATE_STARTED;               
                break;
            case  PLAYER_SEARCHOK:
                state = MP_STATE_MESSAGE_BOD;               
                break;    
            case PLAYER_BUFFERING:
                state = MP_STATE_BUFFERING;
                break;
            case PLAYER_INITOK:
                state = MP_STATE_PARSERED;
                break;
            default:  
                state = MP_STATE_UNKNOWN;                
                break;
    }

    return state;
    
}
static int _map_internal_errno_to_msg(int error,MP_ExtMsgCode* msg)
{  
    if(NULL == msg)
    {
           log_err("NULL extend pointer for err msg\n");
           return -1;
    }  
    msg->type = MP_SERV_SELF;
    msg->org_code = error;
    switch(error)
    {  
        case PLAYER_RD_FAILED:           
            msg->code = MP_PlayerErrSourceMediaData;            
            break;
        case PLAYER_RD_EMPTYP:  
            msg->code =MP_ErrSourceInit;            
            break;
        case PLAYER_RD_TIMEOUT:
            msg->code =MP_PlayerErrSourceMediaData;            
            break;
        case PLAYER_WR_FAILED:		
            //return "error:player write data error";
        case PLAYER_WR_EMPTYP:		
            //return "error:invalid pointer when writing";
        case PLAYER_WR_FINISH:		
            //return "error:player write finish";
        case PLAYER_PTS_ERROR:		
            //return "error:player pts error";
        case PLAYER_NO_DECODER:
            msg->code =MP_ErrMatchValidDecoder;            
            break;            
        case DECODER_RESET_FAILED:	
            msg->code =MP_ErrDecoderReset;             
            break;
        case DECODER_INIT_FAILED:
            msg->code =MP_ErrDecoderInit;                   
            break;
        case PLAYER_UNSUPPORT:  
            msg->code =MP_ErrNoSupportedTrack;             
            break;              
        case FFMPEG_OPEN_FAILED:	
            //return "error:can't open input file";
        case FFMPEG_PARSE_FAILED:	
            //return "error:parse file failed";
        case FFMPEG_EMP_POINTER:   
            //return "error:check invalid pointer";           
            msg->code =MP_ErrSourceInvalid;            
            break;
        case FFMPEG_NO_FILE:  
            msg->code =MP_ErrSourceInvalid;            
            break;            
        case PLAYER_SEEK_OVERSPILL:
            //return "error:seek time point overspill";
            msg->code =MP_ErrSeekOverPlayDuration;
            break;
        case PLAYER_CHECK_CODEC_ERROR:
            //return "error:check codec status error";
            msg->code =MP_ErrMatchValidDecoder;           
            break;
        case PLAYER_UNSUPPORT_VIDEO:
            msg->code = MP_ErrUnsupportedVideoCodec;
            break;
        case PLAYER_UNSUPPORT_AUDIO:
            msg->code = MP_ErrUnsupportedAudioCodec;
            break;    
        default:
            msg->code = MP_ErrUnknow;
            break;            
    }
    return 0;
}
static int socket2_get_command(player_cmd_t *playercmd)
{
    unsigned char buf[MSG_LEN_MAX];
    int len = 0;
    int timeo = 10;//10ms
    int ret  = -1;
    MP_CommandMsg cmd;
    MP_CommandType cmdtype;

    
    memset(buf,0,MSG_LEN_MAX);

    memset((void*)&cmd,0,sizeof(MP_CommandMsg));  
    
    ret = svr_recv_message(&buf,&len,timeo);
    
    if(ret == 0 &&len >0)
    {
        ret = MP_unpack_command_msg(buf,len,&cmd,&cmdtype);   
        if(ret == 0)
        {            
            switch(cmdtype)
            {
                
                case MP_CMD_PLAYFILE: 
                    
                    if(((MP_PlayFileMsgBody*)cmd.auxDat)->isStart >0)
                    {
                        if(NULL !=((MP_PlayFileMsgBody*)cmd.auxDat)->url &&strlen(((MP_PlayFileMsgBody*)cmd.auxDat)->url)>1)
                        {                            
                            playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);                  
                            
                            playercmd->cid += (cmd.head.req_seq_no&0x000000ff);                     
                            playercmd->filename = strdup(((MP_PlayFileMsgBody*)cmd.auxDat)->url);
                            log_trc("[%s]get a file:%s,size is :%d,sync id:%d,client id:%d,cid:%d\n",__FUNCTION__,playercmd->filename,strlen(playercmd->filename)+1,cmd.head.req_seq_no,cmd.head.client_id,playercmd->cid);                            
                            playercmd->ctrl_cmd = CMD_PLAY_START;
                            playercmd->pid = PLAYER_HELPER_PID;
                            switch(((MP_PlayFileMsgBody*)cmd.auxDat)->plmode)
                            {
                                case  MP_PLAY_ALL:
                                    log_trc("_________________video+audio\n");
                                    break;
                                case MP_PLAY_NO_AUDIO:
                                    log_trc("_________________no audio\n");
                                    playercmd->set_mode = CMD_NOAUDIO;                                
                                    break;
                                case MP_PLAY_NO_VIDEO:
                                    log_trc("............................no video\n");
                                    playercmd->set_mode = CMD_NOVIDEO;                                
                                    break;
                                default:
                                    break;

                            }   
                            if(((MP_PlayFileMsgBody*)cmd.auxDat)->isRepeat)
                            {
                                log_trc(".................set loop mode\n");
                                playercmd->set_mode +=CMD_LOOP;   
                            }
                            else
                            {
                                log_trc("__________set noloop mode\n");
                                playercmd->set_mode += CMD_NOLOOP;
                            }
                           
                            free(((MP_PlayFileMsgBody*)cmd.auxDat)->url);

                        }
                        else
                            log_err("Input a invlid file,please check it.\n");
                    }
                    else
                    {
                        if(((MP_PlayFileMsgBody*)cmd.auxDat)->url&&strlen(((MP_PlayFileMsgBody*)cmd.auxDat)->url)>1)
                        {
                            playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                            playercmd->cid += (cmd.head.req_seq_no&0x000000ff);
                            playercmd->pid = PLAYER_HELPER_PID;
                            playercmd->filename = strdup(((MP_PlayFileMsgBody*)cmd.auxDat)->url);
                            log_trc("[%s]Set media source,get a file:%s,size is %d\n",__FUNCTION__,playercmd->filename,strlen(playercmd->filename)+1);                            
                            playercmd->ctrl_cmd = CMD_PLAY;   
                            free(((MP_PlayFileMsgBody*)cmd.auxDat)->url);

                        }
                        else
                            log_err("Input a invlid file,please check it.\n");                    

                        
                    }
                        
                     if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);                     
                    
                    break;
                case MP_CMD_PLAYLIST:
                    break;
                case MP_CMD_ADDSOURCE:
                    if(NULL != cmd.auxDat && cmd.auxDatLen > 0)
                    {
                        playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                        playercmd->cid += (cmd.head.req_seq_no&0x000000ff);  
                        playercmd->filename = strdup((char*)cmd.auxDat);   
                        playercmd->pid = PLAYER_HELPER_PID;
                        log_trc("[%s]Set media source,get a file:%s\n",__FUNCTION__,playercmd->filename); 
                        playercmd->ctrl_cmd = CMD_PLAY; 
                        free(cmd.auxDat);                        
                    }
                    else
                        log_err("[MP_CMD_ADDSOURCE]:Input a invlid file,please check it.\n");     
                    break;
                case MP_CMD_QUITPLAYER:                    
                    playercmd->ctrl_cmd = CMD_EXIT;
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                    if(*(int*)cmd.auxDat >=PID_RANDOM_NUMBER)
                        playercmd->pid = *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid =0;
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                   
                    log_info("quit amplayer,ctrl cmd:%d,pid:%d\n",playercmd->ctrl_cmd,playercmd->pid);
                    break;
                case MP_CMD_PAUSE:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                    playercmd->ctrl_cmd = CMD_PAUSE;
                     if(*(int*)cmd.auxDat >=PID_RANDOM_NUMBER)
                        playercmd->pid = *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid =0;                    
                    
                     if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                   
                    break;
                case MP_CMD_RESUME:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                    playercmd->ctrl_cmd = CMD_RESUME;    
                     if(*(int*)cmd.auxDat >=PID_RANDOM_NUMBER)
                        playercmd->pid = *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid =0;
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                    
                    break;
                    
                case MP_CMD_STOP:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);     
                    playercmd->ctrl_cmd = CMD_STOP;
                     if(*(int*)cmd.auxDat >=PID_RANDOM_NUMBER)
                        playercmd->pid = *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid =0;
                    
                    log_info("receive a stop command,pid:%d\n",playercmd->pid);
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);                       
                   
                    break;
                case MP_CMD_FASTFORWARD:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);                          
                    playercmd->ctrl_cmd = CMD_FF;
                     if(((MP_TrickPlayMsgBody*)cmd.auxDat)->mediaID>=PID_RANDOM_NUMBER)                        
                        playercmd->pid = ((MP_TrickPlayMsgBody*)cmd.auxDat)->mediaID - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid =0;                    
                    
                    playercmd->param =  ((MP_TrickPlayMsgBody*)cmd.auxDat)->speed;
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                   
                    break;
                case MP_CMD_FASTREWIND:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                    playercmd->ctrl_cmd = CMD_FB;   
                     if(((MP_TrickPlayMsgBody*)cmd.auxDat)->mediaID>=PID_RANDOM_NUMBER)  
                        playercmd->pid = ((MP_TrickPlayMsgBody*)cmd.auxDat)->mediaID- PID_RANDOM_NUMBER;
                     else
                        playercmd->pid = 0;
                    playercmd->param =  ((MP_TrickPlayMsgBody*)cmd.auxDat)->speed;
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                    break;
                case MP_CMD_SEEK:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16); 
                    playercmd->ctrl_cmd = CMD_SEARCH;     
                     if(((MP_SeekMsgBody*)cmd.auxDat)->mediaID >=PID_RANDOM_NUMBER)  
                        playercmd->pid = ((MP_SeekMsgBody*)cmd.auxDat)->mediaID - PID_RANDOM_NUMBER;
                     else
                        playercmd->pid =0;
                     
                    playercmd->param = ((MP_SeekMsgBody*)cmd.auxDat)->searchPos;  
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                    
                    break;
                case MP_CMD_MUTE:                    
                    if(((MP_MuteMsgBody*)cmd.auxDat)->isMute)
                    {
                        log_trc("===set mute on\n");   
                        playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                        playercmd->set_mode = CMD_MUTE;
                   
                        playercmd->pid = VOL_HELPER_PID;
                        
                        
                    }
                    else
                    {
                        playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);      
                        log_trc("===set mute off\n");
                        playercmd->set_mode = CMD_UNMUTE;                        
                        playercmd->pid = VOL_HELPER_PID;
                       
                    }
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                    break;
                case MP_CMD_REPEAT:
                    log_info("set loop play option,is loop? %s,player pid:%d\n",((MP_RepeatMsgBody*)cmd.auxDat)->isRepeat>0?"YES!":"NO!",((MP_RepeatMsgBody*)cmd.auxDat)->mediaID- PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    if(((MP_RepeatMsgBody*)cmd.auxDat)->isRepeat)
                        playercmd->set_mode = CMD_LOOP;
                    else
                        playercmd->set_mode = CMD_NOLOOP;     
                    if(((MP_RepeatMsgBody*)cmd.auxDat)->mediaID>= PID_RANDOM_NUMBER)
                        playercmd->pid = ((MP_RepeatMsgBody*)cmd.auxDat)->mediaID- PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    playercmd->cid = MP_CMD_REPEAT;
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                    break;
                case MP_CMD_BLACKOUT:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    if(((MP_BlackoutMsgBody*)cmd.auxDat)->isBlackout)
                        playercmd->set_mode = CMD_BLACKOUT;   
                    else
                        playercmd->set_mode = CMD_NOBLACK;     
                    if(((MP_BlackoutMsgBody*)cmd.auxDat)->mediaID - PID_RANDOM_NUMBER)
                        playercmd->pid = ((MP_BlackoutMsgBody*)cmd.auxDat)->mediaID - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                       free(cmd.auxDat);
                    break;
                case MP_CMD_START:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->ctrl_cmd = CMD_START;
                    if(*(int*)cmd.auxDat >= PID_RANDOM_NUMBER)
                        playercmd->pid = *(int*)cmd.auxDat - PID_RANDOM_NUMBER;  
                    else
                        playercmd->pid = 0;
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);                      
                    break;
                case MP_CMD_SETVOLUME:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->set_mode = CMD_SET_VOLUME;
                    playercmd->param = ((MP_SetVolumeMsgBody*)cmd.auxDat)->vol; 
                 
                    playercmd->pid = VOL_HELPER_PID;  
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    
                    break;
                case MP_CMD_SETTONE:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    if((((MP_SetToneMsgBody*)cmd.auxDat)->mediaID) == VOL_HELPER_PID)
                        playercmd->pid =  (((MP_SetToneMsgBody*)cmd.auxDat)->mediaID);
                    else
                        playercmd->pid = VOL_HELPER_PID;
                    switch(((MP_SetToneMsgBody*)cmd.auxDat)->tone)
                    {
                        case MP_TONE_STEREO:
                            playercmd->set_mode = CMD_SET_STEREO;  
                            break;
                        case MP_TONE_LEFTMONO:
                            playercmd->set_mode = CMD_LEFT_MONO;  
                            break;
                        case MP_TONE_RIGHTMONO:
                            playercmd->set_mode = CMD_RIGHT_MONO;                                               
                            break;
                        case MP_TONE_SWAP:
                            playercmd->set_mode = CMD_SWAP_LR;                                                    
                            break;
                        default:
                            log_err("[%s]Never see this line\n",__FUNCTION__);
                            break;

                    }
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_SETOPTION:
                    break;
                case MP_CMD_SETAUDIOTRACK:
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    log_trc("[%s]: Set audio track pid:%d\n",__FUNCTION__,((MP_SetAudioTrackMsgBody*)cmd.auxDat)->track_pid);
                    playercmd->ctrl_cmd = CMD_SWITCH_AID;
                    playercmd->param = ((MP_SetAudioTrackMsgBody*)cmd.auxDat)->track_pid;     
                    if( (((MP_SetAudioTrackMsgBody*)cmd.auxDat)->mediaID)>= PID_RANDOM_NUMBER)
                        playercmd->pid =  (((MP_SetAudioTrackMsgBody*)cmd.auxDat)->mediaID) - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    
                   
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_SETSUBTITLE:
                     playercmd->cid =((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                     playercmd->ctrl_cmd = CMD_SWITCH_SID;
                     playercmd->param = ((MP_SetSubtitleMsgBody*)cmd.auxDat)->sub_pid;  
                        
                     log_trc("[%s]: Get subtitle pid:%d\n",__FUNCTION__,((MP_SetSubtitleMsgBody*)cmd.auxDat)->sub_pid);
                     if( (((MP_SetSubtitleMsgBody*)cmd.auxDat)->mediaID)>= PID_RANDOM_NUMBER)
                        playercmd->pid =  (((MP_SetSubtitleMsgBody*)cmd.auxDat)->mediaID) - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                     if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                     break;
                case MP_CMD_SETSONGSPECTRUM:    
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->param1 = ((MP_SetSpectrumMsgBody*)cmd.auxDat)->interval; 
                    playercmd->param = ((MP_SetSpectrumMsgBody*)cmd.auxDat)->isStartPop;
                    log_trc("set song spectral switch,isStart? %s,pop interval:%d\n",playercmd->param >0?"YES":"NO",playercmd->param1);
                    if((((MP_SetSpectrumMsgBody*)cmd.auxDat)->mediaID) >= PID_RANDOM_NUMBER)
                        playercmd->pid =  (((MP_SetSpectrumMsgBody*)cmd.auxDat)->mediaID) - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    playercmd->set_mode = CMD_SPECTRUM_SWITCH;
                   
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);      
                    break;
                case MP_CMD_GETVOLUME:    
                    log_trc("get volume[player pid:%d]\n",*(int*)cmd.auxDat);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->info_cmd = CMD_GET_VOLUME;
                   
                    playercmd->pid = VOL_HELPER_PID;                          
                   
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);                    
                    break;
                case MP_CMD_QUERY:
                    break;
                case MP_CMD_GETAUDIOTRICK:
                    log_trc("get audio track[player pid:%d]\n",*(int*)cmd.auxDat -PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    if(NULL != ((MP_GetInfoMsgBody*)cmd.auxDat)->url)
                    {
                        log_trc("get file url:%s\n",((MP_GetInfoMsgBody*)cmd.auxDat)->url);
                        free(((MP_GetInfoMsgBody*)cmd.auxDat)->url);
                    }

                    if(*(int*)cmd.auxDat >= PID_RANDOM_NUMBER)
                        playercmd->pid =   *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid =0;
                    
                    playercmd->info_cmd = CMD_GET_MEDIA_INFO;
                   
                     if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_GETSUBTITLE:
                    log_trc("get subtitle [media id:%d]\n",*(int*)cmd.auxDat -PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    if(NULL != ((MP_GetInfoMsgBody*)cmd.auxDat)->url)
                    {
                        log_trc("get file url:%s\n",((MP_GetInfoMsgBody*)cmd.auxDat)->url);
                        free(((MP_GetInfoMsgBody*)cmd.auxDat)->url);
                    }
                    if(*(int*)cmd.auxDat>= PID_RANDOM_NUMBER)
                        playercmd->pid =   *(int*)cmd.auxDat - PID_RANDOM_NUMBER;  
                    else
                        playercmd->pid = 0;
                    playercmd->info_cmd = CMD_GET_MEDIA_INFO;
                    
                     if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);                  
                    
                    break;
                case MP_CMD_GETPOSITION:                   
                    log_trc("get position[media id:%d]\n",*(int*)cmd.auxDat -PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->info_cmd = CMD_GET_CURTIME;
                    if( *(int*)cmd.auxDat >= PID_RANDOM_NUMBER)
                        playercmd->pid =   *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_GETDURATION:
                    log_trc("get duration[media id:%d]\n",*(int*)cmd.auxDat -PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->info_cmd = CMD_GET_DURATION;

                    if(*(int*)cmd.auxDat>= PID_RANDOM_NUMBER)
                        playercmd->pid =   *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_GETMEDIAINFO:                  
                    log_trc("get media info[player  pid:%d]\n",*(int*)cmd.auxDat -PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    if(NULL != ((MP_GetInfoMsgBody*)cmd.auxDat)->url)
                    {
                        log_trc("get file url:%s\n",((MP_GetInfoMsgBody*)cmd.auxDat)->url);
                        free(((MP_GetInfoMsgBody*)cmd.auxDat)->url);
                    }

                    if(*(int*)cmd.auxDat >= PID_RANDOM_NUMBER) 
                        playercmd->pid =   *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    
                    playercmd->info_cmd = CMD_GET_MEDIA_INFO;
                   
                    log_trc("player cmd :pid :%d,info cmd:%d,cid:%d\n",playercmd->pid,playercmd->info_cmd,playercmd->cid);
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_GETSTATUS:
                    log_trc("get status[media id:%d]\n",*(int*)cmd.auxDat -PID_RANDOM_NUMBER);
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->info_cmd = CMD_GET_PLAY_STA;
                    if(*(int*)cmd.auxDat >= PID_RANDOM_NUMBER)
                        playercmd->pid =   *(int*)cmd.auxDat - PID_RANDOM_NUMBER;
                    else
                        playercmd->pid = 0;
                    
                    if(cmd.auxDat != NULL&&cmd.auxDatLen > 0)
                        free(cmd.auxDat);
                    break;
                case MP_CMD_GETALLMEDIAID:
                    log_trc("get valids pid list\n");
                    playercmd->info_cmd = CMD_LIST_PID;
                    playercmd->cid = ((cmdtype&0x000000ff)<<8)+((cmd.head.client_id&0x0000ffff )<<16);
                    playercmd->pid = LISTPID_HELPER_PID;
                    break;
                default:
                    log_err("[%s]Never see this line\n",__FUNCTION__);
                    break;
                
            }
        }

        log_info("==================get a command from socket2===============\n");

        return 0;
        
    }
   playercmd = NULL;//just for controler logic.
   return -1;
}

static int socket2_update_state(int pid,player_info_t *player_state)
{   

    int ret = -1;
    
    MP_ResponseMsg resp;    
    MP_StateChangedRespBody body;
    unsigned char buf[MSG_LEN_MAX];    
    MP_ExtMsgCode errMsg;
    int len = -1;
    
    if(NULL == player_state )
    {
        log_err("[%s]Must check update failed\n",__FUNCTION__);
        return ret;
    }
    memset(&resp,0,sizeof(MP_ResponseMsg));
    
#if 1
    if(player_state->current_time>0&&player_state->current_time<=player_state->full_time){
        resp.head.type = MP_RESP_TIME_CHANGED;
        resp.auxDat = &player_state->current_time;
        resp.mediaID = pid+ PID_RANDOM_NUMBER;
        resp.auxDatLen = 4;   
        memset(buf,0,MSG_LEN_MAX);
        ret = MP_pack_response_msg(&resp,buf,&len);
        if(ret != 0)
        {
            log_err("failed to pack response msg\n");
            return -1;
        }    
        log_con("[%s]:time changed:%d\n",__FUNCTION__,player_state->current_time);
        ret = svr_post_message(&buf,len); 

        memset(&resp,0,sizeof(MP_ResponseMsg));

    }
#endif    

    //update state
    memset(&body,0,sizeof(MP_StateChangedRespBody));   

    resp.head.type = MP_RESP_STATE_CHANGED;
    
    player_status status = player_state->status;

    if(player_state->last_sta != player_state->status||player_state->error_no < 0)
    {   
        switch(status)
        {      
            
            case PLAYER_STOPED:
                body.state = MP_STATE_STOPED;
                resp.mediaID = pid +PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4;            
                break;
    	    case PLAYER_RUNNING:
                body.state = MP_STATE_PLAYING;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen =  sizeof(MP_StateChangedRespBody) -4;
                break;
            case PLAYER_PAUSE:
                body.state = MP_STATE_PAUSED;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen =  sizeof(MP_StateChangedRespBody) -4;
                break;
            case PLAYER_SEARCHING:
                body.state = MP_STATE_SEARCHING;
                resp.mediaID = pid+PID_RANDOM_NUMBER;

                resp.auxDatLen =  sizeof(MP_StateChangedRespBody) -4;
                break;      
            case PLAYER_INITING:               

                if(player_state->error_no < 0)
                {                    
                    memset((void*)&errMsg,0,sizeof(MP_ExtMsgCode));                   
                    body.state = MP_STATE_NORMALERROR;
                    _map_internal_errno_to_msg(player_state->error_no,&errMsg);                  
                    body.stateEx = &errMsg;     
                    resp.mediaID = pid+PID_RANDOM_NUMBER;
                    resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4 + sizeof(MP_ExtMsgCode);
                }
                else
                {              
                
                    body.state = MP_STATE_INITING;  
                    resp.mediaID = pid+PID_RANDOM_NUMBER;
                    resp.auxDatLen =  sizeof(MP_StateChangedRespBody) -4;
                }
                break;
            case PLAYER_ERROR:
            {
                
                if(player_state->error_no < 0)
                {   
                    
                    memset((void*)&errMsg,0,sizeof(MP_ExtMsgCode));                   
                    body.state = MP_STATE_FATALERROR;
                   _map_internal_errno_to_msg(player_state->error_no,&errMsg);                  
                   body.stateEx = &errMsg;                             
                }
               
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4 + sizeof(MP_ExtMsgCode);
                break;                
                
            }
            case PLAYER_BUFFERING:
                body.state = MP_STATE_BUFFERING;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4;
                break;
            case PLAYER_PLAYEND:
                body.state = MP_STATE_FINISHED;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4;
                break;
            case PLAYER_START:
                body.state = MP_STATE_STARTED;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4; 
                break;
            case  PLAYER_SEARCHOK:
                body.state = MP_STATE_TRICKTOPLAY;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4; 
                break;  
            case  PLAYER_FB_END:
                body.state = MP_STATE_MESSAGE_BOD;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4; 
                break;     
            case  PLAYER_FF_END:
                body.state = MP_STATE_MESSAGE_EOD;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4;    
                break;
            case  PLAYER_INITOK:
                body.state = MP_STATE_PARSERED;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4;       
                break;
            default:  
                body.state = MP_STATE_UNKNOWN;
                resp.mediaID = pid+PID_RANDOM_NUMBER;
                errMsg.type = MP_SERV_SELF;
                errMsg.code = MP_ErrUnknow;
                body.stateEx = &errMsg;
                resp.auxDatLen = sizeof(MP_StateChangedRespBody) -4 + sizeof(MP_ExtMsgCode);
                break;
        }   
         
        resp.auxDat = &body;
        memset(buf,0,MSG_LEN_MAX);
        ret = MP_pack_response_msg(&resp,buf,&len);
        if(ret != 0)
        {
            log_err("failed to pack response msg\n");
            return -1;
        }    
       
        ret = svr_post_message(&buf,len);   
        
        log_con("===========pid[%d],update player status:%d[%d]========\n",pid,body.state,player_state->status);
         
    }  
    
    return ret;

}

static int socket2_ret_info(int pid, int cid, int type, void* pData)
{
    int ret = -1;
    
    MP_ResponseMsg resp;    
    MP_StateChangedRespBody body;
    unsigned char buf[MSG_LEN_MAX];
    MP_SubTitleRespBody subtBody;
    MP_AudioTrackRespBody trackBody;
    MP_AVMediaFileInfo avbody;
    MP_AudioTagInfo taginfo;
    media_info_t* _av_info = NULL; 
    int index = 0;    
    int len = -1;
    int ctype =-1;
    int tmp_value = -1;
    
    memset(&resp,0,sizeof(MP_ResponseMsg));
    resp.head.client_id = ((cid>>16)&0x0000ffff); 
    log_info("asyn response msg,pid:%d,cid:%d,type:%d,client id:%d\n",pid,cid,type, resp.head.client_id);

    
    switch(type)
    {   
        case CTRL_CMD_RESPONSE:
            resp.head.type = MP_RESP_COMMAND;                
            resp.head.req_seq_no = (cid&0x000000ff);    
            if(pid ==PLAYER_NOT_VALID_PID)
            {
                 resp.mediaID = PLAYER_ERROR_PID;   
            }
            else
            {
                 resp.mediaID = pid+ PID_RANDOM_NUMBER; 
            }
           
            log_info("mplayer response instance id:%d,pid:%d\n",resp.head.req_seq_no,pid);
            resp.auxDat = NULL;
            resp.auxDatLen = 0;
            break;
        case CMD_GET_VOLUME:            
            resp.head.type = MP_RESP_VOLUME;                
            tmp_value = *(int*)pData;
            resp.auxDat = &tmp_value;
            resp.mediaID = VOL_HELPER_PID;
            resp.auxDatLen = 4;
            log_con("return volume valume:%d\n",*(int*)pData);
            break;
       case CMD_GET_VOL_RANGE:            
            return ret;
       case CMD_GET_PLAY_STA:
            memset(&body,0,sizeof(MP_StateChangedRespBody));
            resp.head.type = MP_RESP_STATE_CHANGED;     
            resp.mediaID = pid+ PID_RANDOM_NUMBER;   
            body.state = _map_internal__status_to_socket2(*(int*)pData);
            resp.auxDatLen = sizeof(MP_StateChangedRespBody) - 4;
            resp.auxDat = &body;   
            
            break;
       case CMD_GET_CURTIME:
            resp.head.type = MP_RESP_TIME_CHANGED;
            tmp_value = *(int*)pData;
            resp.auxDat = &tmp_value;
            resp.mediaID = pid+ PID_RANDOM_NUMBER;
            resp.auxDatLen = 4;   
            log_con("return current time:%d\n",*(int*)pData);
            break;
       case CMD_GET_DURATION:
            resp.head.type = MP_RESP_DURATION;
            tmp_value = *(int*)pData;
            resp.auxDat = &tmp_value;
            resp.mediaID = pid+ PID_RANDOM_NUMBER;
            resp.auxDatLen = 4;  
            log_con("return duration time:%d\n",*(int*)pData);
            break;
       case  CMD_LIST_PID:           
            {
                MP_ValidMediaIDInfo pids;
                pid_info_t* playerid = (pid_info_t *)pData;
                pids.num = playerid->num;
                for(index = 0;index < _MEDIA_ID_NUM_MAX;index++)
                {
                    pids.media_id[index] = playerid->pid[index]+PID_RANDOM_NUMBER;
                }
                resp.head.type = MP_RESP_VALIDMEDIAID;              
                resp.auxDat= &pids;                   
                resp.mediaID = LISTPID_HELPER_PID;
                resp.auxDatLen = sizeof(MP_ValidMediaIDInfo);
            }
            break;
       case CMD_GET_MEDIA_INFO:
            _av_info = (media_info_t*)pData;
            ctype = ((cid>>8)&0x000000ff);            
            if(ctype == MP_CMD_GETMEDIAINFO)
            {
                resp.auxDatLen = sizeof(MP_AVMediaFileInfo) - 4;
                //memset((void*)&avbody,0,sizeof(MP_AVMediaFileInfo));
                avbody.size = _av_info->stream_info.file_size;
                avbody.has_internal_sub = _av_info->stream_info.has_sub;
                avbody.sub_counts = _av_info->stream_info.total_sub_num;
                avbody.has_vtrack = _av_info->stream_info.has_video;
                avbody.has_atrack = _av_info->stream_info.has_audio;
                avbody.atrack_counts = _av_info->stream_info.total_audio_num;
                 avbody.vtrack_counts = _av_info->stream_info.total_video_num; 
                //avbody.duration = _av_info->video_info.duration;
                avbody.duration = _av_info->stream_info.duration;
                avbody.filetype = _av_info->stream_info.type;

                for(index =0;index < avbody.vtrack_counts;index++)
                {
                    if( NULL != _av_info->video_info[index])
                    {
                        avbody.vinfo[index].video_w = _av_info->video_info[index]->width;
                        avbody.vinfo[index].video_h = _av_info->video_info[index]->height;
                        avbody.vinfo[index].bit_rate = _av_info->video_info[index]->bit_rate;                            
                        avbody.vinfo[index].video_format = _av_info->video_info[index]->format;

                    }
                }    
               
                if(NULL != _av_info->audio_info[0]&&NULL != _av_info->audio_info[0]->audio_tag)
                {
                    int slen = -1;
                    #define TAG_TMP_LEN 512
                    char tmp[TAG_TMP_LEN];                    
                    memset((void*)&taginfo,0,sizeof(MP_AudioTagInfo));
                    
                    avbody.has_tag = 1;
                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN);
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->title,(TITLE_LEN_MAX-1));
                    taginfo.title = strdup(tmp); 
                    slen = strlen( taginfo.title);
                    if(slen >0){   
                        taginfo.title_len = slen +1;     
                        avbody.taginfo_len +=taginfo.title_len;
                        log_con("tag's title,len:%d,tag len:%d\n",taginfo.title_len,avbody.taginfo_len);
                    }
                    else
                        taginfo.title = NULL;
                    
                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN);
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->album,(ALBUM_LEN_MAX-1));                    
                    taginfo.album = strdup(tmp);
                    slen = strlen(taginfo.album);
                    if(slen>0){                        
                        taginfo.album_len =slen+1;
                        avbody.taginfo_len +=taginfo.album_len;
                        log_con("tag's album,len:%d,tag_len:%d\n",taginfo.album_len,avbody.taginfo_len);
                    }
                    else
                        taginfo.album = NULL;
                    
                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN);
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->author,(AUTHOR_LEN_MAX-1));   
                    
                    taginfo.author = strdup(tmp);
                    slen = strlen(taginfo.author);
                    if(slen >0){                        
                        taginfo.author_len = slen+1;  
                        avbody.taginfo_len +=taginfo.author_len;
                        log_con("tag's author,len:%d,tag_len:%d\n",taginfo.author_len,avbody.taginfo_len);
                    }
                    else
                        taginfo.author = NULL;

                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN);      
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->comment,COMMENT_LEN_MAX-1);
                    taginfo.comment = strdup(tmp);
                    slen = strlen(taginfo.comment);
                    if(slen>0){                       
                        taginfo.comment_len = slen+1;
                        avbody.taginfo_len +=taginfo.comment_len;                        
                        log_con("tag's comment,len:%d,tag_len:%d\n",taginfo.comment_len,avbody.taginfo_len);                     
                    }
                    else
                        taginfo.comment= NULL;
                        
                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN); 
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->year,YEAR_LEN_MAX-1);
                    taginfo.year = strdup(tmp);
                    slen = strlen(taginfo.year);
                    if(slen>0){                       
                        taginfo.year_len = slen+1;
                        avbody.taginfo_len +=taginfo.year_len;                        
                        log_con("tag's year:%s,len:%d,tag_len:%d\n",taginfo.year,taginfo.year_len,avbody.taginfo_len);                     
                    }
                    else
                        taginfo.year = NULL;
                    
                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN); 
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->genre,GENRE_LEN_MAX-1);
                    taginfo.genre = strdup(tmp);
                    slen = strlen(taginfo.genre);
                    if(slen>0){
                        
                        taginfo.genre_len = slen+1;
                        avbody.taginfo_len +=taginfo.genre_len;                        
                        log_con("tag's genre,len:%d,tag_len:%d\n",taginfo.genre_len,avbody.taginfo_len);
                    }  
                    else
                        taginfo.genre = NULL;
                    
                    avbody.taginfo_len +=4;
                    memset(tmp,0,TAG_TMP_LEN); 
                    strncpy(tmp,_av_info->audio_info[0]->audio_tag->copyright,COPYRIGHT_LEN_MAX-1);
                    taginfo.copyright = strdup(tmp);
                    slen = strlen(taginfo.copyright);
                    if(slen>0){                        
                        taginfo.copyright_len = slen+1;
                        avbody.taginfo_len +=taginfo.copyright_len;                        
                        log_con("tag's copyright,len:%d,tag_len:%d\n",taginfo.copyright_len,avbody.taginfo_len);                      
                    }  
                    else
                        taginfo.copyright = NULL;

                    avbody.taginfo_len +=4;
                    taginfo.track = _av_info->audio_info[0]->audio_tag->track;
                    
                    avbody.taginfo = &taginfo;                    
                    resp.auxDatLen += avbody.taginfo_len;

                    
                }
                for(index = 0;index < avbody.atrack_counts;index ++)
                {
                    if(NULL != _av_info->audio_info[index])
                    {
                        avbody.ainfo[index].audio_channel = _av_info->audio_info[index]->channel;
                        avbody.ainfo[index].bit_rate = _av_info->audio_info[index]->bit_rate;
                        avbody.ainfo[index].audio_samplerate = _av_info->audio_info[index]->sample_rate;
                        avbody.ainfo[index].audio_format = _av_info->audio_info[index]->aformat;
                        avbody.ainfo[index].track_pid = _av_info->audio_info[index]->id;
                    }
                }                 
               
               	log_con("[%s]total sub counts:%d\n",__FUNCTION__,avbody.sub_counts);
                for(index = 0;index < avbody.sub_counts ;index ++)
                {
                    if(NULL != _av_info->sub_info[index])
                    {
                        //pid                        
                        avbody.item[index].sub_pid = _av_info->sub_info[index]->id;
                        log_con("subtitle pid:%d\n",_av_info->sub_info[index]->id);
                        avbody.item[index].width = _av_info->sub_info[index]->width;
                        avbody.item[index].height = _av_info->sub_info[index]->height;
                        avbody.item[index].resolution = _av_info->sub_info[index]->resolution;
                        avbody.item[index].subtitle_size = _av_info->sub_info[index]->subtitle_size;
                        avbody.item[index].sub_lang = _av_info->sub_info[index]->sub_language;
                        avbody.item[index].internal_external = _av_info->sub_info[index]->internal_external;
                        //format
                        //strncpy(subtBody.item[index].format,_av_info->sub_info[index].format,20);
                        //language  
                    }
                }                  
                
                resp.head.type = MP_RESP_MEDIAINFO;             
                resp.mediaID =  pid+ PID_RANDOM_NUMBER;              
                resp.auxDat = &avbody;
                
            }
            else if(ctype == MP_CMD_GETSUBTITLE)  
            {
                memset((void*)&subtBody,0,sizeof(MP_SubTitleRespBody));
                subtBody.subcount = _av_info->stream_info.total_sub_num;
                for(index = 0;index < subtBody.subcount;index ++)
                {
                    if(NULL != _av_info->sub_info[index])
                    {
                        //pid
                        subtBody.item[index].sub_pid = _av_info->sub_info[index]->id;
                        //format
                        //strncpy(subtBody.item[index].format,_av_info->sub_info[index].format,20);
                        //language  
                    }
                }  
                resp.head.type = MP_RESP_SUBTITLE;
                resp.mediaID = pid+ PID_RANDOM_NUMBER;   
                resp.auxDat = &subtBody;
                resp.auxDatLen = sizeof(MP_SubTitleRespBody);
                            
            }
            else if(ctype == MP_CMD_GETAUDIOTRICK)    
            {               
                trackBody.trackcount = _av_info->stream_info.total_audio_num;
                for(index = 0;index < trackBody.trackcount;index ++)
                {
                    if(NULL != _av_info->audio_info[index])
                    {   //pid                               
                        trackBody.item[index].track_pid = _av_info->audio_info[index]->id;
                        //type   
                        trackBody.item[index].type = _av_info->audio_info[index]->aformat; 

                    }   
                                                 
                }  
                resp.head.type = MP_RESP_AUDIOTRCK;
                resp.mediaID =pid+ PID_RANDOM_NUMBER;    
                resp.auxDat = &trackBody;
                resp.auxDatLen = sizeof(MP_AudioTrackRespBody);

            }
            break;
        default:
            log_err("[%s]:never see this case,impossible switch.\n",__FUNCTION__);
            return -1;
    }
    log_con("async return function:%s%d\n",__FUNCTION__,__LINE__);    
    
    memset(buf,0,MSG_LEN_MAX);
    
    MP_pack_response_msg(&resp,buf,&len);
    if(avbody.taginfo){
    	MP_free_taginfo(avbody.taginfo);
    	avbody.taginfo = NULL;    		
    }
    ret = svr_post_message(&buf,len);  

    return ret;
}

static int socket2_controler_release(global_ctrl_para_t *ctrl)
{
    log_info("release socket2 controler\n");    
    uninit_svr_msg_mngr();
    return 0;
}

player_controler_t socket2_ctrl=
{
    .name="socket2",
    .front_mode=0,
    .init=socket2_controler_init,
    .release=socket2_controler_release,
    .get_command=socket2_get_command,
    .update_state=socket2_update_state,
    .ret_info=socket2_ret_info,
};

int register_socket2_controler(void)
{	
    register_controler(&socket2_ctrl);
    return 0;
}


void __attribute__ ((constructor)) socket2_init(void);
void socket2_init(void )
{
    log_trc("register socket2 amplayer2 plugin\n");
    register_socket2_controler();	
	
}

void __attribute__ ((destructor)) socket2_exit(void);

void socket2_exit(void)
{
    log_trc(" unregister socket2 amplayer2 plugin\n");    
    //socket2_controler_release(NULL);
}



