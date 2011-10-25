/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of http_task lib
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/


#ifndef _HTTP_MANAGER_H_
#define _HTTP_MANAGER_H_

#include <curl/curl.h>

#include "../common/llist.h"
#include "transfer_def.h"

#if defined(__WIN32__) || defined(_MSC_VER)
#include <Windows.h>
#else
#include <pthread.h>
#endif


typedef enum http_mgr_state_{
	HTTP_MGR_NONE,
	HTTP_MGR_LOADED,
	HTTP_MGR_UNLOADED,
}http_mgr_state_e;


typedef enum
{
	TIMER_FLAG_NORMAL,
	TIMER_FLAG_CACHED,
	TIMER_FLAG_CACHED_DNS,
	
	TIMER_FLAG_NONE
}timer_flag_e;

typedef struct http_async_header_list
{
	struct llhead link;
	
	char* name;
	char* value;
}http_async_header_list_t;

typedef struct http_mgr_mem 
{
	char *mem;
	size_t size;
}http_mgr_mem_t;

typedef struct http_async_context
{
	///< need to save file (download in order)
	transfer_taskid_t	task_id;// task id	
	int				type;		// task type
	int				mode;		// task mode
	char* 			url;		// task url	
	int 			file_len;
	int 			curr_len;
	int				saved_len;

	///< runtime validate
	char* 			name;
	char* 			file_name;	
	char* 			post_buff;
	char* 			host;
	char* 			request;
	char* 			buff;
	int				buff_len;
	int				port;
	int 			save_file;
	int				is_cache;	// 0 not cache; 1 cache; 2 cache and managed
	int 			erase_time;
	int				speed;
	int				speed_zero_c;
	int 			last_recved;
	int 			retries;
	xfer_errid_t		errid;

	transfer_task_stat_e	state;

	http_async_header_list_t header_list;
	
	transfer_complete_cb complete_callback;
	CURL * curl;
	struct curl_slist *slist;
	FILE * fd;
	int complete_callbacked;
	
} http_async_context_t;

typedef struct http_mgr_status
{
	unsigned int http_inited;
	unsigned int curl_inited;
	

	unsigned int task_total;
	unsigned int task_running;
	unsigned int task_finished;
	
	CURLM *curl_multi_handle;

	#ifdef __WIN32__
	HANDLE curl_multi_thread;
	#else
	pthread_t curl_multi_thread;
	#endif

} http_mgr_status_t;

typedef struct http_mgr_task_list
{
	struct llhead	link;

	http_async_context_t* cxt;

}http_mgr_list_t;


/**
 * initialize the http manager, and load all the saved tasks
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_init();

/**
 * finalize the http manager
 */
void 
http_mgr_fini();

/**
 * get options
 * @param opt                       [in]    option type
 * @param result                    [out]   option value
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_getopt(transfer_options_e opt, int * result);

/**
 * set options
 * @param opt                       [in]    option type
 * @param value                     [in]    option value
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_setopt(transfer_options_e opt, int value);

/**
 * add a task
 * @param source_path              	[in]    the urs of the transfered file
 * @param saved_path                [in]    the path to save the transfered file
 * @param type	                    [in]   	the created task type
 * @param complete_callback	        [in]   	the created task complete callback
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_add( const char* 		  	src, 
				   const char* 		  	saved_path,
				   int			 	  	type,
				   int					mode,
				   const char* 			post_buff,
				   transfer_complete_cb	complete_callback,
				   transfer_taskid_t* 	taskid);

/**
 * append a http header
 * @param taskid                    [in]    the task id
 * @param complete_callback	        [in]   	the header that to be added
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_append_header(transfer_taskid_t taskid, char *header);

/**
 * close a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
void 
http_mgr_task_close(transfer_taskid_t taskid);


/**
 * remove a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
void 
http_mgr_task_cancel(transfer_taskid_t taskid);

/**
 * start a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_start(transfer_taskid_t taskid);

/**
 * pause a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_pause(transfer_taskid_t taskid);

/**
 * fault a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_fault(transfer_taskid_t taskid, int curr_len, xfer_errid_t errid);

/**
 * set saved data length for a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_saved(transfer_taskid_t taskid, int curr_len);

/**
 * finished a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_task_finish(transfer_taskid_t taskid, xfer_errid_t errid);

/**
 * task complete callback
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t http_mgr_task_complete_cb(transfer_taskid_t taskid, xfer_errid_t errid);

xfer_errid_t 
http_mgr_set_task_state(transfer_taskid_t taskid, transfer_task_stat_e state);

xfer_errid_t 
http_mgr_get_task_state(transfer_taskid_t taskid, transfer_task_stat_e *state);

/**
 * get the task list. Notice the task list is a duplicated copy so that there is no thread conflict
 * @param tasklist                  [out]   the returned task list
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_get_task_list(transfer_task_list_t** tasklist);

/**
 * get the information of the specified task. Notice the task info is a duplicated copy so that there is no thread conflict
 * @param taskid                    [in]    the task id
 * @param taskinfo                  [out]   the returned task info
 * @return                                  error id
 */
xfer_errid_t 
http_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo);


/*
* set file name that save all tasks info 
 * @param file_name                 [in]    file name
 * @return                               	error id
 */
xfer_errid_t
http_mgr_set_tasks_info_file(const char* file_name);

/*
* set file path that save all temp file 
 * @param path                 		[in]    file path
 * @return                               	error id
 */
xfer_errid_t
http_mgr_set_temp_path(const char* path);


xfer_errid_t 
http_mgr_task_request(transfer_taskid_t taskid);

xfer_errid_t http_mgr_get_all_task_list(transfer_task_list_t** tasklist);

void 
http_mgr_release_task_list(transfer_task_list_t* tasklist);

void 
http_mgr_sem_lock_(const char* func, const char* file, int line);

void 
http_mgr_sem_unlock_(const char* func, const char* file, int line);

void
http_mgr_task_close(transfer_taskid_t taskid);

http_async_context_t* 
http_mgr_get_context(transfer_taskid_t taskid);

int 
http_mgr_taskid_validate(transfer_taskid_t taskid);

xfer_errid_t 
http_load_tasks();

xfer_errid_t 
http_save_tasks();

xfer_errid_t 
http_unload_tasks();

int
read_whole_file(char** buff, const char* file_name);

#ifdef HAS_OFFLINE_DEMO
/* just for demo mode */
xfer_errid_t
http_demo_xml_parse(const char* file_name);

xfer_errid_t
http_demo_xml_release();

xfer_errid_t 
http_demo_task_start(transfer_taskid_t taskid);
#endif

#ifdef TEST_DEAD_LOCK
#define http_mgr_sem_lock() 	http_mgr_sem_lock_(NULL, __FILE__, __LINE__)
#define http_mgr_sem_unlock()	http_mgr_sem_unlock_(NULL, __FILE__, __LINE__)
#else
#define http_mgr_sem_lock()		http_mgr_sem_lock_(NULL, NULL, 0)
#define http_mgr_sem_unlock()	http_mgr_sem_unlock_(NULL, NULL, 0)
#endif

//#define XFER_LOG_OUTPUT
#ifdef XFER_LOG_OUTPUT
#define XFER_LOG(a) 		printf(a)
#define XFER_LOG2(a, b) 	printf(a, b)
#define XFER_LOG3(a, b, c) 	printf(a, b, c)
#else
#define XFER_LOG(a) 		
#define XFER_LOG2(a, b) 	
#define XFER_LOG3(a, b, c) 	
#endif	

#ifndef __ROM_
void http_dbg_msg_sent_(transfer_taskid_t taskid, int type);
void http_dbg_msg_done_(transfer_taskid_t taskid, int type);
#define HTTP_DBG_MSG_SENT(taskid,type) http_dbg_msg_sent_(taskid, type);
#define HTTP_DBG_MSG_DONE(taskid,type) http_dbg_msg_done_(taskid, type);
#else
#define HTTP_DBG_MSG_SENT(taskid,type) 
#define HTTP_DBG_MSG_DONE(taskid,type) 
#endif

#endif //_HTTP_MANAGER_H_
