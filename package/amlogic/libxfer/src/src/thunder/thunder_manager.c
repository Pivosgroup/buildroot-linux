/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Thunder Module
 *
 *  Author: Amlogic software
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef __WIN32__
#include <Windows.h>
#else
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#endif

#include "thunder_manager.h"
#include "transfer_def.h"
#include "xfer_common.h"

#include "../common/xfer_debug.h"
#include "../common/transfer_utils.h"
#include "../common/listop.h"

#ifdef HAS_MODULE_THUNDER //define in Makefile
#define TRANSFER_MANAGER_TASK_XUNLEI
#define SUPPORT_THUNDER_KANKAN
#define SUPPORT_THUNDER_VERSION13
#endif

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
#include "embed_thunder.h"


char thunder_mgr_taskinfo[DL_MAX_PATH] = "";
char kankan_tmp_path[DL_MAX_PATH] = "/tmp/";
extern char xfer_path_prefix[];

char xunlei_licence[] = "09092700020000000000001eheff19cu95p6gnuw42";
/*store the license to reregist for examination ---add by lty*/
char license_temp[43] = {0};
//abx_sema_t 			thunder_license_sema  = ABX_INVALID_SEMA;
//sem_t s_thunder_license_sem;

//#define TOTHUNDER(a)	(a-TASKID_THUNDER_BEGIN)
//#define TOXFER(a)		(a+TASKID_THUNDER_BEGIN)

static int delay_for_delete = 100 * 1000; /* 100 ms*/


static int s_thunder_inited = 0;
static int s_thunder_vod_buf_size = 0xA00000;
static transfer_taskid_t thunder_mgr_taskid = 0;
static int s_b_disk = 1 ;
extern char xfer_load_path[DL_MAX_PATH];
int b_enable_kankan ;
#ifndef __ROM_
typedef struct {
	transfer_taskid_t	task_id;
	int					msg_sent;
	int					msg_done;
}THUNDER_DEBUG_STATE_t;
int				   thunder_debug_index = 0;
THUNDER_DEBUG_STATE_t thunder_debug_state[300];
void thunder_dbg_msg_sent_(transfer_taskid_t taskid, int type)
{
	thunder_debug_state[thunder_debug_index%300].task_id = taskid;
	thunder_debug_state[thunder_debug_index%300].msg_sent = type;
	thunder_debug_state[thunder_debug_index++%300].msg_done= -1;
}

void thunder_dbg_msg_done_(transfer_taskid_t taskid, int type)
{
	thunder_debug_state[thunder_debug_index%300].task_id = taskid; 
	thunder_debug_state[thunder_debug_index%300].msg_sent = -1;
	thunder_debug_state[thunder_debug_index++%300].msg_done = type;
}
#endif

typedef struct _thunder_mgr_node
{
	list_t			  	 link;

	transfer_taskid_t 	 taskid;
	transfer_taskid_t 	 curr_taskid;
	char* 			  	 src;
	char* 			  	 src_url;
	char* 			  	 gcid;
	char* 			  	 file_path;
	char*			  	 file_name;
	char*			  	 curr_file_name;
	transfer_task_stat_e task_stat;      ///< task status
	transfer_type_e 	 type;
	u32 bt_files_num;
	u32 *bt_files_index;
	u64_t					 total_size;
	u64_t					 downloaded_size;
	int					 err;
	int 				 file_created;
}thunder_mgr_node_t;

typedef struct _thunder_mgr_list
{
	unsigned int 	   count;
	thunder_mgr_node_t node;
}thunder_mgr_list_t;

int s_thunder_list_sem_inited = 0;
sem_t s_thunder_list_sem;
thunder_mgr_list_t  s_thunder_tasks;
static int s_thunder_tasks_loaded = 0;



static int
thunder_list_set_bt_files_index(transfer_taskid_t taskid,  u32 files_num, u32 *files_index);

void 
thunder_mgr_set_kankan_disk( int b_disk )
{
	s_b_disk = b_disk ;
}

void thunder_mgr_set_tmp_path(const char * path)
{
	strncpy(kankan_tmp_path,path,DL_MAX_PATH);
}


static thunder_mgr_node_t* 
thunder_node_malloc()
{
	thunder_mgr_node_t* node = NULL;

	node = (thunder_mgr_node_t*)xfer_malloc(sizeof(thunder_mgr_node_t));
	if (node == NULL)
		return NULL;
	memset(node, 0, sizeof(thunder_mgr_node_t));

	return node;
}

static void 
thunder_node_free(thunder_mgr_node_t* node)
{
	if (node)
	{
		if (node->src)
		{
			xfer_free(node->src);
			node->src = NULL;
		}
		if (node->src_url)
		{
			xfer_free(node->src_url);
			node->src_url= NULL;
		}
		if (node->gcid)
		{
			xfer_free(node->gcid);
			node->gcid = NULL;
		}
		if (node->file_path)
		{
			xfer_free(node->file_path);
			node->file_path = NULL;
		}
		if (node->file_name)
		{
			xfer_free(node->file_name);
			node->file_name = NULL;
		}
		if (node->curr_file_name)
		{
			xfer_free(node->curr_file_name);
			node->curr_file_name = NULL;
		}
		if(node->bt_files_index)
		{
			xfer_free(node->bt_files_index);
			node->bt_files_index = NULL;
			node->bt_files_num = 0;
		}			
		xfer_free(node);
	}
}

void thunder_list_sem_init()
{
	while(!s_thunder_list_sem_inited)
	{
		if(sem_init(&s_thunder_list_sem, 0, 1) == 0)
		{
			s_thunder_list_sem_inited = 1; 
			break;
		}
		else
		{
			s_thunder_list_sem_inited = 0; 
		} 
	}
}

void thunder_list_sem_lock()
{
	do{
		sem_wait(&s_thunder_list_sem);
	}while(0);
}

void thunder_list_sem_unlock()
{
	sem_post(&s_thunder_list_sem);
}

void thunder_list_sem_close()
{
	while(s_thunder_list_sem_inited)
	{
		sem_destroy(&s_thunder_list_sem);
		s_thunder_list_sem_inited = 0;
		break;
	}
}


static xfer_errid_t 
thunder_list_init()
{
	thunder_list_sem_init();

	thunder_list_sem_lock();

	s_thunder_tasks.count = 0;
	INIT_LIST_HEAD(&(s_thunder_tasks.node.link));

	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static thunder_mgr_node_t* 
thunder_list_get(transfer_taskid_t taskid)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return NULL;

	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			thunder_list_sem_unlock();
			return node;
		}
	}
	thunder_list_sem_unlock();

	return NULL;
}

static transfer_taskid_t 
get_curr_taskid(transfer_taskid_t taskid)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return -1;

	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			thunder_list_sem_unlock();
			return node->curr_taskid;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

static xfer_errid_t 
thunder_list_set_status(transfer_taskid_t taskid, transfer_task_stat_e status, int err)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;

	thunder_list_sem_unlock();

	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			node->task_stat = status;
			if (err != XFER_ERROR_OK)
				node->err = err;
			break;
		}
	}
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static int 
thunder_list_get_status(transfer_taskid_t taskid)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return -1;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			thunder_list_sem_unlock();
			return node->task_stat;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

static xfer_errid_t 
thunder_list_set_val(transfer_taskid_t taskid, transfer_task_stat_e status, int err,
					int total_size, int downloaded_size, const char* curr_file_name, 
					int file_created)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			node->task_stat = status;
			node->total_size = total_size;
			node->downloaded_size = downloaded_size;
			node->err = err;
			if (file_created && node->file_created == 0)
				node->file_created = 1;
				
			if (curr_file_name)
			{
				node->curr_file_name = (char*)xfer_realloc(node->curr_file_name, strlen(curr_file_name)+1);
				if (node->curr_file_name)
				{
					strcpy(node->curr_file_name, curr_file_name);
				}
			}
			break;
		}
	}
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static xfer_errid_t 
thunder_list_get_val(transfer_taskid_t taskid, transfer_task_stat_e* status,
					int* total_size, int* downloaded_size, int* file_created, xfer_errid_t* err)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			*status = node->task_stat;
			*err = node->err;
			*total_size = node->total_size;
			*downloaded_size = node->downloaded_size;
			*file_created = node->file_created;
			break;
		}
	}
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static xfer_errid_t 
thunder_list_set_type(transfer_taskid_t taskid, transfer_type_e type)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			node->type = type;
			break;
		}
	}
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static int 
thunder_list_get_type(transfer_taskid_t taskid)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			thunder_list_sem_unlock();
			return node->type;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

static int
thunder_list_get_task_src_url(transfer_taskid_t taskid, char* src_url)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;

	if(src_url == NULL)
		return -1;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			strcpy(src_url, node->src_url);
			thunder_list_sem_unlock();
			return XFER_ERROR_OK;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

static int
thunder_list_set_task_src_url(transfer_taskid_t taskid, char* src_url)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	if(src_url == NULL)
		return -1;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			node->src_url= (char*)xfer_malloc(strlen(src_url)+1);
			if (node->src_url)
			{
				strcpy(node->src_url, src_url);
			}
			thunder_list_sem_unlock();
			return XFER_ERROR_OK;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

static transfer_taskid_t* 
thunder_list_taskids()
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;
	transfer_taskid_t* taskids = NULL;
	int pos = 0;

	if (s_thunder_tasks.count == 0)
		return NULL;
	
	taskids = (transfer_taskid_t*)xfer_malloc(sizeof(transfer_taskid_t)*s_thunder_tasks.count);
	if (taskids == NULL)
	{
		return NULL;
	}
	memset(taskids, 0, sizeof(transfer_taskid_t)*s_thunder_tasks.count);

	if(s_thunder_tasks.node.link.next == NULL)
		return NULL;

	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		memcpy(taskids+pos, &node->taskid, sizeof(transfer_taskid_t));
		pos++;
	}
	thunder_list_sem_unlock();

	return taskids;
}

static xfer_errid_t 
thunder_list_del(transfer_taskid_t taskid)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			et_stop_task(node->curr_taskid);
			et_delete_task(node->curr_taskid);
			list_del(&node->link);
			thunder_node_free(node);
			s_thunder_tasks.count--;
			break;
		}
	}
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static transfer_taskid_t get_task_id()
{
	do{
	if (thunder_mgr_taskid >= TASKID_THUNDER_END || thunder_mgr_taskid < TASKID_THUNDER_BEGIN)
		thunder_mgr_taskid = TASKID_THUNDER_BEGIN;
	
		thunder_mgr_taskid++;
		
	}while(thunder_list_get(thunder_mgr_taskid));
	
	return thunder_mgr_taskid;
}

static int taskid_validate(transfer_taskid_t taskid)
{
	if (taskid >= TASKID_THUNDER_BEGIN && taskid <= TASKID_THUNDER_END)
		return 1;
	else
		return 0;
}

static thunder_mgr_node_t* 
generate_node(transfer_taskid_t taskid, u32 thunder_taskid, 
	const char* url, const char* file_path, const char* file_name)
{
	thunder_mgr_node_t* node = NULL;

	if (url == NULL || file_path == NULL)
		return NULL;
	
	if ((node = thunder_node_malloc()) == NULL)
		return NULL;

	node->src = (char*)xfer_malloc(strlen(url)+1);
	node->file_path = (char*)xfer_malloc(strlen(file_path)+1);
	if (node->src == NULL || node->file_path == NULL)
	{
		thunder_node_free(node);
		return NULL;
	}
	node->taskid = taskid;
	node->curr_taskid = thunder_taskid;
	node->task_stat = DLST_NONE;
	strcpy(node->src, url);
	strcpy(node->file_path, file_path);
	if (file_name)
	{
		node->file_name = (char*)xfer_malloc(strlen(file_name)+1);
		if (node->file_name == NULL)
		{
			thunder_node_free(node);
			return NULL;
		}
		strcpy(node->file_name, file_name);
	}

	return node;
}

static thunder_mgr_node_t* 
generate_node_ex(transfer_taskid_t taskid, u32 thunder_taskid, 
	const char* url, const char* file_path, const char* file_name, int file_length)
{
	thunder_mgr_node_t* node = NULL;

	if (url == NULL || file_path == NULL)
		return NULL;
	
	if ((node = thunder_node_malloc()) == NULL)
		return NULL;

	node->src = (char*)xfer_malloc(strlen(url)+1);
	node->file_path = (char*)xfer_malloc(strlen(file_path)+1);
	if (node->src == NULL || node->file_path == NULL)
	{
		thunder_node_free(node);
		return NULL;
	}
	node->taskid = taskid;
	node->curr_taskid = thunder_taskid;
	node->total_size = file_length;
	node->task_stat = DLST_NONE;
	strcpy(node->src, url);
	strcpy(node->file_path, file_path);
	if (file_name)
	{
		node->file_name = (char*)xfer_malloc(strlen(file_name)+1);
		if (node->file_name == NULL)
		{
			thunder_node_free(node);
			return NULL;
		}
		strcpy(node->file_name, file_name);
	}

	return node;
}


static xfer_errid_t 
thunder_list_pushfront(transfer_taskid_t taskid, u32 thunder_taskid,
	const char* url, const char* file_path, const char* file_name, int type)
{
	thunder_mgr_node_t* node = NULL;

	if (url == NULL || file_path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if ((node = generate_node(taskid, thunder_taskid, url, file_path, file_name)) == NULL)
		return XFER_ERROR_MALLOC_FAILURE;
	node->type = type;
	thunder_list_sem_lock();
	list_add(&(node->link), &(s_thunder_tasks.node.link));
	s_thunder_tasks.count++;
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static xfer_errid_t thunder_list_pushback(transfer_taskid_t taskid, u32 thunder_taskid,
	const char* url, const char* file_path, const char* file_name, int type)
{
	thunder_mgr_node_t* node = NULL;

	if (url == NULL || file_path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if ((node = generate_node(taskid, thunder_taskid, url, file_path, file_name)) == NULL)
		return XFER_ERROR_MALLOC_FAILURE;
	node->type = type;
	thunder_list_sem_lock();
	list_add_tail(&(node->link), &(s_thunder_tasks.node.link));
	s_thunder_tasks.count++;
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static xfer_errid_t thunder_list_pushback_ex(transfer_taskid_t taskid, u32 thunder_taskid,
	const char* url, const char* file_path, const char* file_name, int type, int file_length)
{
	thunder_mgr_node_t* node = NULL;

	if (url == NULL || file_path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if ((node = generate_node_ex(taskid, thunder_taskid, url, file_path, file_name,file_length)) == NULL)
		return XFER_ERROR_MALLOC_FAILURE;
	node->type = type;
	thunder_list_sem_lock();
	list_add_tail(&(node->link), &(s_thunder_tasks.node.link));
	s_thunder_tasks.count++;
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

static xfer_errid_t thunder_list_clear()
{
	int err_id = 0;
	list_t *next, *cur;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
		
	thunder_mgr_node_t* node = NULL;

	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		et_stop_task(node->curr_taskid);
		while(1)
		{
			err_id = et_delete_task(node->curr_taskid);  
			if(err_id!=4109&&err_id!=4119)
			{
//				debugf(XFER_LOG_MAIN, "***********************thunder_id=%d errid=%d\n",node->curr_taskid,err_id);
				break;
			}
			if(err_id == 4109)
				et_stop_task(node->curr_taskid);
			usleep(delay_for_delete);
		}
		list_del(&node->link);
		thunder_node_free(node);
		s_thunder_tasks.count--;
	}
	s_thunder_tasks.count = 0;
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}

xfer_errid_t 
thunder_mgr_set_length(transfer_taskid_t taskid, u64_t len)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;

	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			node->total_size = len;
			break;
		}
	}
	thunder_list_sem_unlock();

	return XFER_ERROR_OK;
}


int thunder_mgr_taskid_validate(transfer_taskid_t taskid)
{
	if (taskid >= TASKID_THUNDER_BEGIN)
		return 1;
	else
		return 0;
}

int thunder_mgr_task_downloaded(const char* file_path, const char* file_name)
{
#if 0//jpf
	char file[256];
	struct stat st;
	if (file_path == NULL || file_name == NULL)
		return 0;
	
	snprintf(file, 255, "%s/%s", file_path, file_name);
	if (stat(file, &st) >= 0)
	{
		return 1; //printf("task downloaded: %s\n", file);
	}
	
#endif
	return 0;
}

#define POP_ERR_RETURN(dst, len) \
	if ((res = pop_buff_safe(dst, &pos, len, &last_len)) != XFER_ERROR_OK) \
	{	goto exit_flag;  }
xfer_errid_t thunder_load_tasks()
{
	debugf(XFER_LOG_THUNDER, "thunder_load_tasks()\n");
	debugf(XFER_LOG_MAIN, "thunder_load_tasks(%s)\n", thunder_mgr_taskinfo);

	xfer_errid_t res = XFER_ERROR_OK;
	int		last_len = 0;
	char* 	pos = NULL;
	char 	*src = NULL, *src_url = NULL, *gcid = NULL, *file_path = NULL, *file_name = NULL, *curr_file_name = NULL;
	char* 	buff = NULL;
	int 	version = 0;
	u32 bt_files_num = 0;
	u32 *bt_files_index = NULL;	
	char tmp_path[DL_MAX_PATH] = {0};

	if ((last_len = read_whole_file(&buff, thunder_mgr_taskinfo)) < 20)
	{
		debugf(XFER_LOG_MAIN, "read_file(%s) ERROR\n", thunder_mgr_taskinfo);
		res = XFER_ERROR_OK;
		goto exit_flag;
	}
	else
		debugf(XFER_LOG_MAIN, "read_file(%s) OK\n", thunder_mgr_taskinfo);

	if (strncmp(buff, "1.10", 4) == 0)
		version = 1;
	else if (strncmp(buff, "1.11", 4) == 0)
		version = 2;
	
	pos = buff+20;
	last_len -= 20;

	do
	{
		int	  len = 0, task_stat = 0, total_size = 0, downloaded_size = 0, file_created = 0;
		transfer_type_e type = 0;
		transfer_taskid_t taskid = TASKID_INVALID;
		
		POP_ERR_RETURN((void*)&taskid, sizeof(transfer_taskid_t));
		if (!thunder_mgr_taskid_validate(taskid))
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		//taskid = get_task_id(); // fixed status error bug
		if (thunder_list_get(taskid) != NULL)
		{
			transfer_taskid_t init_taskid = taskid;
			taskid = get_task_id();
	/*jpf
			file_task_send_msg(init_taskid, NULL, THUNDER_SAME_TASKID, NULL, 0, taskid, NULL);
			*/
		}
		
		// src
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len <= 0)
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		src = (char*)xfer_malloc(len+1);
		if (src == NULL)
		{
			res = XFER_ERROR_HTTP_MALLOC_FAILURE;
			goto exit_flag;
		}
		POP_ERR_RETURN(src, len);
		src[len] = 0;

		// src_url
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len < 0)
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}		
		else if(len > 0)
		{
			src_url = (char*)xfer_malloc(len+1);
			if (src_url == NULL)
			{
				res = XFER_ERROR_HTTP_MALLOC_FAILURE;
				goto exit_flag;
			}
			POP_ERR_RETURN(src_url, len);
			src_url[len] = 0;		
		}
		
		/*
		// gcid
		if (version >= 2)
		{
			POP_ERR_RETURN((void*)&len, sizeof(int));
			if (len <= 0)
			{
				res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
				goto exit_flag;
			}
			gcid = (char*)xfer_malloc(len+1);
			if (gcid == NULL)
			{
				res = XFER_ERROR_HTTP_MALLOC_FAILURE;
				goto exit_flag;
			}
			POP_ERR_RETURN(gcid, len);
			gcid[len] = 0;
		}
		*/
		
		// file path
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len <= 0)
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		file_path = (char*)xfer_malloc(len+1);
		if (file_path == NULL)
		{
			res = XFER_ERROR_HTTP_MALLOC_FAILURE;
			goto exit_flag;
		}
		POP_ERR_RETURN(file_path, len);

		/* load full path if set path prefix */
		if (xfer_path_prefix[0] && file_path[0] != '/')
		{
			memcpy(tmp_path, xfer_path_prefix, strlen(xfer_path_prefix));
			strcat(tmp_path, file_path);
			len += strlen(xfer_path_prefix);
			file_path = (char *)xfer_realloc(file_path, len+1);
			if (file_path == NULL)
			{
				res = XFER_ERROR_HTTP_MALLOC_FAILURE;
				goto exit_flag;
			}
			memcpy(file_path, tmp_path, len);
		}

#if 0//jpf
		if (len >= strlen(xfer_load_path) 
			&& strncmp(file_path, xfer_load_path, strlen(xfer_load_path)) != 0)
		{
			memcpy(file_path,  xfer_load_path, strlen(xfer_load_path));
		}
#endif
		file_path[len] = 0;
		
		// file name
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len <= 0)
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		file_name = (char*)xfer_malloc(len+1);
		if (file_name == NULL)
		{
			res = XFER_ERROR_HTTP_MALLOC_FAILURE;
			goto exit_flag;
		}
		POP_ERR_RETURN(file_name, len);
		file_name[len] = 0;
		
		// current file name
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len > 0)
		{
			curr_file_name = (char*)xfer_malloc(len+1);
			if (curr_file_name == NULL)
			{
				res = XFER_ERROR_HTTP_MALLOC_FAILURE;
				goto exit_flag;
			}
			POP_ERR_RETURN(curr_file_name, len);
			curr_file_name[len] = 0;
		}
		
		POP_ERR_RETURN((void*)&task_stat, sizeof(transfer_task_stat_e));
		POP_ERR_RETURN((void*)&total_size, sizeof(int));
		POP_ERR_RETURN((void*)&downloaded_size, sizeof(int));
		POP_ERR_RETURN((void*)&type, sizeof(transfer_type_e));

		// bt files index
		if(type == XFER_TASK_XLBT)
		{
			POP_ERR_RETURN((void*)&bt_files_num, sizeof(u32));
			if (bt_files_num > 0)
			{
				bt_files_index = (u32 *)xfer_malloc(bt_files_num * sizeof(u32));
				if (bt_files_index == NULL)
				{
					res = XFER_ERROR_HTTP_MALLOC_FAILURE;
					goto exit_flag;
				}
				POP_ERR_RETURN(bt_files_index, bt_files_num * sizeof(u32));
			}
		}
		
		if (version >= 1)
			POP_ERR_RETURN((void*)&file_created, sizeof(int));
		
		if (task_stat == DLST_NONE)
			task_stat = DLST_ERROR;

		if ((type != XFER_TASK_XLBT)&&(thunder_mgr_task_downloaded(file_path, file_name)))
			goto free_flag;

		if (curr_file_name)
		{
			thunder_list_pushback(taskid, 0, src, file_path, file_name, type);
			thunder_list_set_task_src_url(taskid, src_url);
			if(type == XFER_TASK_XLBT)
				thunder_list_set_bt_files_index(taskid, bt_files_num, bt_files_index);
			thunder_list_set_val(taskid, (task_stat == DLST_DOWNLOADING) ? DLST_STOPPED : task_stat, 
				XFER_ERROR_OK, total_size, downloaded_size, curr_file_name, file_created);
			/* jpf
			if (task_stat == DLST_DOWNLOADING)
				file_task_send_msg(taskid, NULL, FILE_TASK_XSTART, NULL, 0, 0, NULL);
				//thunder_mgr_task_start(taskid);
				*/
			if (task_stat == DLST_DOWNLOADING)
				thunder_mgr_task_start(taskid);
		}
		else
		{
			if (task_stat == DLST_DOWNLOADING)
			{			 	  	
				if(type == XFER_TASK_XLBT)
					thunder_mgr_task_add_bt(src_url, src, file_path, type, file_name, bt_files_index, bt_files_num, &taskid);
				else
					thunder_mgr_task_add_ex(src, total_size, gcid, file_path, type, file_name, &taskid);
				thunder_mgr_set_task_src_url(taskid, src_url);
				if(type == XFER_TASK_XLBT)
					thunder_list_set_bt_files_index(taskid, bt_files_num, bt_files_index);			
				thunder_list_set_val(taskid, DLST_NONE, XFER_ERROR_OK, 
					total_size, downloaded_size, NULL, file_created);

				/*
				file_task_send_msg(taskid, NULL, FILE_TASK_XSTART, NULL, 0, 0, NULL);
				//thunder_mgr_task_start(taskid);
				*/
				thunder_mgr_task_start(taskid);
			}
			else
			{
				thunder_list_pushback(taskid, 0, src, file_path, file_name, type);
				thunder_mgr_set_task_src_url(taskid, src_url);
				if(type == XFER_TASK_XLBT)	
					thunder_list_set_bt_files_index(taskid, bt_files_num, bt_files_index);		
				thunder_list_set_val(taskid, task_stat, XFER_ERROR_OK, 
					total_size, downloaded_size, NULL, file_created);
				thunder_list_set_status(taskid, task_stat, 0);
			}
		}
		//if (task_stat == DLST_ERROR)
		//	file_task_send_msg(taskid, NULL, FILE_TASK_XSTART, NULL, 0, 0, NULL);
		if (task_stat == DLST_ERROR)
			thunder_mgr_task_start(taskid);
				
free_flag:
		if (src) 
		{
			xfer_free(src); 
			src = NULL;
		}
		if (src_url) 
		{
			xfer_free(src_url); 
			src_url = NULL;
		}			
		if (file_path) 
		{
			xfer_free(file_path); 
			file_path = NULL;
		}
		if (file_name) 
		{
			xfer_free(file_name); 
			file_name = NULL;
		}
		if (curr_file_name) 
		{
			xfer_free(curr_file_name); 
			curr_file_name = NULL;
		}
		if (bt_files_index) 
		{
			xfer_free(bt_files_index); 
			bt_files_index = NULL;
			bt_files_num = 0;
		}		
	}while(last_len);

exit_flag:
	if (src) 
	{
		xfer_free(src); 
		src = NULL;
	}
	if (src_url) 
	{
		xfer_free(src_url); 
		src_url = NULL;
	}	
	if (file_path) 
	{
		xfer_free(file_path); 
		file_path = NULL;
	}
	if (file_name) 
	{
		xfer_free(file_name); 
		file_name = NULL;
	}
	if (curr_file_name) 
	{
		xfer_free(curr_file_name); 
		curr_file_name = NULL;
	}
	if (bt_files_index) 
	{
		xfer_free(bt_files_index); 
		bt_files_index = NULL;
		bt_files_num = 0;
	}	
	if (buff)
	{
		xfer_free(buff);
	}

	thunder_list_sem_lock();
	s_thunder_tasks_loaded = 1;
	thunder_list_sem_unlock();

	return res;
}
#undef POP_ERR_RETURN

#define PUSH_ERR_RETURN(src, len) \
	if ((res = push_buff_safe(mem_buf, src, len)) != XFER_ERROR_OK) \
	{	\
		if (mem_buf->buff) \
			xfer_free(mem_buf->buff); \
		thunder_list_sem_unlock(); \
		xfer_free(mem_buf); \
		return res;  \
	}
xfer_errid_t thunder_save_tasks()
{
	xfer_errid_t res = XFER_ERROR_OK;
	list_t *next, *cur;
	thunder_mgr_node_t* curr;
	int	saved_len = 0;
	char i_file_path[DL_MAX_PATH] = {0};
	char *tmp_path = NULL;
	transfer_mem_buff_t* mem_buf = NULL;

	if (!s_thunder_tasks_loaded)
		return XFER_ERROR_OK;
	
	mem_buf = (transfer_mem_buff_t*)xfer_malloc(sizeof(transfer_mem_buff_t));
	if (mem_buf == NULL)
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	memset(mem_buf, 0, sizeof(transfer_mem_buff_t));

	thunder_list_sem_lock();
	PUSH_ERR_RETURN("1.11", 4);
	PUSH_ERR_RETURN("1234567890123456", 16);

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		int len = 0;
		curr = list_entry(cur, thunder_mgr_node_t, link);
		if (!taskid_validate(curr->taskid))
		{
			continue;
		}

		if( XFER_TASK_XLGCID == curr->type )
		{
			// the kankan online task !
			continue;
		}
		PUSH_ERR_RETURN((void*)&curr->taskid, sizeof(transfer_taskid_t));

		len = strlen(curr->src);
		PUSH_ERR_RETURN((void*)&len, sizeof(int));
		PUSH_ERR_RETURN(curr->src, strlen(curr->src));

		if(curr->src_url)
		{
			len = strlen(curr->src_url);
			PUSH_ERR_RETURN((void*)&len, sizeof(int));
			PUSH_ERR_RETURN(curr->src_url, strlen(curr->src_url));
		}
		else
		{
			len = 0;
			PUSH_ERR_RETURN((void*)&len, sizeof(int));
		}
		
		/*
		if(curr->gcid)
		{
			len = strlen(curr->gcid);
			PUSH_ERR_RETURN((void*)&len, sizeof(int));
			PUSH_ERR_RETURN(curr->gcid, strlen(curr->gcid));
		}
		else
		{
			len = 0;
			PUSH_ERR_RETURN((void*)&len, sizeof(int));
		}
		*/
		
		/* save relative path if set path prefix */
		len = strlen(curr->file_path);
		strcpy(i_file_path, curr->file_path);
		tmp_path = i_file_path;
		if (xfer_path_prefix[0])
		{
			tmp_path = strstr(i_file_path, xfer_path_prefix);
			if (tmp_path != NULL)
			{
				len -= strlen(xfer_path_prefix);
				tmp_path += strlen(xfer_path_prefix);
			}
			else
				tmp_path = i_file_path;
		}
		PUSH_ERR_RETURN((void*)&len, sizeof(int));
		PUSH_ERR_RETURN(tmp_path, len);
		//PUSH_ERR_RETURN(curr->file_path, strlen(curr->file_path));
		
		len = strlen(curr->file_name);
		PUSH_ERR_RETURN((void*)&len, sizeof(int));
		PUSH_ERR_RETURN(curr->file_name, strlen(curr->file_name));

		if (curr->curr_file_name)
		{
			len = strlen(curr->curr_file_name);
			PUSH_ERR_RETURN((void*)&len, sizeof(int));
			PUSH_ERR_RETURN(curr->curr_file_name, strlen(curr->curr_file_name));
		}
		else
		{
			int len = 512;
			int curr_name_len = 0;
			char* suffix = NULL;
			if (curr->curr_file_name == NULL)
			{
				curr->curr_file_name = (char*)xfer_malloc(len);
				if (curr->curr_file_name == NULL)
				{
					if (mem_buf->buff)
						xfer_free(mem_buf->buff);
					thunder_list_sem_unlock();
					xfer_free(mem_buf);
					return XFER_ERROR_HTTP_MALLOC_FAILURE;
				}
				memset(curr->curr_file_name, 0, len);
			}
			et_get_task_file_name(curr->curr_taskid, curr->curr_file_name, &len);

			if ((suffix = strstr(curr->curr_file_name, ".td")) != NULL)
			{
				suffix[0] = 0;
			}
			curr_name_len = strlen(curr->curr_file_name);
			PUSH_ERR_RETURN((void*)&curr_name_len, sizeof(int));
			if (curr_name_len > 0)
			{
				PUSH_ERR_RETURN(curr->curr_file_name, strlen(curr->curr_file_name));
			}
			else
			{
				xfer_free(curr->curr_file_name);
				curr->curr_file_name = NULL;
			}
		}
		
		PUSH_ERR_RETURN((void*)&curr->task_stat, sizeof(transfer_task_stat_e));
		PUSH_ERR_RETURN((void*)&curr->total_size, sizeof(int));
		PUSH_ERR_RETURN((void*)&curr->downloaded_size, sizeof(int));
		PUSH_ERR_RETURN((void*)&curr->type, sizeof(transfer_type_e));
		if(curr->type == XFER_TASK_XLBT)
		{
			PUSH_ERR_RETURN((void*)&curr->bt_files_num, sizeof(u32));
			if(curr->bt_files_num > 0)
			{	
				PUSH_ERR_RETURN(curr->bt_files_index, curr->bt_files_num * sizeof(u32));
			}
		}

		PUSH_ERR_RETURN((void*)&curr->file_created, sizeof(int));
	}
	thunder_list_sem_unlock();

	res = write_file_safe(thunder_mgr_taskinfo, mem_buf->buff, mem_buf->length);
	
	if (mem_buf)
	{
		if (mem_buf->buff)
			xfer_free(mem_buf->buff); 
		xfer_free(mem_buf); 
	}
	
	return res;
}
#undef PUSH_ERR_RETURN

xfer_errid_t thunder_unload_tasks()
{
	thunder_list_sem_lock();
	s_thunder_tasks_loaded = 0;
	thunder_list_sem_unlock();

	thunder_list_clear();

	return XFER_ERROR_OK;
}

static int thunder_license_result = 0;
static int thunder_license_expire = 0;
int32 notify_license_result(u32 result, u32 expire_time)
{
	debugf(XFER_LOG_THUNDER, "\n@@@@@ thunder license notify result=%d, expire_time=%d\n", result, expire_time);
	thunder_license_result = result;
	thunder_license_expire = expire_time;

	return 0;
}

static int s_set_license_times = 10 ;
static int s_dly_times = 1; /* 1s */

/* Set to 1, thunder lib can use fixed memory */
static int g_thunder_fix_mem_enable = 0;
/* if U need share thunder memory , make sure the following value is correct */
void * s_thunder_buff = (void *)0x80001000; //fixed memory address
#define TUNDER_FIX_MEM_SIZE (0x1000000)
static int s_thunder_fix_mem_malloced = 0;

xfer_errid_t
thunder_mgr_set_customed_malloc(int memsize, void **mem)
{
	if(g_thunder_fix_mem_enable && (memsize >= TUNDER_FIX_MEM_SIZE))
	{
		if(s_thunder_fix_mem_malloced == 0)
		{
			*mem = (void*)s_thunder_buff;
			s_thunder_fix_mem_malloced = 1;
			debugf(XFER_LOG_MAIN, "\n@@@@@@@@@@ thunder malloc SUCCESS, memsize=%d\n", memsize);
			return 0;
		}
	}
	else 
	{
		*mem = xfer_malloc(memsize);
		if(*mem)
		{
			debugf(XFER_LOG_MAIN, "\n@@@@@@@@@@ thunder malloc SUCCESS, memsize=%d\n", memsize);
			return 0;
		}
	}
	 
	debugf(XFER_LOG_MAIN, "\n@@@@@@@@@@ thunder malloc FAILED, memsize=%d\n", memsize);
	return -1;
}

xfer_errid_t
thunder_mgr_set_customed_free(void* mem, int memsize)
{
	debugf(XFER_LOG_MAIN, "\n@@@@@@@@@@ thunder free, mem=0x%x, memsize=%d\n", (int)mem, memsize);
	if(g_thunder_fix_mem_enable && (mem == (void*)s_thunder_buff))
	{
		if(!s_thunder_fix_mem_malloced)
			return -1;

		s_thunder_fix_mem_malloced = 0;
	}
	else
	{
		xfer_free(mem);
	}
	return 0;
}

xfer_errid_t
thunder_mgr_set_customed_allocator(int fun_idx, void *fun_ptr)
{
	return et_set_customed_interface_mem(fun_idx, fun_ptr);
}

int32 
thunder_mgr_fs_open(char *filepath, int32 flag, u32 *file_id)
{
	int fd = -1 ;
	if( (flag & 0x01) == 0x01 )
	{
		fd = open(filepath, O_CREAT|O_RDWR, S_IRWXG|S_IRWXU) ;
	}
	else if((flag & 0x02) == 0x02 ) 
	{
		fd = open(filepath, O_RDONLY, S_IRWXG|S_IRWXU) ;
	}
	else if((flag & 0x04) == 0x04)
	{
		fd = open(filepath, O_WRONLY, S_IRWXG|S_IRWXU) ;
	}
	else
	{
		fd = open(filepath, O_RDWR, S_IRWXG|S_IRWXU) ;
	}

	debugf(XFER_LOG_MAIN, "\n\033[7m@@@@@ fs_open(), filepath=%s, flag=%d, fd=%d\033[0m\n", filepath, flag, fd);
	if (0 == fd)
	{
		close(fd);
		*file_id = -1;
		return -1;
	}
	
	*file_id = fd;
	return 0;		
}

int32 
thunder_mgr_fs_close(u32 file_id)
{
	debugf(XFER_LOG_MAIN, "\n\033[7m@@@@@ fs_close(), fd=%d\033[0m\n", file_id);
	return close(file_id);		
}

int32 
thunder_mgr_fs_pwrite(u32 file_id, char *buffer, int32 size, uint64 filepos, u32 *writesize)
{
	ssize_t write_size = 0;
	char ibuf[32] = {0};

	strncpy(ibuf, buffer, 3);
	memset(ibuf, 0x0, 32);
	ibuf[4] = '\0';
	debugf(XFER_LOG_MAIN, "\n\033[7m@@@@@ fs_pwrite(), fd=%d, size=%d, filepos=%d\033[0m\n", file_id, size, filepos);
	debugf(XFER_LOG_MAIN, "[%s]\n", ibuf); 
	write_size = pwrite(file_id, buffer, size, filepos);
	*writesize = write_size;
	debugf(XFER_LOG_MAIN, "\n\033[7m fs_pwrite(), *writesize=%d\033[0m\n",	write_size);

	if (*writesize < 0)
		return -1;		
	else
		return 0;		
}

int32
thunder_mgr_fs_enlarge_file(u32 file_id, uint64 expect_filesize, uint64 *cur_filesize)
{
	u32 file_pos = 0;
	int write_size = 0;
	debugf(XFER_LOG_MAIN, "\033[7m@@@@@ lseek START, fd=%d, expect_filesize=%d, file_pos=%d\033[0m\n", file_id, (int)expect_filesize, file_pos);
	file_pos = lseek(file_id, expect_filesize - 1, SEEK_SET);
	if (file_pos < 0) 
	{
		debugf(XFER_LOG_MAIN, "\033[7m@@@@@ lseek ERROR, fd=%d, expect_filesize=%d, file_pos=%d\033[0m\n", file_id, (int)expect_filesize, file_pos);
		*cur_filesize = 0;
		return -1;
	}
	write_size = pwrite(file_id, "&", 1, expect_filesize - 1);
	if (write_size <= 0)
	{
		*cur_filesize = 0;
		debugf(XFER_LOG_MAIN, "pwrite() ERROR\n");
	}
	else
		*cur_filesize = expect_filesize;

	debugf(XFER_LOG_MAIN, "\n\033[7m@@@@@ fs_enlarge_file(), file_id=%d, expect_filesize=%d, cur_filesize=%d\033[0m\n", file_id, (int)expect_filesize, (int)*cur_filesize);
	return 0;
}

xfer_errid_t
thunder_mgr_set_customed_interface(int idx, void *fun_ptr)
{
	return et_set_customed_interface(idx, fun_ptr);
}

xfer_errid_t thunder_mgr_init(char* lic)
{
	int i = 0;
	xfer_errid_t errid = XFER_ERROR_OK;

	if (!s_thunder_inited)
	{
		errid = thunder_mgr_set_customed_allocator(0, (void *)thunder_mgr_set_customed_malloc);
		errid = thunder_mgr_set_customed_allocator(1, (void *)thunder_mgr_set_customed_free);
		errid = thunder_mgr_set_customed_interface(ET_FS_IDX_ENLARGE_FILE, thunder_mgr_fs_enlarge_file);
		//errid = thunder_mgr_set_customed_interface(ET_FS_IDX_PWRITE, thunder_mgr_fs_pwrite);
		errid = thunder_mgr_set_customed_interface(ET_FS_IDX_OPEN, thunder_mgr_fs_open);
		errid = thunder_mgr_set_customed_interface(ET_FS_IDX_CLOSE, thunder_mgr_fs_close);
		errid = et_init(NULL);
		if (errid == XFER_ERROR_OK)
		{
			et_start_http_server(8889);
			s_thunder_inited = 1;
			et_set_license_callback(notify_license_result);
			for(i = 0 ; i < s_set_license_times ; i++)
			{
				if (lic != NULL)
					errid = et_set_license(lic, strlen(lic));
				else
					errid = et_set_license(xunlei_licence, strlen(xunlei_licence));
				if( 0 == errid )
					break ;
				else
					sleep(s_dly_times);
			}	
		}
		errid = et_vod_set_vod_buffer_size(s_thunder_vod_buf_size);
		debugf(XFER_LOG_MAIN, "$$$ set thunder vod buf size=%d  errid=%d\n", s_thunder_vod_buf_size, errid);

		thunder_mgr_setopt(DLOPT_RATELIMIT_MAX_CONNS, 18);
	}

	if (errid == XFER_ERROR_OK)
		errid = thunder_list_init();

	return errid;
}

void thunder_mgr_fini()
{
	if (!s_thunder_inited)
		return;

	thunder_list_sem_lock();
	s_thunder_tasks_loaded = 0;
	thunder_list_sem_unlock();

	thunder_list_clear();
	
	thunder_list_sem_close();

	et_stop_http_server();
	et_uninit();

	s_thunder_inited = 0;
}

xfer_errid_t thunder_mgr_getopt(transfer_options_e opt, int * result)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if (result == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if (opt == DLOPT_RATELIMIT_DOWNLOAD)
	{
		unsigned int upload = 0;
		errid = et_get_limit_speed((unsigned int*)result, &upload);
	}
	else if (opt == DLOPT_RATELIMIT_UPLOAD)
	{
		unsigned int download = 0;
		errid = et_get_limit_speed(&download, (unsigned int*)result);
	}
    else if (opt == DLOPT_RATELIMIT_MAX_TASKS)
    {
		*result = et_get_max_tasks();
    }
    else if (opt == DLOPT_RATELIMIT_MAX_CONNS)
    {
		*result = et_get_max_task_connection();
    }
	
	return errid;
}

xfer_errid_t thunder_mgr_setopt(transfer_options_e opt, int value)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	
	if (opt == DLOPT_RATELIMIT_DOWNLOAD)
	{
		unsigned int download = 0;
		unsigned int upload = 0;
		et_get_limit_speed(&download, &upload);
		errid = et_set_limit_speed(value/1024, upload);
	}
	else if (opt == DLOPT_RATELIMIT_UPLOAD)
	{
		unsigned int download = 0;
		unsigned int upload = 0;
		et_get_limit_speed(&download, &upload);
		errid = et_set_limit_speed(download, value/1024);
	}
    else if (opt == DLOPT_RATELIMIT_MAX_TASKS)
    {
		errid = et_set_max_tasks(value);
    }
    else if (opt == DLOPT_RATELIMIT_MAX_CONNS)
    {
		errid = et_set_max_task_connection(value);
    }
    else if (opt == DLOPT_RATELIMIT_SEED_SWITCH)
    {
		errid = et_set_seed_switch_type(value);
    }
	
	return errid;
}

#define MIN_BT_FILE_SIZE 50*1024
typedef struct 
{
	int task_type;  /* 0: p2sp task,1: bt task */
	
	char url[512];
	char refer_url[512];
	char dir[256];
	char filename[256];
	unsigned char cid[20];
	uint64 filesize;
	#ifdef  NOT_SUPPORT_LARGE_INT_64
	u32 _padding1;
	#endif
	uint64 recv_bytes;
	#ifdef  NOT_SUPPORT_LARGE_INT_64
	u32 _padding2;
	#endif
	int is_created;
	int is_cid;
	unsigned int task_id;

	/* Just for bt task */
	char seed_file_path[256];
	unsigned int  seed_file_path_len;
	unsigned int  download_file_index_array[30];
	unsigned int  file_num;
	int is_default_mode;
	 
}TASK_PARA;

static int
parse_seed_info(char *seed_path, TASK_PARA *p_task_para )
{
	int ret_val = 0, i = 0;
	ET_TORRENT_SEED_INFO * _p_torrent_seed_info =NULL;
	ET_TORRENT_FILE_INFO *file_info_array_ptr=NULL;
	et_set_seed_switch_type(1);
	/* Get the torrent seed file information from seed_path and store the information in the struct: pp_seed_info */
	ret_val = et_get_torrent_seed_info(seed_path, &_p_torrent_seed_info );
	if(ret_val)
	{
		return ret_val;
	}
	if( p_task_para != NULL )
	{
		p_task_para->file_num = 0;		
	}

	file_info_array_ptr = _p_torrent_seed_info->file_info_array_ptr;
	
	for(i=0;i<_p_torrent_seed_info->_file_num;i++)
	{
		if( file_info_array_ptr->_file_size > MIN_BT_FILE_SIZE && p_task_para != NULL )
		{
			p_task_para->download_file_index_array[p_task_para->file_num] = file_info_array_ptr->_file_index;
			p_task_para->file_num++;
		}
		file_info_array_ptr++;
	}
	
	//printf("\n******************* End of torrent seed information ******************\n\n");

	/************************* End of display the torrent seed information to the screen *********************************/
	/* Release the memory in the struct:p_seed_info which malloc by  et_get_torrent_seed_info */
	et_release_torrent_seed_info( _p_torrent_seed_info );
	return 0;
}

/*
 Convert the sting(40 chars hex ) to content id(20 bytes in hex)
 */
static int string_to_cid(const char* str, unsigned char* cid)
{
	int i = 0, is_cid_ok = 0, cid_byte = 0;

	if ((str == NULL) || (strlen(str) != 40) || (cid == NULL))
		return -1;

	for (i = 0; i < 20; i++)
	{
		if (((str[i * 2]) >= '0') && ((str[i * 2]) <= '9'))
		{
			cid_byte = (str[i * 2] - '0') * 16;
		}
		else if (((str[i * 2]) >= 'A') && ((str[i * 2]) <= 'F'))
		{
			cid_byte = ((str[i * 2] - 'A') + 10) * 16;
		}
		else if (((str[i * 2]) >= 'a') && ((str[i * 2]) <= 'f'))
		{
			cid_byte = ((str[i * 2] - 'a') + 10) * 16;
		}
		else
			return -1;

		if (((str[i * 2 + 1]) >= '0') && ((str[i * 2 + 1]) <= '9'))
		{
			cid_byte += (str[i * 2 + 1] - '0');
		}
		else if (((str[i * 2 + 1]) >= 'A') && ((str[i * 2 + 1]) <= 'F'))
		{
			cid_byte += (str[i * 2 + 1] - 'A') + 10;
		}
		else if (((str[i * 2 + 1]) >= 'a') && ((str[i * 2 + 1]) <= 'f'))
		{
			cid_byte += (str[i * 2 + 1] - 'a') + 10;
		}
		else
			return -1;

		cid[i] = cid_byte;
		if ((is_cid_ok == 0) && (cid_byte != 0))
			is_cid_ok = 1;
	}

	if (is_cid_ok == 1)
		return 0;
	else
		return 1;

}

xfer_errid_t thunder_mgr_add_pending(const char* 			src, 
							 	  	const char* 			saved_path,
							 	  	transfer_type_e 	 	type,
							 	  	const char* 			file_name,
							 	  	transfer_taskid_t*		taskid)
{
	*taskid = get_task_id();
	thunder_list_pushback(*taskid, 0, src, saved_path, file_name, type);
	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();
	
	return XFER_ERROR_OK;
}

xfer_errid_t thunder_mgr_add_bt_pending(const char* 			src_url,
									const char*			src,
									int					file_length,
							 	  	const char* 			saved_path,
							 	  	transfer_type_e 	 	type,
							 	  	const char* 			file_name,
							 	  	const u32			file_num,
							 	  	const u32*			file_index_array,
							 	  	transfer_taskid_t*		taskid)
{
	*taskid = get_task_id();
	
	thunder_list_pushback_ex(*taskid, 0, src, saved_path, file_name, type, file_length);	
	thunder_list_set_bt_files_index(*taskid, (u32)file_num, (u32*)file_index_array);
	thunder_mgr_set_task_src_url(*taskid, (char *)src_url);

	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();
	
	return XFER_ERROR_OK;
}

xfer_errid_t thunder_mgr_add_pending_ex(const char* 			src, 
									int 					file_length,
							 	  	const char* 			saved_path,
							 	  	transfer_type_e 	 	type,
							 	  	const char* 			file_name,
							 	  	transfer_taskid_t*		taskid)
{
	*taskid = get_task_id();
	thunder_list_pushback_ex(*taskid, 0, src, saved_path, file_name, type, file_length);
	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();
	
	return XFER_ERROR_OK;
}

int thunder_mgr_task_exist(const char* src, const char* file_path, const char* file_name)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return 0;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (strcmp(node->src, src) == 0 || 
			(strcmp(node->file_path, file_path) == 0 && strcmp(node->file_name, file_name) == 0))
		{
			thunder_list_sem_unlock();
			return 1;
		}
	}
	thunder_list_sem_unlock();

	return 0;
}

int thunder_mgr_task_bt_exist(const char* src, const char* file_path, const char* file_name, const u32 file_num, const u32 *file_index_array)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;
	int i = 0;
	int j = 0;
	int same_file_num = 0;

	if(s_thunder_tasks.node.link.next == NULL)
		return 0;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (	(node->type == XFER_TASK_XLBT) &&
			(strcmp(node->src, src) == 0 || 
			(strcmp(node->file_path, file_path) == 0 && 
			strcmp(node->file_name, file_name) == 0)))
		{
			for(i = 0; i < file_num; i++)
			{
				for(j = 0; j < node->bt_files_num; j++)
				{
					if(file_index_array[i] == node->bt_files_index[j])
						same_file_num++;
				}
			}

			thunder_list_sem_unlock();
			if(same_file_num == file_num)
			{
				return 1;
			}
			else
			{
				return 0;
			}
		}
	}
	thunder_list_sem_unlock();

	return 0;

}

xfer_errid_t thunder_mgr_task_add( const char* 			src, 
							 	  const char* 			saved_path,
							 	  transfer_type_e 	 	type,
							 	  const char* 			file_name,
							 	  transfer_taskid_t*	taskid)
{
	return thunder_mgr_task_add_ex(src, 0, NULL, saved_path, type, file_name, taskid);
}


xfer_errid_t thunder_mgr_task_add_ex(const char* 		src, 
								  	int					file_size,
									const char* 		gcid,
							 	  	const char* 		saved_path,
							 	  	transfer_type_e 	type,
							 	  	const char*      	file_name,
							 	  	transfer_taskid_t* 	taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	u32         thunder_taskid = 0;

	if( !gcid && type != XFER_TASK_XLGCID)
	{
		if (!dir_exist((char*)saved_path) && xfer_mkdir_safe((char *)saved_path) != XFER_ERROR_OK)
		{
			debugf(XFER_LOG_THUNDER, "thunder add task,mkdir error!\n");
			return XFER_ERROR_STORAGE_CREATE_PATH_ERR;
		}
	}

	if (type == XFER_TASK_XLURL)
	{
		errid = et_create_new_task_by_url((char*)src, strlen(src), NULL, 0, NULL, 0, 
			(char*)saved_path, strlen(saved_path), (char*)file_name, strlen(file_name), &thunder_taskid);
		debugf(XFER_LOG_THUNDER, "et_create_new_task_by_url(): errid=%d\n", errid);
		debugf(XFER_LOG_THUNDER, "src=%s, saved_path=%s\n", src, saved_path);
	}
	else if (type == XFER_TASK_XLTCID)
	{
		u8 cid[20];
		string_to_cid(src, cid);
		errid = et_create_new_task_by_tcid(cid, (u64_t)0, (char*)file_name, strlen(file_name), 
			(char*)saved_path, strlen(saved_path), &thunder_taskid);
		debugf(XFER_LOG_THUNDER, "et_create_new_task_by_tcid(), errid=%d\n", errid);
		debugf(XFER_LOG_THUNDER, "cid=%d, src=%s, saved_path=%s, file_name=%s,", cid, src, saved_path, file_name);
	}
	else if (type == XFER_TASK_XLBT)
	{
	//plz use thunder_mgr_task_add_bt() to create thunder bt task.
	/*
		TASK_PARA task_para;

		if ((errid = parse_seed_info((char*)src, &task_para)) == XFER_ERROR_OK)
			errid = et_create_new_bt_task((char*)src, strlen(src), (char*)saved_path, strlen(saved_path), 
				task_para.download_file_index_array, task_para.file_num, &thunder_taskid);
	*/
	}
#ifdef SUPPORT_THUNDER_VERSION13
	else if (type == XFER_TASK_XLGCID)
	{
		u8 cid[20];
		u8 gcid20[20];
		string_to_cid(src, cid);
		string_to_cid(gcid, gcid20);
		if( gcid )
		{
			errid = et_create_task_by_tcid_file_size_gcid(cid, file_size, gcid20, (char*)file_name, strlen(file_name), 
				(char*)kankan_tmp_path, strlen(kankan_tmp_path), &thunder_taskid);
			debugf(XFER_LOG_THUNDER, "[1]cid=%s, gcid=%s, saved_path=%s, file_name=%s,", src, gcid, saved_path, file_name);
			debugf(XFER_LOG_THUNDER, "et_create_task_by_tcid_file_size_gcid(), errid=%d\n", errid);
		}
		else
		{
			errid = et_create_task_by_tcid_file_size_gcid(cid, file_size, gcid20, (char*)file_name, strlen(file_name), 
				(char*)saved_path, strlen(saved_path), &thunder_taskid);
			debugf(XFER_LOG_THUNDER, "[2]cid=%s, gcid=%s, saved_path=%s, file_name=%s,", src, gcid, saved_path, file_name);
			debugf(XFER_LOG_THUNDER, "et_create_task_by_tcid_file_size_gcid(), errid=%d\n", errid);
		}
		//by david liu, force thunder lib to fetch index, 09.07.21 
		//char tmp_buf[64*1024];
		//if(errid == XFER_ERROR_OK&& thunder_taskid)
		//	et_vod_read_file(thunder_taskid, 0, 64*1024, tmp_buf, 100);
		//////
	}
#endif

	if (errid == XFER_ERROR_OK)
	{	
		*taskid = get_task_id();
		if (file_name)
		{
			thunder_list_pushback(*taskid, thunder_taskid, src, saved_path, file_name, type);
		}
		else
		{
			char* name = NULL;
			char* pos = strrchr(src, '/');
			if (pos)
			{
				name = (char*)xfer_malloc(strlen(pos)+1);
				if (name != NULL)
				{
					strcpy(name, pos+1);
				}
			}
			thunder_list_pushback(*taskid, thunder_taskid, src, saved_path, name, type);
			if (name)
			{
				xfer_free(name);
				name = NULL;
			}
		}
	}
	else
	{
		*taskid = TASKID_INVALID;
	}
	
	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	//debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();

	debugf(XFER_LOG_THUNDER, "thunder add task finish: taskid=%d, errid=%d\n", *taskid, errid);
	return errid;
}

xfer_errid_t thunder_mgr_task_add_bt(const char* src_url,
									const char* 		seed_file_path, 
							 	  	const char* 		saved_path,
							 	  	transfer_type_e 	type,
							 	  	const char*      	file_name,
							 	  	u32*				file_index_array,
							 	  	u32				file_num,
							 	  	transfer_taskid_t* 	taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	u32 thunder_taskid = 0;

	if (!dir_exist((char*)saved_path) && xfer_mkdir_safe((char *)saved_path) != XFER_ERROR_OK)
		return XFER_ERROR_STORAGE_CREATE_PATH_ERR;

	if (type == XFER_TASK_XLBT)
	{
		errid = et_create_bt_task((char*)seed_file_path, 
									strlen(seed_file_path), 
									(char*)saved_path, 
									strlen(saved_path), 
									file_index_array,
									file_num,
									1,
									&thunder_taskid);
		debugf(XFER_LOG_THUNDER, "et_create_bt_task(), errid=%d\n", errid);
	}

	if (errid == XFER_ERROR_OK)
	{	
		*taskid = get_task_id();
		if (file_name)
		{
			thunder_list_pushback(*taskid, thunder_taskid, seed_file_path, saved_path, file_name, type);
			thunder_list_set_bt_files_index(*taskid, file_num, file_index_array);
			thunder_mgr_set_task_src_url(*taskid, (char *)src_url);			
		}
		else
		{
			char* name = NULL;
			char* pos = strrchr(seed_file_path, '/');
			if (pos)
			{
				name = (char*)xfer_malloc(strlen(pos)+1);
				if (name != NULL)
				{
					strcpy(name, pos+1);
				}
			}
			thunder_list_pushback(*taskid, thunder_taskid, seed_file_path, saved_path, name, type);
			thunder_list_set_bt_files_index(*taskid, file_num, file_index_array);
			thunder_mgr_set_task_src_url(*taskid, (char *)src_url);			
			if (name)
			{
				xfer_free(name);
				name = NULL;
			}
		}
	}
	else
	{
		*taskid = TASKID_INVALID;
	}
	
	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();
	return errid;
}

static void delete_task_file(transfer_taskid_t taskid)
{
	int err_id = 0;
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return;

	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid && node->type != XFER_TASK_XLBT)
		{
			err_id = et_remove_tmp_file(node->file_path, node->file_name);
			break;
		}
	}
	thunder_list_sem_unlock();
}
void thunder_mgr_task_close(transfer_taskid_t taskid)
{	
	int err_id = 0;
	transfer_taskid_t thunder_taskid = get_curr_taskid(taskid);

	debugf(XFER_LOG_THUNDER, "thunder_mgr_task_close: taskid=%d\n", taskid);
	err_id = et_stop_task(thunder_taskid);
	debugf(XFER_LOG_THUNDER, "et_stop_task: thunder_taskid=%d errid=%d\n", thunder_taskid, err_id);

	err_id = et_remove_task_tmp_file(thunder_taskid);
	debugf(XFER_LOG_THUNDER, "et_remove_task_tmp_file: thunder_taskid=%d errid=%d\n", thunder_taskid, err_id);
	while(1)
	{
		err_id = et_delete_task(thunder_taskid);  
		if(err_id!=4109&&err_id!=4119)
		{
			debugf(XFER_LOG_THUNDER, "*******thunder_id=%d errid=%d\n",thunder_taskid,err_id);
			break;
		}
		if(err_id == 4109)
			err_id = et_stop_task(thunder_taskid);
		debugf(XFER_LOG_THUNDER, "******* Stop Task Retry. thunder_id=%d errid=%d\n", thunder_taskid, err_id);
		usleep(delay_for_delete);
	}

	//delete_task_file(taskid);
	thunder_list_del(taskid);
	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();
}

xfer_errid_t thunder_mgr_task_start(transfer_taskid_t taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	int status = thunder_list_get_status(taskid);
	transfer_taskid_t thunder_taskid = get_curr_taskid(taskid);
	
	debugf(XFER_LOG_THUNDER, "thunder_mgr_task_start: taskid=%d\n", taskid);
	if (status == -1)
		return XFER_ERROR_HTTP_TASKID_ERR;
	if (status == DLST_DOWNLOADING)
		return XFER_ERROR_OK;
	
	if (status != DLST_NONE || 0 == thunder_taskid)
	{
		list_t *next, *cur;
		thunder_mgr_node_t* node = NULL;

		if(s_thunder_tasks.node.link.next == NULL)
			return XFER_ERROR_OK;
		
		thunder_list_sem_lock();
		list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
		{
			node = list_entry(cur, thunder_mgr_node_t, link);
			if (node->taskid == taskid)
			{
				u32 task_id;
				int err_id = 0;
				et_stop_task(thunder_taskid);
				while(1)
				{
					err_id = et_delete_task(thunder_taskid);  
					if(err_id!=4109&&err_id!=4119)
					{
						debugf(XFER_LOG_THUNDER, "[%s:%d]et_delete_task(), thunder_id=%d errid=%d\n", __FILE__, __LINE__, thunder_taskid,err_id);
						break;
					}
					if(err_id == 4109)
						et_stop_task(thunder_taskid);
					usleep(delay_for_delete);
				}
				if (node->type == XFER_TASK_XLURL)
				{
					if(node->curr_file_name)
					{
						errid = et_create_continue_task_by_url(node->src, strlen(node->src), 
							NULL, 0, 
							NULL, 0, 
							node->file_path, strlen(node->file_path),
							node->file_name, strlen(node->file_name), &task_id);
					}
					else
					{
						errid = et_create_continue_task_by_url(node->src, strlen(node->src), 
							NULL, 0, 
							NULL, 0, 
							node->file_path, strlen(node->file_path),
							node->file_name, strlen(node->file_name), &task_id);
					}
					debugf(XFER_LOG_THUNDER, "et_create_continue_task_by_url, errid=%d\n", errid);
				}
				else if (node->type == XFER_TASK_XLTCID)
				{
					u8 cid[20];
					string_to_cid(node->src, cid);
					errid = et_create_continue_task_by_tcid(cid, 
						node->file_name, strlen(node->file_name), 
						node->file_path, strlen(node->file_path), &task_id);
					debugf(XFER_LOG_THUNDER, "et_create_continue_task_by_tcid, errid=%d\n", errid);
				}
				else if (node->type == XFER_TASK_XLBT)
				{
					errid = et_create_bt_task(node->src, strlen(node->src), node->file_path, strlen(node->file_path), node->bt_files_index, node->bt_files_num, 1, &task_id);
					debugf(XFER_LOG_THUNDER, "et_create_bt_task, errid=%d\n", errid);
				/*
					TASK_PARA task_para;

					if ((errid = parse_seed_info(node->src, &task_para)) == XFER_ERROR_OK)
					{
						errid = et_create_new_bt_task(node->src, strlen(node->src), 
							node->file_path, strlen(node->file_path), 
							task_para.download_file_index_array, task_para.file_num, &task_id);
					}
				*/	
				}
#ifdef SUPPORT_THUNDER_VERSION13
				else if (node->type == XFER_TASK_XLGCID)
				{
					u8 cid[20];
					u8 gcid20[20];
					string_to_cid(node->src, cid);
					string_to_cid(node->gcid, gcid20);
					errid = et_create_task_by_tcid_file_size_gcid(cid, node->total_size, gcid20, (char*)node->file_name, strlen(node->file_name), 
						(char*)node->file_path, strlen(node->file_path), &task_id);
					debugf(XFER_LOG_THUNDER, "et_create_task_by_tcid_file_size_gcid, errid=%d\n", errid);
				}
#endif
				
				if (errid == XFER_ERROR_OK)
				{
					node->curr_taskid = thunder_taskid = task_id;
					break;
				}
				else
				{
					if (errid == 4202 || errid == 6159 || errid == 4199)
					{
						//printf("continue task %d err(%d)\n", node->taskid, errid);
						et_remove_tmp_file(node->file_path, node->file_name); // remove *.td and *.td.cfg
						
						if (!dir_exist(node->file_path) && xfer_mkdir_safe(node->file_path) != XFER_ERROR_OK)
						{
							node->err = XFER_ERROR_STORAGE_CREATE_PATH_ERR;
							node->task_stat = DLST_ERROR;
							thunder_list_sem_unlock();
							return XFER_ERROR_STORAGE_CREATE_PATH_ERR;
						}

						if (node->type == XFER_TASK_XLURL)
						{
							errid = et_create_new_task_by_url(node->src, strlen(node->src), NULL, 0, NULL, 0, 
								node->file_path, strlen(node->file_path), node->file_name, strlen(node->file_name), &task_id);
							debugf(XFER_LOG_THUNDER, "et_create_new_task_by_url, errid=%d\n", errid);
						}
						else if (node->type == XFER_TASK_XLTCID)
						{
							u8 cid[20];
							string_to_cid(node->src, cid);
							errid = et_create_new_task_by_tcid(cid, (u64_t)0, node->file_name, strlen(node->file_name), 
								node->file_path, strlen(node->file_path), &task_id);
							debugf(XFER_LOG_THUNDER, "et_create_new_task_by_tcid, errid=%d\n", errid);
						}
						else if (node->type == XFER_TASK_XLBT)
						{
							errid = et_create_bt_task(node->src, strlen(node->src), node->file_path, strlen(node->file_path), node->bt_files_index, node->bt_files_num, 1, &task_id);
							debugf(XFER_LOG_THUNDER, "et_create_bt_task, errid=%d\n", errid);
						/*
							TASK_PARA task_para;

							if ((errid = parse_seed_info(node->src, &task_para)) == XFER_ERROR_OK)
								errid = et_create_new_bt_task(node->src, strlen(node->src), node->file_path, strlen(node->file_path), 
									task_para.download_file_index_array, task_para.file_num, &task_id);
						*/
						}
#ifdef SUPPORT_THUNDER_VERSION13
						else if (node->type == XFER_TASK_XLGCID)
						{
							u8 cid[20];
							u8 gcid20[20];
							string_to_cid(node->src, cid);
							string_to_cid(node->gcid, gcid20);
							errid = et_create_task_by_tcid_file_size_gcid(cid, (u64_t)0, gcid20, (char*)node->file_name, strlen(node->file_name), 
								(char*)node->file_path, strlen(node->file_path), &task_id);
							debugf(XFER_LOG_THUNDER, "et_create_task_by_tcid_file_size_gcid, errid=%d\n", errid);
						}
#endif
						if (node->curr_file_name)
						{
							xfer_free(node->curr_file_name);
							node->curr_file_name = NULL;
						}
						node->curr_taskid = thunder_taskid = task_id;
					}
					
					if (errid != XFER_ERROR_OK)
					{
						node->err = errid;
						node->task_stat = DLST_ERROR;
						thunder_list_sem_unlock();
						return errid;
					}
					break;
				}
			}
		}
		thunder_list_sem_unlock();
		
	}
#ifdef SUPPORT_THUNDER_KANKAN
	if( 0 == s_b_disk )
	{
		b_enable_kankan = 1;
		//realplayer_set_kankan_status(b_enable_kankan);		
		//printf( "set the no disk !\n" ) ;
		errid = et_set_task_no_disk( thunder_taskid ) ;
		debugf(XFER_LOG_THUNDER,  "et_set_task_no_disk(), errid=%d, thunder_taskid=%d\n", errid, thunder_taskid) ;
	}
	else
	{
		b_enable_kankan = 0 ;	
		//realplayer_set_kankan_status(b_enable_kankan);	
	}
#else
	b_enable_kankan = 0 ;	
	//realplayer_set_kankan_status(b_enable_kankan);
#endif	
 	if(errid)
		return errid;
	
	errid = et_start_task(thunder_taskid);
	debugf(XFER_LOG_THUNDER, "et_start_task, thunder_taskid=%d, errid=%d\n", thunder_taskid, errid);
	if (errid == XFER_ERROR_OK)
	{
#ifdef SUPPORT_THUNDER_KANKAN
	/*if( 0 == s_b_disk )
	{
		char tmp_buf[64*1024];
		if(errid == XFER_ERROR_OK&& thunder_taskid)
			et_vod_read_file(thunder_taskid, 0, 64*1024, tmp_buf, 100);
	}*/
#endif
		thunder_list_set_status(taskid, DLST_DOWNLOADING, 0);
	}
	else
	{
		thunder_list_set_status(taskid, DLST_ERROR, errid);
	}

	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();

	debugf(XFER_LOG_THUNDER, "thunder_mgr_task start: taskid=%d, errid=%d\n", taskid, errid);
	return errid;
}

xfer_errid_t thunder_mgr_task_pause(transfer_taskid_t taskid)
{
	int err_id = 0;
	xfer_errid_t errid = XFER_ERROR_OK;
	transfer_taskid_t thunder_taskid = get_curr_taskid(taskid);

	thunder_list_set_status(taskid, DLST_STOPPED, 0);
	/*jpf
	file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
	*/
	debugf(XFER_LOG_THUNDER, "%s:%d->thunder_save_tasks()\n", __FILE__, __LINE__);
	thunder_save_tasks();

	et_stop_task(thunder_taskid);
	while(1)
	{
		err_id = et_delete_task(thunder_taskid);  
		if(err_id!=4109&&err_id!=4119)
		{
//			printf("***********************thunder_id=%d errid=%d\n",thunder_taskid,err_id);
			break;
		}
		if(err_id == 4109)
			et_stop_task(thunder_taskid);
		usleep(delay_for_delete);
	}

	return errid;
}

xfer_errid_t thunder_mgr_get_task_list(transfer_task_list_t** tasklist)
{
	transfer_taskid_t*  pos = NULL;

	*tasklist = (transfer_task_list_t*)xfer_malloc(sizeof(transfer_task_list_t));
	if (*tasklist == NULL)
	{
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*tasklist, 0, sizeof(transfer_task_list_t));
	if (s_thunder_tasks.count == 0)
	{
		return XFER_ERROR_OK;
	}
	(*tasklist)->task_count = s_thunder_tasks.count;
	(*tasklist)->task_ids = thunder_list_taskids();
	return XFER_ERROR_OK;
}

static transfer_task_stat_e
task_status(enum ET_TASK_STATUS status)
{
	switch (status)
	{
		case ET_TASK_IDLE:
			return DLST_NONE;
		case ET_TASK_RUNNING:
			return DLST_DOWNLOADING;
		case ET_TASK_SUCCESS:
			return DLST_FINISHED;
		case ET_TASK_FAILED:
			return DLST_ERROR;
		case ET_TASK_STOPPED:
			return DLST_STOPPED;
		default:
			return DLST_NONE;
	}
}

static xfer_errid_t 
get_task_path_and_name(transfer_taskid_t taskid, char** path, char** name)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;
	int is_save = 0;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			if (node->curr_file_name == NULL || strlen(node->curr_file_name) == 0)
			{
				int len = 512;
				char* suffix = NULL;
				if (node->curr_file_name == NULL)
				{
					node->curr_file_name = (char*)xfer_malloc(len);
					memset(node->curr_file_name, 0, len);
				}
				et_get_task_file_name(node->curr_taskid, node->curr_file_name, &len);

				if (node->curr_file_name && (suffix = strstr(node->curr_file_name, ".td")) != NULL)
				{
					suffix[0] = 0;
					is_save = 1;
				}
				if (node->curr_file_name && (strlen(node->curr_file_name) == 0))
				{
					xfer_free(node->curr_file_name);
					node->curr_file_name = NULL;
				}
			}

			*path = (char*)xfer_malloc(strlen(node->file_path)+1);
			*name = (char*)xfer_malloc(strlen(node->file_name)+1);
			if (*path)
			{
				strcpy(*path, node->file_path);
			}
			if (*name)
			{
				strcpy(*name, node->file_name);
			}
			break;
		}
	}
	thunder_list_sem_unlock();

	/*jpf
	if (is_save)
		file_task_send_msg(TASKID_INVALID, NULL, THUNDER_SAVE_STAT, NULL, 0, 0, NULL);
		*/
	//debugf(XFER_LOG_THUNDER, "thunder_save_tasks, %s:%d\n", __FILE__, __LINE__);
	//thunder_save_tasks();
	
	return XFER_ERROR_OK;
}

static xfer_errid_t 
get_task_src(transfer_taskid_t taskid, char** src)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if ((node->taskid == taskid) && node->src)
		{
			*src = (char*)xfer_malloc(strlen(node->src)+1);
			if (*src)
			{
				strcpy(*src, node->src);
			}
			break;
		}
	}
	thunder_list_sem_unlock();
	
	return XFER_ERROR_OK;
}

static xfer_errid_t 
get_task_src_url(transfer_taskid_t taskid, char** src_url)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if ((node->taskid == taskid) && node->src_url)
		{
			*src_url = (char*)xfer_malloc(strlen(node->src_url)+1);
			if (*src_url)
			{
				strcpy(*src_url, node->src_url);
			}
			break;
		}
	}
	thunder_list_sem_unlock();
	
	return XFER_ERROR_OK;
}

xfer_errid_t thunder_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo)
{
	ET_TASK info;
	int err_id = 0;
	xfer_errid_t errid = XFER_ERROR_OK;
	u32 thunder_taskid = get_curr_taskid(taskid);
	
	errid = et_get_task_info(thunder_taskid, &info);

	*taskinfo = (transfer_task_info_t*)xfer_malloc(sizeof(transfer_task_info_t));
	if ((*taskinfo) == NULL)
	{
		*taskinfo = NULL;
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*taskinfo, 0, sizeof(transfer_task_info_t));
	(*taskinfo)->task_id = taskid;
	(*taskinfo)->thunder_task_id = thunder_taskid;
	
	if (errid == XFER_ERROR_OK)
	{
		if (info._task_status == ET_TASK_FAILED || info._task_status == ET_TASK_SUCCESS)
		{
			et_stop_task(thunder_taskid);
			while(1)
			{
				err_id = et_delete_task(thunder_taskid);  
				if(err_id!=4109&&err_id!=4119)
				{
//					printf("***********************thunder_id=%d errid=%d\n",thunder_taskid,err_id);
					break;
				}
				if(err_id == 4109)
					et_stop_task(thunder_taskid);
				usleep(delay_for_delete);
			}
		}

		errid = get_task_path_and_name(taskid, &(*taskinfo)->task_path, &(*taskinfo)->task_name);
		errid = get_task_src(taskid, &(*taskinfo)->task_src);	
		errid = get_task_src_url(taskid, &(*taskinfo)->task_src_url);	
		if (info._file_size != 0)
		{
			(*taskinfo)->task_stat = task_status(info._task_status);    
			(*taskinfo)->speed = info._speed;          
			(*taskinfo)->total_size = info._file_size;   
			(*taskinfo)->mem_size = info._downloaded_data_size;
			(*taskinfo)->written_size = info._written_data_size;
			(*taskinfo)->downloaded_size = info._file_size * 0.01 * info._progress;
			(*taskinfo)->file_created = (info._file_create_status == ET_FILE_CREATED_SUCCESS);
			if (info._progress == 100 && (*taskinfo)->task_stat == DLST_DOWNLOADING)
				(*taskinfo)->downloaded_size = info._file_size * 0.01 * 99;
			else
				(*taskinfo)->downloaded_size = info._file_size * 0.01 * info._progress;
			if (info._task_status == ET_TASK_FAILED && info._failed_code == 0)
			{
				thunder_list_set_val(taskid, (*taskinfo)->task_stat, XFER_ERROR_THUNDER_BT_ERR, 
					(*taskinfo)->total_size, (*taskinfo)->downloaded_size, NULL, (*taskinfo)->file_created);
				(*taskinfo)->error_id = XFER_ERROR_THUNDER_BT_ERR;   
			}
			else
			{
				/* If there is anything we can do when some download task errors occurred, just do it. */
				/* Maybe UI app needn't care about the detail. */
				if ((info._task_status == ET_TASK_FAILED)
					&& ((info._failed_code == 1025) 
						|| (info._failed_code == 15400) 
						|| (info._failed_code == 15389) 
						|| (info._failed_code == 130) 
						|| (info._failed_code == 102)))
					(*taskinfo)->task_stat = task_status(ET_TASK_RUNNING);    

				thunder_list_set_val(taskid, (*taskinfo)->task_stat, info._failed_code, 
					(*taskinfo)->total_size, (*taskinfo)->downloaded_size, NULL, (*taskinfo)->file_created);
				(*taskinfo)->error_id = info._failed_code;
			}
		}
		else
		{
			if (info._task_status == ET_TASK_FAILED)
			{
				u32 failed_code = info._failed_code;
				if (failed_code == 0)
					failed_code = XFER_ERROR_THUNDER_BT_ERR;
				thunder_list_set_status(taskid, info._task_status, failed_code);
			}
			thunder_list_get_val(taskid, &(*taskinfo)->task_stat, &(*taskinfo)->total_size, 
				&(*taskinfo)->downloaded_size, &(*taskinfo)->file_created, &(*taskinfo)->error_id);
		}
	}
	else
	{
		errid = get_task_path_and_name(taskid, &(*taskinfo)->task_path, &(*taskinfo)->task_name);
		errid = get_task_src(taskid, &(*taskinfo)->task_src);	
		errid = get_task_src_url(taskid, &(*taskinfo)->task_src_url);			
		thunder_list_get_val(taskid, &(*taskinfo)->task_stat, &(*taskinfo)->total_size, 
			&(*taskinfo)->downloaded_size, &(*taskinfo)->file_created, &(*taskinfo)->error_id);
	}

	return errid;
}

xfer_errid_t
thunder_mgr_set_tasks_info_file(const char* file_name)
{
	if (file_name == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
	
	if (strlen(file_name)+1 > DL_MAX_PATH)
	{
		strncpy(thunder_mgr_taskinfo, file_name, DL_MAX_PATH-1);
		thunder_mgr_taskinfo[DL_MAX_PATH-1] = 0;
	}
	else
		strcpy(thunder_mgr_taskinfo, file_name);

	return XFER_ERROR_HTTP_TODO;
}

xfer_errid_t 
thunder_mgr_get_seed_info(char *src, char *seed_path, torrent_seed_info_t** seed_info)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	int name_len = 0;
	int i = 0;
	
	debugf(XFER_LOG_THUNDER, "thunder_mgr_get_seed_info(), src=%s, seed_path=%s\n", src, seed_path);
	ET_TORRENT_SEED_INFO * p_torrent_seed_info =NULL;
	ET_TORRENT_FILE_INFO * p_torrent_file_info =NULL;

	errid = et_get_torrent_seed_info(seed_path, &p_torrent_seed_info );
	debugf(XFER_LOG_THUNDER, "et_get_torrent_seed_info(), errid=%d, seed_path=%s\n", errid, seed_path);

	if(errid)
	{
		return errid;
	}
	
	*seed_info = (torrent_seed_info_t*)xfer_malloc(sizeof(torrent_seed_info_t));
	if (*seed_info == NULL)
	{
		et_release_torrent_seed_info(p_torrent_seed_info);
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*seed_info, 0x0, sizeof(torrent_seed_info_t));
	
	/*total_size*/
	(*seed_info)->total_size = p_torrent_seed_info->_file_total_size;
	/*name*/
	name_len = p_torrent_seed_info->_title_name_len > 255 ? 255 : p_torrent_seed_info->_title_name_len;
	strncpy((*seed_info)->name, p_torrent_seed_info->_title_name, name_len);
	(*seed_info)->name[name_len] = 0;
	/*seed_file_path_len*/
	(*seed_info)->seed_file_path_len = strlen(seed_path);
	/*seed_file_path*/
	strncpy((*seed_info)->seed_file_path, seed_path, (*seed_info)->seed_file_path_len);
	(*seed_info)->seed_file_path[(*seed_info)->seed_file_path_len] = 0;
	/*src_url_len*/
	if(src)
	{
		(*seed_info)->src_url_len = strlen(src);
		/*src_url*/
		strncpy((*seed_info)->src_url, src, (*seed_info)->src_url_len);
		(*seed_info)->src_url[(*seed_info)->src_url_len] = 0;
	}
	else
	{
		(*seed_info)->src_url_len = 0;
		(*seed_info)->src_url[(*seed_info)->src_url_len] = 0;
	}
	
	/*saved_path*/
	//char *path_end = strstr(seed_path, ".torrent");
	char *path_end = strrchr(seed_path, '/');
	if(path_end == NULL)
	{
		(*seed_info)->saved_path[0] = '.';
		(*seed_info)->saved_path[1] = '/';
		(*seed_info)->saved_path[2] = '\0';
		(*seed_info)->saved_path_len = 2;

	}
	else
	{
		int path_len = path_end - seed_path;
		if(path_len > 0 && path_len < 255)
		{
			strncpy((*seed_info)->saved_path, seed_path, path_len);
			(*seed_info)->saved_path[path_len] = '/';
			(*seed_info)->saved_path[path_len+1] = '\0';
			(*seed_info)->saved_path_len = path_len + 2;
		}
		else
		{
			et_release_torrent_seed_info(p_torrent_seed_info);
			return -1;
		}
	}

	/*torrent_file_list*/
	p_torrent_file_info = p_torrent_seed_info->file_info_array_ptr;
	(*seed_info)->torrent_file_list =  (torrent_file_info_t*)xfer_malloc(p_torrent_seed_info->_file_num * sizeof(torrent_file_info_t));
	if ((*seed_info)->torrent_file_list == NULL)
	{
		xfer_free(*seed_info);
		*seed_info = NULL;
		et_release_torrent_seed_info(p_torrent_seed_info);
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	
	/*file_num*/
	(*seed_info)->file_num = 0;
	for(i=0;i<p_torrent_seed_info->_file_num;i++)
	{
//		if( p_torrent_file_info->_file_size > MIN_BT_FILE_SIZE )
		/*
		if( strstr(p_torrent_file_info->_file_name, ".rm") ||
			strstr(p_torrent_file_info->_file_name, ".RM") ||
			strstr(p_torrent_file_info->_file_name, ".avi") ||
			strstr(p_torrent_file_info->_file_name, ".AVI") )
		*/
		{
			(*seed_info)->torrent_file_list[(*seed_info)->file_num] .index= p_torrent_file_info->_file_index;
			(*seed_info)->torrent_file_list[(*seed_info)->file_num] .file_size= p_torrent_file_info->_file_size;
			name_len = p_torrent_file_info->_file_name_len > 255 ? 255 : p_torrent_file_info->_file_name_len;
			strncpy((*seed_info)->torrent_file_list[(*seed_info)->file_num] .file_name, p_torrent_file_info->_file_name, name_len);
			(*seed_info)->torrent_file_list[(*seed_info)->file_num].file_name[name_len]= 0;
			(*seed_info)->download_file_index_array[(*seed_info)->file_num] = p_torrent_file_info->_file_index;
			(*seed_info)->file_num++;
		}
		p_torrent_file_info++;
	}

	/*download_num*/
	(*seed_info)->download_num = 0;
	
	et_release_torrent_seed_info(p_torrent_seed_info);

 	return errid;
}

xfer_errid_t 
thunder_mgr_release_seed_info(torrent_seed_info_t**seed_info)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	
	if (seed_info && *seed_info)
	{
		if((*seed_info)->torrent_file_list)
			xfer_free((*seed_info)->torrent_file_list);
 		xfer_free(*seed_info);
		*seed_info = NULL;
	}
	
	return errid;
}

xfer_errid_t
thunder_mgr_set_license(char* lic)
{
	return et_set_license(lic, strlen(lic));
}

transfer_taskid_t thunder_mgr_get_thunder_taskid(transfer_taskid_t taskid)
{
    return get_curr_taskid(taskid);
}

xfer_errid_t
thunder_mgr_set_kankan_buffer_time(int32 buffer_time )
{
#ifdef SUPPORT_THUNDER_KANKAN
       int32 ret = 0;
	ret = et_vod_set_buffer_time(buffer_time);
	return ret;
#endif
	return XFER_ERROR_OK;
}

xfer_errid_t
thunder_mgr_get_kankan_buffer_percent(int32 task_id ,int32* buffer_time )
{
#ifdef SUPPORT_THUNDER_KANKAN
	int32 tmp_percent = 0;
       int32 ret = 0;
	u32 thunder_taskid = get_curr_taskid(task_id);

	ret = et_vod_get_buffer_percent(thunder_taskid, &tmp_percent);
	*buffer_time = tmp_percent;
	return ret;
#endif
	return XFER_ERROR_OK;
}

xfer_errid_t
thunder_mgr_read_kankan_file(int32 task_id, uint64 start_pos, uint64 len, char * buf, int32 block_time)
{
	xfer_errid_t errid = XFER_ERROR_OK;
#ifdef SUPPORT_THUNDER_KANKAN
	u32 thunder_taskid = get_curr_taskid(task_id);

	errid = et_vod_read_file(thunder_taskid, start_pos, len, buf, block_time);
	debugf(XFER_LOG_THUNDER, "et_vod_read_file(), errid=%d, thunder_taskid=%d\n", errid, thunder_taskid);
#endif
	return errid;
}
//=====================
xfer_errid_t
thunder_mgr_set_license_temp(void)
{
	//abx_sem_wait(thunder_license_sema);
	xfer_errid_t errid = thunder_mgr_set_license(license_temp);
	//abx_sem_signal(thunder_license_sema);
	return errid;
}
/*to get the result of license reregistration for examination ---add by lty*/
u32
thunder_mgr_get_license_result(void)
{

	//abx_sem_wait(thunder_license_sema);
	u32 result = thunder_license_result;
	//abx_sem_signal(thunder_license_sema);
	return result;
}

u32
thunder_mgr_get_license_expire(void)
{
	//abx_sem_wait(thunder_license_sema);
	u32 expire = thunder_license_expire;
	//abx_sem_signal(thunder_license_sema);
	return expire;
}

xfer_errid_t thunder_mgr_get_task_info_for_exam(transfer_taskid_t taskid, transfer_task_info_t** taskinfo, int *error_code)
{
	ET_TASK info;
	xfer_errid_t errid = XFER_ERROR_OK;
	u32 thunder_taskid = get_curr_taskid(taskid);
	
	errid = et_get_task_info(thunder_taskid, &info);

	*taskinfo = (transfer_task_info_t*)xfer_malloc(sizeof(transfer_task_info_t));
	if ((*taskinfo) == NULL)
	{
		*taskinfo = NULL;
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*taskinfo, 0, sizeof(transfer_task_info_t));
	(*taskinfo)->task_id = taskid;
	
	if (errid == XFER_ERROR_OK)
	{
		if (info._task_status == ET_TASK_FAILED || info._task_status == ET_TASK_SUCCESS)
		{
			errid = et_stop_task(thunder_taskid);
		 	errid = et_delete_task(thunder_taskid);
		}

		errid = get_task_path_and_name(taskid, &(*taskinfo)->task_path, &(*taskinfo)->task_name);
		if (info._file_size != 0)
		{
			(*taskinfo)->task_stat = task_status(info._task_status);    
			(*taskinfo)->speed = info._speed;          
			(*taskinfo)->total_size = info._file_size;     
			(*taskinfo)->downloaded_size = info._file_size * 0.01 * info._progress;
			(*taskinfo)->file_created = (info._file_create_status == ET_FILE_CREATED_SUCCESS);
			if (info._progress == 100 && (*taskinfo)->task_stat == DLST_DOWNLOADING)
				(*taskinfo)->downloaded_size = info._file_size * 0.01 * 99;
			else
				(*taskinfo)->downloaded_size = info._file_size * 0.01 * info._progress;
			if (info._task_status == ET_TASK_FAILED && info._failed_code == 0)
			{
				thunder_list_set_val(taskid, (*taskinfo)->task_stat, XFER_ERROR_THUNDER_BT_ERR, 
					(*taskinfo)->total_size, (*taskinfo)->downloaded_size, NULL, (*taskinfo)->file_created);
				(*taskinfo)->error_id = XFER_ERROR_THUNDER_BT_ERR;   
			}
			else
			{
				thunder_list_set_val(taskid, (*taskinfo)->task_stat, info._failed_code, 
					(*taskinfo)->total_size, (*taskinfo)->downloaded_size, NULL, (*taskinfo)->file_created);
				(*taskinfo)->error_id = info._failed_code;
			}
		}
		else
		{
			if (info._task_status == ET_TASK_FAILED)
			{
				u32 failed_code = info._failed_code;
				*error_code = info._failed_code;
				if (failed_code == 0)
					failed_code = XFER_ERROR_THUNDER_BT_ERR;
				thunder_list_set_status(taskid, info._task_status, failed_code);
			}
			thunder_list_get_val(taskid, &(*taskinfo)->task_stat, &(*taskinfo)->total_size, 
				&(*taskinfo)->downloaded_size, &(*taskinfo)->file_created, &(*taskinfo)->error_id);
		}
	}
	else
	{
		errid = get_task_path_and_name(taskid, &(*taskinfo)->task_path, &(*taskinfo)->task_name);
		thunder_list_get_val(taskid, &(*taskinfo)->task_stat, &(*taskinfo)->total_size, 
			&(*taskinfo)->downloaded_size, &(*taskinfo)->file_created, &(*taskinfo)->error_id);
	}

	return errid;
}

xfer_errid_t thunder_mgr_get_task_src_url(transfer_taskid_t taskid, char *src_url)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	errid = thunder_list_get_task_src_url(taskid, src_url);
	return errid;
}

xfer_errid_t thunder_mgr_set_task_src_url(transfer_taskid_t taskid, char *src_url)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	errid = thunder_list_set_task_src_url(taskid, src_url);
	return errid;
}

xfer_errid_t thunder_mgr_get_torrent_info(const char *seed_path, TASK_PARA *task_para)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	errid = parse_seed_info((char*)seed_path, task_para);
	return errid;
}

xfer_errid_t thunder_mgr_get_bt_files_info(transfer_taskid_t taskid, torrent_seed_info_t* seed_info)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	int name_len = 0;
	int i = 0;
	u32 buffer_len = 100;
	u32 file_index_buffer[100];
	char file_path_buffer[256+1];
	char file_name_buffer[256+1];
	int32 file_path_buffer_size = 256;
	int32 file_name_buffer_size = 256;
	ET_BT_FILE * p_bt_file =NULL;
	u32 thunder_taskid = get_curr_taskid(taskid);	

	if(thunder_taskid == 0 || seed_info == NULL)
		return -1;

	errid = et_get_bt_download_file_index(thunder_taskid, &buffer_len, file_index_buffer);

	if(errid != 0 && buffer_len <= 0)
		return errid;
	
	if(seed_info->torrent_file_list)
	{
		xfer_free(seed_info->torrent_file_list);
		seed_info->torrent_file_list = NULL;
	}
	
	seed_info->file_num = buffer_len;
	seed_info->torrent_file_list =  (torrent_file_info_t*)xfer_malloc(seed_info->file_num * sizeof(torrent_file_info_t));
	if (seed_info->torrent_file_list == NULL)
	{
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}

	p_bt_file = (ET_BT_FILE *)xfer_malloc(sizeof(ET_BT_FILE));
	for(i=0; i<seed_info->file_num; i++)
	{
		memset(p_bt_file, 0, sizeof(ET_BT_FILE));
		memset(file_name_buffer, 0, sizeof(file_name_buffer));
		memset(file_path_buffer, 0, sizeof(file_path_buffer));
		file_path_buffer_size = 256;
		file_name_buffer_size = 256;
		seed_info->download_file_index_array[i] = file_index_buffer[i];
		errid = et_get_bt_file_path_and_name(thunder_taskid, 
											seed_info->download_file_index_array[i], 
											file_path_buffer, 
											&file_path_buffer_size, 
											file_name_buffer, 
											&file_name_buffer_size);
		if(errid != 0)
		{
			xfer_free(p_bt_file);
			return errid;
		}
		strncpy(seed_info->torrent_file_list[i].file_name, file_name_buffer, file_name_buffer_size);
		seed_info->torrent_file_list[i].file_name[file_name_buffer_size] = 0;
		
		errid = et_get_bt_file_info(thunder_taskid, seed_info->download_file_index_array[i], p_bt_file);
		if(errid != 0)
		{
			xfer_free(p_bt_file);
			return errid;
		}
		seed_info->torrent_file_list[i].index = p_bt_file->_file_index;
		seed_info->torrent_file_list[i].file_size = p_bt_file->_file_size;
		seed_info->torrent_file_list[i].file_percent = p_bt_file->_file_percent;
		seed_info->torrent_file_list[i].file_status = p_bt_file->_file_status;
		seed_info->torrent_file_list[i].sub_task_err_code = p_bt_file->_sub_task_err_code;
	}

	return errid;
}

static int
thunder_list_get_bt_files_index(transfer_taskid_t taskid, u32 *files_num, u32 **files_index)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;
	int i = 0;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;

	
	if(files_num == NULL || files_index == NULL || *files_index == NULL)
		return -1;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			*files_num = node->bt_files_num;
			if(*files_num > 0)
			{
				for(i = 0; i < *files_num; i++)
				{
					(*files_index)[i] = node->bt_files_index[i];
				}
			}
			thunder_list_sem_unlock();
			return XFER_ERROR_OK;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

static int
thunder_list_set_bt_files_index(transfer_taskid_t taskid,  u32 files_num, u32 *files_index)
{
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;
	int i = 0;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	if(files_num == 0 || files_index == NULL)
		return -1;
	
	thunder_list_sem_lock();

	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if (node->taskid == taskid)
		{
			node->bt_files_num = files_num;
			node->bt_files_index = (u32*)xfer_malloc(files_num*sizeof(u32));
			if(node->bt_files_index == NULL)
			{
				thunder_list_sem_unlock();			
				return XFER_ERROR_HTTP_MALLOC_FAILURE;
			}
			
			for(i = 0; i < node->bt_files_num; i++)
			{
				node->bt_files_index[i] = files_index[i];
			}
			thunder_list_sem_unlock();
			return XFER_ERROR_OK;
		}
	}
	thunder_list_sem_unlock();

	return -1;
}

xfer_errid_t
thunder_mgr_check_busy(transfer_taskid_t taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	list_t *next, *cur;
	thunder_mgr_node_t* node = NULL;
	char tmp_file[DL_MAX_PATH] = {'\0'};
	u64_t filesize = 0;
	struct stat tmp_stat;

	if(s_thunder_tasks.node.link.next == NULL)
		return XFER_ERROR_OK;
	
	thunder_list_sem_lock();
	list_for_each_safe(cur, next, &(s_thunder_tasks.node.link)) 
	{
		node = list_entry(cur, thunder_mgr_node_t, link);
		if ((node->taskid == taskid) && node->file_path && node->file_name)
		{
			snprintf(tmp_file, sizeof(tmp_file), "%s%s.td", node->file_path, node->file_name);			
			filesize = node->total_size;
		}
	}
	thunder_list_sem_unlock();

	debugf(XFER_LOG_MAIN, "tmp_file = %s, filesize=%d\n", tmp_file, filesize);
	errid = stat(tmp_file, &tmp_stat);
	if (errid == 0)
	{
		debugf(XFER_LOG_MAIN, "stat size = %d\n", tmp_stat.st_size);
		if (filesize > tmp_stat.st_size)
			errid = XFER_ERROR_THUNDER_BUSY;
		else
			errid = XFER_ERROR_OK;
	}
	else if (errno == ENOENT)
	{
		debugf(XFER_LOG_MAIN, "file:%s NOT EXIST!\n", tmp_file);
		errid = XFER_ERROR_OK;
	}
	else
		debugf(XFER_LOG_MAIN, "check tmp file ERROR!\n");

	return errid;
}

#endif
