#ifndef MSGMNGR_H
#define MSGMNGR_H

#ifdef __cplusplus
extern "C" {
#endif


#define COMM_TCP_HANDLE "/tmp/.comm_socket_tcp" //just default


#define DEFAULT_QUEUE_SIZE 5

typedef int (*pGetMsgCallback)(void* pt2Obj,void *msg,int msg_size);


//common

int register_get_msg_func(void* pt2Obj,pGetMsgCallback pf);


//for client,just call once in a process    
int init_client_msg_mngr(const char*tcp_ipc_name,int postQueueSize,void* pt2Obj,pGetMsgCallback pf);

int uninit_client_msg_mngr(void);

int test_client_connect_status(void);
#ifdef __cplusplus
}
#endif

#endif // MSGMNGR_H
