#ifndef _MSGQUEUE_H_
#define _MSGQUEUE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "list.h"

typedef uint8_t  UINT8;

struct msg_item_t {
    struct list_head pool;
    struct list_head queue;
    int used;
    time_t ts_recv;//just for debug
    UINT8 *msg_ptr;
    int msg_len;
};

extern int mw_initialize_msg_pool(int max_num);
extern int mw_free_msg_pool();
extern struct msg_item_t* mw_alloc_msg_item(int msg_size);
extern int mw_free_msg_item(struct msg_item_t *p);

extern int mw_post_to_message_queue(struct msg_item_t *p);
extern struct msg_item_t* mw_dequeue_message_queue(int timeout);
extern int mw_clearall_message_queue();
	
#ifdef __cplusplus
}
#endif

#endif //_MSGQUEUE_H_
