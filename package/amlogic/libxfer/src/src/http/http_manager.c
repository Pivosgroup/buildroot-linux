/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Http Module
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#ifdef __WIN32__
#include <Windows.h>
#else
#include <semaphore.h>
#include <pthread.h>
#endif
#include <sys/stat.h>
#include <curl/curl.h>

#include "http_manager.h"
#include "transfer_def.h"
#include "http_curl.h"
#include "../common/transfer_utils.h"
#include "../common/xfer_debug.h"


/**********************************************************************/
#ifndef __ROM_
typedef struct {
	transfer_taskid_t	task_id;
	int					msg_sent;
	int					msg_done;
}HTTP_DEBUG_STATE_t;
int				   http_debug_index = 0;
HTTP_DEBUG_STATE_t http_debug_state[300];
void http_dbg_msg_sent_(transfer_taskid_t taskid, int type)
{
	http_debug_state[http_debug_index%300].task_id = taskid;
	http_debug_state[http_debug_index%300].msg_sent = type;
	http_debug_state[http_debug_index++%300].msg_done= -1;
}

void http_dbg_msg_done_(transfer_taskid_t taskid, int type)
{
	http_debug_state[http_debug_index%300].task_id = taskid; 
	http_debug_state[http_debug_index%300].msg_sent = -1;
	http_debug_state[http_debug_index++%300].msg_done = type;
}
#endif

char http_mgr_taskinfo[DL_MAX_PATH] = "";	// "/mnt/C/Download/http.aml";
char http_mgr_temp[DL_MAX_PATH] = "amlogic_download/";

static int s_http_mgr_max_speed = 1024*1024*1024;
int s_http_mgr_setup_demo = 0;
int s_http_mgr_tasks_max = 10;
int s_http_mgr_redirs_max = 5;
#define HTTP_MGR_TIMEOUT_MAX 300  /*sec*/
#define HTTP_MGR_CONNECT_TIMEOUT_MAX 30 /*sec*/
int s_http_mgr_timeout_max = HTTP_MGR_TIMEOUT_MAX; /* default 300 seconds */
int s_http_mgr_connect_timeout_max = HTTP_MGR_CONNECT_TIMEOUT_MAX; /* default 30 seconds */

static transfer_taskid_t http_mgr_taskid = TASKID_HTTP_BEGIN;

#define HTTP_MGR_TIMEOUT_NORMAL		2000
#define HTTP_MGR_SPEED_ZERO_RESET	30	
#define HTTP_MGR_SPEED_RESET		60000

#define HTTP_SEM_ENABLE 1
#define HTTP_USE_MUTEX 1

#if XFER_DEBUG_ENABLE
//#define HTTP_DEBUG_ENABLE 1
#endif

http_mgr_state_e http_mgr_state = HTTP_MGR_NONE;

char http_load_path[DL_MAX_PATH];

http_mgr_list_t http_mgr_tasks;

typedef struct http_mgr_transfer_count
{
	unsigned int 	total;
	unsigned int 	task;
	unsigned int 	normal;
	unsigned int 	post;
}http_mgr_transfer_count_t;

http_mgr_transfer_count_t	http_mgr_count;

int s_http_sem_inited = 0;
#if HTTP_SEM_ENABLE
#ifdef __WIN32__
HANDLE s_http_async_sem;
#else
#if HTTP_USE_MUTEX
pthread_mutex_t s_http_mutex;
pthread_mutexattr_t s_http_mutexatrr;
#else
sem_t s_http_async_sem;
#endif
#endif
#endif

http_mgr_status_t s_http_mgr_status;


/**********************************************************************/
static http_async_context_t* 
http_mgr_task_create(const char* src,
						  const char* saved_path,
						  int type,
						  int mode,
						  const char* post_buff,
						  transfer_complete_cb complete_cb);

void 
http_mgr_task_free(http_async_context_t* cxt);

/**********************************************************************/
xfer_errid_t http_mgr_add_count(int mode)
{
	if (mode&XFER_SHOW)
	{
		http_mgr_count.task++;
		http_mgr_count.total++;
	}
	else if (mode&XFER_POST)
	{
		http_mgr_count.post++;
		http_mgr_count.total++;
	}
	else
	{
		http_mgr_count.normal++;
		http_mgr_count.total++;
	}

	return XFER_ERROR_OK;
}

xfer_errid_t http_mgr_sub_count(int mode)
{
	if (mode&XFER_SHOW)
	{
		http_mgr_count.task--;
		http_mgr_count.total--;
	}
	else if (mode&XFER_POST)
	{
		http_mgr_count.post--;
		http_mgr_count.total--;
	}
	else
	{
		http_mgr_count.normal--;
		http_mgr_count.total--;
	}
	
	return XFER_ERROR_OK;
}

void http_mgr_sem_init()
{
	debugf(XFER_LOG_HTTP, "http_mgr_sem_init\n");
#if HTTP_SEM_ENABLE
	while(!s_http_sem_inited)
	{
#ifdef __WIN32__
		if((s_http_async_sem = CreateSemaphore( 
								NULL, // default security attributes
								1, // initial count
								1, // maximum count
								NULL)) != NULL) // unnamed semaphore
#else
		#if HTTP_USE_MUTEX
		pthread_mutexattr_init(&s_http_mutexatrr);
		pthread_mutexattr_settype(&s_http_mutexatrr, PTHREAD_MUTEX_RECURSIVE_NP);
		if(pthread_mutex_init(&s_http_mutex, &s_http_mutexatrr) == 0)
		#else
		if(sem_init(&s_http_async_sem, 0, 1) == 0)
		#endif
#endif
		{
			s_http_sem_inited = 1; 
			break;
		}
		else
		{
			s_http_sem_inited = 0; 
		}; 
	}
#endif
}

void http_mgr_sem_lock_(const char* func, const char* file, int line)
{
#if HTTP_SEM_ENABLE
        //debugf("http_mgr_sem_lock() [%s:%d]\n", file, line);
#ifdef __WIN32__
	WaitForSingleObject(s_http_async_sem, INFINITE);
#else
	#if HTTP_USE_MUTEX
	pthread_mutex_lock(&s_http_mutex);
	#else
	sem_wait(&s_http_async_sem);
	#endif
#endif
#endif
}

void http_mgr_sem_unlock_(const char* func, const char* file, int line)
{
#if HTTP_SEM_ENABLE
        //debugf("http_mgr_sem_unlock() [%s:%d]\n", file, line);
#ifdef __WIN32__
	ReleaseSemaphore(s_http_async_sem, 1, NULL);
#else
	#if HTTP_USE_MUTEX
	pthread_mutex_unlock(&s_http_mutex);
	#else
	sem_post(&s_http_async_sem);
	#endif
#endif
#endif
}

void http_mgr_sem_close()
{
#if HTTP_SEM_ENABLE
	debugf(XFER_LOG_HTTP, "http_mgr_sem_close\n");
	while(s_http_sem_inited)
	{
#ifdef __WIN32__
		CloseHandle(s_http_async_sem);
#else
		#if HTTP_USE_MUTEX
		pthread_mutexattr_destroy(&s_http_mutexatrr);
		pthread_mutex_destroy(&s_http_mutex);
		#else
		sem_destroy(&s_http_async_sem);
		#endif
#endif
		s_http_sem_inited=0;
		break;
	}
#endif
}

static transfer_taskid_t 
http_get_task_id()
{
	do{
		if (http_mgr_taskid >= TASKID_HTTP_END || http_mgr_taskid < TASKID_HTTP_BEGIN)
			http_mgr_taskid = TASKID_HTTP_BEGIN;
	
		http_mgr_taskid++;
		
	}while(http_mgr_get_context(http_mgr_taskid));
	
	return http_mgr_taskid;
}

int http_mgr_taskid_validate(transfer_taskid_t taskid)
{
	if (taskid >= TASKID_HTTP_BEGIN && taskid <= TASKID_HTTP_END)
		return 1;
	else
		return 0;
}

http_async_context_t*
http_mgr_get_context(transfer_taskid_t taskid)
{
	http_async_context_t* cxt = NULL;
	struct llhead		*lp, *tmp;
	http_mgr_list_t* 	curr;

	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);

		if (curr->cxt->task_id == taskid)
		{	
			return curr->cxt;
		}
	}
	return cxt;
}

int http_mgr_conn_insert(http_async_context_t* cxt)
{
	http_mgr_list_t * node = (http_mgr_list_t*)malloc(sizeof(http_mgr_list_t));
	if (node == NULL)
	{
		return -1;
	}

	node->cxt = cxt;
	LL_INIT(&node->link);

	http_mgr_sem_lock();
	LL_TAIL(&(http_mgr_tasks.link), &node->link);
	http_mgr_add_count(cxt->mode);
	http_mgr_sem_unlock();
	
	return 0;
}

void http_mgr_conn_erase()
{
	struct llhead *lp, *tmp;
	http_mgr_list_t* curr;

	http_mgr_sem_lock();
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		LL_DEL(&curr->link);
		http_mgr_sub_count(curr->cxt->mode);
		
		curr->cxt->errid = XFER_ERROR_HTTP_STOP;

		http_mgr_task_free(curr->cxt);
		free(curr);
	}
	http_mgr_sem_unlock();
}

#define POP_ERR_RETURN(dst, len) \
	if ((res = pop_buff_safe(dst, &pos, len, &last_len)) != XFER_ERROR_OK) \
	{	goto exit_flag;  }
xfer_errid_t http_load_tasks()
{
	int		last_len = 0;
	char* 	pos = NULL;
	char* 	url = NULL;
	char* 	buff = NULL;
	char* 	file_name = NULL;
	xfer_errid_t res = XFER_ERROR_OK;
	http_async_context_t* cxt = NULL;

	if ((last_len = read_whole_file(&buff, http_mgr_taskinfo)) < 20)
	{
		res = XFER_ERROR_OK;
		goto exit_flag;
	}
	
	http_mgr_sem_lock();
	pos = buff+20;
	last_len -= 20;
	POP_ERR_RETURN((void*)&http_mgr_taskid, 4);
	if (!http_mgr_taskid_validate(http_mgr_taskid))
	{
		res = XFER_ERROR_HTTP_TASKID_ERR;
		goto exit_flag;
	}

	do
	{
		int	  len = 0, file_len = 0, curr_len = 0, state = 0, mode = 0;
		transfer_taskid_t taskid = TASKID_INVALID;
		
		POP_ERR_RETURN((void*)&taskid, sizeof(transfer_taskid_t));
		if (!http_mgr_taskid_validate(taskid))
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len <= 0)
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		url = (char*)malloc(len+1);
		if (url == NULL)
		{
			res = XFER_ERROR_HTTP_MALLOC_FAILURE;
			goto exit_flag;
		}
		POP_ERR_RETURN(url, len);
		url[len] = 0;
		
		POP_ERR_RETURN((void*)&len, sizeof(int));
		if (len <= 0)
		{
			res = XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
			goto exit_flag;
		}
		file_name = (char*)malloc(strlen(http_load_path)+len+1);
		if (file_name == NULL)
		{
			res = XFER_ERROR_HTTP_MALLOC_FAILURE;
			goto exit_flag;
		}
		strcpy(file_name, http_load_path);
		POP_ERR_RETURN(file_name+strlen(http_load_path), len);
		file_name[strlen(http_load_path)+len] = 0;
		
		POP_ERR_RETURN((void*)&file_len, sizeof(int));
		POP_ERR_RETURN((void*)&curr_len, sizeof(int));
		POP_ERR_RETURN((void*)&state, sizeof(int));
		POP_ERR_RETURN((void*)&mode, sizeof(int));

		cxt = http_mgr_task_create(url, file_name, XFER_TASK_HTTP, XFER_SHOW, NULL, NULL);
		if (cxt == NULL)
		{
			res = XFER_ERROR_HTTP_MALLOC_FAILURE;
			goto exit_flag;
		}

		cxt->task_id = taskid;
		if (curr_len > file_len)
		{
			debugf(XFER_LOG_HTTP, "http_load_tasks loading file length %d; current length %d.\n", file_len, curr_len);
			if (file_len != 0)
			{
				file_len = 0;
				curr_len = 0;
			}
		}
 		cxt->file_len 	 = file_len;
 		cxt->saved_len   = curr_len;
		cxt->curr_len 	 = curr_len;
		cxt->last_recved = curr_len;
		cxt->state 		 = state;
		cxt->mode 		 = mode;

		http_mgr_sem_unlock();
		
		http_mgr_conn_insert(cxt);
		/*
		if (cxt->state == DLST_DOWNLOADING)
			http_mgr_task_request(cxt);
		*/
		//if (cxt->state == DLST_ERROR)
		//	file_task_send_msg(cxt->task_id, NULL, FILE_TASK_XSTART, NULL, -1, -1, NULL);
		http_mgr_sem_lock();
		
		free(url);
		url = NULL;
		free(file_name);
		file_name = NULL;
	}while(last_len);

exit_flag:
	if (url) 
	{
		free(url); 
	}
	if (file_name) 
	{
		free(file_name); 
	}
	if (buff)
	{
		free(buff);
	}
	http_mgr_state = HTTP_MGR_LOADED;
	http_mgr_sem_unlock(); 
	return res;
}
#undef POP_ERR_RETURN

#define PUSH_ERR_RETURN(src, len) \
	if ((res = push_buff_safe(mem_buf, src, len)) != XFER_ERROR_OK) \
	{	\
		if (mem_buf->buff) \
			free(mem_buf->buff); \
		http_mgr_sem_unlock(); \
		free(mem_buf); \
		return res;  \
	}

xfer_errid_t http_save_tasks()
{
	struct llhead		*lp, *tmp;
	http_mgr_list_t* 	curr;
	xfer_errid_t			res = XFER_ERROR_NULL;
	transfer_mem_buff_t* mem_buf = (transfer_mem_buff_t*)malloc(sizeof(transfer_mem_buff_t));

	if (mem_buf == NULL)
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	memset(mem_buf, 0, sizeof(transfer_mem_buff_t));

	http_mgr_sem_lock();
	if (http_mgr_state == HTTP_MGR_UNLOADED)
	{
		if (mem_buf->buff) 
			free(mem_buf->buff); 
		http_mgr_sem_unlock();
		free(mem_buf);
		return XFER_ERROR_HTTP_STOP;
	}
	PUSH_ERR_RETURN("1.00", 4);
	PUSH_ERR_RETURN("1234567890123456", 16);
	PUSH_ERR_RETURN((void*)&http_mgr_taskid, sizeof(transfer_taskid_t));
	
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		int len = 0;
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		if (!(curr->cxt->mode&XFER_SHOW) || curr->cxt->errid == XFER_ERROR_HTTP_REMOVED)
			continue;

		PUSH_ERR_RETURN((void*)&curr->cxt->task_id, sizeof(transfer_taskid_t));
		len = strlen(curr->cxt->url);
		PUSH_ERR_RETURN((void*)&len, sizeof(int));
		PUSH_ERR_RETURN(curr->cxt->url, strlen(curr->cxt->url));
		len = strlen(curr->cxt->name);
		PUSH_ERR_RETURN((void*)&len, sizeof(int));
		PUSH_ERR_RETURN(curr->cxt->name, strlen(curr->cxt->name));
		PUSH_ERR_RETURN((void*)&curr->cxt->file_len, sizeof(int));
		PUSH_ERR_RETURN((void*)&curr->cxt->saved_len, sizeof(int));
		PUSH_ERR_RETURN((void*)&curr->cxt->state, sizeof(int));
		PUSH_ERR_RETURN((void*)&curr->cxt->mode, sizeof(int));
	}
	http_mgr_sem_unlock(); 

	res = write_file_safe(http_mgr_taskinfo, mem_buf->buff, mem_buf->length);
	
	if (mem_buf)
	{
		if (mem_buf->buff)
			free(mem_buf->buff); 
		free(mem_buf); 
	}
	
	return res;
}
#undef PUSH_ERR_RETURN

xfer_errid_t http_unload_tasks()
{
	struct llhead		*lp, *tmp;
	http_mgr_list_t* 	curr;

	//http_save_tasks();
	
	http_mgr_sem_lock();
	http_mgr_state = HTTP_MGR_LOADED;
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		if (curr && curr->cxt && curr->cxt->mode&XFER_SHOW)
		{
			LL_DEL(&curr->link);
			http_mgr_sub_count(curr->cxt->mode);
			
			curr->cxt->errid = XFER_ERROR_HTTP_STOP;
			
			http_mgr_task_free(curr->cxt);
			free(curr);
		}
	}
	http_mgr_sem_unlock();

	return XFER_ERROR_OK;
}

void http_mgr_speed()
{
	http_async_context_t * cxt = NULL;
	struct llhead		*lp, *tmp;
	http_mgr_list_t* 	curr;

	http_mgr_sem_lock();
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);

		cxt = curr->cxt;
		//if (cxt->mode&XFER_SHOW)
		{	
			if (cxt->state != DLST_DOWNLOADING && cxt->state != DLST_PENDING)
			{	// delete timer when task is not DLST_DOWNLOADING
				cxt->last_recved = cxt->saved_len;
				cxt->speed = 0;
				cxt->erase_time = 0;
			}
			else
			{
				int curr_recved = cxt->saved_len - cxt->last_recved;
				cxt->erase_time += HTTP_MGR_TIMEOUT_NORMAL;
				cxt->speed = curr_recved / (cxt->erase_time / 1000);
				if (cxt->speed < 0)
					cxt->speed = 0;

				if (cxt->erase_time >= HTTP_MGR_SPEED_RESET)
				{
					cxt->erase_time = 0;
					cxt->last_recved = cxt->saved_len;
				}
				XFER_LOG3("task %d speed %d Bps.\n", cxt->task_id, cxt->speed);

				// restart task with speed is zero in a long time.
				if (cxt->state == DLST_DOWNLOADING && cxt->speed == 0)
					cxt->speed_zero_c++;
				else
					cxt->speed_zero_c = 0;
				if (cxt->speed_zero_c == HTTP_MGR_SPEED_ZERO_RESET
					|| cxt->speed > s_http_mgr_max_speed)
				{
					XFER_LOG2("speed zero restart taskid %d.\n", cxt->task_id);
					cxt->speed_zero_c = 0;
					cxt->state 	= DLST_PENDING;
					cxt->buff 	= NULL;
				}
			}		
		}
	}
	http_mgr_sem_unlock();
}

static void http_curl_init()
{

	if(s_http_mgr_status.curl_inited == 0)
	{
		curl_global_init(CURL_GLOBAL_ALL);

		if(s_http_mgr_status.curl_multi_handle == NULL)
			s_http_mgr_status.curl_multi_handle = curl_multi_init();

		if(s_http_mgr_status.curl_multi_handle == NULL)
		{
			curl_global_cleanup();
			return;
		}

#ifdef __WIN32__
		if((s_http_mgr_status.curl_multi_thread = CreateThread(NULL, 
						0, 
						(LPTHREAD_START_ROUTINE)http_curl_run_win32, 
						(LPVOID)&s_http_mgr_status, 
						0, 
						NULL)) == NULL)
#else
		if(pthread_create(&(s_http_mgr_status.curl_multi_thread),
			NULL,
			http_curl_run,
			(void *)&s_http_mgr_status) != 0)
#endif
		{
			debugf(XFER_LOG_HTTP, "Create thread ERROR\n");
			curl_global_cleanup();
			return;
		}
		
		s_http_mgr_status.curl_inited = 1;
	}
}

static void http_curl_finit()
{
	if(s_http_mgr_status.curl_inited == 1)
	{
#ifdef __WIN32__
		DWORD errcode = 0;
		TerminateThread(
				s_http_mgr_status.curl_multi_thread,
				errcode
		);
		CloseHandle(s_http_mgr_status.curl_multi_thread);
#else
		pthread_cancel(s_http_mgr_status.curl_multi_thread);
#endif
		curl_multi_cleanup(s_http_mgr_status.curl_multi_handle);
		curl_global_cleanup();
		s_http_mgr_status.curl_inited = 0;
	}
}

xfer_errid_t http_mgr_init()
{
	if(s_http_mgr_status.http_inited == 1)
		return XFER_ERROR_OK;	

	s_http_mgr_status.http_inited = 1;
	http_mgr_sem_init();
	LL_INIT(&http_mgr_tasks.link);

	http_curl_init();
	//http_load_tasks();

	return XFER_ERROR_OK;
}

void http_mgr_fini()
{
	if(s_http_mgr_status.http_inited == 0)
		return ;	

	s_http_mgr_status.http_inited = 0;

	http_curl_finit();
	//http_save_tasks();
	http_mgr_conn_erase();
	http_mgr_sem_close();
}

xfer_errid_t http_mgr_get_task_max(int * result)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if (result == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	*result = s_http_mgr_tasks_max;

	return errid;
}

xfer_errid_t http_mgr_set_task_max(int value)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if(s_http_mgr_status.curl_inited == 0)
		return XFER_ERROR_HTTP_UNINITIAL;

	if((value > 0) && (value < DL_HTTP_DOWNLOADING_MAX))
	{
		s_http_mgr_tasks_max = value;
		errid = curl_multi_setopt(s_http_mgr_status.curl_multi_handle, CURLMOPT_MAXCONNECTS, value);
	}

	return errid;
}

xfer_errid_t http_mgr_getopt(transfer_options_e opt, int * result)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if (result == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if (opt == DLOPT_RATELIMIT_DOWNLOAD)
		*result = s_http_mgr_max_speed;
	else if (opt == DLOPT_SETUP_DEMO)
		*result = s_http_mgr_setup_demo;
	else if (opt == DLOPT_RATELIMIT_MAX_TASKS)
		errid = http_mgr_get_task_max(result);
	else if (opt == DLOPT_HTTP_MAX_REDIRS)
		*result = s_http_mgr_redirs_max;
	else if (opt == DLOPT_HTTP_MAX_TIMEOUT)
		*result = s_http_mgr_timeout_max;
	else if (opt == DLOPT_HTTP_MAX_CONNTIMEOUT)
		*result = s_http_mgr_connect_timeout_max;
	
	return errid;
}

xfer_errid_t http_mgr_setopt(transfer_options_e opt, int value)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if (opt == DLOPT_RATELIMIT_DOWNLOAD)
		s_http_mgr_max_speed = value;
	else if (opt == DLOPT_SETUP_DEMO)
		s_http_mgr_setup_demo = value;
	else if (opt == DLOPT_RATELIMIT_MAX_TASKS)
		errid = http_mgr_set_task_max(value);
	else if (opt == DLOPT_HTTP_MAX_REDIRS)
		s_http_mgr_redirs_max = value;
	else if (opt == DLOPT_HTTP_MAX_TIMEOUT)
	{
		if(value < 0)
			s_http_mgr_timeout_max = HTTP_MGR_TIMEOUT_MAX;
		else
			s_http_mgr_timeout_max = value;
	}
	else if (opt == DLOPT_HTTP_MAX_CONNTIMEOUT)
	{
		if(value < 0)
			s_http_mgr_connect_timeout_max = HTTP_MGR_CONNECT_TIMEOUT_MAX;
		else
			s_http_mgr_connect_timeout_max = value;
	}
	
	return errid;
}

static int http_mgr_task_exist(const char* src)
{
	struct llhead	 *lp, *tmp;
	http_mgr_list_t* curr;

	http_mgr_sem_lock();
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		if (curr->cxt && curr->cxt->url && strcmp(curr->cxt->url, src) == 0
			&& curr->cxt->errid != XFER_ERROR_HTTP_REMOVED
			&& curr->cxt->state != DLST_CLOSING
			&& curr->cxt->state != DLST_FINISHED)
		{
			http_mgr_sem_unlock();
			return 1;
		}
	}
	http_mgr_sem_unlock();
	
	return 0;
}

void http_mgr_task_free(http_async_context_t* cxt)
{
	if (cxt)
	{
		struct llhead		*lp, *tmp;
		http_async_header_list_t* curr;

		LL_FOREACH_SAFE( &cxt->header_list.link, lp, tmp) 
		{
			curr = LL_ENTRY( lp, http_async_header_list_t, link);
			LL_DEL(&curr->link);
			
			if (curr->name)
			{
				free(curr->name);
				curr->name = NULL;
			}
			if (curr->value)
			{
				free(curr->value);
				curr->value = NULL;
			}

			free(curr);
		}

		if(cxt->state != DLST_FINISHED)
		{
			//if (!(cxt->mode&XFER_SHOW))
				//file_task_send_msg(cxt->task_id, cxt->file_name, FILE_TASK_FINISH, NULL, 0, 0, NULL);
		}
		
		if (cxt->url)
		{
			free(cxt->url);
			cxt->url = NULL;
		}
		if (cxt->request)
		{
			free(cxt->request);
			cxt->request = NULL;
		}
		if (cxt->name)
		{ 
			free(cxt->name);
			cxt->name = NULL;
		}
		if (cxt->file_name)
		{
			free(cxt->file_name);
			cxt->file_name = NULL;
		}
		if (cxt->post_buff)
		{
			free(cxt->post_buff);
			cxt->post_buff = NULL;
		}
		if (cxt->buff)
		{
			free(cxt->buff);
			cxt->buff = NULL;
		}
		if (cxt->host)
		{
			free(cxt->host);
			cxt->host = NULL;
		}

		cxt->task_id = TASKID_INVALID;
		
		free(cxt);
	}
} 

static http_async_context_t* 
http_mgr_task_create(const char* src,
						  const char* saved_path,
						  int type,
						  int mode,
						  const char* post_buff,
						  transfer_complete_cb complete_cb)
{
	char * pos;

	http_async_context_t* cxt = (http_async_context_t*)malloc(sizeof(http_async_context_t));
	if (cxt == NULL)
	{
		return NULL;
	}
	memset(cxt, 0, sizeof(http_async_context_t));

	LL_INIT(&cxt->header_list.link);

	if (mode&XFER_BUFF)
		cxt->save_file = 0;
	else
		cxt->save_file = 1;

	if (mode&XFER_NOCACHE || mode&XFER_BUFF || mode&XFER_SHOW)
		cxt->is_cache = 0;
	else if (saved_path)
		cxt->is_cache = 1;
	else 
		cxt->is_cache = 2;
		
	cxt->type = type;
	cxt->mode = mode;

	if(post_buff != NULL)
	{
		cxt->post_buff = (char*)malloc(strlen(post_buff)+1);
		if (cxt->post_buff == NULL)
		{
			http_mgr_task_free(cxt);
			return NULL;
		}
		strcpy(cxt->post_buff, post_buff);
	}
	
	cxt->errid = XFER_ERROR_OK;
	cxt->url = (char*)malloc(strlen(src)+1);
	if (cxt->url == NULL)
	{
		http_mgr_task_free(cxt);
		return NULL;
	}
	strcpy(cxt->url, src);

	cxt->host = (char*)malloc(strlen(src)+1);
	strncpy(cxt->host, src, strlen(src));
	cxt->host[strlen(src)] = 0;
	cxt->port = 80;

	cxt->request = (char*)malloc(strlen(src)+1);
	if (cxt->request == NULL)
	{
		http_mgr_task_free(cxt);
		return NULL;
	}
	strcpy(cxt->request, src);

	if(cxt->save_file)/* genarate file name */
	{
		char curr_path[256] = {0};
		char curr_name[256] = {0};

		if (saved_path != NULL && saved_path[0]) 
			strcpy(curr_path, saved_path);
		else
			strcpy(curr_path, http_mgr_temp);

		if(curr_path[strlen(curr_path)-1] == '/')
		{
			strcpy(curr_name, curr_path);

			pos = strrchr(src, '/');
			if(pos)
				strcat(curr_name, pos+1);
			else
				strcat(curr_name, "default");
		}
		else
		{
			strcpy(curr_name, curr_path);
			pos = strrchr(curr_path, '/');
			if(pos)
				*(pos+1) = 0;
			else
				strcpy(curr_path, "./");
		}
		debugf(XFER_LOG_HTTP, "HTTP,curr_path=%s, curr_name=%s\n", curr_path, curr_name);

		/* mkdir and genarate name */
		if(0 == make_dir_recursive(curr_path))
			debugf(XFER_LOG_HTTP, "HTTP mkdir(%s) OK!\n", curr_path);
		else
			debugf(XFER_LOG_HTTP, "http mkdir(%s) error!\n", curr_path);

		if(curr_name == NULL)
		{
			debugf(XFER_LOG_HTTP, "http file_name NULL!\n");
			http_mgr_task_free(cxt);
			return NULL;
		}
		else
		{
			cxt->file_name = (char*)malloc(strlen(curr_name)+1);
			if (cxt->file_name == NULL)
			{
				http_mgr_task_free(cxt);
				return NULL;
			}
			strcpy(cxt->file_name, curr_name);
		}
	}

	cxt->state = DLST_NONE;
	cxt->curl = NULL;
	cxt->complete_callback = complete_cb;

	return cxt;
}

xfer_errid_t http_mgr_task_add(const char* 		src, 
						 	  const char* 			saved_path,
						 	  int					type,
						 	  int 					mode,
						 	  const char* 			post_buff,
						 	  transfer_complete_cb	complete_cb,
						 	  transfer_taskid_t*	taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	http_async_context_t* cxt = NULL;
	if (http_mgr_task_exist(src))
	{
		*taskid = TASKID_INVALID;
		return XFER_ERROR_HTTP_TASK_EXIST;
	}
	
	cxt = http_mgr_task_create(src, saved_path, type, mode, post_buff, complete_cb);
	if (cxt == NULL)
	{
		*taskid = TASKID_INVALID;
		return XFER_ERROR_HTTP_CONTEXT_ERR;
	}

	
	http_mgr_sem_lock();
	cxt->task_id = http_get_task_id();
	http_mgr_sem_unlock();

	*taskid = cxt->task_id;
	http_mgr_conn_insert(cxt);
	
	/* append http client headers */
	if(cxt->task_id > 0)
	{
		errid = http_mgr_task_append_header(cxt->task_id,
							     "Accept: image/gif, image/x-xbitmap, image/jpeg, "
							     "image/pjpeg, application/x-shockwave-flash, application/vnd.ms-excel, "
							     "application/vnd.ms-powerpoint, application/msword, application/xaml+xml, "
							     "application/vnd.ms-xpsdocument, application/x-ms-xbap, application/x-ms-application, */*");
		errid = http_mgr_task_append_header(cxt->task_id,
							     "Accept-Language: zh-cn");
		errid = http_mgr_task_append_header(cxt->task_id,
							     "UA-CPU: x86");
		errid = http_mgr_task_append_header(cxt->task_id,
							     "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1;"
							     "POTU(RR:27091922:0:5183311); .NET CLR 1.1.4322; .NET CLR 2.0.50727; "
							     ".NET CLR 3.0.04506.30; .NET CLR 3.5.20404)");
	}

	XFER_LOG2("add :%d\n", *taskid);
	debugf(XFER_LOG_HTTP, "http_mgr_task_add(), taskid=%d\n", *taskid);
	return XFER_ERROR_OK;
}

xfer_errid_t 
http_mgr_task_append_header(transfer_taskid_t taskid, char *header)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	struct llhead		*lp, *tmp;
	http_mgr_list_t* curr;

	XFER_LOG2("close :%d\n" , taskid);
	debugf(XFER_LOG_HTTP, "http_mgr_add_heade(), taskid=%d, header=%s\n" , taskid, header);
	http_mgr_sem_lock();
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		if (curr->cxt && curr->cxt->task_id == taskid)
		{
			curr->cxt->slist = curl_slist_append(curr->cxt->slist, header);
			break;
		}
	}

	http_mgr_sem_unlock();
	return errid;
}

void http_mgr_task_close(transfer_taskid_t taskid)
{

	http_async_context_t* cxt = NULL;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return;
	}

	/* close a task */
	cxt->state = DLST_CLOSING;
	debugf(XFER_LOG_HTTP, XFER_LOG_MAIN, "http_task_close(), task_id=%d, task_state: CLOSING\n", taskid);

	http_mgr_sem_unlock();
	
	return;
}

void http_mgr_task_cancel(transfer_taskid_t taskid)
{
	struct llhead		*lp, *tmp;
	http_mgr_list_t* curr;
	int mode = XFER_NULL;

	XFER_LOG2("cancel :%d\n", taskid ) ;
	http_mgr_sem_lock();
	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		curr = LL_ENTRY( lp, http_mgr_list_t, link);
		if (curr->cxt && curr->cxt->task_id == taskid)
		{
			curr->cxt->errid = XFER_ERROR_HTTP_REMOVED;

			mode = curr->cxt->mode;

			if(curr->cxt->save_file && curr->cxt->fd)
			{
				fclose(curr->cxt->fd);
				curr->cxt->fd = NULL;
			}
			
			if(curr->cxt->curl)
			{
			 	curl_multi_remove_handle(s_http_mgr_status.curl_multi_handle, curr->cxt->curl);
			 	curl_easy_cleanup(curr->cxt->curl);
				curr->cxt->curl = NULL;
			}
			if(curr->cxt->slist)
			{
				curl_slist_free_all(curr->cxt->slist); 
				curr->cxt->slist = NULL;
			}
			
			break;
		}
	}
	http_mgr_sem_unlock();
}

xfer_errid_t 
http_mgr_task_request(transfer_taskid_t taskid)
{
	int res = 0;
	struct stat stat_buf;
	char host[256];
	transfer_task_stat_e state = DLST_NONE;
	http_async_context_t* cxt = NULL;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		cxt->errid = XFER_ERROR_HTTP_IN_PARAM_NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
	}
	HTTP_DBG_MSG_SENT(cxt->task_id, 96);

	if(cxt->file_name != NULL)
		res = stat(cxt->file_name, &stat_buf);
	
	if (http_mgr_get_context(cxt->task_id) != cxt)
	{
		cxt->errid = XFER_ERROR_HTTP_IN_PARAM_NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
	}

	cxt->state = DLST_NONE;
	cxt->errid = XFER_ERROR_OK;
	cxt->complete_callbacked = 0;
		
	if (cxt->file_name != NULL && res != -1 && cxt->saved_len != 0 && (cxt->file_len >= cxt->saved_len || cxt->file_len == 0))
	{
		if (stat_buf.st_size == 0)
			cxt->saved_len = 0;
		else if (stat_buf.st_size < cxt->saved_len)
			cxt->saved_len = stat_buf.st_size;
	}

	if (cxt->save_file && cxt->file_name != NULL && !(cxt->mode&XFER_SHOW) && cxt->is_cache)
	{
		if (res != -1 && stat_buf.st_size != 0)
		{
			cxt->file_len = stat_buf.st_size;
		}
	}
	strcpy(host, cxt->host);
	state = cxt->state;

	if(cxt->curl == NULL)
	{
		cxt->curl = curl_easy_init( );
		if(cxt->curl == NULL)
		{
			http_mgr_sem_unlock();
			return XFER_ERROR_HTTP_STATUS_ERROR;
		}
		
		if(cxt->url)
		{
			curl_easy_setopt(cxt->curl, CURLOPT_URL, cxt->url);
			curl_easy_setopt(cxt->curl, CURLOPT_PRIVATE, cxt->task_id);

			curl_easy_setopt(cxt->curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(cxt->curl, CURLOPT_MAXREDIRS, s_http_mgr_redirs_max);
			curl_easy_setopt(cxt->curl, CURLOPT_NOSIGNAL, 1L);
			curl_easy_setopt(cxt->curl, CURLOPT_TIMEOUT, s_http_mgr_timeout_max);
			curl_easy_setopt(cxt->curl, CURLOPT_CONNECTTIMEOUT, s_http_mgr_connect_timeout_max);

			curl_easy_setopt(cxt->curl, CURLOPT_NOPROGRESS, 0L);
			curl_easy_setopt(cxt->curl, CURLOPT_PROGRESSFUNCTION, http_curl_progress);
			curl_easy_setopt(cxt->curl, CURLOPT_PROGRESSDATA, cxt);

			curl_easy_setopt(cxt->curl, CURLOPT_HTTPHEADER, cxt->slist);

			if(cxt->save_file == 1)
			{
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEFUNCTION, http_curl_fwrite);
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEDATA, cxt);
			}
			else
			{
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEFUNCTION, http_curl_mwrite);
				curl_easy_setopt(cxt->curl, CURLOPT_WRITEDATA, cxt);
			}
		}
		else
		{
			curl_easy_cleanup(cxt->curl);
			cxt->curl = NULL;
			curl_slist_free_all(cxt->slist); 
			cxt->slist = NULL;
			http_mgr_sem_unlock();
			return XFER_ERROR_HTTP_STATUS_ERROR;
		}

		/* for debug curl */
#ifdef HTTP_DEBUG_ENABLE
		curl_easy_setopt(cxt->curl, CURLOPT_VERBOSE, 1L);
#endif

		res = curl_multi_add_handle(s_http_mgr_status.curl_multi_handle, cxt->curl);
	}
	else
	{
		res = curl_easy_pause(cxt->curl, CURLPAUSE_CONT);
	}
	state = DLST_DOWNLOADING;
	
	if (http_mgr_get_context(cxt->task_id) != cxt)
	{
		res = XFER_ERROR_HTTP_IN_PARAM_NULL;
	}
	else
	{
		cxt->errid = res;
		cxt->state = state;
	}

	http_mgr_sem_unlock();
	return res;
}

xfer_errid_t http_mgr_task_start(transfer_taskid_t taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	http_async_context_t* cxt = NULL;
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);

	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	if (cxt->state == DLST_DOWNLOADING)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_OK;
	}

	/* start a task */
	cxt->state = DLST_STARTING;
	debugf(XFER_LOG_HTTP, XFER_LOG_MAIN, "http_task_start(), task_id=%d, task_state: STARTING\n", taskid);

	http_mgr_sem_unlock();

	return errid;
}

xfer_errid_t http_mgr_task_pause(transfer_taskid_t taskid)
{
	http_async_context_t* cxt = NULL;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	/* pause a task */
	cxt->state = DLST_STOPPING;
	debugf(XFER_LOG_HTTP, XFER_LOG_MAIN, "http_task_stop(), task_id=%d, task_state: STOPPING\n", taskid);

	http_mgr_sem_unlock();
	
	return XFER_ERROR_HTTP_SUCCESS;
}

xfer_errid_t http_mgr_task_fault(transfer_taskid_t taskid, int curr_len, xfer_errid_t errid)
{
	int mode = 0;
	transfer_task_stat_e state = DLST_NONE;
	http_async_context_t* cxt = NULL;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	if (curr_len >= 0)
		cxt->saved_len = curr_len;
	
	cxt->errid 	= errid;
	cxt->state 	= DLST_ERROR;

	mode = cxt->mode;
	state = cxt->state;
	http_mgr_sem_unlock();

	return XFER_ERROR_HTTP_SUCCESS;
}

xfer_errid_t http_mgr_task_saved(transfer_taskid_t taskid, int curr_len)
{
	http_async_context_t* cxt = NULL;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	if (curr_len >= 0)
		cxt->saved_len = curr_len;
	
	http_mgr_sem_unlock();
	return XFER_ERROR_HTTP_SUCCESS;
}

xfer_errid_t http_mgr_task_finish(transfer_taskid_t taskid, xfer_errid_t errid)
{
	http_async_context_t* cxt = NULL;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}
	if (cxt->mode&XFER_SHOW && errid != XFER_ERROR_OK)
	{
		cxt->errid = errid;
		cxt->state = DLST_ERROR;
	}
	else
	{
		cxt->state 	= DLST_FINISHED;
	}
	http_mgr_sem_unlock();
	
	return XFER_ERROR_HTTP_SUCCESS;
}

xfer_errid_t http_mgr_task_complete_cb(transfer_taskid_t taskid, xfer_errid_t errid)
{
	http_async_context_t* cxt = NULL;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}
	if (errid != XFER_ERROR_OK)
	{
		cxt->errid = errid;
		cxt->state = DLST_ERROR;
	}
	else
	{
		cxt->state = DLST_FINISHED;
	}
	if(cxt->save_file && cxt->fd)
	{
		fclose(cxt->fd);
		cxt->fd = NULL;
	}

	http_mgr_sem_unlock();

	/* task complete callback */
	debugf(XFER_LOG_HTTP, ">>>> HTTP task complete: err=%d, taskid=%d\n", errid, taskid);
	if(cxt->complete_callback && (cxt->complete_callbacked == 0))
	{
		cxt->complete_callbacked = 1;
		if(cxt->mode & XFER_BUFF)
			cxt->complete_callback(taskid, cxt->buff, cxt->buff_len, errid);
		else
			cxt->complete_callback(taskid, cxt->file_name, cxt->saved_len, errid);
	}

	return errid;
}

xfer_errid_t http_mgr_set_task_state(transfer_taskid_t taskid, transfer_task_stat_e state)
{
	http_async_context_t* cxt = NULL;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	cxt->state = state;

	http_mgr_sem_unlock();
	return XFER_ERROR_OK;
}

xfer_errid_t http_mgr_get_task_state(transfer_taskid_t taskid, transfer_task_stat_e *state)
{
	http_async_context_t* cxt = NULL;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	*state = cxt->state;

	http_mgr_sem_unlock();
	return XFER_ERROR_OK;
}

xfer_errid_t http_mgr_get_task_list(transfer_task_list_t** tasklist)
{
	struct llhead		*lp, *tmp;
	transfer_taskid_t*  pos = NULL;

	http_mgr_sem_lock();

	*tasklist = (transfer_task_list_t*)malloc(sizeof(transfer_task_list_t));
	if (*tasklist == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*tasklist, 0, sizeof(transfer_task_list_t));
	if (http_mgr_count.task == 0)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_OK;
	}
	(*tasklist)->task_count = http_mgr_count.task;
	(*tasklist)->task_ids = 
		(transfer_taskid_t*)malloc(sizeof(transfer_taskid_t)*(*tasklist)->task_count);
	if ((*tasklist)->task_ids == NULL)
	{
		free(*tasklist);
		*tasklist = NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	pos = (*tasklist)->task_ids;

	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		http_mgr_list_t* 	curr;
		curr = LL_ENTRY( lp, http_mgr_list_t, link);

		if (curr->cxt->mode&XFER_SHOW)
		{	
			if (curr->cxt->errid == XFER_ERROR_HTTP_REMOVED)
			{
				(*tasklist)->task_count--;
				continue;
			}
			*pos = curr->cxt->task_id;
			pos++;
			assert(pos < ((*tasklist)->task_ids + sizeof(transfer_taskid_t)*(*tasklist)->task_count));
		}
	}
	http_mgr_sem_unlock();

	return XFER_ERROR_OK;
}

xfer_errid_t http_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo)
{
	char* pos = NULL;
	http_async_context_t* cxt = NULL;
	transfer_task_stat_e  state = DLST_NONE;
	
	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_TASKID_ERR;
	}
	
	*taskinfo = (transfer_task_info_t*)malloc(sizeof(transfer_task_info_t));
	if ((*taskinfo) == NULL)
	{
		*taskinfo = NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*taskinfo, 0, sizeof(transfer_task_info_t));

	if(cxt->file_name)
	{
		pos = strrchr(cxt->file_name, '/');
		if (pos == NULL)
			pos = cxt->file_name;
		else
		{
			int i_path_len = pos - cxt->file_name  ;
			(*taskinfo)->task_path = (char*)malloc(i_path_len + 2);
			if( (*taskinfo)->task_path )
			{
				memcpy( (*taskinfo)->task_path , cxt->file_name , i_path_len + 1 ) ;
				(*taskinfo)->task_path[ i_path_len + 1 ] = '\0' ;
			}	
		}
		
		(*taskinfo)->task_name = (char*)malloc(strlen(pos)+1);
		if ((*taskinfo)->task_name == NULL)
		{
			free(*taskinfo);
			*taskinfo = NULL;
			http_mgr_sem_unlock();
			return XFER_ERROR_HTTP_MALLOC_FAILURE;
		}
		strcpy((*taskinfo)->task_name, pos+1);
	}
	(*taskinfo)->task_id = taskid;
	(*taskinfo)->task_stat = (cxt->state == DLST_PENDING) ? DLST_DOWNLOADING : cxt->state;
	(*taskinfo)->speed = cxt->speed;
	(*taskinfo)->total_size = cxt->file_len;
	(*taskinfo)->downloaded_size = (cxt->saved_len > cxt->curr_len) ? cxt->saved_len : cxt->curr_len;
	(*taskinfo)->error_id = cxt->errid;

	state = cxt->state;
	http_mgr_sem_unlock();

	return XFER_ERROR_OK;
}

xfer_errid_t
http_mgr_set_tasks_info_file(const char* file_name)
{
	if (file_name == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if (strlen(file_name)+1 > DL_MAX_PATH)
	{
		strncpy(http_mgr_taskinfo, file_name, DL_MAX_PATH-1);
		http_mgr_taskinfo[DL_MAX_PATH-1] = 0;
	}
	else
		strcpy(http_mgr_taskinfo, file_name);
	
	return XFER_ERROR_OK;
}

xfer_errid_t
http_mgr_set_temp_path(const char* path)
{
	if (path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if (strlen(path)+1 > DL_MAX_PATH)
	{
		strncpy(http_mgr_temp, path, DL_MAX_PATH-1);
		http_mgr_temp[DL_MAX_PATH-1] = 0;
	}
	else
		strcpy(http_mgr_temp, path);
	
	return XFER_ERROR_OK;
}

xfer_errid_t http_mgr_get_all_task_list(transfer_task_list_t** tasklist)
{
	struct llhead		*lp, *tmp;
	transfer_taskid_t*  pos = NULL;

	http_mgr_sem_lock();

	*tasklist = (transfer_task_list_t*)malloc(sizeof(transfer_task_list_t));
	if (*tasklist == NULL)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	memset(*tasklist, 0, sizeof(transfer_task_list_t));
	if (http_mgr_count.total == 0)
	{
		http_mgr_sem_unlock();
		return XFER_ERROR_OK;
	}
	(*tasklist)->task_count = http_mgr_count.total;
	(*tasklist)->task_ids = 
		(transfer_taskid_t*)malloc(sizeof(transfer_taskid_t)*(*tasklist)->task_count);
	if ((*tasklist)->task_ids == NULL)
	{
		free(*tasklist);
		*tasklist = NULL;
		http_mgr_sem_unlock();
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	}
	pos = (*tasklist)->task_ids;

	LL_FOREACH_SAFE( &http_mgr_tasks.link, lp, tmp) 
	{
		http_mgr_list_t* 	curr;
		curr = LL_ENTRY( lp, http_mgr_list_t, link);

		//if (curr->cxt->mode&XFER_SHOW)
		{	
			if (curr->cxt->errid == XFER_ERROR_HTTP_REMOVED)
			{
				(*tasklist)->task_count--;
				continue;
			}
			*pos = curr->cxt->task_id;
			pos++;
			assert(pos < ((*tasklist)->task_ids + sizeof(transfer_taskid_t)*(*tasklist)->task_count));
		}
	}
	http_mgr_sem_unlock();

	return XFER_ERROR_OK;
}

void 
http_mgr_release_task_list(transfer_task_list_t* tasklist)
{
    if( tasklist) {
        if( tasklist->task_ids) {
            free(tasklist->task_ids);
        }
        free(tasklist);
    } 
}
