#ifndef MSGMNGR_SVR_H
#define MSGMNGR_SVR_H

#ifdef __cplusplus
extern "C" {
#endif

//for server,just call once in a process

#define MESSAGE_SIZE_MAX 4*1024
int init_svr_msg_mngr(const char*tcp_ipc_name);

int svr_post_message(const void* buf,int msg_len);

int svr_recv_message(const void* buf,int* msg_len,int timeout);

int uninit_svr_msg_mngr();

#ifdef __cplusplus
}
#endif

#endif // MSGMNGR_SVR_H
