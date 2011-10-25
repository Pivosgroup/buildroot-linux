#ifndef  CONTROL_H_SSSSSSS
#define	CONTROL_H_SSSSSSS

#include <player_type.h>
#include <message.h>
#include <player_ctrl.h>

typedef struct 
{
    int pid;
    play_control_t play_ctrl_para;
    struct player_controler *controler;
}thread_ctrl_para_t;

struct player_controler;
typedef struct control_para
{

	//int     argc;
	//char    *argv;	
    int	  	background; //running as daemon
	char 	control_mode[16];
	char    *socket_path;
	char	*socket_rsp_path;
    play_control_t g_play_ctrl_para;	
	struct player_controler *controler;
	void* externlibfd[10];
}global_ctrl_para_t;

typedef struct player_controler
{
		char name[16];
		int front_mode;
		int (*init)(struct control_para *); 
		int (*get_command)(player_cmd_t *);
		int (*update_state)(int pid,player_info_t*);
        int (*ret_info)(int pid, int cid, int type, void *data);
		int (*release)(struct control_para *); 
}player_controler_t;

#define IS_MODE(s,mode) (strlen(s)==strlen(mode) && !memcmp(s,mode,strlen(s)))
void basic_controlers_init(void);
#ifdef ANDROID
int controler_run_bg(struct control_para *p);
void * controler_run(void * para_in);
#else
void controler_run(struct control_para *p);
#endif
int start_controler(struct control_para *player_para);
int register_controler(player_controler_t *controler);

#endif

