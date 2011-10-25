#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#include <stdio.h>
#include <codec.h>
#include <player_ctrl.h>

#include <log_print.h>
#include <controler.h>

#include "socket_base.h"
#include <unistd.h>




#define SOCKET_MAX_CMD 1024


int init_server_socket(char * sname)
{
	int socket_fd = -1;
	struct sockaddr_un name;

	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		log_print("daemon_palyer socket creation failed\n");
		return -1;
	}
	unlink(sname);
	memset(&name,0,sizeof(name));
	name.sun_family = AF_LOCAL;
	strncpy(name.sun_path, sname, sizeof(name.sun_path) - 1);
	if (bind(socket_fd, (struct sockaddr *)&name, SUN_LEN(&name)) < 0) {
		log_print("daemon_player socket bind failed \n");
		return -1;
	}
    if(listen(socket_fd, 5)!=0)
	{
		
		close(socket_fd);
		return -1;
	}
	return socket_fd;
}


static int  connect_response_socket(char *socketname)
{
	int socket_fd;
	struct sockaddr_un name;
	socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		log_print("Error: can not create socket, (%s)\n", strerror(errno));
		return -1;
	} 
	name.sun_family = AF_LOCAL;    
	strncpy(name.sun_path, socketname,sizeof(name.sun_path) - 1);
	if (connect(socket_fd, (struct sockaddr *)&name,
		SUN_LEN(&name)) != 0) {
		log_print("Error: can not connect socket, (%s)\n", strerror(errno));
		close(socket_fd);
		return -1;
	}
	return socket_fd;
}


int waiting_connect(int socket)
{
	struct sockaddr_un client_name;
	socklen_t client_name_len;
	int client_socket_fd;
	client_socket_fd = accept(socket, (struct sockaddr *)&client_name, &client_name_len);
	return client_socket_fd;
}
int socket_free_command(socket_cmd_t *scmd )
{
	if(scmd && scmd->cmd)
		{
		FREE(scmd->cmd);
		return 0;
		}
	return -1;
}
int socket_read_command(int socket,socket_cmd_t *scmd,int wait_msec)
{
	int result;
	int len;
	char *s;
	fd_set set; 
	struct timeval tv;   
	tv.tv_sec= wait_msec / 1000;
	tv.tv_usec = (wait_msec - tv.tv_sec * 1000) * 1000;    
	FD_ZERO(&set);
	FD_SET(socket, &set);
	result = select(socket+1, &set, NULL, NULL, &tv); 
	if(result<0)
	{
        log_print("socket read command failed:socket is closed!\n");
		return -1;/*socket is closed*/
	}
	if(result==0)
	{        
	    return 0;/*no received data*/
	}
	if(!FD_ISSET(socket,&set))
		return 0;
	result= read(socket, &len, sizeof(len));
	if(result>0 &&len>0 && len <SOCKET_MAX_CMD)
	{
    	s=MALLOC(len+1);
    	if(!s)
    	{
        	log_print("not enough memory for %s:socket\n",__FUNCTION__);
        	return -2;
    	}
	}
	else if(result == 0)
	{
        //log_print("socket disconnect,result=%d,[%s]:socket\n",result,__FUNCTION__);
    	return -3;
    }
    else
	{        
    	log_print("socket read failed len %d ,result=%d,[%s]:socket\n",len,result,__FUNCTION__);
    	return -4;
	}
	result= read(socket,s, len);
	if(result<=0)
	{
    	log_print("socket read failed cmd %d ,result=%d,[%s]:socket\n",len,result,__FUNCTION__);
    	return -5;
	}
	scmd->len=len;
	scmd->cmd=s;
	return 1;
}
int socket_send_command(char *sname,socket_cmd_t *scmd)
{
	int socket;
	if(scmd->len<=0 || !scmd->cmd)
		return -1;
	socket=connect_response_socket(sname);
	if(socket<0)
		{
		log_print("response socket connect failed,%d\n",socket);
		return -1;
		}
    write(socket, &scmd->len, sizeof(scmd->len));
    write(socket, scmd->cmd,scmd->len);
    close(socket);
	return 0;
}


