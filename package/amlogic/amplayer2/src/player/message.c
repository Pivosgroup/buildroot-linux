#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h> 
#include <time.h> 
#include <player.h>

#include "player_priv.h"
#include "player_update.h"
#include "thread_mgt.h"


int message_pool_init(play_para_t *para)
{	
	message_pool_t *pool=&para->message_pool;
	pool->message_num=0;
	pool->message_in_index=0;
	pool->message_out_index=0;
	MEMSET(pool->message_list,0,sizeof(pool->message_list));
	pthread_mutex_init(&pool->msg_mutex,NULL);	
	return 0;
}
player_cmd_t * message_alloc(void)
{
	player_cmd_t *cmd;
	cmd=MALLOC(sizeof(player_cmd_t));
	if(cmd)
	{
	    MEMSET(cmd, 0, sizeof(player_cmd_t));
	}
	return cmd;
}

int message_free(player_cmd_t * cmd)
{
	if(cmd)
		FREE(cmd);
	cmd = NULL;
	return 0;
}


int send_message(play_para_t *para,player_cmd_t *cmd)
{
	int ret=-1;
	message_pool_t *pool=&para->message_pool;
	//log_print("[send_message:%d]num=%d in_idx=%d out_idx=%d\n",__LINE__,pool->message_num,pool->message_in_index,pool->message_out_index);
	pthread_mutex_lock(&pool->msg_mutex);
	 if(pool->message_num<MESSAGE_MAX){        
	 	pool->message_list[pool->message_in_index]=cmd;
		pool->message_in_index=(pool->message_in_index+1)%MESSAGE_MAX;
		pool->message_num++;
		wakeup_player_thread(para);
		ret=0;
	 	}
	 else
	 	{
	 	/*message_num is full*/
		player_cmd_t *oldestcmd;
		oldestcmd=pool->message_list[pool->message_in_index];
		FREE(oldestcmd);
		pool->message_out_index=(pool->message_out_index+1)%MESSAGE_MAX;/*del the oldest command*/
		pool->message_list[pool->message_in_index]=cmd;
		pool->message_in_index=(pool->message_in_index+1)%MESSAGE_MAX;
		wakeup_player_thread(para);
		ret=0;
	 	}
	 pthread_mutex_unlock(&pool->msg_mutex);
     log_debug1("[send_message:%d]num=%d in_idx=%d out_idx=%d cmd=%x mode=%d\n",__LINE__,pool->message_num,pool->message_in_index,pool->message_out_index,cmd->ctrl_cmd, cmd->set_mode);
	return ret;
	
}

int send_message_by_pid(int pid,player_cmd_t *cmd)
 {	
 	int ret;
 	play_para_t *player_para;
	player_para=player_open_pid_data(pid);
	if(player_para==NULL)
		return PLAYER_NOT_VALID_PID;
	ret=send_message(player_para,cmd);
	player_close_pid_data(pid);
	return ret;
 }

player_cmd_t * get_message(play_para_t *para)
{
	player_cmd_t *cmd=NULL;
	message_pool_t *pool=&para->message_pool;
    if(pool == NULL)
    {
        log_error("[get_message]pool is null!\n");
        return NULL;
    }    
	pthread_mutex_lock(&pool->msg_mutex);
    //log_print("[get_message]pool=%p msg_num=%d\n",pool,pool->message_num);
	 if(pool->message_num>0){
	 	
	 	cmd=pool->message_list[pool->message_out_index];
		pool->message_out_index=(pool->message_out_index+1)%MESSAGE_MAX;
		pool->message_num--;
        log_print("[get_message:%d]num=%d in_idx=%d out_idx=%d cmd=%x\n",__LINE__,pool->message_num,pool->message_in_index,pool->message_out_index,cmd->ctrl_cmd);
	 	}
	pthread_mutex_unlock(&pool->msg_mutex);    
	return cmd;
}
void clear_all_message(play_para_t *para)
{
	player_cmd_t *cmd;
	while((cmd=get_message(para))!=NULL)
		{
		message_free(cmd);
		}
}
int register_update_callback(callback_t *cb,update_state_fun_t up_fn,int interval_s)
{	
	if(up_fn!=NULL)
		cb->update_statue_callback=up_fn;
	if(interval_s>0)
		cb->update_interval=interval_s;
	return 0;
}

int update_player_states(play_para_t *para,int force)
{
	callback_t *cb = &para->update_state;
	update_state_fun_t fn;	
	fn=cb->update_statue_callback;
	if(fn!=NULL)
	{  	
        if(check_time_interrupt(&cb->callback_old_time, cb->update_interval) || force)	    			  
	  	{              
            para->state.pts_video = get_pts_video();
            para->state.pts_pcrscr = get_pts_pcrscr();
            
	  		player_info_t state;
			MEMCPY(&state,&para->state,sizeof(state)); 
            if(force == 1)
                log_print("**[update_player_states]pid:%d status=%d(last:%d) error=%x\n",para->player_id,state.status,state.last_sta,(-state.error_no));
	  		fn(para->player_id,&state); 
            para->state.error_no = 0;
	  	}         
	}
    else
        log_print("fn NULL!!!!!!!!!!!!!!!!!\n");
	return 0;
}
	

