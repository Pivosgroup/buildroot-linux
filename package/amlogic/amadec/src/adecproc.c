#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>
#include <errno.h>

#include "log.h"
#include "./include/adecproc.h"
#include "adec.h"


#define CMDNUM  15

typedef struct {
    const char *command;
    int (*handler)(char*);
} cmd_table_t;

enum {
    ADEC_STAT_IDLE = 0,
    ADEC_STAT_RUN,
    ADEC_STAT_PAUSE
};

static int cmd_start(char* cmd);
static int cmd_stop(char* cmd);
static int cmd_pause(char* cmd);
static int cmd_quit(char* cmd);
static int cmd_mute(char* cmd);
static int cmd_unmute(char* cmd);
static int cmd_volset(char* cmd);
static int cmd_volget(char* cmd);
static int cmd_left_mono(char* cmd);
static int cmd_right_mono(char* cmd);
static int cmd_stereo(char* cmd);
static int cmd_swap_chanl(char* cmd);
static int cmd_auto_mute(char* cmd);
static int cmd_spectrum_on(char* cmd);
static int cmd_spectrum_off(char* cmd);

static int adec_stat = ADEC_STAT_IDLE;
static const cmd_table_t commands[CMDNUM] =
{
    {
        .command = "stop",
        .handler = cmd_stop,
    },
    {
        .command = "start",
        .handler = cmd_start,
    },
    {
        .command = "pause",
        .handler = cmd_pause,
    },
    {
        .command = "quit",
        .handler = cmd_quit,
    },
    {
    	 .command = "mute",
	 .handler = cmd_mute,
    },
    {
    	 .command = "unmute",
	 .handler = cmd_unmute,
    },
    {
    	 .command = "volset",
	 .handler = cmd_volset,
    },
    {
    	 .command = "volget",
	 .handler = cmd_volget,
    },
    {
    	 .command = "leftmono",
	 .handler = cmd_left_mono,
    },
    {
    	 .command = "rightmono",
	 .handler = cmd_right_mono,
    },
    {
    	 .command = "stereo",
	 .handler = cmd_stereo,
    },
    {
    	 .command = "swap",
	 .handler = cmd_swap_chanl,
    },
    {
    	 .command = "automute",
	 .handler = cmd_auto_mute,
    },
    {
    	 .command = "spectrumon",
	 .handler = cmd_spectrum_on,
    },
    {
    	 .command = "spectrumoff",
	 .handler = cmd_spectrum_off,
    },
    	    
};


static int cmd_spectrum_on(char* cmd)
{
    int interval = -1;
    char* tmp;

    tmp = strchr(cmd, ':');
    if(NULL != tmp)
    {
        tmp = tmp + 1;
        interval = atoi(tmp);
        
        if(interval < 0)
        	interval = 50; 
    }
    
    if(adec_stat == ADEC_STAT_RUN)
   {    
        start_spectrum_notify_task(interval);   
        return 0;

    }    
    return -1;
}

static int cmd_spectrum_off(char* cmd)
{
    stop_spectrum_notify_task();    
    return 0;
}

static int cmd_start(char* cmd)
{
    if (adec_stat == ADEC_STAT_IDLE) {
        if (adec_start() == 0)
            adec_stat = ADEC_STAT_RUN;
    }

    if (adec_stat == ADEC_STAT_PAUSE) {
        adec_resume();
        adec_stat = ADEC_STAT_RUN;
    }

    return 0;
}

static int cmd_stop(char* cmd)
{
    if ((adec_stat == ADEC_STAT_RUN) || (adec_stat == ADEC_STAT_PAUSE)){
	if(adec_stat == ADEC_STAT_PAUSE){
		adec_resume();
		adec_reset();
	}
       	adec_stop();
    }
    adec_stat = ADEC_STAT_IDLE;
    return 0;
}

static int cmd_pause(char* cmd)
{
    if (adec_stat == ADEC_STAT_RUN)
        adec_pause();
    adec_stat = ADEC_STAT_PAUSE;
    return 0;
}

static int cmd_mute(char* cmd)
{
	sound_mute_set("switch playback mute", 0, 1, NULL);
	return 0;
}

static int cmd_unmute(char* cmd)
{
	sound_mute_set("switch playback mute", 1, 1, NULL);
	return 0;
}

static int cmd_volset(char* cmd)
{
	int vol;
	char* tmp;

	tmp = strchr(cmd, ':');
	if(!tmp)
		return 0;
	tmp = tmp + 1;
	vol = atoi(tmp);
	if(vol < 0)
		vol = 0;
	else if(vol > 255)
		vol = 255;

	sound_mute_set("Master Playback Volume" , vol, 1, NULL);
	return 0;
}

static int cmd_volget(char* cmd)
{
	int vol = 0;
	sound_mute_set("Master Playback Volume", 0, 0, &vol );
	if(vol < 0)
		vol = 0;
	else if(vol > 255)
		vol = 255;
	return vol;
}

static int cmd_left_mono(char* cmd)
{
	sound_mute_set("Playback Left Mono", 1, 1, NULL);
	return 0;
}

static int cmd_right_mono(char* cmd)
{
	sound_mute_set("Playback Right Mono", 1, 1, NULL);
	return 0;
}

static int cmd_stereo(char* cmd)
{
	sound_mute_set("Playback Stereo Out", 1, 1, NULL);
	return 0;
}

static int cmd_swap_chanl(char* cmd)
{
	sound_mute_set("Playback Swap Left Right", 1, 1, NULL);
	return 0;
}

static int cmd_auto_mute(char * cmd)
{
  	int auto_mute;
	char* tmp;

	tmp = strchr(cmd, ':');
	if(!tmp)
		return 0;
	tmp = tmp + 1;
	auto_mute = atoi(tmp);
	adec_auto_mute(auto_mute);
    
	return 0;
}

static int cmd_quit(char* cmd)
{
    cmd_stop(cmd);

    return 1;
}

static int cmd_process(char *cmd, int len)
{
    int i;

    for (i=0; i<CMDNUM; i++) {
        if (!strncmp(cmd, commands[i].command, strlen(commands[i].command))) {
            lp(LOG_INFO, "Command: %s", commands[i].command);

            return commands[i].handler(cmd);
        }
    }

    return -1;
}
int ack_command(int socket,char *cmd,int cmd_len,int status)
{
	char *pcmd;
	int len;
	int ret;
	
	if(cmd_len<1)
		return -1;
	pcmd=malloc(cmd_len+32);
	if(pcmd!=NULL)
		{
			char *ps;
			ps=strstr(cmd,":");
			if(ps!=NULL)
				cmd_len=ps-cmd;
			memcpy(pcmd,cmd,cmd_len);
			pcmd[cmd_len]='\0';
			sprintf(pcmd+cmd_len,".ack:%d\n",status);
			len=strlen(pcmd)+1;
	        ret=write(socket, &len, sizeof(len));
			if(ret!=sizeof(len))
				return -1;
	        ret=write(socket, pcmd, len);
			if(ret!=len)
				return -1;
			free(pcmd);
		}
	else
		{
		lp(LOG_INFO, "ack command error,%lx,%d\n",pcmd,cmd_len);
		return 0;
		}
	return 0;
}
static int cmd_server(int client_socket)
{ 
    while (1) {
		int len,rlen;
		char *cmd;
		int res;
	
        if (read(client_socket, &len, sizeof(len)) == 0)
            return 0;
		if(len<0 || len >1024)
			continue;
        cmd = (char *)malloc(len+2);
	
        if (cmd) 
		{
			rlen=read(client_socket, cmd, len);
			cmd[rlen]='\0';
			res=cmd_process(cmd, rlen);

			if(res>=0)
			{
				lp(LOG_INFO, "ack command,%s,%d\n",cmd,res);
				ack_command(client_socket,cmd,len-1,res);
			}
			else
			{
				lp(LOG_INFO, "unknow command,%s,%d\n",cmd,res);	
			}
			
			if(strcmp(cmd,"quit")==0) /*is exit command*/
			{
				free(cmd);
				return 1;
			}
			free(cmd);
        }
		else
		{
			lp(LOG_INFO, "ERROR,alloc memory for command failed%lx\n",cmd);
		}
    }
}

int adecd_process(struct amadec_opt *opt)
{
    int socket_fd=opt->socket_fd;

    int quit;

    lp(LOG_INFO, "adecd_process start");


    adec_init();

    listen(socket_fd, 5);
    do {
        struct sockaddr_un client_name;
        socklen_t client_name_len;
        int client_socket_fd;

        client_socket_fd = accept(socket_fd, (struct sockaddr *)&client_name,
                                  &client_name_len);
        quit = cmd_server(client_socket_fd);
        close(client_socket_fd);
    }
    while (!quit);

   lp(LOG_INFO, "adecd_process exit\n");

    return 0;
}
