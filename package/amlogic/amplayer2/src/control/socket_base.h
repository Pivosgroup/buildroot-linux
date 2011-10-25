#ifndef  SOCKET_BASE_H
#define  SOCKET_BASE_H
#include <unistd.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

typedef struct{
int 	len;
char   *cmd;
}socket_cmd_t;


int init_server_socket(char * sname);
int init_response_socket(char *socketname);
int waiting_connect(int socket);
int socket_free_command(socket_cmd_t *scmd );
int socket_read_command(int socket,socket_cmd_t *scmd,int wait_usec);
int socket_send_command(char *sname,socket_cmd_t *scmd);






#endif

