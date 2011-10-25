#include <stdio.h>
#include <sys/time.h> 
#include <time.h> 
#include <pthread.h>
#include <player.h>

#include "player_priv.h"
#include "play_list.h"

static struct list_head play_list;
static pthread_mutex_t list_mutex;
#define IS_SAME(s,file) ((strlen(file)==strlen(s)) && (memcmp(s,file,strlen(file))==0))
#define MAX_PATH   260
#define FILE_SEPRATOR	';'
int init_play_list(void)
{	
	 pthread_mutex_init(&list_mutex,NULL);
	INIT_LIST_HEAD(&play_list);
	return 0;
}
static play_list_t *alloc_list_struct(char * file)
{
	play_list_t *list;
	list=MALLOC(sizeof(play_list_t)+strlen(file));
	return list;
}
static void free_list_struct(play_list_t * list)
{
	FREE(list);
}
int play_list_add_file(char* file)
{
	play_list_t *plist;
	plist=alloc_list_struct(file);
	if(plist!=NULL)
	{
    	strcpy(plist->file,file);
    	pthread_mutex_lock(&list_mutex);
    	list_add_tail(&plist->list,&play_list);
    	pthread_mutex_unlock(&list_mutex);
	}
	return 0;
}
play_list_t *search_file_on_list(struct  list_head* head,char *file)
{
	play_list_t *plist=NULL;
	pthread_mutex_lock(&list_mutex);
    if(plist != NULL)
	{
        list_for_each_entry(plist,head,list)
	    {
    	    if(IS_SAME(plist->file,file))
    		    break;
	    }
    }
	pthread_mutex_unlock(&list_mutex);
	return plist;
}

int play_list_del_file(char*file)
{
	play_list_t *plist=NULL;
	search_file_on_list(&play_list,file);
	if(plist)
	{
    	pthread_mutex_lock(&list_mutex);
    	list_del(&plist->list);
    	pthread_mutex_unlock(&list_mutex);
    	free_list_struct(plist);
    	return 0;
	}
	return -1;	
}
/*file format =="file;file2;file3;file4"*/
int play_list_add_files(char*files)
{
	char *file;
	char *fptr,*eptr;
	file=MALLOC(MAX_PATH);
	if(file==NULL)
		return PLAYER_NOMEM;
	for(fptr=files;fptr[0]!='\0';fptr=eptr+1)
		{
		char *tptr;
		for(tptr=fptr;tptr[0]!='\0' && tptr[0]!=FILE_SEPRATOR;tptr++);
		eptr=tptr;
		if((eptr-fptr)>MAX_PATH)
			continue;
		MEMCPY(file,files,eptr-fptr);
		file[eptr-fptr]='\0';
		play_list_add_file(file);
		if(tptr[0]=='\0')
			break;
		}	
}
char * play_list_get_file(void)
{
	play_list_t *plist=NULL;
	if(!list_empty(&play_list))
		{
		plist=list_entry(&play_list, play_list_t,list);
		return plist->file;
		}
	return NULL;
}

