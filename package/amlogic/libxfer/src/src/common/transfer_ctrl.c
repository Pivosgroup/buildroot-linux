/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Control Module
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _MSC_VER
#include <time.h>
#else
#include <sys/time.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#if defined(__WIN32__) || defined(_MSC_VER)
#include <Windows.h>
#else
#include <semaphore.h>
#endif
#include "transfer_manager.h"
#include "transfer_ctrl.h"
#include "xfer_debug.h"


#define XFER_FILE_LOOP_TIME 5

#define XFER_TASK_RETRY_MAX 10
static int s_xfer_ctrl_inited = 0;
static int s_xfer_finished_taskid = 0;
static int s_xfer_pending_taskid = 0;
static int s_xfer_finished_show_times = 0;
static int s_xfer_error_taskid = 0;
static int s_xfer_error_code = 0;
static int s_xfer_error_retry = 0;

#define XFER_TASK_IOFO_SHOW 1
static int s_task_count;
static int s_max_task_download_speed;
static int s_max_total_download_speed;
static int s_curr_total_download_speed;

extern char xfer_load_path[];

#ifdef __WIN32__
HANDLE s_xfer_ctrl_thread;
#else
static pthread_t s_xfer_ctrl_thread;
#endif

void *transfer_ctrl_run(void *data);

#ifdef __WIN32__
DWORD WINAPI transfer_ctrl_run_win32( LPVOID lpParam )
{
	transfer_ctrl_run(NULL);

	return TRUE;
}
#endif



extern xfer_errid_t
transfer_mgr_save_tasks();

int transfer_ctrl_init()
{
	if(s_xfer_ctrl_inited == 0)
	{
#ifdef __WIN32__
		if((s_xfer_ctrl_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)transfer_ctrl_run_win32, NULL, 0, NULL)) == NULL)
#else
		if(pthread_create(&(s_xfer_ctrl_thread),
			NULL,
			transfer_ctrl_run,
			NULL) != 0)
#endif
		{
			debugf(XFER_LOG_MAIN, "Create thread ERROR\n");
			return -1;
		}
		s_xfer_ctrl_inited = 1;
	}

	return 0;
}

void transfer_ctrl_fini()
{
	if(s_xfer_ctrl_inited == 1)
	{
#ifdef __WIN32__
		DWORD errcode = 0;
		TerminateThread(
				s_xfer_ctrl_thread,
				errcode
		);
		CloseHandle(s_xfer_ctrl_thread);
#else
		pthread_cancel(s_xfer_ctrl_thread);
#endif

		s_xfer_ctrl_inited = 0;
	}
}

static void transfer_task_info_show(transfer_taskid_t taskid)
{
	int percent = 0;
	char c_taskid[64]	 = {0};
	char c_state[64]	 = {0};
	char c_name[256]	 = {0};
	char c_percent[64] 	 = {0};
	char c_total_size[64] 	 = {0};
	char c_downloaded_size[64] 	 = {0};
	char c_rate[64] 	 = {0};
	char c_errno[64] 	 = {0};
	char c_time[16]		 = {0};
	char c_path[256]  = {0};
	char c_file[256] = {0};
	char c_srcurl[256] = {0};
	char c_src[256] = {0};
	transfer_task_info_t * taskinfo;

	if(transfer_mgr_get_task_info(taskid, &taskinfo) != XFER_ERROR_OK)
		return;

	snprintf(c_taskid , sizeof(c_taskid), "%d", taskinfo->task_id);
	
	switch (taskinfo->task_stat){
		case DLST_STOPPED:
		case DLST_NONE:
			snprintf(c_state , sizeof( c_state ), "%s", "pausing");
			break;
		case DLST_DOWNLOADING:
			snprintf(c_state , sizeof( c_state ), "%s", "running");
			break;
		case DLST_PENDING:
		case DLST_CREATING:
			snprintf(c_state , sizeof( c_state ), "%s", "waiting");
			break;
		case DLST_FINISHED:
			snprintf(c_state , sizeof( c_state ), "%s", "done");
			break;

		case DLST_ERROR:
		default:
			snprintf(c_state , sizeof( c_state ), "%s", "error");
			break;
	}
	
	snprintf(c_name , sizeof( c_name ), "%s", taskinfo->task_name);
	c_name[strlen(c_name)] = 0;

	snprintf(c_src, sizeof( c_src), "%s", taskinfo->task_src);
	c_src[strlen(c_src)] = 0;

	snprintf(c_srcurl, sizeof( c_srcurl), "%s", taskinfo->task_src_url);
	c_srcurl[strlen(c_srcurl)] = 0;
	
	if (taskinfo->total_size > 0)
		percent = (int)((float)taskinfo->downloaded_size * 100 / (float)taskinfo->total_size);
	else
		percent = 0;

	if( DLST_FINISHED == taskinfo->task_stat )
	{
		if( 100 != percent )
			percent = 100 ;
	}
		
	sprintf( c_percent , "%d%%" , percent );
	c_percent[5] = 0;

	sprintf(c_rate, "%.2fKB", (float)taskinfo->speed/1024);
	if(s_max_task_download_speed < taskinfo->speed)
		s_max_task_download_speed = taskinfo->speed;
	if(s_task_count > 0)
	{
		s_curr_total_download_speed += taskinfo->speed;
		s_task_count--;
	}

	if (taskinfo->total_size >= 1024*1024*1024)
		sprintf(c_total_size, "%.1fGB", taskinfo->total_size/1024.0/1024/1024);
	else if(taskinfo->total_size >= 1024*1024) 
		sprintf(c_total_size, "%.1fMB", taskinfo->total_size/1024.0/1024);
	else 
		sprintf(c_total_size, "%.1fKB", taskinfo->total_size/1024.0);

	if (taskinfo->downloaded_size >= 1024*1024*1024)
		sprintf(c_downloaded_size, "%.1fGB", taskinfo->downloaded_size/1024.0/1024/1024);
	else if(taskinfo->downloaded_size >= 1024*1024) 
		sprintf(c_downloaded_size, "%.1fMB", taskinfo->downloaded_size/1024.0/1024);
	else 
		sprintf(c_downloaded_size, "%.1fKB", taskinfo->downloaded_size/1024.0);


	snprintf( c_path , sizeof( c_path ) , "%s" , taskinfo->task_path ) ;
	snprintf( c_file , sizeof( c_file ) , "%s%s" , taskinfo->task_path, taskinfo->task_name ) ;

	if (taskinfo->speed != 0)
	{
		int remain_time = (taskinfo->total_size - taskinfo->downloaded_size) / taskinfo->speed;
		if (remain_time < 120)
		{
			sprintf(c_time, "<2 mins");
		}
		else if (remain_time < 3600)
		{
			sprintf(c_time, "%d:%d",  remain_time/60, remain_time%60);
		}
		else if (remain_time < 24 * 3600)
		{
			sprintf(c_time, "%d:%d:%d", remain_time/3600, remain_time%3600/60, remain_time%60);
		}
		else
		{
			sprintf(c_time, ">1 day");
		}
	}
	else
		sprintf(c_time, " --");

	sprintf(c_errno, "%d", taskinfo->error_id);

	debugf(XFER_LOG_MAIN, "\ttask_id=%s\n", c_taskid);
	debugf(XFER_LOG_MAIN, "\ttask_state=%s\n", c_state);
	debugf(XFER_LOG_MAIN, "\ttask_src=%s\n", c_src);
	debugf(XFER_LOG_MAIN, "\ttask_name=%s\n", c_name);
	debugf(XFER_LOG_MAIN, "\ttask_path=%s\n", c_path);
	debugf(XFER_LOG_MAIN, "\ttask_file=%s\n", c_file);
	debugf(XFER_LOG_MAIN, "\ttask_percent=\033[7m%s\033[0m\n", c_percent);
	debugf(XFER_LOG_MAIN, "\ttask_size=%s/%s\n", c_downloaded_size, c_total_size);
	debugf(XFER_LOG_MAIN, "\ttask downloaded_size/written_size/total_size: (%d) / (%d) / (%d)\n", taskinfo->downloaded_size, taskinfo->written_size, taskinfo->total_size);
	debugf(XFER_LOG_MAIN, "\ttask_rate=\033[7m%s\033[0m\n", c_rate);
	debugf(XFER_LOG_MAIN, "\ttask_remaintime=%s\n", c_time);
	debugf(XFER_LOG_MAIN, "\ttask_errno=%s\n", c_errno);

	transfer_mgr_release_task_info(taskinfo);
}

static void transfer_ctrl_task_sync(transfer_taskid_t taskid)
{
	transfer_task_info_t * taskinfo;

	if(transfer_mgr_get_task_info(taskid, &taskinfo) != XFER_ERROR_OK)
		return;

	if( DLST_FINISHED == taskinfo->task_stat )
	{
		if( s_xfer_finished_taskid == taskinfo->task_id )
			s_xfer_finished_show_times ++ ;
		else if( 0 == s_xfer_finished_taskid )
			s_xfer_finished_taskid =  taskinfo->task_id;
	}
	else if( DLST_PENDING == taskinfo->task_stat )
	{
		if( s_xfer_pending_taskid == 0)
			s_xfer_pending_taskid = taskinfo->task_id;
	}

	if ((taskinfo->error_id == 1025)
		|| (taskinfo->error_id == 15400)
		|| (taskinfo->error_id == 15389)
		|| (taskinfo->error_id == 130)
		|| (taskinfo->error_id == 102))
	{
		s_xfer_error_taskid = taskid;
		s_xfer_error_code = taskinfo->error_id;
	}
		
	transfer_mgr_release_task_info(taskinfo);
}

void *transfer_ctrl_run(void *data)
{
	int errid = 0;
	int i;
	transfer_task_list_t* tasklist = NULL;

	while(1)
	{
		errid = transfer_mgr_get_task_list(&tasklist);

		if((errid == 0) && (tasklist != NULL))
		{
			s_task_count = tasklist->task_count;
#if XFER_TASK_IOFO_SHOW
			if (tasklist->task_count > 0)
			{
				debugf(XFER_LOG_MAIN, "----------xfer task update tasklist->count=%d----------\n", tasklist->task_count);
			}
#endif
			for(i = 0; i < tasklist->task_count; i++)
			{
				transfer_ctrl_task_sync(tasklist->task_ids[i]);

#if XFER_TASK_IOFO_SHOW
				/* task info show */
				debugf(XFER_LOG_MAIN, "[%d] tasklist->task_ids[%d]=%d\n", i+1, i, tasklist->task_ids[i]);
				transfer_task_info_show(tasklist->task_ids[i]);
#endif
			}

#if XFER_TASK_IOFO_SHOW
			if(s_max_total_download_speed < s_curr_total_download_speed)
				s_max_total_download_speed = s_curr_total_download_speed;

			if(tasklist->task_count > 0)
			{
				debugf(XFER_LOG_MAIN, "-----------------------------------------\n");
				debugf(XFER_LOG_MAIN, "-------total speed of current tasks =%.2fKB\n", (float)s_curr_total_download_speed/1024);
				debugf(XFER_LOG_MAIN, "-------max speed of single task =%.2fKB\n", (float)s_max_task_download_speed/1024);
				debugf(XFER_LOG_MAIN, "-------max speed of total tasks =%.2fKB\n", (float)s_max_total_download_speed/1024);
				debugf(XFER_LOG_MAIN, "-----------------------------------------\n\n");
			}
			s_curr_total_download_speed = 0;
#endif


			transfer_mgr_release_task_list(tasklist);

			if( s_xfer_finished_show_times > 1 )
			{
				/* UI app must close the finished task */
				//transfer_mgr_task_close( s_xfer_finished_taskid );
				s_xfer_finished_taskid = 0 ;
				s_xfer_finished_show_times = 0 ;

				if(s_xfer_pending_taskid)
				{
					errid = transfer_mgr_task_start( s_xfer_pending_taskid );
					s_xfer_pending_taskid = 0;
				}
			}

			/* deal with some thunder download errors */
			/* Restart task or reload tasks */
			if(s_xfer_error_code != 0)
			{
				/* restart the error task */
				if (s_xfer_error_taskid != 0)
				{
					transfer_mgr_task_start(s_xfer_error_taskid);
					debugf(XFER_LOG_MAIN, "##### taskid=%d ERROR, errorid=%d, Restart it!\n",
							s_xfer_error_taskid, 
							s_xfer_error_code);
				}

				s_xfer_error_code = 0;
				s_xfer_error_retry++;
				if (s_xfer_error_retry >= XFER_TASK_RETRY_MAX)
				{
					/* reload all tasks */
					transfer_mgr_load(xfer_load_path);

					debugf(XFER_LOG_MAIN, "##### Restart task not work, Reload tasks!\n");

					s_xfer_error_retry = 0;
				}
			}
		}

		transfer_mgr_save_tasks();
#ifdef __WIN32__
		Sleep(XFER_FILE_LOOP_TIME * 1000);
#else
		sleep(XFER_FILE_LOOP_TIME);
#endif
	}

	return NULL;
}

