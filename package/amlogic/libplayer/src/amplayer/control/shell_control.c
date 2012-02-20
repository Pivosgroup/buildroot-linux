#include <stdio.h>
#include <codec.h>
#include <player_ctrl.h>
#include <log_print.h>
#include <controler.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "player_id.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void shell_usage(void)
{
	log_print("\n-----------Player Control Command:----------------\n");
	log_print("\tx:quit\n");
	log_print("\ts:stop\n");
	log_print("\tp:pause\n");
	log_print("\tr:resume\n");
    log_print("\tt:search\n");
    log_print("\tf:forward, fast forward\n");
    log_print("\tb:backward, fast backward\n");
	log_print("\tm:mute\n");
	log_print("\tM:unmute\n");
	log_print("\tv:volget,get currnet sound volume\n");
	log_print("\tV:volset(V:val),set currnet sound volume\n");
    log_print("\ta:aid, set new audio id to play\n");
    log_print("\tu:spectrum, audio switch spectrum\n");
    log_print("\tw:swap, swap audio left and right\n");
    log_print("\tl:lmono, audio set left mono\n");
    log_print("\tL:rmono, audio set right mono\n");
    log_print("\to:stereo, audio set stereo\n");
    log_print("\tc:curtime, get current playing time\n");
    log_print("\td:duration, get stream duration\n");
    log_print("\tS:status, get player status\n");
    log_print("\ti:media, get media infomation\n");   
    log_print("\tk:pid, list all valid player id\n");
    log_print("\ty:sid, set new subtitle id\n");
    log_print("\tz:mutesta, get mute sataus\n");
	log_print("\th:help\n");
}

static int osd_blank(char *path,int cmd)
{
        int fd;
        char  bcmd[16];
        fd=open(path, O_CREAT|O_RDWR | O_TRUNC, 0644);
        if(fd>=0)
                {
                sprintf(bcmd,"%d",cmd);
                write(fd,bcmd,strlen(bcmd));
                close(fd);
                return 0;
                }
        return -1;
}

static int shell_get_command(player_cmd_t *playercmd)
{

	int  ret;
    fd_set rfds;
    struct timeval tv;
	int fd=fileno(stdin);
    
#define is_CMD(str,cmd,s_cmd1,s_cmd2) (strcmp(str,cmd)==0 || strcmp(str,s_cmd1)==0 || strcmp(str,s_cmd2)==0)

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);
	tv.tv_sec = 0;
	tv.tv_usec = 3 * 1000 * 1000;       
	ret = select(fd+1, &rfds, NULL, NULL, &tv);
   
	if(ret > 0 && FD_ISSET(fd, &rfds))
	{            
    	char str[100]={0};
    	scanf("%s",str);
    	//gets(str);
    	if(is_CMD(str,"quit","x","X"))
		{
		    playercmd->ctrl_cmd = CMD_EXIT;
		}
	    else if(is_CMD(str,"stop","s","s"))
		{
		    playercmd->ctrl_cmd = CMD_STOP;
		}
	    else if(is_CMD(str,"pause","p","P"))
		{
		    playercmd->ctrl_cmd = CMD_PAUSE;              
		}
	    else if(is_CMD(str,"resume","r","R"))
		{
		    playercmd->ctrl_cmd = CMD_RESUME;
		}
	    else if(is_CMD(str,"mute","m","m"))
		{			
            playercmd->set_mode = CMD_MUTE ;
		}
	    else if(is_CMD(str,"unmute","M","M"))
		{		
            playercmd->set_mode = CMD_UNMUTE;
		}
	    else if(is_CMD(str,"volget","v","v"))
		{           
            playercmd->info_cmd = CMD_GET_VOLUME;
		}
	    else if(memcmp(str,"volset:",strlen("volset:"))==0 ||
			(str[0]=='V' && str[1]==':'))
    	{            
    		int r=-1;
    		char *p;
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);  
                playercmd->set_mode = CMD_SET_VOLUME;
                playercmd->param = r;
			}
    		else
			{
			    log_print("error command for set vol(fmt:\"V:value\")\n");
			}           
    	}
        else if(memcmp(str,"search:",strlen("search:"))==0 ||
			(str[0]=='t' && str[1]==':')||
			(str[0]=='T' && str[1]==':'))
		{            
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                
                playercmd->ctrl_cmd = CMD_SEARCH;
                playercmd->param = r;                
			}
    		else
			{
			    log_print("error command for search(fmt:\"T:value\")\n");
			}            
		}
        else if(memcmp(str,"forward:",strlen("forward:"))==0 ||
			(str[0]=='f' && str[1]==':')||
			(str[0]=='F' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                 
                playercmd->ctrl_cmd = CMD_FF;
                playercmd->param= r;               
			}
    		else
			{
			    log_print("error command for forward(fmt:\"F:value\")\n");
			}
		}
        else if(memcmp(str,"backward:",strlen("backward:"))==0 ||
			(str[0]=='b' && str[1]==':')||
			(str[0]=='B' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                  
                playercmd->ctrl_cmd = CMD_FB;
                playercmd->param= r;    
                log_print("[%s:%d]fb---step=%d\n",__FUNCTION__, __LINE__, r);
			}
    		else
			{
			    log_print("error command for backward(fmt:\"B:value\")\n");
			}
		}
        else if(memcmp(str,"aid:",strlen("aid:"))==0 ||
			(str[0]=='a' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                  
                playercmd->ctrl_cmd = CMD_SWITCH_AID;
                playercmd->param= r;               
			}
    		else
			{
			    log_print("error command for audio id(fmt:\"a:value\")\n");
			}
		}
        else if(memcmp(str,"sid:",strlen("sid:"))==0 ||
			(str[0]=='y' && str[1]==':'))
		{
    		int r=-1;
    		char *p;            
    		p=strstr(str,":");
    		if(p!=NULL)
			{
    			p++;
    			sscanf(p,"%d",&r);                  
                playercmd->ctrl_cmd = CMD_SWITCH_SID;
                playercmd->param= r;               
			}
    		else
			{
			    log_print("error command for subtitle id(fmt:\"y:value\")\n");
			}
		}
        else if(is_CMD(str, "volrange", "g", "G"))
        {
            playercmd->info_cmd = CMD_GET_VOL_RANGE;
        }
        else if(is_CMD(str, "spectrum", "u", "U"))
        {
            playercmd->set_mode = CMD_SPECTRUM_SWITCH;
        }
        else if(is_CMD(str, "balance", "e", "E"))
        {
            playercmd->set_mode = CMD_SET_BALANCE;
        }
        else if(is_CMD(str, "swap", "w", "W"))
        {
            playercmd->set_mode = CMD_SWAP_LR;
        }       
        else if(is_CMD(str, "lmono", "l", "l"))
        {
            playercmd->set_mode = CMD_LEFT_MONO;
        }
        else if(is_CMD(str, "rmono", "L", "L"))
        {
            playercmd->set_mode = CMD_RIGHT_MONO;
        }
        else if(is_CMD(str, "setreo", "o", "O"))
        {
            playercmd->set_mode = CMD_SET_STEREO;
        }
        else if(is_CMD(str, "curtime", "c", "C"))
        {
            playercmd->info_cmd = CMD_GET_CURTIME;
        }
        else if(is_CMD(str, "duration", "d", "D"))
        {
            playercmd->info_cmd = CMD_GET_DURATION;
        }
        else if(is_CMD(str, "status", "S", "S"))
        {
            playercmd->info_cmd = CMD_GET_PLAY_STA;
        }
        else if(is_CMD(str, "media", "i", "I"))
        {
            playercmd->info_cmd = CMD_GET_MEDIA_INFO;
        }
        else if(is_CMD(str, "pid", "k", "K"))
        {
            playercmd->info_cmd = CMD_LIST_PID;
        }
        else if(is_CMD(str, "mutesta", "z", "z"))
        {
            ret = codec_get_mutesta(NULL);
            log_print("======[shell_get_command]mute status=%d\n",ret);
        }
    	else if(is_CMD(str,"help","h","H"))
    	{
    	    shell_usage();
    	}
    	else
    	{
        	if(strlen(str) > 1) // not only enter
        	    shell_usage();
    	}
	}  
    else if(ret < 0)
        log_print("[shell_get_command] select error!\n");
    //else    //ret = 0
        //log_print("[shell_get_command] select time out!\n");
    //log_print("[shell_get_command:%d]ccmd=%x param=%d\n",__LINE__,playercmd->ctrl_cmd,playercmd->param);    
	if((playercmd->ctrl_cmd!=0) || 
        (playercmd->info_cmd!=0)|| 
        (playercmd->set_mode != 0))
    {
       
        char pid[MAX_PLAYER_THREADS];
        int num = 0;
        num = player_list_pid(pid, MAX_PLAYER_THREADS);
        if(num == 1)
           playercmd->pid = pid[0];
        //log_print("[shell_get_command:%d]num=%d pid=%d param=%d\n",__LINE__,num,playercmd->pid, playercmd->param);
        return 0;
	}
    else
		return -1;
}
static int shell_update_state(int pid,player_info_t *player_info)
{
	#if 0
	log_print("Playing %s(%%%2d.%02d(%d,%d))(%d,%d)\r",  
		                                          player_info->name,
								player_info->current_time*100/(player_info->full_time+1),
								(player_info->current_time*10000/(player_info->full_time+1)%100),
								player_info->current_time,
								player_info->full_time,
								player_info->video_error_cnt,
								player_info->audio_error_cnt);
	#else
    if(player_info)
	{
        //log_print("[PID=%d]Playing %s-%2d.%02d%%-%02d:%02d:%02d(%02d:%02d:%02d)(%d,%d)\r", 
        log_print("[PID=%d]Playing %s-%2d.%02d%%-(%dms) %02d:%02d:%02d(%02d:%02d:%02d)(apts:0x%x,vpts:0x%x,diff:%d)\r", 
								pid,
		                        player_info->name,
								(player_info->current_time+1)*100/(player_info->full_time+1),
								((player_info->current_time+1)*10000/(player_info->full_time+1)%100),
								player_info->current_ms,
								player_info->current_time/3600,
							    (player_info->current_time/60)%60,
								player_info->current_time%60,
								player_info->full_time/3600,
								(player_info->full_time/60)%60,
								player_info->full_time%60,
								//player_info->video_error_cnt,
								//player_info->audio_error_cnt); 								
								player_info->current_pts,
								player_info->pts_video,
								player_info->current_pts - player_info->pts_video);
    }
	#endif
    //we need to blank the osd every seconds.becase the Android can auto change it.
    //
    osd_blank("/sys/class/graphics/fb0/blank",1);


	return 0;
}

static int shell_return_info(int pid, int cid, int type, void *data)
{   
    log_print("**********type=%x\n",type);
    if(type == CMD_GET_VOL_RANGE)
    {
        volume_range_t *range = (volume_range_t *)data;
        log_print("[shell_return_info]pid=%d cid=%d type=%d min=%d max=%d\n", pid, cid, type,range->min,range->max);
    }
    else if(type == CMD_LIST_PID)
    {
        int i;
        pid_info_t* playerid = (pid_info_t *)data;
        log_print("[shell_return_info]pid=%d cid=%d type=%d num=%d\n", pid, cid, type,playerid->num);
        log_print("valid pid:");
        for(i = 0; i < playerid->num; i++)
            log_print("%d",playerid->pid[i]);
        log_print("\n");
    }
    else if(type == CMD_GET_MEDIA_INFO)
    {
        int i;
        media_info_t *minfo = (media_info_t *)data;          
        log_print("\n***************************************\n");
        log_print("stream info:\n");
        log_print("file size:      %lld\n",minfo->stream_info.file_size);
        log_print("file duration:  %d(s)\n",minfo->stream_info.duration);
        log_print("has video:      %d\n",minfo->stream_info.has_video);
        log_print("has audio:      %d\n",minfo->stream_info.has_audio);
        log_print("has subtitle:   %d\n",minfo->stream_info.has_sub);
        log_print("stream bitrate: %d\n",minfo->stream_info.bitrate);
        log_print("video streams:  %d\n",minfo->stream_info.total_video_num);
        log_print("audio streams:  %d\n",minfo->stream_info.total_audio_num);
        log_print("sub streams:    %d\n",minfo->stream_info.total_sub_num);
        log_print("\n");
        for(i=0; i<minfo->stream_info.total_video_num;i++)    
        {
            log_print("video id:       %d\n",minfo->video_info[i]->id);
            log_print("video width:    %d\n",minfo->video_info[i]->width);
            log_print("video height:   %d\n",minfo->video_info[i]->height);
            log_print("video ratio:    %d:%d\n",minfo->video_info[i]->aspect_ratio_num,minfo->video_info[i]->aspect_ratio_den);
            log_print("frame_rate:     %.2f\n",(float)minfo->video_info[i]->frame_rate_num/minfo->video_info[i]->frame_rate_den);
            log_print("video bitrate:  %d\n",minfo->video_info[i]->bit_rate);
            log_print("video format:   %d\n",minfo->video_info[i]->format);
            log_print("video duration: %d\n",minfo->video_info[i]->duartion);
            log_print("----------------------\n");
        }
        for(i=0; i<minfo->stream_info.total_audio_num;i++)    
        {
            log_print("audio id:       %d\n",minfo->audio_info[i]->id);            
            log_print("audio duration: %d\n",minfo->audio_info[i]->duration);
            log_print("audio channel:  %d\n",minfo->audio_info[i]->channel);
            log_print("sample rate:    %d\n",minfo->audio_info[i]->sample_rate);
            log_print("audio format:   %d\n",minfo->audio_info[i]->aformat);
            log_print("audio bitrate:  %d\n",minfo->audio_info[i]->bit_rate);
            if(minfo->audio_info[i]->audio_tag)
            {
                log_print("title:       %s\n",minfo->audio_info[i]->audio_tag->title);  
                log_print("author:      %s\n",minfo->audio_info[i]->audio_tag->author);
                log_print("comment:     %s\n",minfo->audio_info[i]->audio_tag->album);
                log_print("album:       %s\n",minfo->audio_info[i]->audio_tag->comment);
                log_print("year:        %s\n",minfo->audio_info[i]->audio_tag->year);
                log_print("track:       %d\n",minfo->audio_info[i]->audio_tag->track);
                log_print("genre:       %s\n",minfo->audio_info[i]->audio_tag->genre);
                log_print("copyright:   %s\n",minfo->audio_info[i]->audio_tag->copyright);
                log_print("genre:       %c\n",minfo->audio_info[i]->audio_tag->pic);
            }
            log_print("-----------------------\n");
        }
		if(minfo->stream_info.has_sub)
        {
			for(i=0; i<minfo->stream_info.total_sub_num;i++)    
	        {
	            log_print("sub id:       %d\n",minfo->sub_info[i]->id);
	            log_print("inter_ex:     %d\n",minfo->sub_info[i]->internal_external);
	            log_print("sub width:    %d\n",minfo->sub_info[i]->width);
	            log_print("sub height:   %d\n",minfo->sub_info[i]->height);
	            log_print("resolution:   %d\n",minfo->sub_info[i]->resolution);
	            log_print("sub size:     %lld\n",minfo->sub_info[i]->subtitle_size);
	            log_print("language:     %s\n",minfo->sub_info[i]->sub_language);
	        }
	        log_print("***************************************\n");   
		}
    }
    else
    {
        int *val = (int *)data;
        log_print("[shell_return_info]pid=%d cid=%d type=%d data=%d\n", pid, cid, type,*val);    
    }
    return 0;
}

player_controler_t shell_cmd=
{
	.name="shell",
	.front_mode=1,
	.get_command=shell_get_command,
	.update_state=shell_update_state,
	.ret_info=shell_return_info,
};

int register_shell_controler(void)
{	
	register_controler(&shell_cmd);
	return 0;
}

