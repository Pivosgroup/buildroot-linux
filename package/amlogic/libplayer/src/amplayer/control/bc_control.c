/**
 * bc_control
 *  Used when the controller is run in background.
 *  Based on shell_control.
 *
 */
#include <stdio.h>
#include <codec.h>
#include <player_ctrl.h>
#include <log_print.h>
#include <controler.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include "player_id.h"

#include <pthread.h>
#include "adec-external-ctrl.h"

extern void vm_update_state(player_info_t *info);

static uint8_t         bc_initialized = 0;
static pthread_mutex_t bc_cmd_mutex;
static pthread_cond_t  bc_cmd_cond;
static char            bc_cmd_str[100] = {0};

#define BC_CMD_WAIT_SEC 3

#define is_CMD(str,cmd,s_cmd1,s_cmd2) (strcmp(str,cmd)==0 || strcmp(str,s_cmd1)==0 || strcmp(str,s_cmd2)==0)

#if 1
static void * setcmdpt(void* p)
{
    char * cmd = (char*)p;
    int rc;
    rc = pthread_mutex_lock(&bc_cmd_mutex);
    if (rc) 
        goto end;
    strncpy(bc_cmd_str, cmd, sizeof(bc_cmd_str));
    bc_cmd_str[sizeof(bc_cmd_str) - 1] = '\0';
    log_print("setcmdpt() bc_cmd_str=%s\n", bc_cmd_str);

    pthread_cond_signal(&bc_cmd_cond);
end:
    pthread_mutex_unlock(&bc_cmd_mutex);
    if (cmd) FREE(cmd);
    pthread_exit(NULL);
	return NULL;
}

int bc_control_set_command(char *cmd)
{
    pthread_t thread;
    char * dcmd = strdup(cmd);
    if (!bc_initialized) {
        log_print("bc_control_set_command(%s) !bc_initalized\n", dcmd);
        if (dcmd) FREE(dcmd);
        return -1;
    }
    log_print("bc_control_set_command(%s)\n", dcmd);
    return pthread_create(&thread, NULL, setcmdpt, (void*)dcmd);
}
#else
int bc_control_set_command(char *cmd)
{
    int rc;
    if (!cmd || cmd[0] == '\0')
        return (-EINVAL);

    rc = pthread_mutex_lock(&bc_cmd_mutex);
    if (rc)
        return rc;

    strncpy(bc_cmd_str, cmd, sizeof(bc_cmd_str));
    bc_cmd_str[sizeof(bc_cmd_str) - 1] = '\0';

    pthread_cond_signal(&bc_cmd_cond);

    pthread_mutex_unlock(&bc_cmd_mutex);
    return 0;
}
#endif

/** @return 0 on success */
static int bc_control_init(struct control_para * para)
{
    int rc;
    log_print("[bc_control_init]\n");
    bc_cmd_str[0] = '\0';
#if 0
    if (amadec_thread_init() !=0 ) {
        log_print("amadec_thread_init failed\n");
    }else{
        log_print("amadec_thread_init done\n");
    }
#endif        
    rc = (pthread_mutex_init(&bc_cmd_mutex, NULL) ||
          pthread_cond_init(&bc_cmd_cond, NULL));
    if (rc == 0)
        bc_initialized = 1;
    return rc;
}

/**
 * This should be called by controler_run in the controler thread.
 */
static int bc_control_get_command(player_cmd_t *playercmd)
{
    int ret;
    char str[100]={0};
    struct timespec ts;
    struct timeval tp;

    ret = pthread_mutex_lock(&bc_cmd_mutex);
    if (ret)
        return ret;

    //ret = pthread_cond_wait(&bc_cmd_cond, &bc_cmd_mutex);
    if (bc_cmd_str[0] == '\0') {
        /* Wait for command */
        gettimeofday(&tp, NULL);
        ts.tv_sec = tp.tv_sec;
        ts.tv_nsec = tp.tv_usec * 1000;
        ts.tv_sec += BC_CMD_WAIT_SEC;
        //log_print("bc_control_get_command waiting..\n");
        ret = pthread_cond_timedwait(&bc_cmd_cond, &bc_cmd_mutex, &ts);
        if (ret == ETIMEDOUT) {
            //log_print("bc_control_get_command timed out ");
            pthread_mutex_unlock(&bc_cmd_mutex);
            return -1;
        }
        else if (ret) {
            //log_print("bc_control_get_command wait err=%d\n", ret);
            pthread_mutex_unlock(&bc_cmd_mutex);
            return ret;
        }
    }
    strncpy(str, bc_cmd_str, sizeof(str));
    str[sizeof(str) - 1] = '\0';
    bc_cmd_str[0] = '\0';

    pthread_mutex_unlock(&bc_cmd_mutex);

    log_print("bc_control_get_command() got cmd %s", str);

    if(1)
    {
        if (is_CMD(str, "quitctrl", "quitctrl", "quitctrl"))
        {
            return 0xbad; //quit controller thread
        }
        else if(strncmp(str, "play:", 5) == 0) {
            playercmd->filename = strdup(str + 5);
            if (playercmd->filename && playercmd->filename[0] != '\0') {
                playercmd->ctrl_cmd = CMD_PLAY_START;
            }
            else {
                if (playercmd->filename) FREE(playercmd->filename);
                playercmd->filename = NULL;
            }
        }
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
            log_print("======[bc_get_command]mute status=%d\n",ret);
        }
    }
    log_print("[bc_get_command:%d]cmd str=%s ctrl_cmd=0x%x info_cmd=0x%x set_mode=0x%x\n",
              __LINE__, str, playercmd->ctrl_cmd, playercmd->info_cmd, playercmd->set_mode);
    if((playercmd->ctrl_cmd!=0) ||
        (playercmd->info_cmd!=0)||
        (playercmd->set_mode != 0))
    {

        char pid[MAX_PLAYER_THREADS];
        int num = 0;
        num = player_list_pid(pid, MAX_PLAYER_THREADS);
        if(num == 1)
            playercmd->pid = pid[0];
        log_print("[bc_get_command:%d]num=%d pid=%d param=%d\n",__LINE__,num,playercmd->pid, playercmd->param);
		log_print("[bc_get_command:%d]cmd str=%s ctrl_cmd=0x%x info_cmd=0x%x set_mode=0x%x\n",
              __LINE__, str, playercmd->ctrl_cmd, playercmd->info_cmd, playercmd->set_mode);
        return 0;
    }
    else
        return -1;
}
static int player_status_revert2old(player_status newsta)
{
	switch(newsta)
	{
		case PLAYER_INITING:
			return 6;
		case PLAYER_INITOK:
			return 13;

		case PLAYER_RUNNING: 
			return 2;
		case PLAYER_BUFFERING:
			return 12;
		case PLAYER_PAUSE:
			return 3;
		case PLAYER_SEARCHING:
			return 4;
	
		case PLAYER_SEARCHOK:
			return 5;
		case PLAYER_START:
			return 9;
		case PLAYER_FF_END:
			return 10;
		case PLAYER_FB_END:
			return 11;
	
		case PLAYER_ERROR:
			return 7;
		case PLAYER_PLAYEND:
			return 8;
		case PLAYER_STOPED:
			return 1;		
		default: 
			;
	}
	return -1;
}
static int bc_control_update_state(int pid,player_info_t *player_info)
{
    if(player_info)
    {
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
		player_info->status = player_status_revert2old(player_info->status);
        vm_update_state(player_info);
    }
    return 0;
}

extern int update_audio_info(int total_audio_num, int audio_id, int audio_format, int Flag);
static int bc_control_return_info(int pid, int cid, int type, void *data)
{
    log_print("**********type=%x\n",type);
    if(type == CMD_GET_VOL_RANGE)
    {
        volume_range_t *range = (volume_range_t *)data;
        log_print("[bc_return_info]pid=%d cid=%d type=%d min=%d max=%d\n", pid, cid, type,range->min,range->max);
    }
    else if(type == CMD_LIST_PID)
    {
        int i;
        pid_info_t* playerid = (pid_info_t *)data;
        log_print("[bc_return_info]pid=%d cid=%d type=%d num=%d\n", pid, cid, type,playerid->num);
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
        log_print("current audio index:  %d\n",minfo->stream_info.cur_audio_index);
        log_print("sub streams:    %d\n",minfo->stream_info.total_sub_num);
        log_print("\n");
		if(minfo->stream_info.has_video)
		{
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
		}
		if(minfo->stream_info.has_audio)
		{
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
	            update_audio_info(minfo->stream_info.total_audio_num, minfo->audio_info[i]->id, minfo->audio_info[i]->aformat, 0);
	            if (minfo->stream_info.cur_audio_index == i+1)
	            	update_audio_info(minfo->stream_info.total_audio_num, minfo->audio_info[i]->id, minfo->audio_info[i]->aformat, 1);
	        }
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
        log_print("[bc_return_info]pid=%d cid=%d type=%d data=%d\n", 
                  pid, cid, type, (val ? *val : 0));
    }

    return 0;
}

static int bc_control_release(struct control_para * para)
{
    log_print("bc_control_release enter\n");
    bc_cmd_str[0] = '\0';
    bc_initialized = 0;
    pthread_mutex_destroy(&bc_cmd_mutex);
    pthread_cond_destroy(&bc_cmd_cond);
    //amadec_thread_exit();
    log_print("bc_control_release exit\n");
    return 0;
}

player_controler_t bc_cmd =
{
    .name           = "bc",
    .front_mode     = 0,
    .init           = bc_control_init,
    .get_command    = bc_control_get_command,
    .update_state   = bc_control_update_state,
    .ret_info       = bc_control_return_info,
    .release        = bc_control_release,
};

int register_bc_controler(void)
{
    register_controler(&bc_cmd);
    return 0;
}

