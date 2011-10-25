#ifndef MSGMNGR_INTERNAL_H
#define MSGMNGR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#define MESSAGE_SIZE_MAX 1024*4
//utils functions

//asynchronous api
int cli_post_message(const void* buf,int msg_len);

//synchronous api. when timeout is -1,will block flow.
int cli_send_message(const void *buf, unsigned int msg_len,void *reply, unsigned int *rsize,int timeout);


//some ugly codes.
int cli_get_request_seq_no(void);
int cli_release_instid_pool(void);

int cli_get_amplayer_task_id(int mplayerinstid);
int cli_wait_bind_response(int mplayerinstid,int timeout);
int cli_get_mplayer_inst_id(int pid);

int cli_close_player_instance(int mplayerinstid);

int cli_bind_inst_with_amplayer(int mplayerinstid,int pid);

#ifdef __cplusplus
}
#endif

#endif // MSGMNGR_INTERNAL_H

