/*******************************************************************
 * 
 *  Copyright (C) 2008 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of transfer_mgr_manager lib
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/


#ifndef _TRANSFER_MANAGER_H_
#define _TRANSFER_MANAGER_H_

#include "net/ABoxBase/abx_error.h"
#include "transfer_def.h"


/**
 * initialize the transferer, and load all the saved tasks
 * @return                                  error id
 * @notes: using default thunder license if enable thunder manager
 */
abx_errid_t 
transfer_mgr_init();

/**
 * initialize the transferer, and load all the saved tasks
 * @param lic                      [in]    	thunder license
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_init_ex(char* lic);

/**
 * finalize the transferer, and save all tasks.
 */
void 
transfer_mgr_fini();

/**
 * load external disk tasks.
 * @param path                      [in]    the path of tasks info file, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                                  error id
 */
abx_errid_t
transfer_mgr_load(const char* path);

/**
 * unload external disk tasks.
 * @param path                      [in]    the path of tasks info file, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_unload(const char* path);

/**
 * get options
 * @param opt                       [in]    option type
 * @param type                      [in]    task type
 * @param result                    [out]   option value
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_getopt(transfer_options_e opt, transfer_type_e type, int * result);

/**
 * set options
 * @param opt                       [in]    option type
 * @param type                      [in]    task type
 * @param value                     [in]    option value
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_setopt(transfer_options_e opt, transfer_type_e type, int value);

/**
 * add a task
 * @param source_path              	[in]    the url of the transfered file or the name of torrent file
 * @param saved_path                [in]    the path to save the transfered file
 * @param type	                    [in]   	the created task type
 * @param post_buff	                [in]   	the http posted data, only mode has XFER_POST
 * @param user_callback	        	[in]   	the complete callback, only http and mode has no XFER_SHOW
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_task_add(const char* 		 	source_path, 
					  const char* 		 	saved_path,
					  transfer_type_e		type,
					  int					mode,
					  const char* 			post_buff,
					  transfer_complete_cb	complete_callback,
					  transfer_taskid_t* 	taskid);
 
/**
 * add a task extend
 * @param source_path              	[in]    the url of the transfered file or the name of torrent file
 * @param saved_path                [in]    the path to save the transfered file
 * @param type	                    [in]   	the created task type
 * @param buff                      [in]     the buff current used task name. 
 * @param buff_len                  [in]   	the length of buff
 * @param post_buff	                [in]   	the http posted data, only mode has XFER_POST
 * @param user_callback	        	[in]   	the complete callback, only http and mode has no XFER_SHOW
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_task_add_ex(const char* 		 	source_path, 
					  	 const char* 		 	saved_path,
					  	 const char* 			buff,
					  	 int					buff_len,
					  	 transfer_type_e		type,
					  	 int					mode,
					  	 const char* 			post_buff,
					  	 transfer_complete_cb	complete_callback, /* DON'T try to free input parameter pointer of callback!!! */
					  	 transfer_taskid_t* 	taskid);

/**
 * add a task
 * @param source_path              	[in]    the url of the transfered file or the name of torrent file
 * @param saved_path                [in]    the path to save the transfered file
 * @param task_len	                [in]    task length need download
 * @param type	                    [in]   	the created task type
 * @param post_buff	                [in]   	the http posted data, only mode has XFER_POST
 * @param user_callback	        	[in]   	the complete callback, only http and mode has no XFER_SHOW
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
abx_errid_t transfer_mgr_task_add_ex2(const char* 		 	source_path, 
									  const char* 		 	saved_path, 
									  u64					task_len,
									  transfer_type_e 	 	type,
									  int					mode,
									  const char* 			post_buff,
									  transfer_complete_cb	complete_callback,
									  transfer_taskid_t* 	taskid);
/**
 * add a task with thunder gcid
 * @param source_path              	[in]    the url of the transfered file or the name of torrent file
 * @param saved_path                [in]    the path to save the transfered file
 * @param task_len	                [in]    task length need download
 * @param type	                    [in]   	the created task type
 * @param post_buff	                [in]   	the http posted data, only mode has XFER_POST
 * @param user_callback	        	[in]   	the complete callback, only http and mode has no XFER_SHOW
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
abx_errid_t transfer_mgr_task_add_gcid( const char* 		source_path, 
									  	const char*			gcid,
									  	const char* 		saved_path, 
									  	u64					task_len,
									  	transfer_type_e 	type,
									  	const char* 		file_name,
									  	transfer_taskid_t* 	taskid);

/**
 * start a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_task_start(transfer_taskid_t taskid);

/**
 * pause a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_task_pause(transfer_taskid_t taskid);

/**
 * cancel a task
 * @param taskid                    [in]    the task id
 */
void 
transfer_mgr_task_cancel(transfer_taskid_t taskid);

/**
 * cancel a task with callback
 * @param taskid                    [in]    the task id
 * @param cancel_callback	        [in]   	the cancel callback
 */
void 
transfer_mgr_task_cancel_ex(transfer_taskid_t taskid, transfer_cancel_cb cancel_callback);

/**
 * close a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
void 
transfer_mgr_task_close(transfer_taskid_t taskid);


/**
 * get the task list. Notice the task list is a duplicated copy so that there is no thread conflict
 * @param tasklist                  [out]   the returned task list
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_get_task_list(transfer_task_list_t** tasklist);

/**
 * release the result returned by transfer_mgr_get_task_list()
 * @param tasklist                  [in]    the task list
 */
void 
transfer_mgr_release_task_list(transfer_task_list_t* tasklist);

/**
 * get the information of the specified task. Notice the task info is a duplicated copy so that there is no thread conflict
 * @param taskid                    [in]    the task id
 * @param taskinfo                  [out]   the returned task info
 * @return                                  error id
 */
abx_errid_t 
transfer_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo);

/**
 * release the result returned by transfer_mgr_get_task_info()
 * @param taskinfo                  [in]    the task info
 */
void 
transfer_mgr_release_task_info(transfer_task_info_t* taskinfo);

/**
 * get need reserved disk space size.
 * @return                                  need reserved disk space size(unit MB)
 */
int 
transfer_mgr_reserved_space();

/*
* set file path that save all tasks info 
 * @param path		                [in]    file path, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                               	error id
 * @notes: you'd best calling this nearby calling transfer_mgr_init, of course, you can call this function anytime.
 */
abx_errid_t
transfer_mgr_set_tasks_info_path(const char* path);

/*
* set file path that save all temp file 
 * @param path                 		[in]    file path, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                               	error id
 * @notes: you'd best calling this nearby calling transfer_mgr_init, of course, you can call this function anywhere.
 */
abx_errid_t
transfer_mgr_set_temp_path(const char* path);

abx_errid_t
transfer_mgr_set_kankan_temp_path(const char* path);

abx_errid_t
transfer_mgr_set_demo_path(const char* path);

abx_errid_t 
transfer_mgr_get_seed_info(char *src, char *seed_path, torrent_seed_info_t** seed_info);

abx_errid_t 
transfer_mgr_release_seed_info(torrent_seed_info_t** seed_info);

abx_errid_t
transfer_mgr_set_stat_callback(transfer_status_cb status_callback);

/*
* enable cache manager
 * @notes: default is enable. if disable, you must manager cached local files for youself(/mnt/C/http_async).
 */
void 
transfer_mgr_enable_cache_file(INT8U benable);

abx_errid_t
transfer_mgr_set_customed_allocator(int fun_idx, void *fun_ptr);

abx_errid_t
transfer_mgr_set_license(char* lic);

transfer_taskid_t
transfer_mgr_get_thunder_taskid(transfer_taskid_t taskid);

int get_tansfer_type(transfer_taskid_t taskid) ;

abx_errid_t
transfer_mgr_get_task_src_url(transfer_taskid_t taskid, char* src_url);

abx_errid_t
transfer_mgr_set_task_src_url(transfer_taskid_t taskid, char* src_url);

abx_errid_t
transfer_mgr_task_add_bt( const char* 		source_url, 
										const char* 		source_path, 
									  	const char* 		saved_path,  
									  	const char* 		file_name,
									  	u32*				file_index_array,
									  	u32				file_num,
									  	transfer_taskid_t* 	taskid);
abx_errid_t 
transfer_mgr_get_bt_files_info(transfer_taskid_t taskid, torrent_seed_info_t* seed_info);
#endif
