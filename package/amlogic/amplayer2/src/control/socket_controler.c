#include <stdio.h>
#include <player.h>
#include <player_ctrl.h>
#include <log_print.h>
#include <controler.h>
#include "socket_base.h"

#define LOG_FN "/tmp/amplayer.log"
#define PLAYER_SOCKET_NAME "/tmp/player_socket"
#define PLAYER_RESPONSE_MSG "/tmp/player_response"
#define CMDNUMS  6

int server_socket=-1;
int client_socket=-1;

#define play_command_fmt1	"play:file"
#define play_command_fmt2	"play::file_list_file"
#define play_command_fmt3	"play:::file:file:..."

typedef struct {
    const char *command;
    int (*handler)(player_cmd_t *, socket_cmd_t *);
} cmd_table_t;

static int socket_send_cmd(socket_cmd_t *s_cmd)
{    
	if(s_cmd->len<=0 || !s_cmd->cmd)
		return -1;
    
	while(client_socket<0)
	{
		client_socket=waiting_connect(server_socket);
	}  
   
	if(write(client_socket, &s_cmd->len, sizeof(s_cmd->len))<0)
	{
        close(client_socket);
        client_socket = -1;
        return -1;
    }
   
    if(write(client_socket, &s_cmd->cmd,s_cmd->len)<0)
    {
        close(client_socket);
        client_socket = -1;
        return -1;
    }
   
    return 0;
}

static int socket_play(player_cmd_t *cmd, socket_cmd_t *scmd)
{
	char * ptmp;
	if( !(ptmp = strchr(scmd->cmd, ':'))) {
			log_print("no input file name!\n");
			return -1;
		}
	if(ptmp[0]==':')
		{
		/*file*/
		cmd->ctrl_cmd = CMD_PLAY;
		cmd->filename=MALLOC(strlen(ptmp));
		if(cmd->filename==NULL)
			return -1;
		strcpy(cmd->filename,ptmp+1);
		}
	return 0;
}
static int socket_stop(player_cmd_t * cmd, socket_cmd_t * scmd)
{
	cmd->ctrl_cmd = CMD_STOP;
	return 0;
}
static int socket_pause(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	cmd->ctrl_cmd = CMD_PAUSE;
	return 0;
}
static int socket_resume(player_cmd_t *cmd, socket_cmd_t *scmd)
{
	cmd->ctrl_cmd = CMD_RESUME;
	return 0;
}
static int socket_quit(player_cmd_t * cmd,socket_cmd_t * scmd)
{
	cmd->ctrl_cmd = CMD_EXIT;
	return 0;
}
static int socket_loop(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	cmd->set_mode = CMD_LOOP;   
	return 0;
}
static int socket_noloop(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	cmd->set_mode = CMD_NOLOOP;   
	return 0;
}
static int socket_clear_blackout(player_cmd_t * cmd,socket_cmd_t * scmd)
{
	cmd->set_mode = CMD_NOBLACK;   
	return 0;
}
static int socket_set_blackout(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	cmd->set_mode = CMD_BLACKOUT;   
	return 0;
}
static int socket_serach_time(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	char *ptmp;    
	if( !(ptmp = strchr(scmd->cmd, ':'))) {
			log_print("no input time point!\n");
			return -1;
		}
	ptmp++;
	cmd->ctrl_cmd = CMD_SEARCH;
	cmd->param=atol(ptmp);
	return 0;
}
static int socket_fast_forward(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	char *ptmp;    
	if( !(ptmp = strchr(scmd->cmd, ':'))) {
			log_print("no input fast forward step!\n");
			return -1;
		}
	ptmp++;
	cmd->ctrl_cmd = CMD_FF;
	cmd->param=atol(ptmp);    
	return 0;
}
static int socket_fast_backward(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	char *ptmp;    
	if( !(ptmp = strchr(scmd->cmd, ':'))) {
			log_print("no input fast rewind step!\n");
			return -1;
		}
    ptmp++;
	cmd->ctrl_cmd = CMD_FB;
	cmd->param=atol(ptmp);
	return 0;
}

static int socket_set_novideo(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	cmd->set_mode = CMD_NOVIDEO;   
	return 0;
}

static int socket_set_noaudio(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	cmd->set_mode = CMD_NOAUDIO;   
	return 0;
}

static int socket_mute(player_cmd_t *cmd,socket_cmd_t *scmd)
{
    cmd->set_mode = CMD_MUTE;
	return 0;
}

static int socket_unmute(player_cmd_t *cmd,socket_cmd_t *scmd)
{
    cmd->set_mode = CMD_UNMUTE;	
	return 0;
}

static int socket_volset(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	char *p;
	int vol;
	p = strchr(scmd->cmd, ':');
	if(p != NULL){
		p++;
		vol = atol(p);
		cmd->set_mode = CMD_SET_VOLUME;
        cmd->param = vol;
	}
	else{
		log_print("error command for set vol(fmt:\"V:value\")\n");
	}
	return 0;
}

static int socket_volget(player_cmd_t *cmd,socket_cmd_t *scmd)
{
    #if 0
	int value;
	socket_cmd_t s_cmd;
	s_cmd.cmd = MALLOC(32);
	value = audio_get_volume(0);
	sprintf(s_cmd.cmd, "vol:%d", value);
	log_print("[socket_send_info]cmd=%s\n",s_cmd.cmd);
	s_cmd.len = strlen(s_cmd.cmd)+1;
	socket_send_cmd(&s_cmd);  
	FREE(s_cmd.cmd);
	#else
    cmd->set_mode = CMD_GET_VOLUME;    
    #endif
	return 0;
}

static int socket_send_info(player_cmd_t *cmd,socket_cmd_t *scmd)
{
	socket_cmd_t s_cmd;
    player_info_t player_info;
    char *ptmp;    
    int pid = -1;
    
	if( !(ptmp = strchr(scmd->cmd, ':'))) {
			log_print("no input pid!\n");
			return -1;
		}
	ptmp++;    
    pid = atol(ptmp);    
	player_get_play_info(pid,&player_info);       
    s_cmd.cmd = MALLOC(128);
    sprintf(s_cmd.cmd,"status:%d:fulltime:%d:curtime:%d",player_info.status,player_info.full_time,player_info.current_time);
    log_print("[socket_send_info]cmd=%s\n",s_cmd.cmd);
    s_cmd.len = strlen(s_cmd.cmd)+1;         
	socket_send_cmd(&s_cmd);    
    FREE(s_cmd.cmd);
	return 0;
}
static int socket_send_media(player_cmd_t *cmd,socket_cmd_t *scmd)
{
    #if 0
    socket_cmd_t s_cmd;    
    media_info_t info;
	player_get_media_info(0,&info);
    s_cmd.cmd = MALLOC(128);
    sprintf(s_cmd.cmd,"media:aspect_ratio:%d:bit_rate:%d:width:%d:height:%d", 
                       info.video_info.aspect_ratio,
                       info.stream_info.bitrate,
                       info.video_info.video_width,
                       info.video_info.video_height);
    log_print("[socket_send_info]cmd=%s\n",s_cmd.cmd);
    s_cmd.len = strlen(s_cmd.cmd)+1;      
	socket_send_cmd(&s_cmd);    
    FREE(s_cmd.cmd);
    #endif
    return 0;
}

static int socket_ret_info(int pid, int cid, int type, void *data)
{
    socket_cmd_t s_cmd;    
    s_cmd.cmd = MALLOC(64);
    if(type == CMD_GET_VOL_RANGE)
    {
        volume_range_t *range=(volume_range_t *)data;
        sprintf(s_cmd.cmd,"pid:%d:cid:%d:cmd:%x:min:%d:max:%d", pid, cid, type,range->min, range->max);
    }
    else
    {
        int *value=(int *)data;
        sprintf(s_cmd.cmd,"pid:%d:cid:%d:cmd:%x:value:%d", pid, cid, type, *value);
    }
    log_print("[socket_send_info]cmd=%s\n",s_cmd.cmd);
    s_cmd.len = strlen(s_cmd.cmd)+1;      
	socket_send_cmd(&s_cmd);    
    FREE(s_cmd.cmd);
    return 0;
}

cmd_table_t command_table[]=
{
    {
        .command = "stop",
        .handler = socket_stop,
    },
    {
        .command = "play",
        .handler = socket_play,
    },
    {
        .command = "pause",
        .handler = socket_pause,
    },
    {
    	 .command = "resume",
	    .handler = socket_resume, 
    },
    {
        .command = "quit",
        .handler = socket_quit,
    },
    {
        .command = "search",
	    .handler = socket_serach_time,
    },
    {
        .command = "info",
	    .handler = socket_send_info,
    },
    {
        .command = "media",
	    .handler = socket_send_media,
    },
    {
        .command = "loop",
	    .handler = socket_loop,
    },
    {
        .command = "noloop",
	    .handler = socket_noloop,
    },
    {
        .command = "noblack",
	    .handler = socket_clear_blackout,
    },
    {
        .command = "blackout",
	    .handler = socket_set_blackout,
    },  
    {
        .command = "fb",
        .handler = socket_fast_backward,
    },
    {
        .command = "ff",
        .handler = socket_fast_forward,        
    },
    {
        .command = "novideo",
        .handler = socket_set_novideo,        
    },
    {
        .command = "noaudio",
        .handler = socket_set_noaudio,       
    },
    {
        .command = "mute",
    	.handler = socket_mute,
    },
    {
        .command = "unmute",
    	.handler = socket_unmute,
    },
    {
        .command = "volset",
    	.handler = socket_volset,
    },
    {
        .command = "volget",
    	.handler = socket_volget,
    }
};


static int socket_parser_cmd(player_cmd_t *playercmd,socket_cmd_t *scmd)
{
	cmd_table_t *cmd_p;
	int i;
	int ret;
	log_print("get command string=%d,%s\n",scmd->len,scmd->cmd);
	for(i=0;i<sizeof(command_table)/sizeof(cmd_table_t);i++)
		{
		cmd_p=&command_table[i];		
		if(!memcmp(scmd->cmd,cmd_p->command,strlen(cmd_p->command)))		
			{            
			ret=cmd_p->handler(playercmd,scmd);
			goto arc_cmd_ok;
			}
		}
	
	return -1;
arc_cmd_ok:	
	return ret;
}

static int socket_controler_init(global_ctrl_para_t *ctrl)
{
	if(!ctrl->socket_path)
	{
        ctrl->socket_path= PLAYER_SOCKET_NAME;        
	}
	if(!ctrl->socket_rsp_path)
	{
        ctrl->socket_rsp_path= PLAYER_RESPONSE_MSG;
	}   
	server_socket=init_server_socket(ctrl->socket_path);
	if(server_socket<0)
		{
		log_print("init_server_socket failed\n");
		return -1;
		}
	return 0;
}

static int socket_controler_release(global_ctrl_para_t *ctrl)
{
	if(client_socket>0)
		close(client_socket);
	if(server_socket>0)
		close(server_socket);
	if(ctrl->socket_path)
		unlink(ctrl->socket_path);
	return 0;
}

static int socket_get_command(player_cmd_t *playercmd)
{
	socket_cmd_t scmd;
	int ret;
	int wait=1000;   //ms
    MEMSET(&scmd,0,sizeof(socket_cmd_t));
	while(client_socket<0)
	{
		client_socket=waiting_connect(server_socket);
	}    
	ret=socket_read_command(client_socket,&scmd,wait);
	if(ret<0)
	{
        close(client_socket);
		client_socket=-1;      
		return ret;
	}
	else if(ret==0)
	{
		/*time out*/
		return  -1;
	}    
	ret=socket_parser_cmd(playercmd,&scmd);
	socket_free_command(&scmd);
	return ret;
}
static char *sta_convert2_string(int pid, player_info_t *player_state)
{
    player_status status = player_state->status;
    switch(status)
    {
        case PLAYER_STOPED:
            return "stopped";
	    case PLAYER_RUNNING:
            return "running";
        case PLAYER_PAUSE:
            return "pause";
        case PLAYER_SEARCHING:
            return "searching";
        case PLAYER_SEARCHOK:
            return "searchOK";
        case PLAYER_INITING:
            return "initing";
        case PLAYER_ERROR:
        {
            if(player_state->error_no < 0)              
                return player_error_msg(player_state->error_no);            
            else 
                return "error";
            
        }
        case PLAYER_PLAYEND:
            return "playend";
        case PLAYER_START:
            return "start";
        default:
            return "unknown_status";
    }
}

static int socket_update_state(int pid, player_info_t *player_state)
{    
    int ret = 0;
    socket_cmd_t s_cmd;      
    
    if(player_state->last_sta != player_state->status)
    {               
        s_cmd.cmd = MALLOC(128);         
        sprintf(s_cmd.cmd,"%s",sta_convert2_string(pid,player_state));         
        log_print("[socket_update_state]last=%d curr=%d message---%s\n",player_state->last_sta,player_state->status,s_cmd.cmd);
        s_cmd.len = strlen(s_cmd.cmd)+1;   
    	ret = socket_send_command(PLAYER_RESPONSE_MSG,&s_cmd);
        FREE(s_cmd.cmd);   
        player_state->last_sta = player_state->status;        
    }    
    return ret;
}
player_controler_t socket_ctrl=
{
	.name="socket",
	.front_mode=0,
	.init=socket_controler_init,
	.release=socket_controler_release,
	.get_command=socket_get_command,
	.update_state=socket_update_state,
	.ret_info=socket_ret_info,
};

int register_socket_controler(void)
{	
	register_controler(&socket_ctrl);
	return 0;
}


