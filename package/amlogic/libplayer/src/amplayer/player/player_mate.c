/********************************************
 * name : player_control.c
 * function: player's mate thread
 * date     : 2011.10.11
 * Please don't call any ffmpeg function in this thread.
 * recevice upper level cmd and get lowlevel status.
 * Send player infos to uperlevel;
 ********************************************/
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <player.h>

#include "player_priv.h"
#include "thread_mgt.h"
#include <semaphore.h>

#define mate_print(fmt,args...) 	log_debug(fmt,##args)
//#define mate_print(fmt...)

struct player_mate{
    pthread_mutex_t  pthread_mutex;
    pthread_cond_t   pthread_cond;
    pthread_t        pthread_id;
    player_status    player_state;
	sem_t   		 mate_sem;
	int 			 work_intervals;
	int 			 wake_delay;
	int 			 mate_isrunng;
	int 			 mate_should_sleep;
	int 			 mate_should_exit;
};
static int player_mate_thread_run(play_para_t *player);


void *player_mate_init(play_para_t *player,int intervals){
	pthread_t       tid;
	pthread_attr_t pthread_attr;
	struct player_mate *mate;
	int ret;
	player->player_mate=NULL;
	mate=MALLOC(sizeof(struct player_mate));
	if(!mate)
		return NULL;
	pthread_attr_init(&pthread_attr);
	pthread_attr_setstacksize(&pthread_attr, 409600);   //default joinable
	pthread_mutex_init(&mate->pthread_mutex, NULL);
	pthread_cond_init(&mate->pthread_cond, NULL);
	ret = sem_init(&mate->mate_sem,   0,   0); 
	mate->mate_isrunng=0;
	mate->mate_should_sleep=0;
	mate->mate_should_exit=0;
	mate->work_intervals=intervals;
	log_print("player mate init ok mate=%x(%d)\n",mate,sizeof(struct player_mate));
	player->player_mate=mate;
  	ret = pthread_create(&tid, &pthread_attr, (void*)&player_mate_thread_run, (void*)player);
	mate->pthread_id=tid;
	return (void *)mate;
}

int player_mate_wake(play_para_t *player,int delay)
{
	int ret;
	struct player_mate *mate=(struct player_mate *)(player->player_mate);
	if(!mate) return -1;
	if(delay>0)
		mate->wake_delay=delay;
	else
		mate->wake_delay=0;
	mate->mate_should_sleep=0;
	ret = sem_post(&mate->mate_sem);/*wake wate thread*/
	return ret;
}
int player_mate_sleep(play_para_t *player)
{
	struct player_mate *mate=(struct player_mate *)(player->player_mate);
	int ret=0;
	if(!mate) return -1;
	while(mate->mate_isrunng){
		pthread_mutex_lock(&mate->pthread_mutex);
		mate->mate_should_sleep=1;
		ret = pthread_cond_signal(&mate->pthread_cond);
		pthread_mutex_unlock(&mate->pthread_mutex);
		if(mate->mate_isrunng)
		{
			player_thread_wait(player,1000);
		}
	}
	return ret;
}
int player_mate_release(play_para_t *player)
{
	struct player_mate *mate=(struct player_mate *)(player->player_mate);
	int ret;
	if(!mate) return -1;
	mate_print("try release mate thread now\n");
	pthread_mutex_lock(&mate->pthread_mutex);
	mate->mate_should_sleep=1;
	mate->mate_should_exit=1;
	mate->wake_delay=0;
	ret = sem_post(&mate->mate_sem);/*wake mate thread*/
	ret = sem_post(&mate->mate_sem);/*wake mate thread*/
	ret = sem_post(&mate->mate_sem);/*wake mate thread*/
	ret = pthread_cond_signal(&mate->pthread_cond);
	pthread_mutex_unlock(&mate->pthread_mutex);
	mate_print("wait mate thread exit\n");
	ret = pthread_join(mate->pthread_id, NULL);
	mate_print("mate thread exited\n");
	FREE(player->player_mate);
	player->player_mate=NULL;
	return ret;
}

static int player_mate_thread_wait(struct player_mate *mate,int microseconds)
{
	struct timespec pthread_ts;
	struct timeval now;
	int ret;

	gettimeofday(&now, NULL);
	pthread_ts.tv_sec = now.tv_sec + (microseconds + now.tv_usec) / 1000000;
	pthread_ts.tv_nsec = ((microseconds + now.tv_usec) * 1000) % 1000000000;
	pthread_mutex_lock(&mate->pthread_mutex);
	ret = pthread_cond_timedwait(&mate->pthread_cond, &mate->pthread_mutex, &pthread_ts);
	pthread_mutex_unlock(&mate->pthread_mutex);
	return ret;
}


static int player_mate_thread_cmd_proxy(play_para_t *player,struct player_mate *mate)
{
	player_cmd_t *cmd=NULL;
	int ret;
	/*
	check the cmd & do for main thread;
	*/ 
	lock_message_pool(player);
	cmd=peek_message_locked(player);
	if(cmd){
		mate_print("[MATE]Get cmd-------------------------[%x],[%x]\n",cmd->ctrl_cmd,(CMD_START | CMD_PAUSE | CMD_RESUME | CMD_SWITCH_AID));
		if(((cmd->ctrl_cmd) & ( CMD_START | CMD_PAUSE | CMD_RESUME | CMD_SWITCH_AID)) || 
			((cmd->set_mode) & (CMD_LOOP | CMD_NOLOOP | CMD_EN_AUTOBUF | CMD_SET_AUTOBUF_LEV)))
			cmd=get_message_locked(player);
		else
			cmd=NULL;
	}
	unlock_message_pool(player);
	if(!cmd) return 0;/*no I can handle cmd*/
	
	if(cmd->ctrl_cmd & CMD_PAUSE ){
		mate_print("[MATE]Get puase cmd\n");
		 if (get_player_state(player) != PLAYER_PAUSE) {
            ret = codec_pause(player->codec);
            if (ret != 0) {
                log_error("[%s:%d]pause failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
            }
			player->playctrl_info.pause_flag=1;
            set_player_state(player, PLAYER_PAUSE);
            update_playing_info(player);
            update_player_states(player, 1);
        }
	}else if((cmd->ctrl_cmd & CMD_RESUME) || (cmd->ctrl_cmd &CMD_START))
	{
		mate_print("[MATE]Get resume cmd\n");
		 if (get_player_state(player) == PLAYER_PAUSE) {
            ret = codec_resume(player->codec);
            if (ret != 0) {
                log_error("[%s:%d]resume failed!ret=%d\n", __FUNCTION__, __LINE__, ret);
            }
			player->playctrl_info.pause_flag=0;
            set_player_state(player, PLAYER_RUNNING);
            update_playing_info(player);
            update_player_states(player, 1);
        }
	}else if (cmd->ctrl_cmd & CMD_SWITCH_AID) {
        player->playctrl_info.seek_base_audio = 1;
        player->playctrl_info.switch_audio_id = cmd->param;
		set_black_policy(0);
    } else if (cmd->set_mode & CMD_LOOP) {
        player->playctrl_info.loop_flag = 1;
    } else if (cmd->set_mode & CMD_NOLOOP) {
        player->playctrl_info.loop_flag = 0;
    }else if (cmd->set_mode & CMD_EN_AUTOBUF) {
        player->buffering_enable = cmd->param;        
    } else if (cmd->set_mode & CMD_SET_AUTOBUF_LEV) {
		player->buffering_threshhold_min = cmd->f_param;
		player->buffering_threshhold_middle = cmd->f_param1;
		player->buffering_threshhold_max = cmd->f_param2;    
    } 
	message_free(cmd);
	return 0;
}


static int player_mate_thread_run_ll(play_para_t *player,struct player_mate *mate)
{
	mate_print("[MATE]player's mate start do a day work now\n ");
	player_mate_thread_cmd_proxy(player,mate);/**/
	mate_print("[MATE]player's mate finixhed cmd proxy\n ");
	update_playing_info(player);
	mate_print("[MATE]player's mate finished update playing info\n ");
    update_player_states(player, 0);
	mate_print("[MATE]player's mate finished one day work now\n ");
	return 0;
}

static int player_mate_thread_run_l(play_para_t *player,struct player_mate *mate)
{
	int worktimenum=0;
	mate_print("[MATE]player's mate start work now!\n ");
	while(!mate->mate_should_sleep && !mate->mate_should_exit){
		worktimenum++;
		player_mate_thread_run_ll(player,mate);
		mate_print("[MATE]player's mate sleep now %d\n ",worktimenum);
		player_mate_thread_wait(mate,mate->work_intervals);
		mate_print("[MATE]player's mate wake and try get next day work\n ");
	}
	mate_print("[MATE]player's mate exit work now!\n ");
	return 0;
}

static int player_mate_thread_run(play_para_t *player)
{
	struct player_mate *mate=player->player_mate;
	while(!mate->mate_should_exit){
		sem_wait(&mate->mate_sem);/*wait main thread to do heavy works*/
		mate->mate_isrunng=1;
		if(mate->wake_delay>0)
			player_mate_thread_wait(mate,mate->wake_delay);/*do wait,if main thread may wake fast*/
		mate->wake_delay=0;
		if(!mate->mate_should_sleep)	
			player_mate_thread_run_l(player,mate);		
		wakeup_player_thread(player);
		mate->mate_isrunng=0;
	}
	return 0;
}

