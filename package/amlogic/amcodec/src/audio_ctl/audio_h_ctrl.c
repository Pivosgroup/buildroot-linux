#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "audio_h_ctrl.h"
#include <pthread.h>
#undef CODEC_PRINT
#define CODEC_PRINT(fmt,arg...) printf(fmt,##arg)
static pthread_mutex_t command_mutex=PTHREAD_MUTEX_INITIALIZER;


static int post_command(int socket,char *cmd,int waiting_ack,int *status)
{
    int len,rlen;
    int cmd_len;
	int rc;
	
    cmd_len=strlen(cmd);		
    len = cmd_len+ 1;
    rc=write(socket, &len, sizeof(len));
	if(rc<=0)
		return -1;
    rc=write(socket, cmd, len);
	if(rc<=0)
		return -1;
    if(waiting_ack)
    	{
    		fd_set set; 
			char *rcmd;
			struct timeval tv;   
			
			tv.tv_sec=5;
			tv.tv_usec = 0;    
			FD_ZERO(&set);
			FD_SET(socket, &set);
			rc = select(socket+1, &set, NULL, NULL, &tv); 
			if (rc < 0 || !FD_ISSET(socket,&set))   //error
				{
				CODEC_PRINT("Get command ack error\n");
				return -1;
				}
			rlen=read(socket,&len,sizeof(len));
			if(rlen<=0)
			{
				CODEC_PRINT("post command failed\n");
				return -1;
			}
			if(len<0 || len >1024)
				return -1; 
			rcmd=malloc(len+2);
			if(rcmd==NULL)
			{
				CODEC_PRINT("post command,amloc memory error!\n");
				return -1;
			}
			
			rlen=read(socket,rcmd,len);
			if(rlen<=0)
				{
				free(rcmd);
				return -1;
				}
			rcmd[rlen]='\0'; 
			if(memcmp(cmd,rcmd,cmd_len)==0)
			{
				sscanf(rcmd+cmd_len,".ack:%d",status); 
				return 0;
			}
			else
			{

			}
			free(rcmd);
    	}
}


int audio_trans_cmd(char * cmd)
{
    int socket_fd;
    struct sockaddr_un name;
    int i, len;
	int status=-1;
	CODEC_PRINT("audio_trans_cmd  command=%s\n",cmd);
	pthread_mutex_lock(&command_mutex);	
    socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        CODEC_PRINT("Error: can not create socket, (%s)\n", strerror(errno));
        return -1;
    }

    name.sun_family = AF_LOCAL;
    strncpy(name.sun_path, AMADECD_SOCKET_NAME,
            sizeof(name.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr *)&name,
            SUN_LEN(&name)) != 0) {
        CODEC_PRINT("Error: can not connect socket, (%s)\n", strerror(errno));
        close(socket_fd);
        return -2;
    }
    post_command(socket_fd,cmd,1,&status);
    close(socket_fd);
	pthread_mutex_unlock(&command_mutex);	
    return status;
}




