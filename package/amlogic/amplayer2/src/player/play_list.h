
#ifndef PLAYER_LIST_H
#define PLAYER_LIST_H
#include "list.h"
typedef struct 
{
	struct list_head list;
	char file[4];
}play_list_t;

char * play_list_get_file(void);
int play_list_add_file(char* file);
#endif

