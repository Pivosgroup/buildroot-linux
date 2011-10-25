//peter
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "mp_api.h"
#include "mp_api_sync.h"
#include "mp_types.h"
#include "mp_utils.h"
#include "mp_events.h"

#include <sys/time.h>
#ifndef WIN32
#include <termios.h>
#endif

#ifndef WIN32

#include "plist_mngr.h"

int mygetch(void)
{
	struct termios oldt,newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}
#endif

typedef struct MP_Context{
      char  url[1024];
      int isStart;
      int isLoop;
      int st;
      int kp_duration;
      MP_MediaPlayMode plmode;
      int media_id;
      int isMute;
      int vol;
      int isBlackout;
      int speed;
      MP_Tone tone;
      int isStartPop;
      int interval;
      int sub_pid;
      int track_pid;
      int isAsync;
      int test_counts;
}MP_Context_T;

static int timechanged_flag = 0; 
typedef enum{
      EMU_STEP_NONE = 0,
      EMU_STEP_PLAY  = 1,             
      EMU_STEP_PAUSE = 2,            
      EMU_STEP_RESUME =3,          
      EMU_STEP_START=4,    
      EMU_STEP_FF = 5,
      EMU_STEP_RR = 6,
      EMU_STEP_SEEK = 7,
      EMU_STEP_MUTE= 8,
      EMU_STEP_SETVOL=9,
      EMU_STEP_GETVOL = 10,
      EMU_STEP_SETTONE= 11,
      EMU_STEP_GETPOS = 12,
      EMU_STEP_GETDUR = 13,
      EMU_STEP_SETLOOP = 14,
      EMU_STEP_SETBLACKOUT = 15,    
      EMU_STEP_STOP = 16,
      EMU_STEP_SPECTRUM = 17,
      EMU_STEP_GETSTATUS = 18,
      EMU_STEP_SETSUBT = 19,
      EMU_STEP_MENU = 20, 
      EMU_STEP_EXIT = 21,      
      EMU_STEP_ATRACK = 22,
      EMU_STEP_GETATRACK = 23,
      EMU_STEP_GETSUBT = 24,
      EMU_STEP_GETAVMEDIAINFO = 25,   
      EMU_STEP_PAUSETORESUME = 26,
      EMU_STEP_LISTALLMEDIAID = 27,
      EMU_STEP_CLOSEMEDIAINST,
      EMU_STEP_QUITPLAYER = 29,
      EMU_STEP_PLAYSTOP,
      EMU_SETP_ADDFILETOPLIST,
      EMU_STEP_STARTPLIST,
      EMU_STEP_STOPPLIST,
      EMU_STEP_VIEWPLIST,
      EMU_STEP_PLAYNEXT,
      EMU_STEP_ADDDIRTOPLIST,
}EMU_STEP;

#define SCREEN_SPLITER            "***************************************************************************\r\n"

static int media_id = 0;

#define EMU_INPUT(tip,defavalue,buffer,buffersize) {\
		static char buff_input[2048];\
		printf(tip);\
		printf("[%s]\r\n",defavalue==NULL?"":defavalue);\
		memset(tmpcommand,0,sizeof(tmpcommand));\
		memset(buff_input,0,sizeof(buff_input));\
		gets(tmpcommand);\
		if(strcmp(tmpcommand,"q")==0){tmpneedexit=1;break;}\
		else if( (strcmp(tmpcommand,"y")!=0)&&(strcmp(tmpcommand,"")!=0))\
		{\
			memcpy(buff_input,tmpcommand,sizeof(tmpcommand));\
			snprintf(buffer,buffersize,"%s",buff_input);\
		}\
		else if(( strcmp(tmpcommand,"y")==0)||(strcmp(tmpcommand,"")==0))\
		{\
			memcpy(buff_input,defavalue,sizeof(defavalue));\
			snprintf(buffer,buffersize,"%s",buff_input);\
		}\
	}\

int _ext_code_map_to_msg_dump(MP_ExtMsgCode* ext)
{
    assert(NULL !=ext);

    if(ext->type == MP_SERV_SELF)
    {
        switch(ext->code)
        {
            case MP_PlayerErrSourceMediaData:
                printf("player read file error\n");
                break;
            case MP_ErrSourceInit:
                printf("invalid pointer when reading\n");
                break;
            case MP_ErrMatchValidDecoder:
                printf("can't find valid decoder\n");
                break;
            case MP_ErrDecoderReset:
                printf("decoder reset failed\n");
                break;
            case MP_ErrDecoderInit:
                printf("decoder init failed\n");
                break;
            case MP_ErrNoSupportedTrack:
                printf("player unsupport file type\n");
                break;
            case MP_ErrSourceInvalid:
                printf("can't open input file\n");
                break;            
            case MP_ErrSeekOverPlayDuration:   
                printf("seek time point overspill\n");
                break;            
            case  MP_ErrFatal:
                printf("player play failed\n");
                break;
            case MP_ErrUnsupportedAudioCodec:
                printf("player not support audio codec\n");
                break;
            case MP_ErrUnsupportedVideoCodec:
                printf("player not support video codec\n");
                break;
            case  MP_ErrUnknow:
                printf("unkown error code\n");
                break;                    
        }

    }

    return 0;

}
int _media_info_dump(MP_AVMediaFileInfo* minfo)
{
    int i = 0;
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    printf("======||file size:%lld\n",minfo->size);
    printf("======||file type:%d\n",minfo->filetype); 
    printf("======||has internal subtitle?:%s\n",minfo->has_internal_sub>0?"YES!":"NO!");
    printf("======||internal subtile counts:%d\n",minfo->sub_counts);
    printf("======||has video track?:%s\n",minfo->has_vtrack>0?"YES!":"NO!");
    printf("======||has audio track?:%s\n",minfo->has_atrack>0?"YES!":"NO!");    
     printf("======||has tag?:%s\n",minfo->has_tag>0?"YES!":"NO!");
   
    if(minfo->has_vtrack>0)
    {
        printf("======||video counts:%d\n",minfo->vtrack_counts);
        printf("======||duration:%d\n",minfo->duration);
        printf("======||video width:%d\n",minfo->vinfo[0].video_w);
        printf("======||video height:%d\n",minfo->vinfo[0].video_h);
        printf("======||video bitrate:%d\n",minfo->vinfo[0].bit_rate);
        printf("======||video format:%d\n",minfo->vinfo[0].video_format);

    }
    if(minfo->atrack_counts > 0)
    {
        printf("======||audio counts:%d\n",minfo->atrack_counts);
        if(minfo->has_tag)
        {
            if(NULL !=minfo->taginfo)
            {
                printf("======||track title:%s",minfo->taginfo->title!=NULL?minfo->taginfo->title:"unknow");   
                printf("\n======||track album:%s",minfo->taginfo->album!=NULL?minfo->taginfo->album:"unknow"); 
                printf("\n======||track author:%s\n",minfo->taginfo->author!=NULL?minfo->taginfo->author:"unknow");
                printf("\n======||track year:%s\n",minfo->taginfo->year!=NULL?minfo->taginfo->year:"unknow");
                printf("\n======||track comment:%s\n",minfo->taginfo->comment!=NULL?minfo->taginfo->comment:"unknow"); 
                printf("\n======||track genre:%s\n",minfo->taginfo->genre!=NULL?minfo->taginfo->genre:"unknow");
                printf("\n======||track copyright:%s\n",minfo->taginfo->copyright!=NULL?minfo->taginfo->copyright:"unknow");  
                printf("\n======||track track:%d\n",minfo->taginfo->track);  
            }
            

        }
        for(i = 0;i<minfo->atrack_counts;i++)
        {
            printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
            printf("======||%d 'st audio track pid:%d\n",i,minfo->ainfo[i].track_pid);
            printf("======||%d 'st audio track codec type:%d\n",i,minfo->ainfo[i].audio_format);
            printf("======||%d 'st audio track audio_channel:%d\n",i,minfo->ainfo[i].audio_channel);
            printf("======||%d 'st audio track bit_rate:%d\n",i,minfo->ainfo[i].bit_rate);
            printf("======||%d 'st audio track audio_samplerate:%d\n",i,minfo->ainfo[i].audio_samplerate);
            printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
            
        }

        for(i = 0;i<minfo->sub_counts;i++)
        {
            if(0 == minfo->item[i].internal_external){
                printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
                printf("======||%d 'st internal subtitle pid:%d\n",i,minfo->item[i].sub_pid);   
                printf("======||%d 'st internal subtitle language:%d\n",i,minfo->item[i].sub_lang); 
                printf("======||%d 'st internal subtitle width:%d\n",i,minfo->item[i].width); 
                printf("======||%d 'st internal subtitle height:%d\n",i,minfo->item[i].height); 
                printf("======||%d 'st internal subtitle resolution:%d\n",i,minfo->item[i].resolution); 
                printf("======||%d 'st internal subtitle subtitle size:%d\n",i,minfo->item[i].subtitle_size); 
                printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");       

            }

        }
            
            
    }
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    return 0;
}
int inform_msg_notify(void * obj,MP_ResponseMsg *msgt)
{    
    MP_ResponseType type;
    int i = 0;    
   
    if(msgt !=NULL)
    {
        type = msgt->head.type;
        if(type == MP_RESP_STATE_CHANGED)
        {
        	  printf("===========================playback response start============================\n");
              printf("=================media id:%d==================\n",msgt->mediaID);
              
              printf("====== current status code:%d\n",((MP_StateChangedRespBody*)msgt->auxDat)->state);

              switch(((MP_StateChangedRespBody*)msgt->auxDat)->state)
              {                  
              case MP_STATE_STOPED:
                  printf("====== current status:%s\n","player stop");
                  MP_InactiveMediaID(msgt->mediaID);//just for my playlist,not for you
                  MP_CloseMediaID(msgt->mediaID);
                  break;
              case MP_STATE_PLAYING:
                  printf("====== current status:%s\n","player running");
                  break;
              case MP_STATE_PAUSED:
                  printf("====== current status:%s\n","player paused");
                  break;              
              case MP_STATE_MESSAGE_BOD:
                  printf("====== current status:%s\n","player seeking to start position");
                  break;              
              case MP_STATE_BUFFERINGDONE:
                  printf("====== current status:%s\n","player buffering done");
                  break;
              case MP_STATE_INITING:
                  printf("====== current status:%s\n","player initing");
                  break;
              case MP_STATE_STARTED:
                  printf("====== current status:%s\n","player start");                 
                  break;
              case MP_STATE_NORMALERROR: 
                  printf("====== current status:%s\n","normal error");      
                  
                  break;
              case MP_STATE_FINISHED:
                  MP_InactiveMediaID(msgt->mediaID);
                  MP_CloseMediaID(msgt->mediaID);
                  printf("====== current status:%s\n","play end");   
                  break;
              case MP_STATE_FATALERROR:
                  MP_InactiveMediaID(msgt->mediaID);
                  MP_CloseMediaID(msgt->mediaID);
                  printf("====== current status:%s\n","fatal error");   
                  break;
              case MP_STATE_MESSAGE_EOD:
                  printf("====== current status:%s\n","no data or end of data");   
                  break;
              case MP_STATE_SEARCHING:
                  printf("====== current status:%s\n","player searching");   
                  break;        
             case MP_STATE_BUFFERING:
                  printf("====== current status:%s\n","player buffering");   
                  break;   
             case MP_STATE_PARSERED:     
                  MP_ActiveMediaID(msgt->mediaID);
                  printf("====== current status:%s\n","player media info ready after parsered");
                  break;
             case MP_STATE_TRICKTOPLAY:
                  printf("====== current status:%s\n","player state changed from trick to normal play");
                  break;                  
             default:
                  printf("never see this line\n");
                  break;
              }

              if(NULL != ((MP_StateChangedRespBody*)msgt->auxDat)->stateEx&& (unsigned int)(msgt->auxDatLen)> sizeof(MP_StateChangedRespBody) -4)
              {
                    printf("====== error code:%d\n",((MP_ExtMsgCode*)((MP_StateChangedRespBody*)msgt->auxDat)->stateEx)->code);
                    printf("====== error orginal code:%x\n",-(((MP_ExtMsgCode*)((MP_StateChangedRespBody*)msgt->auxDat)->stateEx)->org_code));
                    printf("====== error message:");
                    _ext_code_map_to_msg_dump((MP_ExtMsgCode*)((MP_StateChangedRespBody*)msgt->auxDat)->stateEx);
                   
              }

        	 printf("=============================playback response end============================\n");     
              
        }
        else if(type == MP_RESP_DURATION)
        {
			  printf("===========================playback response start============================\n");
              printf("====== play total time:%d\n",*(int*)msgt->auxDat); 
              printf("=============================playback response end============================\n");    
             
        }
        
        else if(type == MP_RESP_TIME_CHANGED)
        {
            if(timechanged_flag ==1){
            	printf("===========================playback response start============================\n");
                printf("======= play current postion:%d\n",*(int*)msgt->auxDat); 
                printf("=============================playback response end============================\n");  
                     
            }      
             

        }
        else if(type == MP_RESP_BUFFERING_PORGRESS)
        {
        	printf("===========================playback response start============================\n");
            printf("======== play buffering percentage:%d%%\n",*(int*)msgt->auxDat);  
            printf("=============================playback response end============================\n");           

        }
        else if(type == MP_RESP_VOLUME)
        {
        	printf("===========================playback response start============================\n");
            printf("======== play volume:%d\n",*(int*)msgt->auxDat);
          	printf("=============================playback response end============================\n");
        
        }	
        else if(type == MP_RESP_MEDIAINFO)
        {  
        	printf("===========================playback response start============================\n");         
            _media_info_dump(msgt->auxDat);
            MP_InactiveMediaID(msgt->mediaID);
            printf("=============================playback response end============================\n");
           
        }
        else if(type == MP_RESP_SUBTITLE)
        {
        	printf("===========================playback response start============================\n");
            for(i = 0;i< ((MP_SubTitleRespBody*)msgt->auxDat)->subcount;i++)
            {
                printf("======||%d 'st internal subtitle pid:%d\n",i,((MP_SubTitleRespBody*)msgt->auxDat)->item[i].sub_pid);
                //printf("======||%d 'st internal subtitle format:%s\n",i,((MP_SubTitleRespBody*)msgt->auxDat)->item[i].format);
            }
            printf("=============================playback response end============================\n");
            
        }
        else if(type == MP_RESP_AUDIOTRCK)
        {
        	printf("===========================playback response start============================\n");
            for(i = 0;i< ((MP_AudioTrackRespBody*)msgt->auxDat)->trackcount;i++)
            {
                printf("======||%d 'st audio track pid:%d\n",i,((MP_AudioTrackRespBody*)msgt->auxDat)->item[i].track_pid);
            }
            printf("=============================playback response end============================\n");
           
        }   
        MP_free_response_msg(msgt);    

    }
    
    

    return 0;

}

int mp_run_tool()
{
    int tmpneedexit=0;
    int ret = -1;
    char tmpcommand[2048];
    MP_Context_T context;
    EMU_STEP tmpstep = EMU_STEP_MENU;
    int index = 0;

    memset((void*)&context,0,sizeof(MP_Context_T));
    
    ret = MP_CreateMPClient();
   
    pGetRespMsgCallback foo = &inform_msg_notify;    

    MP_RegPlayerRespMsgListener(NULL,foo);
    
    strcpy(context.url,"/home/nba.rmvb");
    
    while (!tmpneedexit) {     
        memset(tmpcommand,0,sizeof(tmpcommand));

        switch (tmpstep) {           
        case EMU_STEP_PLAY:               
            do	{
            		memset(context.url,0,1024);
	              EMU_INPUT("Please Input play url[default:'y']:","/home/nba.rmvb",context.url,1024);	              	
	              EMU_INPUT("Please Input start option[1/0]","0",tmpcommand,sizeof(tmpcommand));
	              context.isStart = atoi(tmpcommand);	              
	              if(context.isStart==0)
	              {
	              	ret = MP_AddMediaSource(context.url);
                           MP_ActiveMediaID(ret);
                           printf("get mplayer mediaID:%d\n",ret);
	              }	
	              else	
	              {	
		              EMU_INPUT("Please Input loop option[1/0]","0",tmpcommand,sizeof(tmpcommand));
		              context.isLoop = atoi(tmpcommand);		              
		              EMU_INPUT("Please Input play mode[0:all,1:no audio,2:no video]","0",tmpcommand,sizeof(tmpcommand));
		              context.plmode = atoi(tmpcommand);
		
		              ret = MP_PlayMediaSource(context.url,context.isLoop,context.plmode);
		              printf("=======||get mplayer mediaID:%d||======\n",ret);
                           
								}
	              tmpstep = EMU_STEP_MENU;  
	              }while(0);                  
	              
	              break;
        case EMU_STEP_PAUSE:
                do {
                    EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));                 
                    context.media_id = atoi(tmpcommand);                    
                    MP_pause(context.media_id);
                    tmpstep = EMU_STEP_MENU;
                } while (0);
                break;
        case EMU_STEP_START:
            do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                MP_start(context.media_id);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;        
        case EMU_STEP_RESUME:
            do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                MP_resume(context.media_id);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;  
        case EMU_STEP_STOP:
            do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                MP_stop(context.media_id);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break; 
        case EMU_STEP_MUTE:
            do {
                //EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                //context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input mute option[mute:1/unmute:0]","0",tmpcommand,sizeof(tmpcommand));
                context.isMute = atoi(tmpcommand);
                MP_mute(VOL_MEDIA_ID,context.isMute);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;                 
         case EMU_STEP_FF:
         	   do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input speed","0",tmpcommand,sizeof(tmpcommand));
                context.speed = atoi(tmpcommand);
                MP_fastforward(context.media_id,context.speed);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;      
         case EMU_STEP_RR:
         	   do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input speed","0",tmpcommand,sizeof(tmpcommand));
                context.speed = atoi(tmpcommand);
                MP_rewind(context.media_id,context.speed);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;          	
         case EMU_STEP_SEEK: 
         	   do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input seek position[0<pos<duration]","0",tmpcommand,sizeof(tmpcommand));
                context.speed = atoi(tmpcommand);
                //EMU_INPUT("Please Input start option","0",tmpcommand,sizeof(tmpcommand));
                //context.isStart = atoi(tmpcommand);
                MP_seek(context.media_id,context.speed,context.isStart);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;         	           
         case EMU_STEP_SETVOL:
         	   do {
                //EMU_INPUT("Please Input media_id[default:0]","0",tmpcommand,sizeof(tmpcommand));
                //context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input volume[0~255]","0",tmpcommand,sizeof(tmpcommand));
                context.vol = atoi(tmpcommand);
                MP_SetVolume(VOL_MEDIA_ID,context.vol);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;         	  
         case EMU_STEP_GETVOL:
         	  do {
                //EMU_INPUT("Please Input media_id[default:0]","0",tmpcommand,sizeof(tmpcommand));
                //context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please choose asynchronouse or synchronouse option[1/0]","1",tmpcommand,sizeof(tmpcommand));
                context.isAsync  = atoi(tmpcommand);
                if(context.isAsync == 1)
                    MP_GetVolume(VOL_MEDIA_ID);
                else
                {
                    MP_GetVolumeSync(VOL_MEDIA_ID,&ret);

                    printf("=== current volume:%d ======\n",ret);
                }
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;          	
         case EMU_STEP_SETTONE:
          	do {
                //EMU_INPUT("Please Input media_id[default:0]","0",tmpcommand,sizeof(tmpcommand));
                //context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input tone[0:stereo;1:left;2:right;3:swap]","0",tmpcommand,sizeof(tmpcommand));
                if(atoi(tmpcommand) == 0)
                	context.tone = MP_TONE_STEREO;
                else if(atoi(tmpcommand) == 1)
                	context.tone = MP_TONE_LEFTMONO;
                else if(atoi(tmpcommand) == 2)
                	context.tone = MP_TONE_RIGHTMONO;
                else if(atoi(tmpcommand) == 3)
                	context.tone = MP_TONE_SWAP;
                else
                	context.tone = MP_TONE_STEREO;				
                MP_SetTone(media_id,context.tone);
                tmpstep = EMU_STEP_MENU;
            } while (0); 
            break;       	           	  
         case EMU_STEP_GETPOS:
         	  do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please choose asynchronouse or synchronouse option[1/0]","1",tmpcommand,sizeof(tmpcommand));
                context.isAsync  = atoi(tmpcommand);
                if(context.isAsync == 1){
                    if(timechanged_flag == 0) 
                        timechanged_flag= 1;
                    else
                        timechanged_flag = 0;
                    MP_GetPosition(context.media_id);

                }    
                else
                {
                    MP_GetPositionSync(context.media_id,&ret);
                    printf("======play current pos[sync]:%d ======\n",ret);
                }
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;         	
         case EMU_STEP_GETDUR :
         	  do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please choose asynchronouse or synchronouse option[1/0]","1",tmpcommand,sizeof(tmpcommand));
                context.isAsync  = atoi(tmpcommand);
                if(context.isAsync == 1)
                    MP_GetDuration(context.media_id);    
                else
                {
                    MP_GetDurationSync(context.media_id,&ret);
                    printf("======play total[sync]:%d ======\n",ret);
                }
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;          	
         case EMU_STEP_SETLOOP :
         	   do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input repeat option","0",tmpcommand,sizeof(tmpcommand));
                context.isLoop = atoi(tmpcommand);
                MP_SetRepeat(context.media_id,context.isLoop);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;          	
         case EMU_STEP_SETBLACKOUT: 
         	   do {
                EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                context.media_id = atoi(tmpcommand);
                EMU_INPUT("Please Input blackout option[blackout:1/unblackout:0]","0",tmpcommand,sizeof(tmpcommand));
                context.isBlackout = atoi(tmpcommand);
                MP_SetVideoBlackOut(context.media_id,context.isBlackout);
                tmpstep = EMU_STEP_MENU;
            } while (0);
            break;  
         case EMU_STEP_SPECTRUM:
       	do{
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand);
              EMU_INPUT("Please Input song spectrum option[0/1]","0",tmpcommand,sizeof(tmpcommand));
              context.isStartPop = atoi(tmpcommand);
              EMU_INPUT("Please Input song spectrum pop interval in ms","300",tmpcommand,sizeof(tmpcommand));
              context.interval = atoi(tmpcommand);
              MP_SetSongSepctralOut(context.media_id,context.isStartPop,context.interval);
              tmpstep = EMU_STEP_MENU;
            } while (0);	
         		break;  
         case EMU_STEP_GETSTATUS:
         	 do {
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand); 
              MP_GetStatus(context.media_id);
              tmpstep = EMU_STEP_MENU;
            } while (0);	
         		break; 
         case EMU_STEP_SETSUBT:
         	 do{
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand);
              EMU_INPUT("Please Input subtitle pid[default: off[0xffff]]","0xffff",tmpcommand,sizeof(tmpcommand));
              context.sub_pid = atoi(tmpcommand);              
              MP_SetInternalSubtitleOut(context.media_id,context.sub_pid);
              tmpstep = EMU_STEP_MENU;
            } while (0);	
         		break;
         case EMU_STEP_EXIT:
         	  tmpneedexit = 1; 
                break;
         case EMU_STEP_ATRACK:
          	 do{
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand);
              EMU_INPUT("Please Input audio track pid[0]","0",tmpcommand,sizeof(tmpcommand));
              context.track_pid = atoi(tmpcommand);              
              MP_SetAudioTrack(context.media_id,context.track_pid);
              tmpstep = EMU_STEP_MENU;
            } while (0);	
         		break;  
         case EMU_STEP_GETSUBT:
         	do {
                  EMU_INPUT("Please Input multi-subt sample file url:","/home/nba.rmvb",context.url,1024);
                  EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
                  context.media_id = atoi(tmpcommand); 
                  if(strlen(context.url )> 3)
                    MP_GetSubtitleInfo(context.url,context.media_id);
                  else
                    MP_GetSubtitleInfo(NULL,context.media_id);
                  tmpstep = EMU_STEP_MENU;
            } while (0);	         		
            break;		      	 
         case EMU_STEP_GETATRACK:
         	do {
              EMU_INPUT("Please Input multi-audiotrack sample file url","/home/nba.rmvb",context.url,1024);
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand); 
              if(strlen(context.url) > 3)
                MP_GetAudioTrackInfo(context.url,context.media_id);
              else
                MP_GetAudioTrackInfo(NULL,context.media_id);
              tmpstep = EMU_STEP_MENU;
            } while (0);	
         	break;   
            
         case EMU_STEP_GETAVMEDIAINFO:
         	do {
              EMU_INPUT("Please Input media sample file url","/home/nba.rmvb",context.url,1024);
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand); 
              if(strlen(context.url) > 3)
                MP_GetMediaInfo(context.url,context.media_id);
              else
                MP_GetMediaInfo(NULL,context.media_id);
              tmpstep = EMU_STEP_MENU;
            } while (0);	
            break;      
         case EMU_STEP_PAUSETORESUME:
            do {
              EMU_INPUT("Please Input Pause<->Resume benchmark counts[default:y]","3",tmpcommand,sizeof(tmpcommand));            
              context.test_counts = atoi(tmpcommand);
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand); 
              for(index = 0;index <context.test_counts;index++)
              {
                    MP_pause(context.media_id);
                    sleep(2);
                    MP_resume(context.media_id);
                    sleep(2);
              }
              tmpstep = EMU_STEP_MENU;
            } while (0);	
            break;  
         case EMU_STEP_QUITPLAYER:
           do {              
               MP_TerminateAmplayerProcess();
               tmpstep = EMU_STEP_MENU;
            } while (0);	
         	break; 

         case   EMU_STEP_LISTALLMEDIAID:
            do {   
                MP_ValidMediaIDInfo allmediaid;
                memset((void*)&allmediaid,0,sizeof(MP_ValidMediaIDInfo));
               MP_GetValidMediaIDSync(&allmediaid);
               printf("=============================playback response start===========================\n");
               for(index =0;index < allmediaid.num;index++)
                    printf("%d'st mediaID:%d\n",index,allmediaid.media_id[index]);
               printf("=============================playback response end============================\n");
               tmpstep = EMU_STEP_MENU;
            } while (0);	
           break; 
         case EMU_STEP_CLOSEMEDIAINST:
            do {             
              EMU_INPUT("Please Input media_id[default:11]","11",tmpcommand,sizeof(tmpcommand));
              context.media_id = atoi(tmpcommand); 
              MP_CloseMediaID(context.media_id); 
              tmpstep = EMU_STEP_MENU;
            } while (0); 
            break;
         case EMU_STEP_PLAYSTOP:
            EMU_INPUT("Please Input media sample file url","/home/nba.rmvb",context.url,1024);
            EMU_INPUT("Please Input Play<->Stop benchmark counts[default:y]","3",tmpcommand,sizeof(tmpcommand));       
            context.test_counts = atoi(tmpcommand);    
            EMU_INPUT("Please Input Play<->Stop benchmark interval in seconds.[default:y]","10",tmpcommand,sizeof(tmpcommand));            
            context.interval = atoi(tmpcommand);  
            for(index = 0;index <context.test_counts;index++)
            {
                ret =MP_AddMediaSource(context.url);
                if(ret<=0)
                {                    
                    printf("failed to get media id\n");
                    continue;
                }
                printf("======|get a mediaid:%d\n",ret);
                usleep(context.interval);
                MP_GetMediaInfo(NULL,ret);
                MP_stop(ret);
                MP_CloseMediaID(ret);
                ret = 0;
           }
           tmpstep = EMU_STEP_MENU;
                
            
            break;
         case EMU_STEP_STARTPLIST:
           do {
                int mode =0;
              EMU_INPUT("Please choose playmode,0:order play.1:loop play,default:","0",tmpcommand,sizeof(tmpcommand));
              mode = atoi(tmpcommand);   
              if(mode == 0)
              {
                    printf("start play file in playlist in order\n");
                    MP_StartAutoPlayTask(0);

              }  
              else if(mode == 1)
                {
                    printf("start play file in playlist in loop\n");
                
                    MP_StartAutoPlayTask(1);

              }
              tmpstep = EMU_STEP_MENU;
            } while (0);         		
         	break; 
         case EMU_SETP_ADDFILETOPLIST:
         	do {
              EMU_INPUT("Please Input media sample file url","/home/nba.rmvb",context.url,1024);
              if(strlen(context.url) > 3)
              {
                    
        		MP_AddFileToList(context.url);        		
 		}
              tmpstep = EMU_STEP_MENU;
            } while (0);
         	break;
         case EMU_STEP_STOPPLIST:
         		do {
              MP_StopAutoPlayTask();
              tmpstep = EMU_STEP_MENU;
            } while (0);
            
         	break;	
         case EMU_STEP_VIEWPLIST:
            do{
                MP_DumpPlayList();
                tmpstep =EMU_STEP_MENU; 
            }while(0);
            break;
         case EMU_STEP_PLAYNEXT:
            do{
                MP_PlayNextFile();
                tmpstep =EMU_STEP_MENU; 
            }while(0);
            break; 
         case EMU_STEP_ADDDIRTOPLIST:
            do {
              EMU_INPUT("Please Input added file directory","/media/usba1",context.url,1024);
              if(strlen(context.url) > 3 &&strlen(context.url)<512)
              {
                    
        		MP_AddDirToPlayList(context.url);        		
 		}
              tmpstep = EMU_STEP_MENU;
             } while (0);
         	break;
         case EMU_STEP_NONE:   
            break;
         case EMU_STEP_MENU:
            do {
                    printf(SCREEN_SPLITER);
                    printf("                     socket2 player emulator                v1.4\n");
                    printf(SCREEN_SPLITER);
                    printf("* Please choose one option                                 *\r\n");
                    printf("* 1   Play a media/Set media source[isStart = 0]           *\r\n");
                    printf("* 2   Pause play                                           *\r\n");
                    printf("* 3   Resume play                                          *\r\n");
                    printf("* 4   Stop play                                            *\r\n");
                    printf("* 5   Muteon/Muteoff                                       *\r\n");   
                    printf("* 6   Fastforward                                          *\r\n");  
                    printf("* 7   Fastrewind                                       	   *\r\n");  
                    printf("* 8   Seek                                             	   *\r\n");  
                    printf("* 9   Get volume                                           *\r\n"); 
                    printf("* 10  Set volume                                           *\r\n"); 
                    printf("* 11  Set tone                                             *\r\n"); 
                    printf("* 12  Get position                                         *\r\n");    
                    printf("* 13  Get duration                                         *\r\n");  
                    printf("* 14  Set repeat                                           *\r\n"); 
                    printf("* 15  Set blackout                                         *\r\n"); 
                    printf("* 16  Set song sepctrum                                    *\r\n");
                    printf("* 17  Get Status                                           *\r\n");
                    printf("* 18  Set internal-subtitle                                *\r\n");
                    printf("* 19  Set audio track                                      *\r\n");
                    printf("* 20  Get audio track info                                 *\r\n");
                    printf("* 21  Get internal subtitle info                           *\r\n");
                    printf("* 22  Get av media info                                    *\r\n");
                    printf("* 23  Start play media source                              *\r\n");
                    printf("* 24  PauseToResume benchmark test                         *\r\n"); 
                    printf("* 25  List all mediaID                                     *\r\n"); 
                    printf("* 26  Close a mediaplayer instance                         *\r\n");   
                    printf("* 27  PlayToStop benchmark test                            *\r\n");                                      
                    printf("* 29  Terminate amplayer2 process                          *\r\n");
                    printf("* 30  Quit tools                                           *\r\n");           
                    printf(SCREEN_SPLITER);										
                    printf("* 40  Start playlist autoplay mode                         *\r\n"); 
                    printf("* 41  Add a file to playlist                               *\r\n"); 
                    printf("* 42  Stop playlist autoplay mode                          *\r\n"); 
                    printf("* 43  View items in playlist                               *\r\n");
                    printf("* 44  Play next items in playlist                          *\r\n");
                    printf("* 45  Add a directory to playlist                          *\r\n");
                    printf(SCREEN_SPLITER);  
                    EMU_INPUT("Please Input Choose:","",tmpcommand,sizeof(tmpcommand));
                    
                    if(strcmp(tmpcommand,"1")==0)
                    {
                    	
                    	tmpstep = EMU_STEP_PLAY;
                    } 
                    else if (strcmp(tmpcommand,"2")==0) 
                    {
                      	tmpstep = EMU_STEP_PAUSE;
                    } 
                    else if (strcmp(tmpcommand,"3")==0)
                    {
                    	tmpstep = EMU_STEP_RESUME;
                    } 
                    else if (strcmp(tmpcommand,"4")==0)
                    {
                    	tmpstep = EMU_STEP_STOP;
                    } 
                    else if (strcmp(tmpcommand,"5")==0) 
                    {
                        tmpstep = EMU_STEP_MUTE;
                    } 
                    else if (strcmp(tmpcommand,"6")==0) 
                    {
                        tmpstep = EMU_STEP_FF;
                    } 
                    else if (strcmp(tmpcommand,"7")==0) 
                    {
                        tmpstep = EMU_STEP_RR;
                    } 
                    else if (strcmp(tmpcommand,"8")==0) 
                    {
                        tmpstep = EMU_STEP_SEEK;
                    }
                    else if (strcmp(tmpcommand,"9")==0) 
                    {
                        tmpstep = EMU_STEP_GETVOL;
                    }  
                    else if (strcmp(tmpcommand,"10")==0) 
                    {
                        tmpstep = EMU_STEP_SETVOL;
                    }                        
                    else if (strcmp(tmpcommand,"11")==0) 
                    {
                        tmpstep = EMU_STEP_SETTONE;
                    }                        
                    else if (strcmp(tmpcommand,"12")==0) 
                    {
                        tmpstep = EMU_STEP_GETPOS;
                    }
                    else if (strcmp(tmpcommand,"13")==0) 
                    {
                        tmpstep = EMU_STEP_GETDUR;
                    }
	    							else if (strcmp(tmpcommand,"14")==0) 
                    {
                        tmpstep = EMU_STEP_SETLOOP;
                    }
                    else if (strcmp(tmpcommand,"15")==0) 
                    {
                        tmpstep = EMU_STEP_SETBLACKOUT;
                    }
                    else if (strcmp(tmpcommand,"16")== 0)
                    {
                    		tmpstep = EMU_STEP_SPECTRUM;
                    }	
                    else if (strcmp(tmpcommand,"17")== 0)
                    {
                    		tmpstep = EMU_STEP_GETSTATUS;
                    }	
                    else if (strcmp(tmpcommand,"18")== 0)
                    {
                    		tmpstep = EMU_STEP_SETSUBT;
                    }	
                    else if (strcmp(tmpcommand,"19")== 0)
                    {
                        tmpstep = EMU_STEP_ATRACK;
                    }	
                    else if (strcmp(tmpcommand,"20")==0) 
                    {
                        tmpstep = EMU_STEP_GETATRACK;
                    }
                    else if (strcmp(tmpcommand,"21")== 0)
                    {
                        tmpstep = EMU_STEP_GETSUBT;
                    }
                    else if (strcmp(tmpcommand,"22")== 0)
                    {
                        tmpstep = EMU_STEP_GETAVMEDIAINFO;
                    }
                    else if (strcmp(tmpcommand,"23")== 0)
                    {
                        tmpstep = EMU_STEP_START;
                    }
                     else if (strcmp(tmpcommand,"24")== 0)
                    {
                        tmpstep = EMU_STEP_PAUSETORESUME;
                    }
                    else if (strcmp(tmpcommand,"29")== 0)
                    {
                        tmpstep = EMU_STEP_QUITPLAYER;
                    }
                    else if (strcmp(tmpcommand,"30")==0) 
                    {
                        tmpstep = EMU_STEP_EXIT;
                    }
                    else if (strcmp(tmpcommand,"25")==0) 
                    {
                        tmpstep = EMU_STEP_LISTALLMEDIAID;
                    }
                    else if (strcmp(tmpcommand,"26")==0) 
                    {
                        tmpstep = EMU_STEP_CLOSEMEDIAINST;
                    }
                     else if (strcmp(tmpcommand,"27")==0) 
                    {
                        tmpstep = EMU_STEP_PLAYSTOP;
                    }
                    else if (strcmp(tmpcommand,"40")==0)
                    {
                    		tmpstep = EMU_STEP_STARTPLIST;
                    }
                    else if (strcmp(tmpcommand,"41")==0)
                  	{
                  			tmpstep = EMU_SETP_ADDFILETOPLIST;
                  	}
                  	else if (strcmp(tmpcommand,"42")==0)
                  	{
                  			tmpstep = EMU_STEP_STOPPLIST;
                  	}
                    else if(strcmp(tmpcommand,"43")==0)
                    {
                        tmpstep = EMU_STEP_VIEWPLIST;
                    }
                    else if(strcmp(tmpcommand,"44")==0)
                    {
                        tmpstep = EMU_STEP_PLAYNEXT;
                    }
                    else if(strcmp(tmpcommand,"45")==0)
                    {
                        tmpstep = EMU_STEP_ADDDIRTOPLIST;
                    }
                    
            } while (0);
            
            break;          

        }
    }
 

    MP_ReleaseMPClient();

    return 0;
}


int main()
{
    mp_run_tool();

    return 0;
}
 
