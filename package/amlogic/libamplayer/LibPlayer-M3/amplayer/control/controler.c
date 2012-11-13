#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>

#include <player.h>
#include <player_ctrl.h>
#include <log_print.h>
#include <player_set_sys.h>
#include <controler.h>
#include <unistd.h>

#include "shell_control.h"
//#include "socket_controler.h"

#ifdef ANDROID
#include <pthread.h>
#endif


#define MAX_CONTROLER 16
#define VALID_CMD       (0)
#define INVALID_CMD     (-100)
static  player_controler_t *control_list[MAX_CONTROLER];
static int control_index=0;
int update_statue(int pid,player_info_t *);

static thread_ctrl_para_t *construct_thread_para(play_control_t *g, struct player_controler *controler)
{   
    thread_ctrl_para_t *t_para = NULL;
    t_para = MALLOC(sizeof(thread_ctrl_para_t));  
    if(!t_para)
        return NULL;  
    MEMSET(t_para, 0, sizeof(thread_ctrl_para_t));
    MEMCPY(&t_para->play_ctrl_para, g, sizeof(play_control_t));
    t_para->controler = controler;
    log_print("[construct_thread_para:%d]t=%p pid=%d g->filename=%s\n",__LINE__,t_para, t_para->pid,g->file_name);
    return t_para;
}

static int destruct_thread_para(thread_ctrl_para_t *t_para)
{  
	if(t_para!=NULL)
	{ 
        if(t_para->play_ctrl_para.file_name!=NULL)
        {
            FREE(t_para->play_ctrl_para.file_name);
            t_para->play_ctrl_para.file_name = NULL;
        }
        FREE(t_para);    
        t_para = NULL;        
        return 0;
	}
	else	
        return -1;	 
}

int start_controler(global_ctrl_para_t *player_para)
{
	int i;
	player_controler_t *controler=NULL;
	if(player_para->control_mode[0]=='\0') /*not set*/
		strcpy(player_para->control_mode,"shell");
	for(i=0;i<control_index;i++)
		{          
		if(IS_MODE(player_para->control_mode,control_list[i]->name))
			{
			controler=control_list[i];
			break;
			}
		}
	if(!controler)
		{
		log_error("can't find the controler %s\n",player_para->control_mode);
		return -1;
		}
	if(controler->front_mode)
		player_para->background=0;
	player_para->controler=controler;
	return 0;
}

int register_controler(player_controler_t *controler)
{
	if(control_index<MAX_CONTROLER)
		{
		control_list[control_index++]=controler;
		}
	else
		{
		log_error("register_controler %s failed,too many controlers,num=%d\n",controler->name,control_index);
		return -1;
		}
	return 0;
}


void basic_controlers_init(void)
{
	control_index=0;
	MEMSET(control_list,0,sizeof(control_list));
		register_shell_controler();
}

int get_command(global_ctrl_para_t *p,player_cmd_t *playercmd)
{
	if(!playercmd)
		return -1;
	return p->controler->get_command(playercmd);
}

int update_statue(int pid,player_info_t *player_state)
{
	thread_ctrl_para_t *t;
	player_controler_t *ctrl;
	if(!player_state)
		return -1;
	t=(void *)player_get_extern_priv(pid);
	if(t!=NULL)
	{
    	ctrl=t->controler;  
    	if(NULL!=ctrl->update_state)        
            return ctrl->update_state(pid,player_state);      	
        else           
            return 0;            
	}
	else	
        return -1;	
}
static void clear_player_ctrl_para(int pid, global_ctrl_para_t *p)
{
    play_control_t *para = &p->g_play_ctrl_para;    
    if(para)
    {
        para->file_name = NULL;
        para->audio_index = -1;
        para->video_index = -1;
        para->loop_mode = 0;
        para->nosound = 0;
        para->novideo = 0;    
    }
}

static int is_valid_command(int pid,player_cmd_t *cmd)
{
    player_status sta;
    if(check_pid_valid(pid))
    {
        sta = player_get_state(pid);
        log_print("[is_valid_command:%d]pid=%d sta=%d\n", __LINE__, pid, sta);
        if(sta == PLAYER_STOPED || sta == PLAYER_ERROR || sta == PLAYER_PLAYEND)
        {
            log_print("[is_valid_command:%d]pid=%d cmd->ctrl_cmd=%x (%x)\n", __LINE__,pid,cmd->ctrl_cmd, (CMD_STOP | CMD_SEARCH | CMD_FB | CMD_FF | CMD_SWITCH_AID | CMD_SWITCH_SID));
            if(cmd->ctrl_cmd & (CMD_STOP | CMD_SEARCH | CMD_FB | CMD_FF | CMD_SWITCH_AID | CMD_SWITCH_SID))
            {            
                return INVALID_CMD;
            }
        }        
    } 
    if(cmd->ctrl_cmd != 0 || cmd->set_mode != 0 || cmd->info_cmd != 0)
        return VALID_CMD;
    else
        return INVALID_CMD;
}

static int parse_command(global_ctrl_para_t *p, player_cmd_t *cmd)
{    
    int ret = 0;    
	int pid = cmd->pid;	 
    thread_ctrl_para_t *t;
    //log_print("[parse_command:%d]pid=%d ctrl_cmd=%x set_mode=%x info=%x\n", __LINE__,pid, cmd->ctrl_cmd, cmd->set_mode,cmd->info_cmd);
    ret = is_valid_command(pid,cmd);
    if(ret == VALID_CMD)
    {
        if(cmd->set_mode != 0)
            log_print("[parse_command:%d]pid=%d set_mode=%x param=%d %x\n",__LINE__,pid,cmd->set_mode, cmd->param,cmd->param1);
        //set play mode
        if(cmd->set_mode & CMD_LOOP)
        {    
            if(check_pid_valid(pid))
            {
                player_loop(pid);
                t = (void *)player_get_extern_priv(pid);
                if(t!=NULL)
                    t->play_ctrl_para.loop_mode = 1;
            }
            else
                p->g_play_ctrl_para.loop_mode = 1;
        }
        else if(cmd->set_mode & CMD_NOLOOP)
        {
            if(check_pid_valid(pid))
            {
                player_noloop(pid);
                t = (void *)player_get_extern_priv(pid);
                if(t!=NULL)
                    t->play_ctrl_para.loop_mode = 0;
            }
            else
                p->g_play_ctrl_para.loop_mode = 0;
        }

        if(cmd->set_mode & CMD_NOBLACK)
        {
            set_black_policy(0);        
        }
        else if(cmd->set_mode & CMD_BLACKOUT)
        {
            set_black_policy(1);        
        }

        if(cmd->set_mode & CMD_NOAUDIO)
        {
            p->g_play_ctrl_para.nosound = 1;
        }    
        
        if(cmd->set_mode & CMD_NOVIDEO)
        {
            p->g_play_ctrl_para.novideo = 1;
        } 

        if(cmd->set_mode & CMD_MUTE)
        {
            audio_set_mute(pid, 1);
        }   
        else if(cmd->set_mode & CMD_UNMUTE)
        {
            audio_set_mute(pid, 0);
        }
        if(cmd->set_mode & CMD_SET_VOLUME)
        {
            int r = -1;           
            int val = cmd->param;
            log_print("[CMD_SET_VOLUME]pid=[%d] val=%d\n",cmd->pid,val);
            r = audio_set_volume(pid, val);     
            if(r < 0)                
                log_error("[CMD_SET_VOLUME]pid:[%d] failed! r=%x\n",cmd->pid,r);
        }
          if(cmd->set_mode & CMD_SPECTRUM_SWITCH)
        {
            int r = -1;           
            int isStart = cmd->param;
            int interval= cmd->param1;
            r = audio_set_spectrum_switch(pid, isStart, interval);
            if(r < 0)                
                log_error("[CMD_SPECTRUM_SWITCH]pid:[%d] failed!r=%x\n",cmd->pid,r);
        }
        if(cmd->set_mode & CMD_SET_BALANCE)
        {
            int r = -1;            
            int balance = cmd->param;
            r = audio_set_volume_balance(pid, balance);
            if(r < 0)                
                log_error("[CMD_SET_BALANCE]pid:[%d] failed!r=%x\n",cmd->pid,r);
        }
        if(cmd->set_mode & CMD_SWAP_LR)
        {
            int r = -1;            
            r = audio_swap_left_right(pid);
            if(r < 0)                
                log_error("[CMD_SWAP_LR]pid:[%d] r=%x\n",cmd->pid,r);
        }
        if(cmd->set_mode & CMD_LEFT_MONO)
        {
            int r = -1;            
            r = audio_left_mono(pid);
            if(r < 0)                
                log_error("[CMD_LEFT_MONO]pid:[%d] failed!r=%x\n",cmd->pid,r);
        }
         if(cmd->set_mode & CMD_RIGHT_MONO)
        {
            int r = -1;            
            r = audio_right_mono(pid);
            if(r < 0)                
                log_error("[CMD_RIGHT_MONO]pid:[%d]r=%x\n",cmd->pid,r);
        }
        if(cmd->set_mode & CMD_SET_STEREO)
        {
            int r = -1;           
            r = audio_stereo(pid);
            if(r < 0)                
                log_error("[CMD_SET_STEREO]pid:[%d] r=%x\n",cmd->pid,r);
        }
        if(cmd->info_cmd != 0)
            log_print("[parse_command:%d]pid:[%d] info_cmd=%x\n",__LINE__,cmd->pid,cmd->info_cmd);
        //get information
        if(cmd->info_cmd & CMD_GET_VOLUME)
        {
            int r = -1;
	    float volume;
            r = audio_get_volume(pid, &volume);
            log_print("pid:[%d] get audio_volume=%d\n",cmd->pid,r);
            if(r >= 0)
            {
                log_print("pid:[%d]get audio_volume=%f\n", cmd->pid,volume);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_VOLUME, &r);         
            }
            else
                log_error("[CMD_GET_VOLUME]pid[%d]failed!r=%x\n",cmd->pid,r);
        }
        if(cmd->info_cmd & CMD_GET_PLAY_STA)
        {
            int r = -1;
            r = player_get_state(pid);
            log_print("pid[%d] get player status=%d\n",cmd->pid,r);
            if(r >= 0)
            {
                log_print("pid[%d] get player status=%d\n",cmd->pid,r);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_PLAY_STA, &r);   
            }
            else
                log_error("[CMD_GET_PLAY_STA]pid[%d] failed!r=%x\n",cmd->pid, r);
        }
        if(cmd->info_cmd & CMD_GET_VOL_RANGE)
        {
            int r = -1;
            volume_range_t range;
            r = audio_get_volume_range(pid, &range.min, &range.max);
            if(r >= 0)
            {
                log_debug("pid[%d] get volume rang:%d~%d\n",cmd->pid,range.min,range.max);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_VOL_RANGE, &range);   
            }
            else
                log_error("[CMD_GET_VOL_RANGE]pid[%d] failed!r=%x\n",cmd->pid,r);
        }

        if(cmd->info_cmd & CMD_GET_CURTIME)
        {
            int r = -1;
            player_info_t info;
            r = player_get_play_info(pid, &info);
            if(r>=0)
            {
                r = info.current_time;
                log_print("pid[%d] get current time=%d\n",cmd->pid,r);
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_CURTIME, &r);    
            }
            else
                log_error("[CMD_GET_CURTIME]pid[%d] failed!r=%x\n",cmd->pid, r);
        }
        if(cmd->info_cmd & CMD_GET_DURATION)
        {
            int r = -1;
            player_info_t info;
            r = player_get_play_info(pid, &info);
            if(r >= 0)
            {
                r=info.full_time;
                if(p->controler->ret_info)
                    p->controler->ret_info(pid, cmd->cid, CMD_GET_DURATION, &r);            
            }
            else
                log_error("[CMD_GET_DURATION]pkd[%d] faild!r=%x\n",cmd->pid, r);
        }  
        if(cmd->info_cmd & CMD_GET_MEDIA_INFO)
        {
            int r = -1;
            media_info_t minfo;
            r = player_get_media_info(pid, &minfo);
            log_debug("pid[%d]get medai_info=%d\n",cmd->pid, r);
            if(r==PLAYER_SUCCESS && p->controler->ret_info)
                p->controler->ret_info(pid, cmd->cid, CMD_GET_MEDIA_INFO, &minfo);  
        }
        if(cmd->info_cmd & CMD_LIST_PID)
        {
            int r = -1;
            pid_info_t p_id;
            r = player_list_allpid(&p_id); 
            if(p->controler->ret_info)
                p->controler->ret_info(pid, cmd->cid, CMD_LIST_PID, &p_id);  
        }
        
        
        //control command
        if(cmd->ctrl_cmd > 0)
            log_print("[parse_command:%d]pid[%d]:cmd=%x param=%d\n",__LINE__,cmd->pid,cmd->ctrl_cmd,cmd->param);
        
        if(cmd->ctrl_cmd & CMD_EXIT)
    	{
    		//player_stop(pid);
            //clear_player_ctrl_para(pid, p);
            player_progress_exit();
    		ret = -1;
    	}
    	else if(cmd->ctrl_cmd & CMD_STOP)
    	{     
    	    player_stop(pid);            
            clear_player_ctrl_para(pid, p);                  
            ret = 1;
    	}
        else if(cmd->ctrl_cmd & CMD_PLAY || cmd->ctrl_cmd & CMD_PLAY_START)
        {
            thread_ctrl_para_t *t_para;  
            int ret = -1;
            
            t_para = construct_thread_para(&p->g_play_ctrl_para, p->controler);  
            if(player_register_update_callback(&t_para->play_ctrl_para.callback_fn,&update_statue,CALLBACK_INTERVAL)!=PLAYER_SUCCESS)
                log_error("[controler_run:%d]player_register_update_callback failed!\n", __LINE__); 
            
    		if(t_para)
    		{
				if(cmd->param1!=-1)
				{
					t_para->play_ctrl_para.t_pos = cmd->param1;
					log_info("[%s:%d]assigned start position: %ds\n",__FUNCTION__,__LINE__,t_para->play_ctrl_para.t_pos);
				}
                if(cmd->ctrl_cmd & CMD_PLAY)
                {
                    t_para->play_ctrl_para.need_start = 1;
                }                
        		if(NULL!=cmd->filename)    		
                {
                    if(t_para->play_ctrl_para.file_name !=NULL)
                    {
                        FREE(t_para->play_ctrl_para.file_name);
                        t_para->play_ctrl_para.file_name = NULL;
                    }                   
                    t_para->play_ctrl_para.file_name = MALLOC(strlen(cmd->filename)+1);                   
        		}                
        		strcpy(t_para->play_ctrl_para.file_name,cmd->filename);       
                log_print("[parse_command:%d]t_para=%p filename=%s\n",__LINE__,t_para,t_para->play_ctrl_para.file_name);
    			ret=player_start(&t_para->play_ctrl_para,(unsigned long)t_para); 
                if(ret < 0)
                {
                    log_error("[parse_command]start player failed, %x\n", ret);
                    if(p->controler->ret_info)            
                        p->controler->ret_info(ret, cmd->cid, CTRL_CMD_RESPONSE, NULL);   
                    FREE(t_para);
                    return ret;
                }
                t_para->pid = ret;
                if(p->controler->ret_info)            
                    p->controler->ret_info(t_para->pid, cmd->cid, CTRL_CMD_RESPONSE, NULL);             
                log_print("[%s:%d]player started! pid=%d \n",__FUNCTION__, __LINE__, t_para->pid);  
    		}
            else
                ret = -1;
		}
        else if(cmd->ctrl_cmd & CMD_START)
	    {
	        player_send_message(pid, cmd);
	    }
		else if(cmd->ctrl_cmd & CMD_PAUSE)
	    {
	        player_pause(pid);
	    }
	    else if(cmd->ctrl_cmd & CMD_RESUME)
	    {
	        player_resume(pid);
	    }    
	    else if(cmd->ctrl_cmd & CMD_SEARCH)
	    {
            int s_time = cmd->param;
	        player_timesearch(pid,s_time);
	    }
	    else if(cmd->ctrl_cmd & CMD_FF)
	    {        
            int speed = cmd->param;
	        player_forward(pid,speed);
	    }
	    else if(cmd->ctrl_cmd & CMD_FB)
	    {        
            int speed = cmd->param;
	        player_backward(pid,speed);
	    }     
	    else if(cmd->ctrl_cmd & CMD_SWITCH_AID)
	    {
            int audio_id = cmd->param;
	        player_aid(pid,audio_id);
	    }
        else if(cmd->ctrl_cmd & CMD_SWITCH_SID)
        {
            int sub_id = cmd->param;
            player_sid(pid, sub_id);
        }
	}   
    return ret;
}

#ifdef ANDROID
int controler_run_bg(global_ctrl_para_t *p)
{
    pthread_t thread;
    int rc;
    log_print("[controler_run_bg:%d]starting controler thread!!\n", __LINE__);
    rc = pthread_create(&thread, NULL, controler_run, (void*)p);
    if (rc) {
        log_print("[controler_run_bg:%d]ERROR; start failed rc=%d\n", __LINE__,rc);
    }
    return rc;
}
#endif

#ifndef ANDROID
void controler_run(global_ctrl_para_t *p)
{
#else
void * controler_run(void * para_in)
{
    global_ctrl_para_t *p = (global_ctrl_para_t*)para_in;
#endif
    player_controler_t *ctrl=p->controler;    
    int ret=0;  
    player_status pstatus;
    pid_info_t player_id;
    int stop_flag=0, error_flag = 0,end_flag = 0;
    int quitctrl = 0;
    
    int i;
    int pid;
    thread_ctrl_para_t *t_para; 
    
    log_print("[controler_run]enter\n");
    if(ctrl->init)
    {
    	ret=ctrl->init(p);
    	if(ret<0)
    	{
    		log_print("[controler_run:%d]control init failed,%x\n",__LINE__,ret);
    		return NULL;
    	}		
    }
    
    if(p->g_play_ctrl_para.file_name!=NULL)
    {
        t_para = construct_thread_para(&p->g_play_ctrl_para,p->controler);  
        if(player_register_update_callback(&t_para->play_ctrl_para.callback_fn,&update_statue,CALLBACK_INTERVAL)!=PLAYER_SUCCESS)
            log_error("[controler_run:%d]player_register_update_callback failed!\n", __LINE__);  
        log_print("[%s:%d]t_para=%p\n",__FUNCTION__, __LINE__, t_para);
		
        if(t_para)
        {
			if(p->g_play_ctrl_para.t_pos!=-1)
			{
				t_para->play_ctrl_para.t_pos = p->g_play_ctrl_para.t_pos;
				log_info("[%s:%d]assigned start position: %ds\n",__FUNCTION__,__LINE__,t_para->play_ctrl_para.t_pos);
			}
            pid=player_start(&t_para->play_ctrl_para,(unsigned long)t_para); 
            if(pid < 0)
            {
                log_error("[controler_run:%d]start player failed, %x\n",__LINE__,pid);
                FREE(t_para);
                goto exit;
            }          
            t_para->pid = pid;                      
        }
        else
            goto exit;
    }
    
	do
    {      
		player_cmd_t cmd;
		cmd.ctrl_cmd = 0;
        cmd.set_mode = 0;
        cmd.info_cmd = 0;  
        
		ret = get_command(p,&cmd);           
		if(ret==0)
		{
            if(cmd.ctrl_cmd!=0 || cmd.info_cmd!=0 ||cmd.set_mode!=0)
                log_print("\n**********[get_command:%d]pid=%d",__LINE__,cmd.pid);
            if(cmd.ctrl_cmd!=0)
            {
                log_print("ctrl_cmd=0x%x ",cmd.ctrl_cmd);
                if((cmd.ctrl_cmd & CMD_PLAY) || (cmd.ctrl_cmd & CMD_PLAY_START))
                    log_print("filename=%s ",cmd.filename);
                else
                    log_print("param=%d %d ", cmd.param,cmd.param1);
            }
            if(cmd.info_cmd!=0) log_print("info_cmd=0x%x ",cmd.info_cmd);
            if(cmd.set_mode!=0) log_print("set_mode=0x%x ",cmd.set_mode);            
            log_print("\n");

            ret = parse_command(p,&cmd);     
    		if(ret<0)
            {
                if(p->background && ret == INVALID_CMD)                                                        
                    log_print("pid[%d]:invalid command under current player state\n",cmd.pid);    
                else
                {
                    log_print("[controler_run:%d]pid[%d]:parse command return %d,break from loop!\n",__LINE__,cmd.pid,ret);
                    break;
                }
    		}            
        }
		else if(ret == 0x5a5a)
		{
            log_print("[controler_run:%d]pid[%d]:0x5a5a,break from loop!\n",__LINE__,cmd.pid);
            break;     
		}
        else if (ret == 0xbad)
        {
            log_print("[controler_run:%d]quitctrl=1",__LINE__);
            quitctrl = 1;
        }

        ret = player_list_allpid(&player_id);    
        //log_print("[controler_run:%d]get_player_state=%d i=%d pid=%d ret=%d num=%d\n",__LINE__,pstatus,i,pid,ret,player_id.num);

        int loop_flag=0;
        //check all player_thread player status            
        for(i = 0; i < player_id.num; i++)
        {
            ret = 0;
            pid = player_id.pid[i]; 
            pstatus = player_get_state(pid);  
            t_para = (void *)player_get_extern_priv(pid);
        	if(t_para!=NULL)
        	{
                loop_flag = t_para->play_ctrl_para.loop_mode;
            }
            ret = (pstatus==PLAYER_STOPED)+ (pstatus==PLAYER_ERROR) +
                  (pstatus==PLAYER_PLAYEND && (!loop_flag));
            
            if(pstatus==PLAYER_STOPED)             
                stop_flag ++;   
            else if(pstatus==PLAYER_ERROR)    
                error_flag ++;
            else if(pstatus==PLAYER_PLAYEND && (!loop_flag))             
                end_flag ++;     
            
            if(ret || quitctrl)
            {
                log_print("[controler_run:%d]get_player_state=%d i=%d pid=%d ret=%d num=%d\n",__LINE__,pstatus,i,pid,ret,player_id.num);
                player_exit(pid);
                destruct_thread_para(t_para);                
                log_print("[controler_run:%d]bg=%d\n",__LINE__,p->background);
            }
        }
        if (quitctrl) {
            log_print("[controler_run] quitctrl==1, quitting\n");
            break;
        }
		#ifndef ANDROID
        if(p->background)
            continue;
        else 
		#endif
		if((stop_flag+error_flag + end_flag) == player_id.num && player_id.num > 0)
        {
            log_print("[controler_run:%d]stop=%d error=%d end=%d num=%d\n",__LINE__,stop_flag,error_flag,end_flag,player_id.num);
            break;  /*exit when is  not backmode*/
        }
    }while(1);
exit:	if(ctrl->release)
		    ctrl->release(p);
#ifdef ANDROID
    player_progress_exit(); //?
    log_print("[controler_run:%d]thread exiting\n", __LINE__);
    return NULL;
#endif
}


