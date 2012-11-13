#ifndef _PLAYER_ITEMLIST_H_
#define _PLAYER_ITEMLIST_H_

#include "list.h"
#include <pthread.h>


#define ITEMLIST_WITH_LOCK

struct item {
    struct list_head list;
    unsigned long item_data;
};

struct itemlist {
    struct list_head list;
#ifdef ITEMLIST_WITH_LOCK
    pthread_mutex_t list_mutex;
    int muti_threads_access;
#endif
    int item_count;
    int max_items;
    int item_ext_buf_size;
    int reject_same_item_data;
};

typedef void(*data_free_fun)(void *);
typedef void(*data_is_match)(unsigned long data1, unsigned data2);

int itemlist_init(struct itemlist *itemlist);
struct item * item_alloc(int ext);
void item_free(struct item *item);


int itemlist_add_tail(struct itemlist *itemlist, struct item *item);
struct item * itemlist_get_head(struct itemlist *itemlist);
struct item * itemlist_get_tail(struct itemlist *itemlist);
struct item * itemlist_peek_head(struct itemlist *itemlist);
struct item * itemlist_peek_tail(struct itemlist *itemlist);
struct item *  itemlist_get_match_item(struct itemlist *itemlist, unsigned long data);
int itemlist_del_match_data_item(struct itemlist *itemlist, unsigned long data);
int itemlist_have_match_data(struct itemlist *itemlist, unsigned long data);


int itemlist_clean(struct itemlist *itemlist, data_free_fun free_fun);
int itemlist_add_tail_data(struct itemlist *itemlist, unsigned long data);
int itemlist_get_head_data(struct itemlist *itemlist, unsigned long *data);
int itemlist_get_tail_data(struct itemlist *itemlist, unsigned long *data);
int itemlist_peek_head_data(struct itemlist *itemlist, unsigned long *data);
int itemlist_peek_tail_data(struct itemlist *itemlist, unsigned long *data);
int itemlist_clean_data(struct itemlist *itemlist, data_free_fun free_fun);

#endif

