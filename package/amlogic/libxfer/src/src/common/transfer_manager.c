/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Manager 
 *
 *  Author: Amlogic software
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef __WIN32__ 
#ifndef _MSC_VER
#include <sys/vfs.h>
#endif
#endif
#include "xfer_common.h"
#include "xfer_debug.h"
#include "transfer_utils.h"
#include "transfer_ctrl.h"
#include "transfer_manager.h"


#ifdef __WIN32__
#include <Windows.h>
#else
#include <semaphore.h>
#endif


//#define TRANSFER_MANAGER_TASK_BT
#define TRANSFER_MANAGER_TASK_HTTP

#ifdef HAS_MODULE_THUNDER //define in Makefile
#define TRANSFER_MANAGER_TASK_XUNLEI
#endif

/* various implement */
#ifdef TRANSFER_MANAGER_TASK_BT
#include "net/ABoxBT/aboxbt.h"
#endif
#ifdef TRANSFER_MANAGER_TASK_HTTP
#include "../http/http_manager.h"
//#include "file_task.h"
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
#include "../thunder/thunder_manager.h"
#endif

//#include "crypto/file_encode.h"

#define XFER_SEM_ENABLE 1

#define TASK_TYPE_OFFSET	16

static int xfer_init_flag = 0;
static int s_xfer_register_flag = 0;
static int xfer_load_flag = 0;
static int xfer_http_load_flag = 0;
static int xfer_thunder_load_flag = 0;

static transfer_task_list_t* pending_tasks = NULL;

char xfer_path_prefix[DL_MAX_PATH] = "";
char xfer_load_path[DL_MAX_PATH] = "amlogic_download/"; // "/mnt/C/Download/";
static char xfer_pending_file[DL_MAX_PATH] = ""; // "amlogic_download/pend.aml";
char xfer_demo_path[DL_MAX_PATH] = "";

static int s_transfer_sem_inited = 0;
#if XFER_SEM_ENABLE 
#ifdef __WIN32__
HANDLE s_transfer_sem;
#else
sem_t s_transfer_sem;
#endif
#endif

//static AVTimer_t xfer_mgr_timer = AVTIMER_INVALID;

#ifdef TRANSFER_MANAGER_TASK_BT
static int xfer_bt_max_download = 1024*1024*1024;
static int xfer_bt_max_upload   = 1024*1024*1024;
#endif

static int xfer_max_tasks = 64;
#ifdef TEST_DEAD_LOCK
int  xfer_mgr_lock_flag = 0;
int  xfer_mgr_log_count = 0;
char xfer_mgr_run_line[512];
#endif

static int xfer_thunder_enabled=1;
#if 0
int 				xfer_cancel_cxt = -1;
transfer_cancel_cb	xfer_cancel_cb = NULL;
#endif
transfer_status_cb 	xfer_status_cb = NULL;
transfer_complete_cb xfer_complete_cb = NULL;


void transmgr_enable_thunder(char bEnable)
{
	xfer_thunder_enabled=bEnable;
}

void transfer_sem_init()
{
	debugf(XFER_LOG_DETAIL, "transfer_sem_init\n");
#if XFER_SEM_ENABLE 
	while(!s_transfer_sem_inited)
	{
#ifdef __WIN32__
		if((s_transfer_sem = CreateSemaphore( 
								NULL, // default security attributes
								1, // initial count
								1, // maximum count
								NULL)) != NULL) // unnamed semaphore
#else
		if(sem_init(&s_transfer_sem, 0, 1) == 0)
#endif
		{
			s_transfer_sem_inited = 1; 
			break;
		}
		else
		{
			s_transfer_sem_inited = 0; 
		}; 
	}
#endif
#ifdef TEST_DEAD_LOCK
	xfer_mgr_lock_flag = 0;
#endif

}

void transfer_sem_lock_(const char* func, const char* file, int line)
{
#ifdef TEST_DEAD_LOCK
	debugf(XFER_LOG_DETAIL, "transfer_sem_lock:%s,%d\n", file, line);
	xfer_mgr_log_count++; 
	sprintf(xfer_mgr_run_line+(xfer_mgr_log_count%5)*100, "@%d func:%s, %s(%d)", xfer_mgr_log_count, func, file, line); 
#endif

#if XFER_SEM_ENABLE 
#ifdef __WIN32__
	WaitForSingleObject(s_transfer_sem, INFINITE);
#else
	sem_wait(&s_transfer_sem);
#endif
#endif

#ifdef TEST_DEAD_LOCK
	sprintf(xfer_mgr_run_line+(xfer_mgr_log_count%5)*100, "L@%d func:%s, %s(%d)", xfer_mgr_log_count, func, file, line); 
	xfer_mgr_lock_flag ++;
#endif
}

void transfer_sem_unlock_(const char* func, const char* file, int line)
{
#ifdef TEST_DEAD_LOCK
	debugf(XFER_LOG_DETAIL, "transfer_sem_unlock:%s,%d\n", file, line);
	xfer_mgr_log_count++; 
	sprintf(xfer_mgr_run_line+(xfer_mgr_log_count%5)*100, "U@%d func:%s, %s(%d)", xfer_mgr_log_count, func, file, line); 
#endif
#if XFER_SEM_ENABLE 
#ifdef __WIN32__
	ReleaseSemaphore(s_transfer_sem, 1, NULL);
#else
	sem_post(&s_transfer_sem);
#endif
#endif

#ifdef TEST_DEAD_LOCK
	xfer_mgr_lock_flag --;
#endif
}

void transfer_sem_close()
{
#if XFER_SEM_ENABLE 
	debugf(XFER_LOG_DETAIL, "transfer_sem_close\n");
	while(s_transfer_sem_inited)
	{
#ifdef __WIN32__
		CloseHandle(s_transfer_sem);
#else
		sem_destroy(&s_transfer_sem);
#endif
		s_transfer_sem_inited=0;
		break;
	}
#endif
#ifdef TEST_DEAD_LOCK
	xfer_mgr_lock_flag = 0;
#endif
}

#ifdef TEST_DEAD_LOCK
#define transfer_sem_lock() 	transfer_sem_lock_(NULL, __FILE__, __LINE__)
#define transfer_sem_unlock()	transfer_sem_unlock_(NULL, __FILE__, __LINE__)
#else
#define transfer_sem_lock()		transfer_sem_lock_(NULL, NULL, 0)
#define transfer_sem_unlock()	transfer_sem_unlock_(NULL, NULL, 0)
#endif

int push_pending_task(transfer_taskid_t taskid);

static int init_pending_tasks()
{
	transfer_sem_lock();
	if (pending_tasks == NULL)
	{
		pending_tasks = 
			(transfer_task_list_t*)xfer_malloc(sizeof(transfer_task_list_t));
		if (pending_tasks == NULL)
		{
			transfer_sem_unlock();
			return XFER_ERROR_MALLOC_FAILURE;
		}
		memset(pending_tasks, 0, sizeof(transfer_task_list_t));
	}
	transfer_sem_unlock();

	return XFER_ERROR_OK;
}

static int load_pending_tasks()
{
	int			pendings = 0;
	char* 		buff = NULL;
	char* 		pos = NULL;
	int 		last_len = 0;
	int			taskids_len = 0;

	if ((last_len = read_whole_file(&buff, xfer_pending_file)) < 24)
	{
		if (buff)
			xfer_free(buff);
		return XFER_ERROR_OK;
	}
	pos = buff + 20;
	last_len -= 20;

	memcpy((void*)&pendings, pos, sizeof(int));
	if (pendings < 0 || pendings > 1024)
	{
		xfer_free(buff);
		return XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
	}
	pos += sizeof(int);
	last_len -= sizeof(int);

	transfer_sem_lock();
	if (pending_tasks == NULL)
	{
		pending_tasks = 
			(transfer_task_list_t*)xfer_malloc(sizeof(transfer_task_list_t));
		if (pending_tasks == NULL)
		{
			xfer_free(buff);
			transfer_sem_unlock();
			return XFER_ERROR_MALLOC_FAILURE;
		}
		memset(pending_tasks, 0, sizeof(transfer_task_list_t));
	}
	pending_tasks->task_count = pendings;
	
	taskids_len = pending_tasks->task_count * sizeof(transfer_taskid_t);
	if (last_len < taskids_len)
	{
		pending_tasks->task_count = 0;
		xfer_free(buff);
		transfer_sem_unlock();
		return XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
	}
	if (taskids_len > 10*sizeof(transfer_taskid_t))
		pending_tasks->capacity = taskids_len;
	else
		pending_tasks->capacity = 10*sizeof(transfer_taskid_t);
	pending_tasks->task_ids = (transfer_taskid_t*)xfer_malloc(pending_tasks->capacity);
	if (pending_tasks->task_ids == NULL)
	{
		pending_tasks->capacity = 0;
		pending_tasks->task_count = 0;
		xfer_free(buff);
		transfer_sem_unlock();
		return XFER_ERROR_MALLOC_FAILURE;
	}
	memcpy((void*)pending_tasks->task_ids, pos, taskids_len);
	transfer_sem_unlock();
	
	xfer_free(buff);
	return XFER_ERROR_HTTP_TASKID_ERR;
}

static int save_pending_tasks()
{
	char* pos = NULL;
	int   res = 0;
	int	  buff_len = 20 + sizeof(int) + sizeof(transfer_taskid_t)*pending_tasks->task_count;
	char* buff = (char*)xfer_malloc(buff_len);

	if (buff == NULL)
		return XFER_ERROR_HTTP_MALLOC_FAILURE;

	pos = buff;
	memcpy(pos, "1.00", 4);
	pos += 4;
	//memcpy(pos, "1234567890123456", 16);
	pos += 16;
	transfer_sem_lock();
	memcpy(pos, (void*)&pending_tasks->task_count, sizeof(int));
	pos += sizeof(int);
	memcpy(pos, pending_tasks->task_ids, sizeof(transfer_taskid_t)*pending_tasks->task_count);
	transfer_sem_unlock();

	res = write_file_safe(xfer_pending_file, buff, buff_len);
	
	xfer_free(buff);
	return res;
}

static int empty_pending_tasks()
{
	transfer_sem_lock();
	if (pending_tasks)
	{
		pending_tasks->task_count = 0;
	}
	transfer_sem_unlock();

	return XFER_ERROR_OK;
}

static int clear_pending_tasks()
{
	transfer_sem_lock();
	if (pending_tasks)
	{
		if (pending_tasks->task_ids)
		{
			xfer_free(pending_tasks->task_ids);
			pending_tasks->task_ids = NULL;
		}
		xfer_free(pending_tasks);
		pending_tasks = NULL;
	}
	transfer_sem_unlock();

	return XFER_ERROR_OK;
}

static int is_pending_task(transfer_taskid_t taskid)
{
	int i = 0;
	transfer_sem_lock();
	if (pending_tasks == NULL || pending_tasks->task_count == 0)
	{
		transfer_sem_unlock();
		return 0;
	}
	
	for (i = 0; i < pending_tasks->task_count; i++)
	{
		if (pending_tasks->task_ids[i] == taskid)
		{
			transfer_sem_unlock();
			return 1;
		}
	}
	transfer_sem_unlock();
	return 0;
}

int get_tansfer_type(transfer_taskid_t taskid)
{		
	if (taskid >= TASKID_BT_BEGIN && taskid <= TASKID_BT_END)
		return XFER_TASK_BT;
	else if (taskid >= TASKID_HTTP_BEGIN && taskid <= TASKID_HTTP_END)
	{
		return XFER_TASK_HTTP;
	}
	else if (taskid >= TASKID_THUNDER_BEGIN && taskid < TASKID_THUNDER_END)
		return XFER_TASK_XLURL;

	return XFER_TASK_ERROR;
}

static 
int get_transfer_mode(transfer_taskid_t taskid)
{
	int mode = XFER_NULL;
#ifdef TRANSFER_MANAGER_TASK_HTTP
	http_async_context_t* cxt = NULL;

	http_mgr_sem_lock();
	cxt = http_mgr_get_context(taskid);
	if (cxt)
		mode = cxt->mode;
	http_mgr_sem_unlock();
#endif
	return mode;
}

#if 0
static void xfer_timer_callback(void *arg)
{
	static int count = 0;
	file_task_send_msg(TASKID_INVALID, NULL, FILE_TASK_LIMIT, NULL, 0, 0, NULL);
}
#endif

void transfer_mgr_complete_callback(transfer_taskid_t taskid, const char* result, int len, int errid)
{
	debugf(XFER_LOG_DETAIL, ">>>> transfer_mgr_complete_callback(), taskid=%d, errno=%d.\n", taskid, errid);

	return;
}

xfer_errid_t transfer_mgr_init_ex(char* lic)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if (xfer_init_flag)
		return XFER_ERROR_OK;

	debugf(XFER_LOG_DETAIL, "xfer_init\n");
	transfer_sem_init();
	//original_key_init();

	xfer_init_flag++;
	errid = init_pending_tasks();

	errid = transfer_ctrl_init();

	/* xfer log init */
	xfer_log_init();

/* move following module inits into transfer_mgr_module_register function
	transfer_sem_lock();

#ifdef TRANSFER_MANAGER_TASK_HTTP
	errid = http_mgr_init();
#endif

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
if(xfer_thunder_enabled)
    errid = thunder_mgr_init(lic);
#endif

	transfer_sem_unlock();
*/
	xfer_complete_cb = transfer_mgr_complete_callback;

	return errid;
}

xfer_errid_t 
transfer_mgr_init()
{
	return transfer_mgr_init_ex(NULL);
}

void transfer_mgr_fini()
{
	debugf(XFER_LOG_DETAIL, "xfer_fini\n");
	if (!xfer_init_flag)
		return;

	save_pending_tasks();
	clear_pending_tasks();
	
	transfer_sem_lock();
	xfer_init_flag--;
	xfer_load_flag = 0;
	s_xfer_register_flag = 0;
	transfer_sem_unlock();

	transfer_ctrl_fini();

#ifdef TRANSFER_MANAGER_TASK_BT
	spbt_fini();
#endif

#ifdef TRANSFER_MANAGER_TASK_HTTP
	http_mgr_fini();
#endif

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
if(xfer_thunder_enabled)
	thunder_mgr_fini();
#endif

	xfer_log_uninit();
	transfer_sem_close();
	debugf(XFER_LOG_DETAIL, "xfer_fini OK\n");
}

xfer_errid_t transfer_mgr_module_register(transfer_module_type_e type, void *param)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	debugf(XFER_LOG_DETAIL, "xfer_module_register: type=%d\n", type);
	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;

#ifdef TRANSFER_MANAGER_TASK_HTTP
	if (XFER_MODULE_HTTP & type)
	{
		errid = http_mgr_init();
		if(errid == XFER_ERROR_OK)
			s_xfer_register_flag |= XFER_MODULE_HTTP;
		debugf(XFER_LOG_DETAIL, "xfer_module http registered: flag=%d\n", s_xfer_register_flag);
	}
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if (XFER_MODULE_THUNDER & type)
	{
		if(xfer_thunder_enabled)
		errid = thunder_mgr_init((char *)param);
		if(errid == XFER_ERROR_OK)
			s_xfer_register_flag |= XFER_MODULE_THUNDER;
		debugf(XFER_LOG_DETAIL, "xfer_module thunder registered: flag=%d\n", s_xfer_register_flag);
	}
#endif

	return errid;
}

xfer_errid_t transfer_mgr_module_unregister(transfer_module_type_e type)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;

#ifdef TRANSFER_MANAGER_TASK_HTTP
	if (XFER_MODULE_HTTP & type)
	{
		http_mgr_fini();
		s_xfer_register_flag &= ~XFER_MODULE_HTTP;
		debugf(XFER_LOG_DETAIL, "xfer_module http unregistered: flag=%d\n", s_xfer_register_flag);
	}
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if (XFER_MODULE_THUNDER & type)
	{
		if(xfer_thunder_enabled)
		thunder_mgr_fini();
		s_xfer_register_flag &= ~XFER_MODULE_THUNDER;
		debugf(XFER_LOG_DETAIL, "xfer_module thunder unregistered: flag=%d\n", s_xfer_register_flag);
	}
#endif

	return errid;
}

static xfer_errid_t get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo)
{
	xfer_errid_t errid = XFER_ERROR_NULL;
	int type = get_tansfer_type(taskid);

	transfer_sem_lock();
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_get_task_info(taskid, taskinfo);
		if (errid == XFER_ERROR_OK)
		{
			char* alias = (char*)xfer_malloc(127);
			if (alias)
			{
				memset(alias, 0, 127);
				sbpt_get_task_alias(taskid, alias, 126);
				if (strlen(alias) > 0)
				{
					xfer_free((*taskinfo)->task_name);
					(*taskinfo)->task_name = alias;
				}
				else
					xfer_free(alias);
			}
		}
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		errid = http_mgr_get_task_info(taskid, taskinfo);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
        errid = thunder_mgr_get_task_info(taskid, taskinfo);
#endif
	}
	transfer_sem_unlock();

	if (errid == XFER_ERROR_OK && taskinfo && (*taskinfo)
		 && (*taskinfo)->task_stat == DLST_ERROR && (*taskinfo)->error_id == 130)
	{
		push_pending_task(taskid);
	}

	return errid;

}

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
static xfer_errid_t
resume_tasks()
{
	debugf(XFER_LOG_DETAIL, "resume_task\n");
	transfer_task_list_t*	tasklist = NULL;
	transfer_task_info_t*	taskinfo = NULL;
	xfer_errid_t err = XFER_ERROR_NULL;
	int i = 0;
	
	if( (err = transfer_mgr_get_task_list(&tasklist)) != XFER_ERROR_OK)
		return 1;
	
	for(i = 0; i < tasklist->task_count; i++)
	{
		if ((err = get_task_info(tasklist->task_ids[i], &taskinfo)) != XFER_ERROR_OK)
		{
			transfer_mgr_release_task_list(tasklist);
			return 1;
		}

		if (taskinfo->task_stat == DLST_ERROR || taskinfo->task_stat == DLST_PENDING)
			push_pending_task(taskinfo->task_id);
		
		transfer_mgr_release_task_info(taskinfo);
	}
	transfer_mgr_release_task_list(tasklist);
	
	return 0;
}
#endif

xfer_errid_t 
transfer_mgr_unload(const char* path)
{
	if (!xfer_load_flag)
		return XFER_ERROR_UNLOADED;

#if 0
	if (xfer_mgr_timer != AVTIMER_INVALID)
	{
		AVTimerDelete(xfer_mgr_timer);
		xfer_mgr_timer = AVTIMER_INVALID;
	}
#endif

	save_pending_tasks();
	empty_pending_tasks();

#ifdef TRANSFER_MANAGER_TASK_BT
	spbt_fini();
#endif

#ifdef TRANSFER_MANAGER_TASK_HTTP
	if(s_xfer_register_flag & XFER_MODULE_HTTP)
		http_unload_tasks();
#endif

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
if(xfer_thunder_enabled)
	if(s_xfer_register_flag & XFER_MODULE_THUNDER)
		thunder_unload_tasks();
#endif

	transfer_sem_lock();
	xfer_load_flag = 0;
	transfer_sem_unlock();

	return XFER_ERROR_OK;
}

xfer_errid_t
transfer_mgr_load(const char* path)
{
	char i_path[DL_MAX_PATH] = {0};

	xfer_errid_t errid = XFER_ERROR_OK;
	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;

	if (NULL == path)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if (xfer_load_flag)
		transfer_mgr_unload(NULL);

	if (path[0] != '/' && xfer_path_prefix[0])
	{
		strcpy(i_path, xfer_path_prefix);
		strcat(i_path, path);
	}
	else
		strcpy(i_path, path);
	debugf(XFER_LOG_MAIN, "transfer_mgr_load(%s)\n", i_path);

	make_dir_recursive(i_path);
	transfer_mgr_set_tasks_info_path(i_path);
	load_pending_tasks();

	transfer_sem_lock();
	xfer_load_flag = 1;
	
#ifdef TRANSFER_MANAGER_TASK_BT
	spbt_set_savepath(path);
	errid = spbt_init();
	if (errid != XFER_ERROR_OK)
	{
		return errid;
	}
	spbt_setopt(DLOPT_RATELIMIT_DOWNLOAD, xfer_bt_max_download);
	spbt_setopt(DLOPT_RATELIMIT_DOWNLOAD, xfer_bt_max_upload);
#endif

#ifdef TRANSFER_MANAGER_TASK_HTTP
	if(s_xfer_register_flag & XFER_MODULE_HTTP)
	{
		//http_load_tasks();
		xfer_http_load_flag = 1;
	}
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
if(xfer_thunder_enabled)
{
	if(s_xfer_register_flag & XFER_MODULE_THUNDER)
	{
		thunder_mgr_setopt(DLOPT_RATELIMIT_MAX_TASKS, DL_THUNDER_DOWNLOADING_MAX);
		errid = thunder_load_tasks();
		xfer_thunder_load_flag = 1;
	}
}
#endif
	transfer_sem_unlock();

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	resume_tasks();
#endif

	return errid;
}

xfer_errid_t
transfer_mgr_save_tasks()
{
	xfer_errid_t errid = XFER_ERROR_OK;
	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;

	if (!xfer_load_flag)
	{
		//debugf(XFER_LOG_DETAIL, "transfer_mgr_save_tasks() return, unloaded\n");
		return XFER_ERROR_UNLOADED;
	}

	transfer_sem_lock();
#ifdef TRANSFER_MANAGER_TASK_HTTP
	if(s_xfer_register_flag & XFER_MODULE_HTTP)
		errid = http_save_tasks();
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if(xfer_thunder_enabled)
	{
		if(s_xfer_register_flag & XFER_MODULE_THUNDER)
		{
			errid = thunder_save_tasks();
		}
	}
#endif
	transfer_sem_unlock();

	return XFER_ERROR_OK;
}

xfer_errid_t transfer_mgr_getopt(transfer_options_e opt, transfer_type_e type, int * result)
{	
	xfer_errid_t errid = XFER_ERROR_NULL;

	transfer_sem_lock();
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_getopt(opt, result);
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		errid = http_mgr_getopt(opt, result);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
        errid = thunder_mgr_getopt(opt, result);
#endif
	}

	transfer_sem_unlock();
	
	return errid;
}

xfer_errid_t transfer_mgr_setopt(transfer_options_e opt, transfer_type_e type, int value)
{
	xfer_errid_t errid = XFER_ERROR_NULL;

	transfer_sem_lock();
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		if (xfer_load_flag)
			errid = spbt_setopt(opt, value);
		else
		{
			switch (opt)
			{
				case DLOPT_RATELIMIT_DOWNLOAD:
					xfer_bt_max_download = value;
					break;
				case DLOPT_RATELIMIT_UPLOAD:
					xfer_bt_max_upload = value;
					break;
				default:
					break;
			}
		}
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		errid = http_mgr_setopt(opt, value);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
        errid = thunder_mgr_setopt(opt, value);
#endif
	}

	transfer_sem_unlock();

	return errid;
}

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
static char * check_file_name( char * c_file_name )
{
	int i, j;

	if( !c_file_name )
		return NULL ;
	const char c_invalid[] = "<>\\/|:*?\"";
		
	for(i = 0 ; i < strlen( c_invalid ) ; i ++ )
	{
		for(j = 0 ; j < strlen( c_file_name ) ; j ++ )
		{
			if( c_invalid[i] == c_file_name[j] )
			{
				c_file_name[j] = '_' ;
			}
		}
	}
	return c_file_name ;
}
#endif

static xfer_errid_t transfer_mgr_task_add_ex(const char* 		 	source_path, 
								  	const char* 		 	saved_path, 
									const char* 			buff,
								  	int						buff_len,
								  	transfer_type_e 	 	type,
								 	int						mode,
								  	const char* 			post_buff,
								  	transfer_complete_cb	complete_callback,
								  	transfer_taskid_t* 		taskid)
{	
	xfer_errid_t errid = XFER_ERROR_NULL;
	transfer_task_list_t* tasklist = NULL;

	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;

	if (mode&XFER_SHOW || type != XFER_TASK_HTTP)
	{
		if ( transfer_mgr_get_task_list(&tasklist) == XFER_ERROR_OK 
			&& tasklist->task_count >= xfer_max_tasks)
		{
			transfer_mgr_release_task_list(tasklist);
			return XFER_ERROR_BEYOND_MAX_TASKS;
		}
		transfer_mgr_release_task_list(tasklist);
	}
	
	transfer_sem_lock();
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_task_add(source_path, saved_path, taskid);
		if (errid == XFER_ERROR_OK && buff != NULL)
		{
			char* alias = (char*)xfer_malloc(buff_len+1);
			if (alias)
			{
				strncpy(alias, buff, buff_len);
				alias[buff_len] = 0;
				sbpt_set_task_alias(*taskid, alias);
				xfer_free(alias);
			}
		}
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		errid = http_mgr_task_add(source_path, saved_path, type, mode, post_buff, complete_callback, taskid);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLTCID || XFER_TASK_XLURL)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
		char file_name[256] = {0};
		char i_saved_path[256] = {0};
		char *tmp = NULL;

		int len = strlen(saved_path);
		strncpy(i_saved_path, saved_path, len);
		tmp = strrchr(i_saved_path, '/');
		if(tmp == NULL)
			tmp = strrchr(i_saved_path, '\\');
		if(tmp == NULL)
		{
			debugf(XFER_LOG_DETAIL, "wrong saved path: %s\n", saved_path);
			*taskid = TASKID_INVALID;
			return XFER_ERROR_HTTP_TASK_EXIST;
		}
		else
		{
			*(tmp+1) = 0;
		}

		i_saved_path[len] = '\0';

		tmp = strrchr(saved_path, '/');
		if(tmp == NULL)
			tmp = strrchr(saved_path, '\\');
		if((tmp != NULL) && (strlen(tmp) > 1))
		{
			strcpy(file_name, tmp+1);
		}
		else
		{
			tmp = strrchr(source_path, '/');
			if(tmp == NULL)
				tmp = strrchr(source_path, '\\');
			if((tmp != NULL) && (strlen(tmp) > 1))
			{
				strcpy(file_name, tmp+1);
			}
		}
		check_file_name(file_name);

		debugf(XFER_LOG_DETAIL, "i_saved_path=%s, file_name=%s\n", i_saved_path, file_name);

		transfer_sem_unlock();
		if (thunder_mgr_task_exist(source_path, i_saved_path, file_name))
		{
			*taskid = TASKID_INVALID;
			return XFER_ERROR_HTTP_TASK_EXIST;
		}
		if (thunder_mgr_task_downloaded(i_saved_path, file_name))
		{
			*taskid = TASKID_INVALID;
			return XFER_ERROR_THUNDER_DOWNLOADED;
		}

		if (resource_limited())
		{
			thunder_mgr_add_pending(source_path, i_saved_path, type, file_name, taskid);
			errid = push_pending_task(*taskid);
		}
		else
		{
			errid = thunder_mgr_task_add(source_path, i_saved_path, type, file_name, taskid);
			if(4103 == errid)
			{
				thunder_mgr_add_pending(source_path, i_saved_path, type, file_name, taskid);
				errid = push_pending_task(*taskid);
			}
		}
		transfer_sem_lock();
#endif
	}
	transfer_sem_unlock();

	return errid;
}

xfer_errid_t transfer_mgr_task_add(const char* 		 	source_path, 
								  const char* 		 	saved_path, 
								  transfer_type_e 	 	type,
								  int					mode,
								  const char* 			post_buff,
								  transfer_complete_cb	complete_callback,
								  transfer_taskid_t* 	taskid)
{	
	int len = 0;
	char i_src_path[DL_MAX_PATH] = {0};
	char i_saved_path[DL_MAX_PATH] = {0};

	debugf(XFER_LOG_DETAIL, "xfer_mgr_task add\n");
	if (!xfer_init_flag)
	{
		debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, xfer not inited\n");
		return XFER_ERROR_HTTP_UNINITIAL;
	}

	switch(type)
	{
		case XFER_TASK_HTTP:
			if(!(s_xfer_register_flag & XFER_MODULE_HTTP))
			{
				debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, http module not registered\n");
				return XFER_ERROR_HTTP_UNINITIAL;
			}
			break;
		case XFER_TASK_XLURL:
		case XFER_TASK_XLBT:
		case XFER_TASK_XLTCID:
		case XFER_TASK_XLGCID:
			if(!(s_xfer_register_flag & XFER_MODULE_THUNDER))
			{
				debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, thunder module not registered\n");
				return XFER_ERROR_HTTP_UNINITIAL;
			}
			if (!xfer_thunder_load_flag)
			{
				debugf(XFER_LOG_DETAIL, "NOTE: xfer download path not set, use default path=%s\n", xfer_load_path);
				transfer_mgr_load(xfer_load_path);
			}
			break;

		default:
			debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, unknow download type:%d\n", type);
			return XFER_ERROR_HTTP_UNINITIAL;
	}

	if(source_path == NULL)
		return XFER_ERROR_NULL;
	else
	{
		len = strlen(source_path);
		strncpy(i_src_path, source_path, len);
		i_src_path[len] = '\0';

		if(saved_path != NULL)
		{
			if ((saved_path[0] != '/') && xfer_path_prefix[0])
			{
				strcpy(i_saved_path, xfer_path_prefix);
				strcat(i_saved_path, saved_path);
				i_saved_path[DL_MAX_PATH - 1] = '\0';
			
			}
			else 
			{
				len = strlen(saved_path);
				strncpy(i_saved_path, saved_path, len);
				i_saved_path[len] = '\0';
			}
		}
		debugf(XFER_LOG_MAIN, "(src_path=%s, saved_path=%s)\n", i_src_path, i_saved_path);
	}

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	/* it is thunder download task, set disk */
	thunder_mgr_set_kankan_disk(1);
#endif
	if(complete_callback)
		return transfer_mgr_task_add_ex(i_src_path, i_saved_path, NULL, 0, 
			type, mode, post_buff, complete_callback, taskid);
	else
		return transfer_mgr_task_add_ex(i_src_path, i_saved_path, NULL, 0, 
			type, mode, post_buff, xfer_complete_cb, taskid);
}

xfer_errid_t transfer_mgr_task_add_vod( const char* 		cid, 
									  	const char*			gcid,
									  	const char* 		saved_path, 
									  	u64_t				task_len,
									  	transfer_type_e 	type,
									  	const char* 		file_name,
									  	transfer_taskid_t* 	taskid)
{
	int len = 0;
	char i_src_path[DL_MAX_PATH] = {0};
	char i_saved_path[DL_MAX_PATH] = {0};

	debugf(XFER_LOG_DETAIL, "xfer_mgr_task_vod() start\n");
	if (!xfer_init_flag)
	{
		debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, xfer not inited\n");
		return XFER_ERROR_HTTP_UNINITIAL;
	}

	switch(type)
	{
		case XFER_TASK_XLGCID:
			if(!(s_xfer_register_flag & XFER_MODULE_THUNDER))
			{
				debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, thunder module not registered\n");
				return XFER_ERROR_HTTP_UNINITIAL;
			}
			if (!xfer_thunder_load_flag)
			{
				debugf(XFER_LOG_DETAIL, "NOTE: xfer download path not set, use default path=%s\n", xfer_load_path);
				transfer_mgr_load(xfer_load_path);
			}
			break;

		default:
			debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, error download type:%d\n", type);
			return XFER_ERROR_HTTP_UNINITIAL;
	}

	if(cid == NULL)
		return XFER_ERROR_NULL;
	else
	{
		len = strlen(cid);
		strncpy(i_src_path, cid, len);
		i_src_path[len] = '\0';

		if(saved_path != NULL)
		{
			len = strlen(saved_path);
			strncpy(i_saved_path, saved_path, len);
			i_saved_path[len] = '\0';
		}
		debugf(XFER_LOG_DETAIL, "(src_path=%s, saved_path=%s)\n", i_src_path, i_saved_path);
	}

	return transfer_mgr_task_add_gcid(i_src_path, gcid, i_saved_path, task_len, type, file_name, taskid);
}

xfer_errid_t transfer_mgr_task_add_gcid( const char* 		source_path, 
									  	const char*			gcid,
									  	const char* 		saved_path, 
									  	u64_t				task_len,
									  	transfer_type_e 	type,
									  	const char* 		file_name,
									  	transfer_taskid_t* 	taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;

	debugf(XFER_LOG_DETAIL, "xfer_mgr_gcid_task() start\n");
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if(gcid)
	{
		debugf(XFER_LOG_DETAIL, "thunder_mgr_task_add_ex() start\n");
		errid = thunder_mgr_task_add_ex(source_path, task_len, gcid, saved_path, type, file_name, taskid);
		/* it is thunder kankan task, set no disk */
		thunder_mgr_set_kankan_disk(0);
		if (XFER_ERROR_OK == errid)
		{
			thunder_mgr_set_length(*taskid, task_len);
		}
		return errid;
	}
	else
	{
		/* it is thunder download task, set disk */
		thunder_mgr_set_kankan_disk(1);
	}

	transfer_sem_lock();
	if (thunder_mgr_task_exist(source_path, saved_path, file_name))
	{
		*taskid = TASKID_INVALID;
        	transfer_sem_unlock();
		return XFER_ERROR_HTTP_TASK_EXIST;
	}
	if (thunder_mgr_task_downloaded(saved_path, file_name))
	{
		*taskid = TASKID_INVALID;
        	transfer_sem_unlock();
		return XFER_ERROR_THUNDER_DOWNLOADED;
	}

	transfer_sem_unlock();
    
	if (resource_limited())
	{
		thunder_mgr_add_pending_ex(source_path, task_len, saved_path, type, file_name, taskid);
		errid = push_pending_task(*taskid);
	}
	else
		errid = thunder_mgr_task_add_ex(source_path, task_len, gcid, saved_path, type, file_name, taskid);

#endif

	debugf(XFER_LOG_DETAIL, "xfer_mgr_gcid_task errid=%d\n", errid);
	return errid;

}


xfer_errid_t 
transfer_mgr_task_append_http_header(transfer_taskid_t taskid, char *header)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	int len = 0;
	char *buff = NULL;
	int type = get_tansfer_type(taskid);

	if (!xfer_init_flag)
	{
		debugf(XFER_LOG_DETAIL, "add http header error, xfer not inited\n");
		return XFER_ERROR_HTTP_UNINITIAL;
	}

	if(!(s_xfer_register_flag & XFER_MODULE_HTTP))
	{
		debugf(XFER_LOG_DETAIL, "add http header error, http module not registered\n");
		return XFER_ERROR_HTTP_UNINITIAL;
	}

#ifdef TRANSFER_MANAGER_TASK_HTTP
	if(type != XFER_TASK_HTTP)
	{
		debugf(XFER_LOG_DETAIL, "add http header error, task type error\n");
		return XFER_ERROR_HTTP_TASKID_ERR;
	}

	if(header == NULL)
	{
		debugf(XFER_LOG_DETAIL, "add http header error, param NULL\n");
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
	}

	len = strlen(header);
	buff = (char*)xfer_malloc(len + 1);
	if(buff == NULL)
	{
		debugf(XFER_LOG_DETAIL, "add http header error, malloc ERROR\n");
		return XFER_ERROR_MALLOC_FAILURE;
	}
	memcpy(buff, header, len);
	buff[len] = 0;

	errid = http_mgr_task_append_header(taskid, buff);
	xfer_free(buff);
#endif

	return errid;
}

transfer_taskid_t pop_pending_task()
{
	transfer_taskid_t taskid = TASKID_INVALID;
	
	transfer_sem_lock();
	if (pending_tasks && pending_tasks->task_count > 0)
	{
		//pending_tasks->task_count--;
		taskid = pending_tasks->task_ids[pending_tasks->task_count-1];
		//save_pending = 1;
	}
	transfer_sem_unlock();

	return taskid;
}

int remove_pending_task(transfer_taskid_t taskid)
{
	int exist = 0;
	int i = 0;
	
	transfer_sem_lock();
	if (pending_tasks == NULL || pending_tasks->task_count == 0)
	{
		transfer_sem_unlock();
		return XFER_ERROR_OK;
	}
	
	for (i = 0; i < pending_tasks->task_count; i++)
	{
		if (pending_tasks->task_ids[i] == taskid)
		{
			exist = 1;
			pending_tasks->task_count--;//printf("taskid %d task count %d. RM\n", taskid, pending_tasks->task_count);
		}
		if (exist && (i < pending_tasks->task_count))
		{
			pending_tasks->task_ids[i] = pending_tasks->task_ids[i+1];
		}
	}
	transfer_sem_unlock();
	if (exist)
		save_pending_tasks();

	return XFER_ERROR_OK;
}

void transfer_mgr_task_close(transfer_taskid_t taskid)
{
	int type = 0;
	if (!xfer_init_flag)
	{
		debugf(XFER_LOG_DETAIL, "xfer_mgr_task close error, xfer not inited\n");
		return;
	}

	//transfer_taskid_t pending = TASKID_INVALID;
	type = get_tansfer_type(taskid);

	transfer_sem_lock();
	if (type == XFER_TASK_BT)	
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		spbt_task_remove(taskid, 0);
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		http_mgr_task_close(taskid);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLTCID || XFER_TASK_XLURL)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
		thunder_mgr_task_close(taskid);
#endif
	}

	transfer_sem_unlock();

	remove_pending_task(taskid);
}

void transfer_mgr_task_close_ex(transfer_module_type_e module, char *disk)
{
	if (!xfer_init_flag)
	{
		debugf(XFER_LOG_DETAIL, "xfer_mgr_task close error, xfer not inited\n");
		return;
	}

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	transfer_task_list_t*	tasklist = NULL;
	transfer_task_info_t*	taskinfo = NULL;
	xfer_errid_t err = XFER_ERROR_NULL;
	int type  = 0;
	int i = 0;
	
	if( (err = transfer_mgr_get_task_list(&tasklist)) != XFER_ERROR_OK)
	{
		debugf(XFER_LOG_DETAIL, "close_all_task, get list error\n");
		return;
	}
	
	for(i = 0; i < tasklist->task_count; i++)
	{
		if ((err = get_task_info(tasklist->task_ids[i], &taskinfo)) != XFER_ERROR_OK)
		{
			transfer_mgr_release_task_list(tasklist);
			debugf(XFER_LOG_DETAIL, "close_all_task, get task info error\n");
			return;
		}

		/* TODO: deal with the param disk */
		type = get_tansfer_type(taskinfo->task_id);
		if ((type == XFER_TASK_XLURL) || (type == XFER_TASK_XLTCID) || (type == XFER_TASK_XLBT) || (type == XFER_TASK_XLGCID)) 
			thunder_mgr_task_close(taskinfo->task_id);

		transfer_mgr_release_task_info(taskinfo);
	}

	transfer_mgr_release_task_list(tasklist);
#endif
}

/*
void transfer_mgr_task_cancel(transfer_taskid_t taskid)
{
	int type = get_tansfer_type(taskid);
	if (!xfer_init_flag)
		return;

	transfer_sem_lock();
	if (type == XFER_TASK_BT)	
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		spbt_task_remove(taskid, 1);
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		http_mgr_task_cancel(taskid);
#endif
	}
	else if(type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID || type == XFER_TASK_XLBT)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
		thunder_mgr_task_cancel(taskid);
#endif
	}
	transfer_sem_unlock();

	remove_pending_task(taskid);
}

void 
transfer_mgr_task_cancel_ex(transfer_taskid_t taskid, transfer_cancel_cb cancel_callback)
{
	xfer_cancel_cxt = taskid;
	xfer_cancel_cb = cancel_callback;
	transfer_mgr_task_cancel(taskid);
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if (get_tansfer_type(taskid) == XFER_TASK_XLURL)
		cancel_callback(taskid);
#endif
}
*/

xfer_errid_t
transfer_mgr_set_complete_callback(transfer_complete_cb complete_callback)
{
	xfer_complete_cb = complete_callback;

	return XFER_ERROR_OK;
}

static int call_status_callback(int stat)
{
	static int status = -1;
	
	if (xfer_status_cb && (status == -1 || status != stat))
	{
		status = stat;
		xfer_status_cb(status);
	}

	return XFER_ERROR_OK;
}

int resource_limited()
{
	transfer_task_list_t*	tasklist = NULL;
	transfer_task_info_t*	taskinfo = NULL;
	xfer_errid_t err = XFER_ERROR_NULL;
	int downloading = 0;
	int i = 0;
	
	if( (err = transfer_mgr_get_task_list(&tasklist)) != XFER_ERROR_OK)
	{
		debugf(XFER_LOG_DETAIL, "resource_limited, get list error\n");
		return 1;
	}
	
	//debugf(XFER_LOG_DETAIL, "resource_limited, get task list: task_count=%d\n", tasklist->task_count);
	for(i = 0; i < tasklist->task_count; i++)
	{
		if ((err = get_task_info(tasklist->task_ids[i], &taskinfo)) != XFER_ERROR_OK)
		{
			transfer_mgr_release_task_list(tasklist);
			call_status_callback(downloading>0);
			debugf(XFER_LOG_DETAIL, "resource_limited, get task info error\n");
			return 1;
		}
		//debugf(XFER_LOG_DETAIL, "resource_limited, get task info:taskid=%d, state:%d\n", taskinfo->task_id, taskinfo->task_stat);

		if (taskinfo->task_stat == DLST_DOWNLOADING 
				|| taskinfo->task_stat == DLST_CREATING)
			downloading++;
		transfer_mgr_release_task_info(taskinfo);
		
		if (downloading == DL_THUNDER_DOWNLOADING_MAX)
		{
			transfer_mgr_release_task_list(tasklist);
			call_status_callback(downloading>0);
			//debugf(XFER_LOG_DETAIL, "resource_limited, downloading task max=%d\n", downloading);
			return 1;
		}
	}
	transfer_mgr_release_task_list(tasklist);
	
	call_status_callback(downloading>0);
	//debugf(XFER_LOG_DETAIL, "resource_limited return 0, downloading task max=%d\n", downloading);
	return 0;
}

int push_pending_task(transfer_taskid_t taskid)
{
	int save_pending = 0;
	int i = 0;
	
	transfer_sem_lock();
	if (pending_tasks->task_count == 0)
	{
		pending_tasks->capacity = 10*sizeof(transfer_taskid_t);
		pending_tasks->task_ids = (transfer_taskid_t*)xfer_malloc(pending_tasks->capacity);
		if (pending_tasks->task_ids == NULL)
		{
			pending_tasks->capacity = 0;
			transfer_sem_unlock();
			return XFER_ERROR_HTTP_MALLOC_FAILURE;
		}
		pending_tasks->task_ids[0] = taskid;
		pending_tasks->task_count ++;
		debugf(XFER_LOG_DETAIL, "pending taskid %d task count %d.\n", taskid, pending_tasks->task_count);
		save_pending = 1;
	}
	else
	{
		transfer_taskid_t* taskids = NULL;
		for (i = 0; i < pending_tasks->task_count; i++)
		{
			if (pending_tasks->task_ids[i] == taskid)
			{
				transfer_sem_unlock();
				return XFER_ERROR_OK;
			}
		}

		if (pending_tasks->capacity < (pending_tasks->task_count + 1)*sizeof(transfer_taskid_t))
		{
			pending_tasks->capacity *= 2;
			taskids = (transfer_taskid_t*)xfer_malloc(pending_tasks->capacity);
			if (taskids == NULL)
			{
				pending_tasks->capacity /= 2;
				transfer_sem_unlock();
				return XFER_ERROR_HTTP_MALLOC_FAILURE;
			}
			memcpy(taskids+1, pending_tasks->task_ids, sizeof(transfer_taskid_t) * pending_tasks->task_count);
			xfer_free(pending_tasks->task_ids);
			pending_tasks->task_ids = taskids;
		}
		else 
		{
			memmove(pending_tasks->task_ids+1, pending_tasks->task_ids, sizeof(transfer_taskid_t) * pending_tasks->task_count);
		}
		pending_tasks->task_ids[0] = taskid;
		pending_tasks->task_count ++;
		debugf(XFER_LOG_DETAIL, "pending taskid %d task count %d.\n", taskid, pending_tasks->task_count);
		save_pending = 1;
	}
	transfer_sem_unlock();
	if (save_pending)
		save_pending_tasks();
	
	return XFER_ERROR_OK;
}

xfer_errid_t transfer_mgr_task_start(transfer_taskid_t taskid)
{
	xfer_errid_t errid = XFER_ERROR_NULL;
	int type = get_tansfer_type(taskid);

	debugf(XFER_LOG_DETAIL, "xfer_mgr_task start: id=%d\n", taskid);
	if (!xfer_init_flag)
		return XFER_ERROR_UNLOADED;

	if ((type == XFER_TASK_BT || 
		type == XFER_TASK_XLURL || type == XFER_TASK_XLBT || type == XFER_TASK_XLURL ||
		(type == XFER_TASK_HTTP && get_transfer_mode(taskid)&XFER_SHOW)) 
		&& resource_limited())
	{
		push_pending_task(taskid);
		return XFER_ERROR_OK;
	}
	
	transfer_sem_lock();
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_task_start(taskid);
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		errid = http_mgr_task_start(taskid);
#endif
	}
	else if (type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID || type == XFER_TASK_XLBT)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
		errid = thunder_mgr_task_start(taskid);
#endif
	}
	transfer_sem_unlock();

	return errid;
}

xfer_errid_t transfer_mgr_task_pause(transfer_taskid_t taskid)
{
	xfer_errid_t errid = XFER_ERROR_NULL;
	//transfer_taskid_t pending = TASKID_INVALID;
	int type = get_tansfer_type(taskid);

	debugf(XFER_LOG_DETAIL, "xfer_mgr_task pause: id=%d\n", taskid);
	if (!xfer_init_flag)
		return XFER_ERROR_UNLOADED;

	transfer_sem_lock();
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_task_pause(taskid);
#endif
	}
	else if(type == XFER_TASK_HTTP)
	{
#ifdef TRANSFER_MANAGER_TASK_HTTP
		errid = http_mgr_task_pause(taskid);
#endif
	}
	else if(type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID || type == XFER_TASK_XLBT)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
		errid = thunder_mgr_task_pause(taskid);
#endif
	}
	transfer_sem_unlock();

	if (errid == XFER_ERROR_OK)
	{
		remove_pending_task(taskid);
	
		/*if ((pending = pop_pending_task()) != TASKID_INVALID)
		{
			transfer_mgr_task_start(pending);
		}*/
	}

	debugf(XFER_LOG_DETAIL, "xfer_mgr_task pause: id=%d errid=%d\n", taskid, errid);
	return errid;
}


xfer_errid_t transfer_mgr_get_task_list(transfer_task_list_t** tasklist)
{
	char* curr = NULL;
	xfer_errid_t errid = XFER_ERROR_NULL;
	transfer_task_list_t* bt_list = NULL;
	transfer_task_list_t* http_list = NULL;
	transfer_task_list_t* thunder_list = NULL;

	///< get task list severally
	transfer_sem_lock();
	/*if (!xfer_load_flag)
	{
		transfer_sem_unlock();
		*tasklist = NULL;
		return XFER_ERROR_UNLOADED;
	}*/
#ifdef TRANSFER_MANAGER_TASK_BT
	errid = spbt_get_task_list(&bt_list);
#endif
#ifdef TRANSFER_MANAGER_TASK_HTTP
	if(s_xfer_register_flag & XFER_MODULE_HTTP)
		errid = http_mgr_get_task_list(&http_list);
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if(s_xfer_register_flag & XFER_MODULE_THUNDER)
		errid = thunder_mgr_get_task_list(&thunder_list);
#endif
	transfer_sem_unlock();

	///< merge all lists
	if ( bt_list == NULL && http_list == NULL && thunder_list == NULL)
	{		
		*tasklist = NULL;
		return errid;
	}
	*tasklist = (transfer_task_list_t*)xfer_malloc(sizeof(transfer_task_list_t));
	if (*tasklist == NULL)
	{
		if (bt_list) 
			transfer_mgr_release_task_list(bt_list);
		if (http_list) 
			transfer_mgr_release_task_list(http_list);
		if (thunder_list) 
			transfer_mgr_release_task_list(thunder_list);
		return XFER_ERROR_MALLOC_FAILURE;
	}
	
	memset(*tasklist, 0, sizeof(transfer_task_list_t));
	if (bt_list != NULL)
		(*tasklist)->task_count += bt_list->task_count;
	if (http_list != NULL)
		(*tasklist)->task_count += http_list->task_count;
	if (thunder_list != NULL)
		(*tasklist)->task_count += thunder_list->task_count;

	if ((*tasklist)->task_count == 0)
	{
		if (bt_list) 
			transfer_mgr_release_task_list(bt_list);
		if (http_list) 
			transfer_mgr_release_task_list(http_list);
		if (thunder_list) 
			transfer_mgr_release_task_list(thunder_list);
		
		return XFER_ERROR_OK;
	}

	(*tasklist)->task_ids = (transfer_taskid_t*)xfer_malloc(sizeof(transfer_taskid_t)*(*tasklist)->task_count);
	if ((*tasklist)->task_ids == NULL)
	{
		if (bt_list) 
			transfer_mgr_release_task_list(bt_list);
		if (http_list) 
			transfer_mgr_release_task_list(http_list);
		if (thunder_list) 
			transfer_mgr_release_task_list(thunder_list);
		transfer_mgr_release_task_list(*tasklist);
		*tasklist = NULL;

		return XFER_ERROR_MALLOC_FAILURE;
	}
	
	curr = (char*)(*tasklist)->task_ids;
	if (bt_list != NULL)
	{
		memcpy(curr, bt_list->task_ids, bt_list->task_count*sizeof(transfer_taskid_t));
		curr += bt_list->task_count*sizeof(transfer_taskid_t);
		transfer_mgr_release_task_list(bt_list);
	}
	if (http_list != NULL)
	{
		memcpy(curr, http_list->task_ids, http_list->task_count*sizeof(transfer_taskid_t));
		curr += http_list->task_count*sizeof(transfer_taskid_t);
		transfer_mgr_release_task_list(http_list);
	}
	if (thunder_list != NULL)
	{
		memcpy(curr, thunder_list->task_ids, thunder_list->task_count*sizeof(transfer_taskid_t));
		curr += thunder_list->task_count*sizeof(transfer_taskid_t);
		transfer_mgr_release_task_list(thunder_list);
	}

	return XFER_ERROR_OK;
}

void transfer_mgr_release_task_list(transfer_task_list_t* tasklist)
{
    if( tasklist) {
        if( tasklist->task_ids) {
            xfer_free(tasklist->task_ids);
        }
        xfer_free(tasklist);
    } 
}

xfer_errid_t transfer_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo)
{
	xfer_errid_t errid = XFER_ERROR_NULL;

	*taskinfo = NULL;
	if (!xfer_init_flag)
		return XFER_ERROR_UNLOADED;

	errid = get_task_info(taskid, taskinfo);

	if (errid == XFER_ERROR_OK && taskinfo && (*taskinfo) && is_pending_task(taskid))
	{
		if ((*taskinfo)->task_stat == DLST_DOWNLOADING)
			remove_pending_task(taskid);
		else
			(*taskinfo)->task_stat = DLST_PENDING;
	}

	return errid;
}

void transfer_mgr_release_task_info(transfer_task_info_t* taskinfo)
{
    if( taskinfo) {
        if( taskinfo->task_name) {
            xfer_free(taskinfo->task_name);
        }
        if( taskinfo->task_path) {
            xfer_free(taskinfo->task_path);
        }
	if( taskinfo->task_src_url) {
            xfer_free(taskinfo->task_src_url);
        }	
	if( taskinfo->bt_task_peers_info.pbt_task_peer_info) {
	 	xfer_free( taskinfo->bt_task_peers_info.pbt_task_peer_info);
	 }
        xfer_free(taskinfo);            
    }
}

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
static int dir_size_recursive(char* fullpath)
{
	int				size = 0;
#ifdef AVOS
	struct dirent*	dirp;
	DIR*			dp;
	int 			iswritepath = 0;
	char*			ptr;
	
	ptr = fullpath + strlen(fullpath);	/* point to end of fullpath */

	if ( (dp = opendir(fullpath)) == NULL)	/* can't read directory */
	{
		return 0;
	}

	while ( (dirp = readdir(dp)) != NULL) 
	{
		if (strcmp(dirp->d_name, ".") == 0  || strcmp(dirp->d_name, "..") == 0)
			continue;		/* ignore dot and dot-dot */

		if (dirp->d_lnamlen != 0)
		{
			strncpy(ptr, dirp->d_lname, dirp->d_lnamlen);
			*(ptr + dirp->d_lnamlen) = 0;
		}
		else
		{
			strcpy(ptr, dirp->d_name);	/* append name after slash */
		}

		if (!S_ISDIR(dirp->d_mode))
		{
			struct stat st;
			if ((stat(fullpath, &st)) != -1)
				size += st.st_size / (1024*1024);
		}
		else
		{
			char* tmp = fullpath + strlen(fullpath);	/* point to end of fullpath */
			*tmp++ = '/';
			*tmp = 0;
			size += dir_size_recursive(fullpath);		/* recursive */
		}
	}
	ptr[-1] = 0;	/* erase everything from slash onwards */
	closedir(dp);
#endif

	return (size);
}
#endif

int 
transfer_mgr_reserved_space()
{
	int	reserved = 0;
	int i = 0;
	
	if (!xfer_init_flag)
		return 0;

#ifdef TRANSFER_MANAGER_TASK_BT
	transfer_task_list_t* bt_list = NULL;
	xfer_errid_t errid = spbt_get_task_list(&bt_list);
	
	if (bt_list == NULL || bt_list->task_count == 0)
		return 0;
	
	for (i = 0; i < bt_list->task_count; i++)
	{
		transfer_task_info_t* taskinfo = NULL;
		errid = spbt_get_task_info(bt_list->task_ids[i], &taskinfo);
		if (errid == XFER_ERROR_OK)
		{
			int need = 0;
			char path[DL_MAX_PATH*4];
			
			strcpy(path, taskinfo->task_path);
			need = taskinfo->total_size / (1024*1024) - dir_size_recursive(path);
			if (need > 0)
				reserved += need;
		}
		spbt_release_task_info(taskinfo);
	}
	spbt_release_task_list(bt_list);
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	transfer_task_list_t* thunder_list = NULL;
	xfer_errid_t errid = thunder_mgr_get_task_list(&thunder_list);
	
	if (thunder_list == NULL || thunder_list->task_count == 0)
		return 0;
	
	for (i = 0; i < thunder_list->task_count; i++)
	{
		transfer_task_info_t* taskinfo = NULL;
		errid = thunder_mgr_get_task_info(thunder_list->task_ids[i], &taskinfo);
		if (errid == XFER_ERROR_OK && !(taskinfo->file_created))
			reserved += taskinfo->total_size / (1024*1024);

		transfer_mgr_release_task_info(taskinfo);
	}
	transfer_mgr_release_task_list(thunder_list);
#endif
	return reserved;
}

xfer_errid_t
transfer_mgr_set_tasks_info_path(const char* path)
{
	char file_name[DL_MAX_PATH] = "\0";
	
	if (path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	strcpy(xfer_load_path, path);
#ifdef TRANSFER_MANAGER_TASK_BT
	strcpy(file_name, path);
	strcat(file_name, "spbt.tasks");
	spbt_set_tasks_info_file(file_name);

    strcpy( file_name, path);
    strcat( file_name, "dht.info");
  spbt_set_dht_info_file(file_name);
	
#endif
#ifdef TRANSFER_MANAGER_TASK_HTTP
	strcpy(file_name, path);
	strcat(file_name, "http.aml");
	http_mgr_set_tasks_info_file(file_name);
#endif
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	strcpy(file_name, path);
	strcat(file_name, "thunder.aml");
	thunder_mgr_set_tasks_info_file(file_name);
#endif
	strcpy(xfer_pending_file, path);
	strcat(xfer_pending_file, "pend.aml");
	
	return XFER_ERROR_OK;
}

xfer_errid_t
transfer_mgr_create_aml_file(const char* path)
{
	char i_path[DL_MAX_PATH] = {0};

	xfer_errid_t errid = XFER_ERROR_OK;
	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;

	if (NULL == path)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if (path[0] != '/' && xfer_path_prefix[0])
	{
		strcpy(i_path, xfer_path_prefix);
		strcat(i_path, path);
	}
	else
		strcpy(i_path, path);
	debugf(XFER_LOG_MAIN, "transfer_mgr_load(%s)\n", i_path);

	make_dir_recursive(i_path);
	transfer_mgr_set_tasks_info_path(i_path);

	transfer_sem_lock();
	xfer_load_flag = 1;
	
#ifdef TRANSFER_MANAGER_TASK_HTTP
	if(s_xfer_register_flag & XFER_MODULE_HTTP)
		http_load_tasks();
#endif
	transfer_sem_unlock();

	return errid;
}

xfer_errid_t
transfer_mgr_set_temp_path(const char* path)
{
	if (path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
#ifdef TRANSFER_MANAGER_TASK_HTTP
	http_mgr_set_temp_path(path);
#endif
	return XFER_ERROR_OK;
}

xfer_errid_t
transfer_mgr_set_kankan_temp_path(const char* path)
{
	if (path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	thunder_mgr_set_tmp_path(path);
#endif
	return XFER_ERROR_OK;
}

xfer_errid_t
transfer_mgr_set_demo_path(const char* path)
{
	char file_name[DL_MAX_PATH+10];
	
	if (path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	strcpy(xfer_demo_path, path);
	if (path[strlen(path)-1] != '/')
		strcat(xfer_demo_path, "/");
	strcpy(file_name, xfer_demo_path);
	strcat(file_name, "demo.xml");

	/* parse demo xml file */
#ifdef HAS_OFFLINE_DEMO
	http_demo_xml_parse(file_name);
#endif
	
	return XFER_ERROR_OK;
}

xfer_errid_t
transfer_mgr_set_path_prefix(const char* path)
{
	if (path == NULL)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	strcpy(xfer_path_prefix, path);
	debugf(XFER_LOG_MAIN, "set_path_prefix:%s\n", path);
	return XFER_ERROR_OK;
}

xfer_errid_t 
transfer_mgr_get_seed_info(char *src, char *seed_path, torrent_seed_info_t** seed_info)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if(seed_info)
	{
		if(*seed_info)
			transfer_mgr_release_seed_info(seed_info);
		errid = thunder_mgr_get_seed_info(src, seed_path, seed_info);
	}
#endif

	return errid;
}

xfer_errid_t 
transfer_mgr_release_seed_info(torrent_seed_info_t**seed_info)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if(seed_info && (*seed_info))
		errid = thunder_mgr_release_seed_info(seed_info);
#endif

	return errid;	
}

xfer_errid_t
transfer_mgr_set_stat_callback(transfer_status_cb status_callback)
{
	xfer_status_cb = status_callback;

	return XFER_ERROR_OK;
}

void transfer_mgr_enable_cache_file(unsigned char benable)
{
#ifdef TRANSFER_MANAGER_TASK_HTTP
	//http_mgr_enable_cache_file(benable);
#endif
}

xfer_errid_t
transfer_mgr_set_customed_allocator(int fun_idx, void *fun_ptr)
{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	return thunder_mgr_set_customed_allocator(fun_idx, fun_ptr);
#else
	return XFER_ERROR_OK;
#endif
}

xfer_errid_t
transfer_mgr_set_license(char* lic)
{
	if (!xfer_init_flag)
		return XFER_ERROR_HTTP_UNINITIAL;
	
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	return thunder_mgr_set_license(lic);
#else
	return XFER_ERROR_OK;
#endif
}

transfer_taskid_t
transfer_mgr_get_thunder_taskid(transfer_taskid_t taskid)
{
	if (!xfer_init_flag)
		return TASKID_INVALID;
	
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	return thunder_mgr_get_thunder_taskid(taskid);
#else
	return TASKID_INVALID;
#endif
}

xfer_errid_t
transfer_mgr_set_task_src_url(transfer_taskid_t taskid, char* src_url)
{
	xfer_errid_t errid = XFER_ERROR_NULL;
	int type = get_tansfer_type(taskid);

	transfer_sem_lock();
	
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_set_task_src_url(taskid, src_url);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
        errid = thunder_mgr_set_task_src_url(taskid, src_url);
#endif
	}
	
	transfer_sem_unlock();
	return errid;
}

xfer_errid_t
transfer_mgr_get_task_src_url(transfer_taskid_t taskid, char* src_url)
{
	xfer_errid_t errid = XFER_ERROR_NULL;
	int type = get_tansfer_type(taskid);

	transfer_sem_lock();
	
	if (type == XFER_TASK_BT)
	{
#ifdef TRANSFER_MANAGER_TASK_BT
		errid = spbt_get_task_src_url(taskid, src_url);
#endif
	}
	else if(type == XFER_TASK_XLBT || type == XFER_TASK_XLURL || type == XFER_TASK_XLTCID)
	{
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
        errid = thunder_mgr_get_task_src_url(taskid, src_url);
#endif
	}
	
	transfer_sem_unlock();
	return errid;
}

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
static float 
transfer_get_disk_freespace(char *disk_or_path)
{
	int ret = 0;
	float freesize = 0 ;
	struct statfs sfs;
	
	if(disk_or_path == NULL)
		return 0;

	ret = statfs(disk_or_path, &sfs);
	if(ret != 0)
		return 0;

	freesize = sfs.f_bavail * sfs.f_bsize;

	return freesize;
}
#endif

xfer_errid_t
transfer_mgr_task_bt_downloaded(const char *torrent, const char *saved_path, u32_t *file_num, u32_t *file_index_array)
{
	xfer_errid_t errid = XFER_ERROR_OK;

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	int i = 0;
	int j = 0;
	char file[256];
	struct stat st;
	u32_t i_file_num;
	u32_t i_file_index_array[50];
	torrent_seed_info_t *seed_info = NULL;
	
	if (torrent == NULL || saved_path == NULL)
		return XFER_ERROR_OK;

	transfer_mgr_get_seed_info(NULL, (char *)torrent, &seed_info);

	i_file_num = 0;
	for(i = 0; i < *file_num; i++)
	{
		u32_t file_index = file_index_array[i];
		u32_t k = 0;
		for(k = 0; k < seed_info->file_num; k++)
			if(seed_info->torrent_file_list[k].index == file_index)
			{
				char * file_name = seed_info->torrent_file_list[k].file_name;
				snprintf(file, 255, "%s%s", saved_path, file_name);
				if (stat(file, &st) < 0)
				{ 
					i_file_index_array[i_file_num] = file_index_array[i];
					i_file_num++;
				}
			}
	}

	transfer_mgr_release_seed_info(&seed_info);
	
	if(i_file_num > 0)
	{
		*file_num = i_file_num;
		for(j = 0; j < i_file_num; j++)
		{
			file_index_array[j] = i_file_index_array[j];
		}
		errid = XFER_ERROR_OK;
	}
	else
		errid = -1;
#endif
	return errid;
}

xfer_errid_t
transfer_mgr_task_bt_diskspace_check(const char *torrent, const char *saved_path, int *file_size)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	float f_free_size = transfer_get_disk_freespace( (char *)saved_path );
	float f_reserved_size = (float)transfer_mgr_reserved_space() * 1024 * 1024;
	float f_seed_size = 0.0;
	torrent_seed_info_t *seed_info = NULL;

	transfer_mgr_get_seed_info(NULL, (char *)torrent, &seed_info);
	f_seed_size = 1.0 * seed_info->total_size;
	*file_size = (int)seed_info->total_size;
				
	if( f_seed_size + f_reserved_size + 10.0*1024*1024 > f_free_size )
	{
		errid = -4 ;
	}

	transfer_mgr_release_seed_info(&seed_info);
#endif

	return errid;
}

xfer_errid_t
transfer_mgr_task_add_bt_ex( const char* 		source_url,
										const char* 		seed_path, 
									  	const char* 		saved_path,  
									  	const char* 		file_name,
									  	u32_t*			file_index_array,
									  	u32_t				file_num,
									  	transfer_taskid_t* 	taskid)
{
	xfer_errid_t errid = XFER_ERROR_OK;

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	u32_t i_file_num = 0;
	u32_t i_file_index_array[50];
	i_file_num = file_num;
	int file_size = 0;
	int i = 0;
	memset(i_file_index_array, 0, sizeof(i_file_index_array));
	for(i = 0; i < i_file_num; i++)
	{	
		i_file_index_array[i] = file_index_array[i];
	}

	transfer_sem_lock();
	debugf(XFER_LOG_DETAIL, "bt exist()\n");
	if (thunder_mgr_task_bt_exist(seed_path, saved_path, file_name, file_num, file_index_array))
	{
		*taskid = TASKID_INVALID;
        	transfer_sem_unlock();
		return XFER_ERROR_HTTP_TASK_EXIST;
	}

	/*if (transfer_mgr_task_bt_downloaded(seed_path, saved_path, &i_file_num, i_file_index_array))
	{
		*taskid = TASKID_INVALID;
        	transfer_sem_unlock();
		return XFER_ERROR_THUNDER_DOWNLOADED;
	}
	
	debugf(XFER_LOG_DETAIL, "bt diskspace check()\n");
	if ((errid = transfer_mgr_task_bt_diskspace_check(seed_path, saved_path, &file_size)) != XFER_ERROR_OK)
	{
		*taskid = TASKID_INVALID;
        	transfer_sem_unlock();
		return errid;//DISK_SPACE_NOT_ENOUGH
	}
	*/

	transfer_sem_unlock();
    
	if (resource_limited())
	{
		thunder_mgr_add_bt_pending(source_url, seed_path, file_size, saved_path, XFER_TASK_XLBT, file_name, i_file_num, i_file_index_array, taskid);
		errid = push_pending_task(*taskid);
	}
	else
		errid = thunder_mgr_task_add_bt(source_url, seed_path, saved_path, XFER_TASK_XLBT, file_name, i_file_index_array, i_file_num, taskid);
	
#endif

	return errid;
}

xfer_errid_t
transfer_mgr_task_add_bt( const char* 		source_url,
						const char* 		seed_path, 
					  	const char* 		saved_path,  
					  	const char* 		file_name,
					  	u32_t*				file_index_array,
					  	u32_t				file_num,
					  	transfer_taskid_t* 	taskid)
{
	char i_path[DL_MAX_PATH] = {0};

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	debugf(XFER_LOG_DETAIL, "xfer_mgr_task add\n");
	if (!xfer_init_flag)
	{
		debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, xfer not inited\n");
		return XFER_ERROR_HTTP_UNINITIAL;
	}

	if(!(s_xfer_register_flag & XFER_MODULE_THUNDER))
	{
		debugf(XFER_LOG_DETAIL, "xfer_mgr_task add error, thunder module not registered\n");
		return XFER_ERROR_HTTP_UNINITIAL;
	}

	if(saved_path == NULL)
		return XFER_ERROR_THUNDER_BT_ERR;

	if (saved_path[0] != '/' && xfer_path_prefix[0])
	{
		strcpy(i_path, xfer_path_prefix);
		strcat(i_path, saved_path);
	}
	else
		strcpy(i_path, saved_path);
	//printf("transfer_mgr_load(%s)\n", i_path);

	if (!xfer_load_flag)
	{
		debugf(XFER_LOG_DETAIL, "NOTE: xfer download path not set, use default path=%s\n", xfer_load_path);
		transfer_mgr_load(xfer_load_path);
	}

#endif

	return transfer_mgr_task_add_bt_ex(source_url, seed_path, i_path, file_name, file_index_array, file_num, taskid);
}

xfer_errid_t 
transfer_mgr_get_bt_files_info(transfer_taskid_t taskid, torrent_seed_info_t* seed_info)
{
	xfer_errid_t errid = XFER_ERROR_OK;
	
#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	if(seed_info)
	{
		errid = thunder_mgr_get_bt_files_info(taskid, seed_info);
	}
	else
	{
		errid = -1;
	}
#endif

	return errid;
}

xfer_errid_t
transfer_mgr_get_kankan_buffer_percent(transfer_taskid_t taskid, int *percent)
{
	xfer_errid_t errid = XFER_ERROR_OK;

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	errid = thunder_mgr_get_kankan_buffer_percent(taskid, percent);
#endif
	return errid;
}

xfer_errid_t
transfer_mgr_read_kankan_file(transfer_taskid_t taskid, u64_t start_pos, u64_t len, char *buf, int block_time)
{
	xfer_errid_t errid = XFER_ERROR_OK;

#ifdef TRANSFER_MANAGER_TASK_XUNLEI
	errid = thunder_mgr_read_kankan_file(taskid, start_pos, len, buf, block_time);
#endif
	return errid;
}

xfer_errid_t
transfer_mgr_task_check_busy(transfer_taskid_t taskid)
{
#ifdef  TRANSFER_MANAGER_TASK_XUNLEI
	return thunder_mgr_check_busy(taskid);
#else
	return 0;
#endif
}
