#ifndef _COMM_SOCKET_UDP_H_
#define _COMM_SOCKET_UDP_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define IPC_UDP_NAME_MAX 128
#define WAIT_FOREVER -1

extern int Comm_SocketIPCCreate(const char* ipc_name);
extern int Comm_SocketIPCSend(int fd,const char* dest_name,void *data, unsigned int data_len);
extern int Comm_SocketIPC_FlushSend(int  fd,const char* dest_name,void *data, unsigned int data_len);
extern int Comm_SocketIPCRecv(int fd , void* data, unsigned int data_len, int timeout,char* from_name);
extern void Comm_SocketIPCClose(int fd);

#ifdef  __cplusplus
}
#endif

#endif

