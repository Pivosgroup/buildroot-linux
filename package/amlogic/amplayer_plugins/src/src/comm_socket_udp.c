#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include "comm_socket_udp.h"


#define IPC_DEFAULT_SOCKET_BUF 2*1024

int Comm_SocketIPCCreate(const char* ipc_name)
{
    int ret;
    unsigned int opt_flag = IPC_DEFAULT_SOCKET_BUF;
    unsigned int size;
    struct sockaddr_un name;
    int  fd;
    
    if(strlen(ipc_name) > IPC_UDP_NAME_MAX)
    {
        printf("too long ipc path, %d larger than %d\n",strlen(ipc_name),IPC_UDP_NAME_MAX);
        return -1;
    }
       
    fd= socket(AF_UNIX,SOCK_DGRAM, 0);
    if (fd < 0)
    {
        printf("[%s] can't create socket",__FUNCTION__);
        return -1;
    }
     
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&opt_flag, sizeof(opt_flag));

    name.sun_family = AF_UNIX;
    
    memset(name.sun_path,0,sizeof(name.sun_path));   
    
    strcpy(name.sun_path,ipc_name); 
    
    size = SUN_LEN(&name);
    
    unlink(ipc_name);
    
    ret = bind(fd, (struct sockaddr *)&name, size);
    if (ret < 0)
    {
        printf("[%s] can't bind socket,err msg:%s\n",__FUNCTION__,strerror(errno));
        close(fd);
        return -1;
    }    
    
    return fd;
}

int  Comm_SocketIPCSend(int fd,const char* dest_name,void *data,unsigned int data_len)
{
    int ret = 0;
    struct sockaddr_un to_addr;
    unsigned int size = 0;
    memset(&to_addr, 0,sizeof(struct sockaddr_un));
    to_addr.sun_family = AF_UNIX;
    strncpy(to_addr.sun_path,dest_name,strlen(dest_name));
    size = sizeof(struct sockaddr_un);
    ret = sendto(fd,data, data_len, MSG_DONTWAIT, (struct sockaddr *)&to_addr, size);
    if ( ret <= 0 )
        return -1;
    return 0;
}
int Comm_SocketIPC_FlushSend(int fd,const char* dest_name,void *data, unsigned int data_len)
{
    int ret = -1;
    unsigned int opt_flag = 0;
    struct sockaddr_un  to_addr;
    unsigned int size = 0;
    memset(&to_addr, 0,sizeof(struct sockaddr_un));
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&opt_flag, sizeof(opt_flag));
    to_addr.sun_family = AF_UNIX;
    strncpy(to_addr.sun_path, dest_name, strlen(dest_name));
    size = sizeof(struct sockaddr_un);
    ret= sendto(fd, data, data_len, 0, (struct sockaddr *)&to_addr, size);
    opt_flag= IPC_DEFAULT_SOCKET_BUF;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&opt_flag, sizeof(opt_flag));
    if ( ret <= 0 )
        return -1;
    return 0;
}
int Comm_SocketIPCRecv(int fd , void* data, unsigned int data_len, int timeout,char* from_name)
{
    int ret = -1;
    struct sockaddr_un  from;
    socklen_t fromlen = sizeof(from);
    if(from_name ==NULL)
    {
        fromlen = 0;
    }
    if (timeout != WAIT_FOREVER)
    {
        fd_set  rfds;
        struct timeval timeo;
        FD_ZERO(&rfds);
        FD_SET(fd, &rfds);
        timeo.tv_sec  = timeout / 1000;
        timeo.tv_usec = (timeout % 1000) * 1000;
        ret = select(fd+1,&rfds, NULL, NULL, &timeo);
        if (ret <= 0)
        {
            return -6;
        }
    }
   
    memset(from.sun_path,0,sizeof(from.sun_path));
    
    ret = recvfrom(fd, data, data_len, 0, (struct sockaddr *)&from,&fromlen);
    
    if(ret>0&&from_name!=NULL)
       strncpy(from_name,from.sun_path,IPC_UDP_NAME_MAX);
    else
       return -1;
    return 0;
}

void Comm_SocketIPCClose(int fd)
{
    if(fd >0)
        close(fd);
}
