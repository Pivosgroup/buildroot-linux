#ifndef COMM_SOCKET_TCP_H
#define COMM_SOCKET_TCP_H


#ifdef __cplusplus
extern "C" {
#endif

#define COMMON_MSG_SIZE 1024*4

#define IPC_NAME_MAX 256

/*
 *Description:Create socket for server
 *@ return: value:if >0£¬ok,others failed.
 */
int open_stream(const char* ipc_name);

/*
 *Description:wait client to connect server 
 *
 *@ return: if >0,ok,others failed.
 */

int wait_for_connection(int svr_fd,int timeout,char* from_name,int*from_len);
/*
 *Description:remove socket for server
 *@ return: if = 0£¬ok,others failed.
 */
int close_stream(int svr_fd);

/*
 *Description:connect socket to server
 *@ Returns fd if all OK, <0 on error.
 *@ -1 retry means just connect once.
 */

int connect_stream(const char* ipc_name,int retry,int *client_uid);

int disconnect_stream(int client_fd);

/*Call these apis after connected*/

/*
 *@ -1 timeout means wait infinitely
 *@ send_msg will return until finish sending all the data
 */
int send_msg(int ipc_fd,const void *data, unsigned int data_len,void *reply, unsigned int *rsize,int timeout);
int post_msg(int ipc_fd,const void *data, unsigned int data_len);
int recv_msg(int ipc_fd,void* data, unsigned int* data_len,int timeout);


#ifdef __cplusplus
}
#endif

#endif // COMM_SOCKET_TCP_H
