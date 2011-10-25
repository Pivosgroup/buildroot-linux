/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Sync Module
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

#include "transfer_manager.h"
#include "xfer_update.h"

#ifdef __WIN32__
#include <Windows.h>
#else
#include <semaphore.h>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

#define XFER_FILE_LOOP_TIME 5

static int s_xfer_update_task_inited = 0;
static int s_finished_taskid = 0;
static int s_finished_show_times = 0;
#ifdef __WIN32__
HANDLE s_xfer_update_task_thread;
#else
static pthread_t s_xfer_update_task_thread;
#endif
static int s_task_count;
static int s_max_task_download_speed;
static int s_max_total_download_speed;
static int s_curr_total_download_speed;


void *xfer_update_task_run(void *data);

#ifdef __WIN32__
DWORD WINAPI xfer_update_task_run_win32( LPVOID lpParam )
{
	xfer_update_task_run(NULL);

	return TRUE;
}
#endif

int xfer_update_task_init()
{
	if(s_xfer_update_task_inited == 0)
	{
		#ifdef __WIN32__
		if((s_xfer_update_task_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)xfer_update_task_run_win32, NULL, 0, NULL)) == NULL)
		#else
		if(pthread_create(&(s_xfer_update_task_thread),
			NULL,
			xfer_update_task_run,
			NULL) != 0)
		#endif
		{
			printf("Create thread ERROR\n");
			return -1;
		}
		s_xfer_update_task_inited = 1;
	}

	return 0;
}

void xfer_update_task_fini()
{
	if(s_xfer_update_task_inited == 1)
	{
		#ifdef __WIN32__
		DWORD errcode = 0;
		TerminateThread(
				s_xfer_update_task_thread,
				errcode
		);
		CloseHandle(s_xfer_update_task_thread);
		#else
		pthread_cancel(s_xfer_update_task_thread);
		#endif
		s_xfer_update_task_inited = 0;
	}
}

static void xfer_task_info_show(transfer_taskid_t taskid)
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
		if( s_finished_taskid == taskinfo->task_id )
			s_finished_show_times ++ ;
		else if( 0 == s_finished_taskid )
			s_finished_taskid =  taskinfo->task_id ;

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

	printf("\ttask_id=%s\n", c_taskid);
	printf("\ttask_state=%s\n", c_state);
	printf("\ttask_src=%s\n", c_src);
	printf("\ttask_name=%s\n", c_name);
	printf("\ttask_path=%s\n", c_path);
	printf("\ttask_file=%s\n", c_file);
	printf("\ttask_rate=%s\n", c_rate);
	printf("\ttask_percent=%s\n", c_percent);
	printf("\ttask_size=%d/%d\n", taskinfo->downloaded_size, taskinfo->total_size);
	printf("\ttask_size=%s/%s\n", c_downloaded_size, c_total_size);
	printf("\ttask_remaintime=%s\n", c_time);
	printf("\ttask_errno=%s\n", c_errno);

	transfer_mgr_release_task_info(taskinfo);
}

void *xfer_update_task_run(void *data)
{
	int errid = 0;
	int i;
	transfer_task_list_t* tasklist = NULL;

	while(1)
	{
		printf("\n----------xfer task update ----------\n");
		errid = transfer_mgr_get_task_list(&tasklist);

		if((errid != 0) || (tasklist == NULL))
		{
			printf("----------xfer task update tasklist is NULL,errid=%d----------\n", errid);
		}
		else
		{
			printf("----------xfer task update tasklist->count=%d----------\n", tasklist->task_count);
			s_task_count = tasklist->task_count;
			for(i = 0; i < tasklist->task_count; i++)
			{
				printf("[%d] tasklist->task_ids[%d]=%d\n", i+1, i, tasklist->task_ids[i]);
				xfer_task_info_show(tasklist->task_ids[i]);
			}

			if(s_max_total_download_speed < s_curr_total_download_speed)
				s_max_total_download_speed = s_curr_total_download_speed;

			if(tasklist->task_count > 0)
			{
				printf("\n-----------------------------------------\n");
				printf("-------max speed of single task = %.2fKB\n", (float)s_max_task_download_speed/1024);
				printf("-------max speed of total tasks = %.2fKB\n", (float)s_max_total_download_speed/1024);
				printf("-------current speed of total tasks = %.2fKB\n", (float)s_curr_total_download_speed/1024);
				printf("-----------------------------------------\n");
			}
			s_curr_total_download_speed = 0;

			transfer_mgr_release_task_list(tasklist);
		}
#ifdef __WIN32__
        Sleep(XFER_FILE_LOOP_TIME * 1000);
#else
		sleep(XFER_FILE_LOOP_TIME);
#endif
	}

	return NULL;
}


