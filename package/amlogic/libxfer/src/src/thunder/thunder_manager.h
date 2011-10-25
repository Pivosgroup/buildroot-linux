/*******************************************************************
 * 
 *  Copyright (C) 2008 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of Thunder Module
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/

#ifndef _THUNDER_MANAGER_H_
#define _THUNDER_MANAGER_H_


#include "transfer_def.h"
#include "xfer_common.h"
#include "embed_thunder.h"

/**
 * initialize the http manager, and load all the saved tasks
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_init(char* lic);

/**
 * finalize the http manager
 */
void 
thunder_mgr_fini();

/**
 * get options
 * @param opt                       [in]    option type
 * @param result                    [out]   option value
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_getopt(transfer_options_e opt, int * result);

/**
 * set options
 * @param opt                       [in]    option type
 * @param value                     [in]    option value
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_setopt(transfer_options_e opt, int value);

/**
 * add a task as pending
 * @param source_path              	[in]    the urs of the transfered file
 * @param saved_path                [in]    the path to save the transfered file
 * @param type	                    [in]   	the created task type
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
xfer_errid_t thunder_mgr_add_pending(const char* 			src, 
							 	  	const char* 			saved_path,
							 	  	transfer_type_e 	 	type,
							 	  	const char* 			file_name,
							 	  	transfer_taskid_t*	taskid);
/**
 * add a task as pending
 * @param source_path              	[in]    the urs of the transfered file
 * @param saved_path                [in]    the path to save the transfered file
 * @param type	                    [in]   	the created task type
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
xfer_errid_t thunder_mgr_add_pending_ex(const char* 			src, 
									int 					file_length,
							 	  	const char* 			saved_path,
							 	  	transfer_type_e 	 	type,
							 	  	const char* 			file_name,
							 	  	transfer_taskid_t*	taskid);


xfer_errid_t thunder_mgr_add_bt_pending(const char* 			src_url,
									const char*			src,
									int					file_length,
							 	  	const char* 			saved_path,
							 	  	transfer_type_e 	 	type,
							 	  	const char* 			file_name,
							 	  	const u32			file_num,
							 	  	const u32*			file_index_array,
							 	  	transfer_taskid_t*		taskid);

/**
 * add a task
 * @param source_path              	[in]    the urs of the transfered file
 * @param saved_path                [in]    the path to save the transfered file
 * @param type	                    [in]   	the created task type
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_task_add( const char* 		  	src, 
					  const char* 		  	saved_path,
					  transfer_type_e 	 	type,
					  const char* 			file_name,
					  transfer_taskid_t* 	taskid);

xfer_errid_t thunder_mgr_task_add_ex(const char* 		src, 
									int 				file_size,
									const char* 		gcid,
							 	  	const char* 		saved_path,
							 	  	transfer_type_e 	type,
							 	  	const char*      	file_name,
							 	  	transfer_taskid_t* 	taskid);

xfer_errid_t thunder_mgr_task_add_bt(const char* 		src_url,
									const char* 		seed_file_path, 
							 	  	const char* 		saved_path,
							 	  	transfer_type_e 	type,
							 	  	const char*      	file_name,
							 	  	u32*				file_index_array,
							 	  	u32				file_num,
							 	  	transfer_taskid_t* 	taskid);

/**
 * close a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
void 
thunder_mgr_task_close(transfer_taskid_t taskid);

/**
 * start a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_task_start(transfer_taskid_t taskid);

/**
 * pause a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_task_pause(transfer_taskid_t taskid);

/**
 * get the task list. Notice the task list is a duplicated copy so that there is no thread conflict
 * @param tasklist                  [out]   the returned task list
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_get_task_list(transfer_task_list_t** tasklist);

/**
 * get the information of the specified task. Notice the task info is a duplicated copy so that there is no thread conflict
 * @param taskid                    [in]    the task id
 * @param taskinfo                  [out]   the returned task info
 * @return                                  error id
 */
xfer_errid_t 
thunder_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo);


/*
* set file name that save all tasks info 
 * @param file_name                 [in]    file name
 * @return                               	error id
 */
xfer_errid_t
thunder_mgr_set_tasks_info_file(const char* file_name);

int 
thunder_mgr_taskid_validate(transfer_taskid_t taskid);

xfer_errid_t 
thunder_mgr_set_length(transfer_taskid_t taskid, u64_t len);

int 
thunder_mgr_task_exist(const char* src, const char* file_path, const char* file_name);

int 
thunder_mgr_task_bt_exist(const char* src, const char* file_path, const char* file_name, const u32 file_num, const u32 *file_index_array);

xfer_errid_t 
thunder_load_tasks();

xfer_errid_t 
thunder_save_tasks();

xfer_errid_t 
thunder_unload_tasks();

xfer_errid_t 
thunder_mgr_get_seed_info(char *src, char *seed_path, torrent_seed_info_t** seed_info);

xfer_errid_t 
thunder_mgr_release_seed_info(torrent_seed_info_t** seed_info);

int 
thunder_mgr_task_downloaded(const char* file_path, const char* file_name);

xfer_errid_t
thunder_mgr_set_license(char* lic);

xfer_errid_t
thunder_mgr_set_customed_allocator(int fun_idx, void *fun_ptr);

xfer_errid_t
thunder_mgr_check_busy(transfer_taskid_t taskid);

transfer_taskid_t 
thunder_mgr_get_thunder_taskid(transfer_taskid_t taskid);

void 
thunder_mgr_set_kankan_disk( int b_disk );	

xfer_errid_t
thunder_mgr_set_kankan_buffer_time(int32 buffer_time );

xfer_errid_t
thunder_mgr_get_kankan_buffer_percent(int32 task_id ,int32* buffer_time );

xfer_errid_t
thunder_mgr_read_kankan_file(int32 task_id, uint64 start_pos, uint64 len, char * buf, int32 block_time);

xfer_errid_t
thunder_mgr_set_license_temp(void);

u32
thunder_mgr_get_license_result(void);

u32
thunder_mgr_get_license_expire(void);

xfer_errid_t thunder_mgr_get_task_info_for_exam(transfer_taskid_t taskid, transfer_task_info_t** taskinfo, int *error_code);

xfer_errid_t thunder_mgr_set_task_src_url(transfer_taskid_t taskid, char *src_url);

xfer_errid_t thunder_mgr_get_task_src_url(transfer_taskid_t taskid, char *src_url);

xfer_errid_t thunder_mgr_get_bt_files_info(transfer_taskid_t taskid, torrent_seed_info_t* seed_info);

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
void thunder_dbg_msg_sent_(transfer_taskid_t taskid, int type);
void thunder_dbg_msg_done_(transfer_taskid_t taskid, int type);
#define THUNDER_DBG_MSG_SENT(taskid,type) thunder_dbg_msg_sent_(taskid, type);
#define THUNDER_DBG_MSG_DONE(taskid,type) thunder_dbg_msg_done_(taskid, type);
#else
#define THUNDER_DBG_MSG_SENT(taskid,type) 
#define THUNDER_DBG_MSG_DONE(taskid,type) 
#endif

#endif
