#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>          /* See NOTES */
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <fcntl.h>

#include "comm_socket_tcp.h"
#include "mp_log.h"

#define CLIENT_PATH "/tmp/"
#define MAX_POLL_RETRY 3

static int _read_data(int fd, void*buf, int size, int timeo);
//static int _write_data(int fd, void *buf, int size, int retry);

int open_stream(const char* ipc_name)
{
    assert(ipc_name != NULL);
    int socket_fd_ = -1;
    struct sockaddr_un name;
    int ret = -1;
    int reuse = 1;
    socket_fd_ = socket(PF_LOCAL, SOCK_STREAM, 0);

    if (socket_fd_ < 0)
    {
        log_err("[%s]socket creation failed\n",__FILE__);
        return socket_fd_;
    }
    ret = setsockopt(socket_fd_,SOL_SOCKET,SO_REUSEADDR,(void*)&reuse,sizeof(int));
    if(ret == -1)
    {
        log_err("failed set connect socket opt,result:%s\n",strerror(errno));
        close(socket_fd_);
        return -1;
    }

    unlink(ipc_name);
    memset(&name,0,sizeof(name));
    name.sun_family = AF_LOCAL;
    strncpy(name.sun_path,ipc_name, sizeof(name.sun_path)-1);
    if (bind(socket_fd_, (struct sockaddr *)&name, SUN_LEN(&name)) < 0)
    {
        log_err("[%s]:socket bind failed \n",__FILE__);
        return -1;
    }
    if(listen(socket_fd_,5)!=0)
    {
        log_err("[%s]socket listen failed\n",__FILE__);
        close(socket_fd_);
        return -1;
    }
    return socket_fd_;
}

int close_stream(int svr_fd)
{
    if(svr_fd >0)
    {
        shutdown(svr_fd,SHUT_RDWR);
        close(svr_fd);
        return 0;
    }
    return -1;
}

int wait_for_connection(int svr_fd,int timeout,char* from_name,int*from_len)
{
    int client_fd = -1;
    struct sockaddr_un client_name;
    socklen_t client_name_len;

    fd_set  rfds;
    struct timeval timeo;

    int ret = -1;
    if(svr_fd <=0)
    {
        log_err("[%s]wrong server socket handle.%d\n",__FILE__,svr_fd);
        return -1;
    }


    client_name_len = sizeof(client_name);
    memset(client_name.sun_path,0,sizeof(client_name.sun_path));

    if (timeout != -1)
    {
        FD_ZERO(&rfds);
        FD_SET(svr_fd, &rfds);
        timeo.tv_sec  = timeout / 1000;
        timeo.tv_usec = (timeout % 1000) * 1000;
        ret = select(svr_fd +1,&rfds, NULL, NULL, &timeo);
        if(ret< 0)
        {
            log_err("[%s]socket read command failed:socket is closed!\n",__FILE__);
            return -1;/*socket is closed*/
        }
        else if(ret ==0)
        {
            log_wrn("[%s]no client connected.\n",__FUNCTION__);
            return -6;//no receive data
        }
        if(!FD_ISSET(svr_fd,&rfds))
            return -5;//no data
    }


    client_fd = accept(svr_fd, (struct sockaddr *)&client_name, &client_name_len);
    if(client_fd < 0)
    {
        log_err("[%s]Failed to accept client's connection.,%s\n",__FILE__,strerror(errno));
        return -1;
    }

    log_info("[%s]:Succeed to get a client[%s]connect.\n",__FILE__,client_name.sun_path);

    strncpy(from_name,client_name.sun_path,IPC_NAME_MAX);
    *from_len = strlen(client_name.sun_path)+1;

    return client_fd;

}
int connect_stream(const char* ipc_name,int retry,int *client_uid)
{

    struct sockaddr_un name;
    int socket_fd;    
    
    assert(ipc_name != NULL&& NULL != client_uid);

    socket_fd = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        log_err("Error: can not create socket, (%s)\n", strerror(errno));
        return -1;
    }


    /* fill socket address structure with our address */
    memset(&name, 0, sizeof(name));    
    name.sun_family = AF_LOCAL;
    *client_uid = getpid();
    sprintf(name.sun_path, "%s.socket_client_id[%d]", CLIENT_PATH,*client_uid);

     /* in case it already exists */
    unlink(name.sun_path);

    if (bind(socket_fd, (struct sockaddr *)&name, SUN_LEN(&name)) < 0)
    {
        close(socket_fd);
        log_err("failed to bind,error msg:%s\n",strerror(errno));
        return -2;

    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_LOCAL;
    strncpy(name.sun_path, ipc_name,sizeof(name.sun_path)- 1);

    if(retry >0)
    {
        do
        {
            if (connect(socket_fd,(struct sockaddr *)&name, SUN_LEN(&name)) == 0)
            {
                return socket_fd;
            }
            usleep(1000);

        }while(retry--);

        //log_err("Error: can not connect socket, (%s)\n", strerror(errno));
        close(socket_fd);
        return -1;

    }
    //just connect once.
    if (connect(socket_fd,(struct sockaddr *)&name, SUN_LEN(&name)) != 0)
    {
        log_err("Error: can not connect socket, (%s)\n", strerror(errno));
        close(socket_fd);
        return -1;
    }
    return socket_fd;
}


int disconnect_stream(int client_fd)
{
    if(client_fd >0)
    {
        shutdown(client_fd,SHUT_RDWR);
        close(client_fd);
        return 0;
    }
    return -1;
}

/*
static int _write_data(int fd, void *buf, int size, int retry)
{
    int offset = 0, left = size, len;
    if(retry <=0)
        retry = MAX_POLL_RETRY;

    while (left > 0)
    {
        len = write(fd, buf + offset, left);
        if (len < 0)
        {
            if (errno == EINTR)
            {
                if (--retry <= 0)
                {
                    printf("write_data: send interrupted! fd=%d\n", fd);
                    return -1;
                }
                continue;
            }
            printf("write_data: write error, %d, %s, fd=%d, len=%d\n",\
                       errno, strerror(errno), fd, len);
            return -1;
        }
        if (len == 0)
        {
            printf("write_data: Link broken!%d, %s, fd=%d\n", errno, strerror(errno), fd);
            return -3;
        }
        offset += len;
        left -= len;
    }

    return offset;
}
*/
int send_msg(int ipc_fd,const void *data, unsigned int data_len,void *reply, unsigned int *rsize,int timeout)
{
    int ret =-1;
    unsigned int len = 0;

    char sv[COMMON_MSG_SIZE];
    memset(sv,0,COMMON_MSG_SIZE);

    if(ipc_fd <= 0)
    {
        log_err("Error: Must connect server firstly\n");
        return -1;
    }

    if(data_len > COMMON_MSG_SIZE)
    {
        log_err("Error: post data's lenght must less than %d\n",COMMON_MSG_SIZE);
        return -2;

    }


    ret = write(ipc_fd,&data_len,sizeof(int));
    if(ret <= 0)
    {
        log_err("Error: failed to write length of data, (%s)\n", strerror(errno));
        return -1;
    }
    ret = write(ipc_fd,data,data_len);
    if(ret <= 0)
    {
        log_err("[%s]Error: failed to write data, (%s)\n", __FUNCTION__,strerror(errno));
        return -1;
    }

    ret = recv_msg(ipc_fd,&sv,&len,timeout);
    if(ret == -1)
    {
        log_err("failed to receive data to server,reconnect\n");
        *rsize = 0;
        return -1;

    }

    *rsize = len;
    
    memcpy(reply,sv,len);

    return 0;

}

int post_msg(int ipc_fd,const void *data, unsigned int data_len)
{
    int ret = -1;
    if(ipc_fd <= 0)
    {
        log_err("Error: Must connect server firstly\n");
        return -1;
    }
    if(data_len > COMMON_MSG_SIZE)
    {
        log_err("Error: post data's lenght must less than %d\n",COMMON_MSG_SIZE);
        return -2;

    }
    ret = write(ipc_fd,&data_len,sizeof(int));
    if(ret <= 0)
    {
        log_err("[%s]Error: failed to write length of data, (%s)\n", __FUNCTION__,strerror(errno));
        return -1;
    }
    ret = write(ipc_fd,data,data_len);
    if(ret <= 0)
    {
        log_err("[%s]Error: failed to write data, (%s)\n", __FUNCTION__,strerror(errno));
        return -1;
    }

    return 0;
}


static int _read_data(int fd, void *buf, int size, int timeo)
{
    int offset = 0, left = size, len;
    struct pollfd pfd;
    int retry = MAX_POLL_RETRY;
    int i;

    while (left > 0)
    {
        pfd.fd = fd;
        pfd.events = POLLIN | POLLERR | POLLHUP | POLLNVAL;
        errno = 0;
        i = poll(&pfd, 1,  timeo);
        if (i < 0)
        {
            if (errno == EINTR)
            {
                if (--retry <= 0)
                {
                    log_err("poll interrupted!\n");
                    return -1;
                }
                continue;
            }
            log_err("poll error: %s!\n",strerror(errno));
            return -1;  // error
        }
        else if(0 == i)
        {
            log_err("poll timeout!offset:%d\n",offset);
            if (offset)
                return offset;
            return -1;
        }

        if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))
            return -1;

        len = read(fd, buf + offset, left);
        if (len < 0)
        {
            if (errno == EINTR|| errno == EWOULDBLOCK || errno ==EAGAIN)
            {
                if (--retry <= 0)
                {
                    log_err("read interrupted!\n");
                    return -1;
                }
                continue;
            }//len == -1,refer to link broken
            return -3;
        }
        else if (len == 0&&errno !=0)
        {//link broken?
            log_err("Maybe a signal,Link broken!\n");
            return -3;
        }
        offset += len;
        left -= len;
    }

    return offset;
}

int recv_msg(int ipc_fd,void* data, unsigned int* data_len,int timeout)
{
    int ret = -1;
    fd_set  rfds;
    struct timeval timeo;
    int len = 0;
    char sv[COMMON_MSG_SIZE];

    memset(sv,0,COMMON_MSG_SIZE);

    if(ipc_fd <= 0)
    {
        log_err("Error: Must connect server/accept client firstly\n");
        return -1;
    }

    if (timeout != -1)
    {

        FD_ZERO(&rfds);
        FD_SET(ipc_fd, &rfds);
        timeo.tv_sec  = timeout / 1000;
        timeo.tv_usec = (timeout % 1000) * 1000;
        ret = select(ipc_fd+1,&rfds, NULL, NULL, &timeo);
        if(ret< 0)
        {
            if (errno == EINTR)
            {
                log_info("[%s]: %s\n",__FUNCTION__,strerror(errno));
                return -4;   
            }
            log_err("[%s]socket read command failed:socket is closed.errcode:%d[%s]!\n",__FILE__,errno,strerror(errno));
            return -1;/*socket is closed*/
        }
        else if(ret ==0)
            return -6;//no receive data
        
        if(!FD_ISSET(ipc_fd,&rfds))
            return -5;//no data
            
    }

    ret= read(ipc_fd, &len, sizeof(len));
    if(ret <0 ||len >COMMON_MSG_SIZE)
    {
        if (errno != EINTR&& errno != EWOULDBLOCK && errno !=EAGAIN)
        {
            *data_len = 0;
            log_err("[%s]%d:socket read failed len %d ,result=%d,socket:%d\n",__FILE__,__LINE__,len,ret,ipc_fd);
            return -1;

        }  
        
        *data_len = 0;
        log_err("[%s]%d:No data can be read,len %d ,result=%d,socket:%d\n",__FILE__,__LINE__,len,ret,ipc_fd);
        return -3;
    }
    else if(ret == 0)
    {      

        if(errno !=0)
        {
            *data_len = 0;
            log_err("[%s]%d:socket read command failed:link broken,errocode:%d[%s]\n",__FUNCTION__,__LINE__,errno,strerror(errno));
            return -1;
        }  
        else if(len == 0)
        {
            *data_len = 0;
            //log_err("[%s]%d:no data,errocode:%d[%s]\n",__FUNCTION__,__LINE__,errno,strerror(errno));
            return -6;

        }    
    }
   
    ret = _read_data(ipc_fd,sv,len,timeout);
    if(ret <= 0)
    {
        if(ret == -3)
        {
            *data_len = 0;
            log_err("[%s]%d:socket read command failed:socket is closed!,%s\n",__FUNCTION__,__LINE__,strerror(errno));
            return -1;
        }
        else
        {
            *data_len = 0;
            log_err("[%s]%d:socket read failed cmd %d ,result=%d,socket\n",__FILE__,__LINE__,len,ret);
            return -3;
        }

    }
    else if(ret != len)
    {
        *data_len = 0;
        log_err("[%s]%d:socket read failed cmd %d ,result=%d,socket\n",__FILE__,__LINE__,len,ret);
        return -3;
    }

    *data_len = len;

    memcpy((char*)data,sv,len);

    return 0;

}
