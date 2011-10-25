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
#include <pthread.h>

#include"msgmngr_svr.h"
#include"comm_socket_tcp.h"
#include "mp_log.h"

#define COMM_TCP_HANDLE "/tmp/.comm_socket_tcp" //just default


#define MAX_VALUE(m1,m2) (((m1)>(m2))?(m1):(m2))
#define MIN_VALUE(m1,m2) (((m1)>(m2))?(m2):(m1))

//just for multi client supports
typedef struct{
    int cSocket;//client id
    int sSocket;//server rx path
    char *cli_ipc_name;
    int bActive;//is active? 1:0
}SocketClient;

#define MAX_CLIENTS 5


static SocketClient clientpool[MAX_CLIENTS];

static int _listen_fd = -1;
static int _max_fd = -1; // max fd for select use
static fd_set allset;

static int _client_total = 0;

static pthread_mutex_t client_pool_mutex;

#define CLIENT_POOL_LOCK()    pthread_mutex_lock(&client_pool_mutex)
#define CLIENT_POOL_UNLOCK()  pthread_mutex_unlock(&client_pool_mutex)


static int initClientPool()
{
    memset((void*)&clientpool,0,MAX_CLIENTS*sizeof(SocketClient));
    _client_total = 0;
    return 0;
}
static void dumpClientPool()
{
    int i = 0;
    log_con("=======================client pool dump start========================\n");
    for (i=0;i<MAX_CLIENTS;i++)
    {
        if(1 == clientpool[i].bActive)
        {
            log_con("%dst node's client socket handle:%d\n",i,clientpool[i].cSocket);
            log_con("%dst node's server socket handle:%d\n",i,clientpool[i].sSocket);
            log_con("%dst node's client socket path:%s\n",i,clientpool[i].cli_ipc_name);
        }
    }
    log_con("=======================client pool dump end==========================\n");
}

static int addClientToPool(SocketClient* cli)
{
    int i = 0;
    int ret = -1;
    if(NULL == cli || NULL == cli->cli_ipc_name)
    {
        log_err("failed to add client to clientpool\n");
        return -1;
    }
    CLIENT_POOL_LOCK();

    //remove invalid old node
    
    for (i=0;i<MAX_CLIENTS;i++)
    {
        if(1==cli->bActive&& clientpool[i].cSocket == cli->cSocket)
        {           
            FD_CLR(clientpool[i].sSocket, &allset);
            close(clientpool[i].sSocket);
            clientpool[i].sSocket = 0;
            clientpool[i].cSocket = 0;
            if(NULL != clientpool[i].cli_ipc_name)
            {
                free(clientpool[i].cli_ipc_name);
                clientpool[i].cli_ipc_name = NULL;
            }
            
            clientpool[i].bActive = 0;
        }
    }
    
    for (i=0;i<MAX_CLIENTS;i++)
    {        
        if(0 == clientpool[i].bActive)
        {
            clientpool[i].cSocket=cli->cSocket;
            clientpool[i].sSocket=cli->sSocket;
            clientpool[i].cli_ipc_name = strdup(cli->cli_ipc_name);
            clientpool[i].bActive = 1;
            ret = 0;
            _client_total ++;
            dumpClientPool();
            break;
        }        
    }

    CLIENT_POOL_UNLOCK();
    return ret;
}

static int delClientFromPool(int sSocket)
{
    int i = -1;
    CLIENT_POOL_LOCK();
    for(i = 0;i<MAX_CLIENTS;i++)
    {
        if(sSocket == clientpool[i].sSocket)
        {
            if(NULL != clientpool[i].cli_ipc_name)
            {
                free(clientpool[i].cli_ipc_name);
                clientpool[i].cli_ipc_name = NULL;
            }
            clientpool[i].cSocket = 0;
            clientpool[i].sSocket = 0;
            clientpool[i].bActive = 0;
            _client_total --;
            CLIENT_POOL_UNLOCK();
            dumpClientPool();
            return 0;
        }
    }
    log_err("can't find a node from pool\n");
    CLIENT_POOL_UNLOCK();
    return -1;
}



static void clearClientPool()
{    
    int i = -1;
    for(i = 0;i<MAX_CLIENTS;i++)
    {
        if(NULL != clientpool[i].cli_ipc_name)
        {
            free(clientpool[i].cli_ipc_name);
            clientpool[i].cli_ipc_name = NULL;
        }

    }
    memset(clientpool,0,MAX_CLIENTS*sizeof(SocketClient));

    _client_total = 0;
}


int init_svr_msg_mngr(const char*tcp_ipc_name)
{
    if(NULL != tcp_ipc_name&&(strlen(tcp_ipc_name)<IPC_NAME_MAX-1))
    {

        _listen_fd = open_stream(tcp_ipc_name);

    }
    else
        _listen_fd = open_stream(COMM_TCP_HANDLE);

    if(_listen_fd <= 0)
    {
        log_err("failed to open stream for server\n");
        return -1;
    }

    FD_ZERO(&allset);

    _max_fd = _listen_fd;

    FD_SET(_max_fd, &allset);

    initClientPool();
    return 0;
}

int svr_post_message(const void* buf,int msg_len)
{
    int ret = -1;
    int fd_c = 0;
    int fd_s = -1;   
    int isFound = -1;
    int i = -1;    

    if(NULL == buf||msg_len >=MESSAGE_SIZE_MAX|| msg_len < 0)
    {
        log_err("invalid msg,please check it\n");
        return -1;
    }    
    
    memcpy(&fd_c,buf,sizeof(int));
   

    if(fd_c == 0)//broadcast message.
    {
        
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            CLIENT_POOL_LOCK();
            if( clientpool[i].bActive == 1)
            {
                fd_s = clientpool[i].sSocket;

            }  
            else
            {
                fd_s = -1;    
                CLIENT_POOL_UNLOCK();  
                continue;
            }
            CLIENT_POOL_UNLOCK();  
             if(fd_s <= 0)
            {
                log_err("client not connect with server\n");
                continue;
            }
           
            ret = post_msg(fd_s,buf,msg_len);
            if(ret == -1)
            {               
                FD_CLR(fd_s,&allset); 
                close_stream(fd_s);
                delClientFromPool(fd_s);
                               
            }
                
        }

        return 0;
       
          
    }
    else
    {
        CLIENT_POOL_LOCK();
        for (i = 0; i < MAX_CLIENTS; i++)
        {

            if (fd_c == clientpool[i].cSocket&&clientpool[i].bActive == 1)
            {
                fd_s = clientpool[i].sSocket;   
                isFound = 1;
                break;
            }
        }
        CLIENT_POOL_UNLOCK();
    }
    
    if(isFound != 1&&fd_c >0&&fd_s <=0)
    {       
        fd_s = clientpool[0].sSocket;
        log_con("can't find client[%d] after reconnect,so response default client[%d]\n",fd_c,fd_s);
    }
        
    if(fd_s <= 0)
    {
        log_err("no client connect with server\n");
        return -1;
    }

    ret = post_msg(fd_s,buf,msg_len);
    if(ret == -1)
    {        
        FD_CLR(fd_s,&allset);
        close_stream(fd_s);    
        delClientFromPool(fd_s);
        return -1;
    }
    return 0;

}

#define MAX_POLL_RETRY 3
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
            }
            return -3;
        }
        if (len == 0)
        {
            log_err("Link broken!\n");
            return -3;
        }
        offset += len;
        left -= len;
    }

    return offset;
}

int svr_recv_message(const void* buf,int* msg_len,int timeout)
{
    int ret = -1;
    fd_set  rfds;
    SocketClient node;
    struct timeval timeo;
    int len = 0;
    char sv[COMMON_MSG_SIZE];
    char *ptmp = NULL;  
    struct sockaddr_un client_name;
    socklen_t client_name_len;
    int i = -1;

    memset(sv,0,COMMON_MSG_SIZE);
    memset((void*)&node,0,sizeof(SocketClient));

    client_name_len = sizeof(client_name);
    memset(client_name.sun_path,0,sizeof(client_name.sun_path));

    if(_max_fd <= 0)
    {
        log_err("Error: Must call init_svr_msg_mngr api firstly\n");
        return -1;
    }


    if (timeout != -1)
    {       
        rfds = allset;//* rset gets modified each time around */

        timeo.tv_sec  = timeout / 1000;
        timeo.tv_usec = (timeout % 1000) * 1000;
        ret = select(_max_fd+1,&rfds, NULL, NULL, &timeo);
        if(ret< 0)
        {
            log_err("[%s]socket select failed!\n",__FILE__);
            return -1;
        }   
        else if(ret ==0)
            return -6;//no receive data

        if(FD_ISSET(_listen_fd,&rfds))
        {
            node.sSocket = accept(_listen_fd, (struct sockaddr *)&client_name, &client_name_len);
            if(node.sSocket < 0)
            {
                log_err("[%s]Failed to accept client's connection.,%s\n",__FILE__,strerror(errno));
                return -1;
            }

            log_info("Succeed to get a client connect.path:%s,server handle:%d\n",client_name.sun_path,node.sSocket);

            node.cli_ipc_name = (char*)malloc(client_name_len*sizeof(char));
            if(NULL == node.cli_ipc_name)
            {
                log_err("failed malloc memory for %s,size:%d\n",client_name.sun_path,client_name_len);
                close(node.sSocket);
                return -1;
            }

            strncpy(node.cli_ipc_name,client_name.sun_path,client_name_len);

            if( !(ptmp = strchr(client_name.sun_path,'[')))
            {
                log_err("client path can't find '[' tag!\n");
                free(node.cli_ipc_name);
                node.cli_ipc_name = NULL;
                close(node.sSocket);
                return -1;
            }
            ptmp++;            
            node.cSocket=atol(ptmp);

            node.bActive = 1;

            ret = addClientToPool(&node);
            if(ret != 0)
            {
                log_err("current server just accept %d node,drop it",MAX_CLIENTS);
                free(node.cli_ipc_name);
                node.cli_ipc_name = NULL;
                close(node.sSocket);
                return -1;
            }
            free(node.cli_ipc_name);
            node.cli_ipc_name = NULL;
            _max_fd = MAX_VALUE(_max_fd,node.sSocket);
            FD_SET(node.sSocket, &allset);

            return 0;
        }        

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if ((clientpool[i].sSocket) < 0)
                continue;
            if (FD_ISSET(clientpool[i].sSocket, &rfds))
            {
                /* read argument buffer from client */                
                ret= read(clientpool[i].sSocket, &len, sizeof(len));
                if(ret <0 ||len >COMMON_MSG_SIZE)
                {
                    if (errno != EINTR&& errno != EWOULDBLOCK && errno !=EAGAIN)
                    {
                        *msg_len = 0;
                        FD_CLR(clientpool[i].sSocket, &allset);
                        close_stream(clientpool[i].sSocket);
                        delClientFromPool(clientpool[i].sSocket);
                        clientpool[i].sSocket = -1;
                        
                        log_err("[%s]%d:socket read failed,len %d ,result=%d,socket:%d\n",__FILE__,__LINE__,len,ret,clientpool[i].sSocket);
                        return -1;

                    }  
                    *msg_len = 0;
                    log_err("[%s]%d:No data can be read,len %d ,result=%d,socket:%d\n",__FILE__,__LINE__,len,ret,clientpool[i].sSocket);
                    return -3;
                }
                else if(ret == 0)
                {//maybe link broken.
                    *msg_len = 0;  
                    FD_CLR(clientpool[i].sSocket, &allset);
                    close_stream(clientpool[i].sSocket);
                    delClientFromPool(clientpool[i].sSocket);
                    clientpool[i].sSocket = -1;
                    log_err("[%s]%d:socket read command failed:socket is closed!,%s\n",__FUNCTION__,__LINE__,strerror(errno));

                    return -3;
                }

                ret = _read_data(clientpool[i].sSocket,sv,len,timeout);
                if(ret <= 0)
                {
                    if(ret == -3)
                    {
                        *msg_len = 0;
                        FD_CLR(clientpool[i].sSocket, &allset);
                        close_stream(clientpool[i].sSocket);
                        delClientFromPool(clientpool[i].sSocket);
                        clientpool[i].sSocket = -1;
                        log_err("[%s]%d:socket read command failed:socket is closed!,%s\n",__FUNCTION__,__LINE__,strerror(errno));
                        return -1;
                    }
                    else
                    {
                        *msg_len = 0;
                        log_err("[%s]%d:socket read failed cmd %d ,result=%d,socket\n",__FILE__,__LINE__,len,ret);
                        return -3;
                    }

                }
                else if(ret != len)
                {
                    *msg_len = 0;
                    log_err("[%s]%d:socket read failed cmd %d ,result=%d,socket\n",__FILE__,__LINE__,len,ret);
                    return -3;
                }

                *msg_len = len;

                memcpy((char*)buf,sv,len);

                return 0;
            }
         }



    }
    return -1;
}

int uninit_svr_msg_mngr()
{
    int i;
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if(clientpool[i].bActive == 1)
            disconnect_stream(clientpool[i].sSocket);
    }


    clearClientPool();

    close_stream(_listen_fd);

    return 0;
}
