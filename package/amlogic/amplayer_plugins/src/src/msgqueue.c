#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <getopt.h>
#include <pthread.h>

#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>

#include "msgqueue.h"
#include "mp_log.h"

#define MSG_ITEM_FREE       0
#define MSG_ITEM_IN_USE     1

//--------------------------------------------------------------------


struct msg_item_t *msg_item_array;
static int msg_item_num_free;

static struct list_head msg_item_pool;
static pthread_mutex_t msg_item_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MSG_ITEM_POOL_LOCK()   pthread_mutex_lock(&msg_item_mutex)
#define MSG_ITEM_POOL_UNLOCK() pthread_mutex_unlock(&msg_item_mutex)

int initialize_message_queue();

int mw_initialize_msg_pool(int max_num)
{
    int i =-1;

    if (max_num <= 0)
    {
        log_err("failed, max_num=%d\n", max_num);
        return -1;
    }

    msg_item_num_free = max_num;

    msg_item_array = (struct msg_item_t *)malloc(max_num * sizeof(struct msg_item_t));
    if (msg_item_array == NULL) {
        log_err("malloc failed, errno=%d, errstr:%s\n", errno, strerror(errno));
        return -1;
    }
    memset(msg_item_array, 0, max_num * sizeof(struct msg_item_t));

    initialize_message_queue();

    log_info("initializing msg_item_pool, max_num=%d\n", max_num);

    INIT_LIST_HEAD(&msg_item_pool);

    for (i = 0; i < msg_item_num_free;i++) {
        INIT_LIST_HEAD(&msg_item_array[i].pool);
        list_add(&msg_item_array[i].pool,&msg_item_pool);
        INIT_LIST_HEAD(&msg_item_array[i].queue);
    }

    return 0;
}

int mw_free_msg_pool()
{
    free(msg_item_array);
    INIT_LIST_HEAD(&msg_item_pool);
    return 0;
}


struct msg_item_t* mw_alloc_msg_item(int msg_size)
{
    struct msg_item_t *p = NULL;
    char *buf = NULL;

    if (msg_size <= 0)
    {
        log_err("failed, msg_size=%d\n", msg_size);
        return (struct msg_item*)-1;
    }

    buf = (char *)malloc(msg_size);
    if (buf == NULL)
    {
        log_err("malloc failed in alloc_msg_item, errno=%d, errstr:%s\n", errno, strerror(errno));
        return (struct msg_item*)-1;
    }

    MSG_ITEM_POOL_LOCK();
    if (list_empty(&msg_item_pool))
    {
        log_err("msg_item_pool is empty, alloc fail, msg_item_num_free=%d\n", msg_item_num_free);
        free(buf);
        MSG_ITEM_POOL_UNLOCK();
        return (struct msg_item*)-1;
    }
    p = list_entry(msg_item_pool.prev, struct msg_item_t, pool);
    list_del(&p->pool);

    p->msg_ptr = buf;
    p->msg_len = msg_size;

    INIT_LIST_HEAD(&p->queue);
    p->used = MSG_ITEM_IN_USE;
    msg_item_num_free --;

    log_info("allocate msg_item, current free %d\n", msg_item_num_free);

    MSG_ITEM_POOL_UNLOCK();

    return p;
}

int mw_free_msg_item(struct msg_item_t *p)
{
    MSG_ITEM_POOL_LOCK();

    if (p->msg_ptr != 0)
    {
        free(p->msg_ptr);
        p->msg_ptr = 0;
        p->msg_len = 0;
    }

    memset(p, 0, sizeof(struct msg_item_t));
    INIT_LIST_HEAD(&p->pool);
    INIT_LIST_HEAD(&p->queue);

    list_add(&p->pool, &msg_item_pool);
    msg_item_num_free ++;

    log_info("free msg_item, current free %d\n", msg_item_num_free);

    MSG_ITEM_POOL_UNLOCK();

    return 0;
}



//--------------------------------------------------------------------

static struct list_head message_queue;
static int message_queue_item_num;
static pthread_mutex_t message_queue_mutex;
static pthread_cond_t message_queue_cond;

#define MESSAGE_QUEUE_LOCK()    pthread_mutex_lock(&message_queue_mutex)
#define MESSAGE_QUEUE_UNLOCK()  pthread_mutex_unlock(&message_queue_mutex)
#define MESSAGE_SIGNAL()  pthread_cond_signal(&message_queue_cond)
#define MESSAGE_WAIT()    pthread_cond_wait(&message_queue_cond, &message_queue_mutex)
#define MESSAGE_TIMED_WAIT(TSPTR) pthread_cond_timedwait(&message_queue_cond, &message_queue_mutex, TSPTR)

int initialize_message_queue()
{
    pthread_mutexattr_t mutex_attr;

    INIT_LIST_HEAD(&message_queue);
    message_queue_item_num = 0;

    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&message_queue_mutex, &mutex_attr);

    pthread_cond_init(&message_queue_cond, 0);

    return 0;
}

int mw_post_to_message_queue(struct msg_item_t *p)
{

    MESSAGE_QUEUE_LOCK();

    time(&p->ts_recv);
    list_add(&p->queue, &message_queue);
    message_queue_item_num++;

    log_info("msgqueue, got one msg, Current message_queue number %d\n", message_queue_item_num);

    MESSAGE_QUEUE_UNLOCK();
    MESSAGE_SIGNAL();


    return 0;
}

// timeout: unit: milisecond
struct msg_item_t* mw_dequeue_message_queue(int timeout)
{
    struct msg_item_t *p;
    int rc;
    struct timespec ts;
    struct timeval cur_time;

    MESSAGE_QUEUE_LOCK();

    while (list_empty(&message_queue))
    {
        if (timeout > 0)
        {
            gettimeofday(&cur_time,(struct timezone*)0);

            ts.tv_nsec = (cur_time.tv_usec * 1000L + ((long)timeout % 1000) * 1000000L);
            ts.tv_sec = cur_time.tv_sec + timeout / 1000;
            if (ts.tv_nsec >= 1000000000L)
            {
                ts.tv_nsec -= 1000000000L;
                ts.tv_sec += 1;
            }

            rc = MESSAGE_TIMED_WAIT(&ts);
            if (rc == 0)
            {
                break;
            } else
            {
                if (rc != ETIMEDOUT)
                {
                    //Wait timed out
                    log_wrn("pthread_cond_timedwait, errno=%d, errstr=%s\n",
                                    errno, strerror(errno));
                }

                MESSAGE_QUEUE_UNLOCK();
                return NULL;
            }
        }
        else if (timeout == -1)
        {
            // wait until message received
            MESSAGE_WAIT();
        } else
        {
            // return immediately if no message
            MESSAGE_QUEUE_UNLOCK();
            return NULL;
        }
    }

    p = list_entry(message_queue.prev, struct msg_item_t, queue);
    list_del(&p->queue);
    message_queue_item_num--;
    MESSAGE_QUEUE_UNLOCK();

    return p;
}

int mw_clearall_message_queue()
{
    struct msg_item_t *p = NULL;
    struct msg_item_t *q = NULL;

    MESSAGE_QUEUE_LOCK();

    list_for_each_entry_safe(p,q,&message_queue,queue)
    {
        if(p)
        {
            list_del(&p->queue);
            mw_free_msg_item(p);
            message_queue_item_num--;
            log_info("[%s]Remove a item from message queue\n",__FUNCTION__);
        }
    }

    log_info("[%s]current item count:%d\n",__FUNCTION__,message_queue_item_num);

    p = NULL;
    q = NULL;

    MESSAGE_QUEUE_UNLOCK();

    return 0;
}
