#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <codec.h>
#include <player_type.h>
#include <player_error.h>
#include <message.h>


int 	player_init();
int     player_start(play_control_t *p,unsigned long  priv);
int 	player_stop(int pid);
int     player_exit(int pid);
int 	player_pause(int pid);
int	 	player_resume(int pid);
int 	player_timesearch(int pid,int s_time);
int     player_forward(int pid,int speed);
int     player_backward(int pid,int speed);
int     player_aid(int pid,int audio_id);
int     player_sid(int pid,int sub_id);
int 	player_progress_exit(void);
int     player_list_allpid(pid_info_t *pid);
int     check_pid_valid(int pid);
player_status 	player_get_state(int pid);
int 			player_get_play_info(int pid,player_info_t *info);
int 			player_get_media_info(int pid,media_info_t *minfo);
unsigned long 	player_get_extern_priv(int pid);


int audio_set_mute(int pid,int mute);
int audio_get_volume_range(int pid,int *min,int *max);
int audio_set_volume(int pid,int val);
int audio_get_volume(int pid);
int audio_set_volume_balance(int pid,int balance);
int audio_swap_left_right(int pid);
int audio_left_mono(int pid);
int audio_right_mono(int pid);
int audio_stereo(int pid);
int audio_set_spectrum_switch(int pid,int isStart,int interval);
int player_register_update_callback(callback_t *cb,update_state_fun_t up_fn,int interval_s);

/*return can free cmd*/
int player_send_message(int pid,player_cmd_t *cmd);

//control interface
int         player_loop(int pid);
int         player_noloop(int pid);

#endif

