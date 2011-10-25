#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define AMADECD_SOCKET_NAME "/tmp/amadec_socket"

int post_command(int socket,char *cmd,int waiting_ack)
{
    int len,rlen;
   int cmd_len;
    cmd_len=strlen(cmd);		
    len = cmd_len+ 1;
    write(socket, &len, sizeof(len));
    write(socket, cmd, len);
    if(waiting_ack)
    	{
			char *rcmd;
			rlen=read(socket,&len,sizeof(len));
			if(rlen<=0)
			{
				printf("post command failed\n");
				return -1;
			}
			rcmd=malloc(rlen);
			if(rcmd==NULL)
			{
				printf("post command,amloc memory error!\n");
				return -1;
			}
			len=read(socket,rcmd,rlen);
			if(len<=0)
				return -1;
			if(memcmp(cmd,rcmd,cmd_len)==0)
			{
				if(memcmp(rcmd+cmd_len,".ack",strlen(".ack")))
				{
					printf("command ok,%s\n",cmd);
					return 0;
				}
				if(memcmp(rcmd+cmd_len,".unknow",strlen(".unknow")))
				{
					printf("amadecd can't support this command,[%s]\n",cmd);
					return -2;
				}
			}
			else
			{

			}
    	}
}

int main(int argc, char * argv[])
{
    int socket_fd;
    struct sockaddr_un name;
    int i, len;

    socket_fd = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Error: can not create socket, (%s)\n", strerror(errno));
        return -1;
    }

    name.sun_family = AF_LOCAL;
    strncpy(name.sun_path, AMADECD_SOCKET_NAME,
            sizeof(name.sun_path) - 1);

    if (connect(socket_fd, (struct sockaddr *)&name,
            SUN_LEN(&name)) != 0) {
        printf("Error: can not connect socket, (%s)\n", strerror(errno));
        close(socket_fd);
        return -2;
    }

    for (i = 1; i < argc; i++) {
		post_command(socket_fd,argv[i],len);
    }

    close(socket_fd);
    return 0;
}
