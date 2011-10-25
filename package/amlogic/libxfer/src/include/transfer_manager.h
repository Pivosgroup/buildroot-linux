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

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER

#ifdef BUILDING_LIBXFER
#define XFER_EXTERN  __declspec(dllexport)
#else
#define XFER_EXTERN  __declspec(dllimport)
#endif

#else
#define XFER_EXTERN
#endif
#include "transfer_def.h"

/**
 * initialize the transferer
 * @return                                  error id
 */
xfer_errid_t 
transfer_mgr_init();

/**
 * initialize the transferer
 * @param lic                      [in]    	thunder license
 * @return                                  error id
 */
#if 0
xfer_errid_t 
transfer_mgr_init_ex(char* lic);
#endif

/**
 * finalize the transferer, and save all tasks.
 */
void 
transfer_mgr_fini();

/**
 * register transfer modules.
 * @param type						[in]    the type of transfer module.
 * @param param                     [in]    the parameter needed by transfer module registration. can be NULL if not needed. 
 * @return                                  error id
 */
xfer_errid_t 
transfer_mgr_module_register(transfer_module_type_e type, void *param);

/**
 * register transfer modules.
 * @param type						[in]    the type of transfer module.
 * @return                                  error id
 */
xfer_errid_t 
transfer_mgr_module_unregister(transfer_module_type_e type);

/**
 * load external disk tasks.
 * @return                                  error id
 */
xfer_errid_t
transfer_mgr_save();

/**
 * load external disk tasks.
 * @param path                      [in]    the path of tasks info file, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                                  error id
 * NOTE: If there are not tmp files(http.aml,pend.aml and thunder.aml) in the path, they will be created.
 */
xfer_errid_t
transfer_mgr_load(const char* path);

/**
 * unload external disk tasks.
 * @param path                      [in]    the path of tasks info file, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                                  error id
 * NOTE: If the argument path equals NULL, we will unload the current download path.
 */
xfer_errid_t
transfer_mgr_unload(const char* path);

/**
 * create download temp file such as pend.aml,thunder.aml.
 * @param path                      [in]    the path of tasks info file, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                                  error id
 * NOTE: If this function is not called, and transfer_mgr_load() is also not called, when adding a http task, tmp file will not be created. But when adding a thunder task, tmp file will be created in default download path(./amlogic_download/).
 */
xfer_errid_t
transfer_mgr_create_aml_file(const char* path);

/**
 * get options
 * @param opt                       [in]    option type
 * @param type                      [in]    task type
 * @param result                    [out]   option value
 * @return                                  error id
 */
xfer_errid_t 
transfer_mgr_getopt(transfer_options_e opt, transfer_type_e type, int * result);

/**
 * set options
 * @param opt                       [in]    option type
 * @param type                      [in]    task type
 * @param value                     [in]    option value
 * @return                                  error id
 */
xfer_errid_t 
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
xfer_errid_t 
transfer_mgr_task_add(const char* 		 	source_path, 
					  const char* 		 	saved_path,
					  transfer_type_e		type,
					  int					mode,
					  const char* 			post_buff,
					  transfer_complete_cb	complete_callback,
					  transfer_taskid_t* 	taskid);
 
/**
 * append a http header
 * @param taskid                    [in]    the task id
 * @param header                    [in]    the header that to be added
 * @return                                  error id
 * NOTES: This function just can be called before the task starts, and the task teyp must be XFER_TASK_HTTP.
 * The param header could be like this: "Accept-Language: zh-cn".
 */
xfer_errid_t 
transfer_mgr_task_append_http_header(transfer_taskid_t taskid, char *header);

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
#if 0
xfer_errid_t 
transfer_mgr_task_add_ex(const char* 		 	source_path, 
					  	 const char* 		 	saved_path,
					  	 const char* 			buff,
					  	 int					buff_len,
					  	 transfer_type_e		type,
					  	 int					mode,
					  	 const char* 			post_buff,
					  	 transfer_complete_cb	complete_callback, /* DON'T try to free input parameter pointer of callback!!! */
					  	 transfer_taskid_t* 	taskid);
#endif

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
#if 0
xfer_errid_t transfer_mgr_task_add_ex2(const char* 		 	source_path, 
									  const char* 		 	saved_path, 
									  u64_t					task_len,
									  transfer_type_e 	 	type,
									  int					mode,
									  const char* 			post_buff,
									  transfer_complete_cb	complete_callback,
									  transfer_taskid_t* 	taskid);
#endif
/**
 * add a thunder vod task 
 * @param cid						[in]    the cid of the kankan task 
 * @param gcid						[in]    the gcid of the kankan task 
 * @param saved_path                [in]    the path to save the transfered file
 * @param task_len	                [in]    task length of the kankan task
 * @param type	                    [in]   	the created task type
 * @param file name                 [in]   	the created kankan task name
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
xfer_errid_t transfer_mgr_task_add_vod( const char* 		cid, 
									  	const char*			gcid,
									  	const char* 		saved_path, 
									  	u64_t					task_len,
									  	transfer_type_e 	type,
									  	const char* 		file_name,
									  	transfer_taskid_t* 	taskid);


/**
 * add a task with thunder gcid
 * @param source_path              	[in]    the url of the transfered file or the name of torrent file
 * @param gcid						[in]    the gcid of the task 
 * @param saved_path                [in]    the path to save the transfered file
 * @param task_len	                [in]    task length need download
 * @param type	                    [in]   	the created task type
 * @param file name                 [in]   	the created task name
 * @param taskid                    [out]   the created task id
 * @return                                  error id
 */
xfer_errid_t transfer_mgr_task_add_gcid( const char* 		source_path, 
									  	const char*			gcid,
									  	const char* 		saved_path, 
									  	u64_t					task_len,
									  	transfer_type_e 	type,
									  	const char* 		file_name,
									  	transfer_taskid_t* 	taskid);

/**
 * start a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
transfer_mgr_task_start(transfer_taskid_t taskid);

/**
 * pause a task
 * @param taskid                    [in]    the task id
 * @return                                  error id
 */
xfer_errid_t 
transfer_mgr_task_pause(transfer_taskid_t taskid);

/**
 * cancel a task
 * @param taskid                    [in]    the task id
void 
transfer_mgr_task_cancel(transfer_taskid_t taskid);
 */

/**
 * cancel a task with callback
 * @param taskid                    [in]    the task id
 * @param cancel_callback	        [in]   	the cancel callback
void 
transfer_mgr_task_cancel_ex(transfer_taskid_t taskid, transfer_cancel_cb cancel_callback);
 */

/**
 * close a task
 * @param taskid                    [in]    the task id
 */
void 
transfer_mgr_task_close(transfer_taskid_t taskid);

/**
 * close the tasks that type specified by param 'type', and download disk specified by 'disk'.
 * @param type						[in]    the task type
 * @param path						[in]    the download path
 *
 * NOTES: currently only valid for type XFER_MODULE_THUNDER
 */
void 
transfer_mgr_task_close_ex(transfer_module_type_e type, char *disk);


/**
 * get the task list. Notice the task list is a duplicated copy so that there is no thread conflict
 * @param tasklist                  [out]   the returned task list
 * @return                                  error id
 */
xfer_errid_t 
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
xfer_errid_t 
transfer_mgr_get_task_info(transfer_taskid_t taskid, transfer_task_info_t** taskinfo);

/**
 * release the result returned by transfer_mgr_get_task_info()
 * @param taskinfo                  [in]    the task info
 */
void 
transfer_mgr_release_task_info(transfer_task_info_t* taskinfo);

#if 0
/**
 * get need reserved disk space size.
 * @return                                  need reserved disk space size(unit MB)
 */
int 
transfer_mgr_reserved_space();
#endif

/*
* set file path that save all tasks info 
 * @param path		                [in]    file path, MUST add "\" as the end of this path and not more 200 bytes.
 * @return                               	error id
 * @notes: you'd best calling this nearby calling transfer_mgr_init, of course, you can call this function anytime.
 */
xfer_errid_t
transfer_mgr_set_tasks_info_path(const char* path);

/*
* set complete callback function 
 * @param path		                [in]    complete callback function.
 * @return                               	error id
 * @notes: you'd better call this nearby calling transfer_mgr_init, of course, you can call this function anytime. 
 * If complete callback function not set, the default callback function will be called when task completed.
 */
xfer_errid_t
transfer_mgr_set_complete_callback(transfer_complete_cb complete_callback);

xfer_errid_t
transfer_mgr_set_demo_path(const char* path);

/*
* set relative path prefix 
 * @param path                 		[in]    file path, MUST add "/" as the end of this path and not more 200 bytes.
 * @return                               	error id
 * @notes: you'd better call this function before add task or load task.  
 */
xfer_errid_t
transfer_mgr_set_path_prefix(const char* path);

xfer_errid_t 
transfer_mgr_get_seed_info(char *src, char *seed_path, torrent_seed_info_t** seed_info);

xfer_errid_t 
transfer_mgr_release_seed_info(torrent_seed_info_t** seed_info);

#if 0
xfer_errid_t
transfer_mgr_set_stat_callback(transfer_status_cb status_callback);
#endif

/*
* enable cache manager
 * @notes: default is enable. if disable, you must manager cached local files for youself(/mnt/C/http_async).
 */
#if 0
void 
transfer_mgr_enable_cache_file(unsigned char benable);

xfer_errid_t
transfer_mgr_set_customed_allocator(int fun_idx, void *fun_ptr);

xfer_errid_t
transfer_mgr_set_license(char* lic);

int get_tansfer_type(transfer_taskid_t taskid) ;
#endif

transfer_taskid_t
transfer_mgr_get_thunder_taskid(transfer_taskid_t taskid);

xfer_errid_t
transfer_mgr_get_task_src_url(transfer_taskid_t taskid, char* src_url);

xfer_errid_t
transfer_mgr_set_task_src_url(transfer_taskid_t taskid, char* src_url);

xfer_errid_t
transfer_mgr_task_add_bt( const char* 		source_url, 
										const char* 		source_path, 
									  	const char* 		saved_path,  
									  	const char* 		file_name,
									  	u32_t*				file_index_array,
										u32_t				file_num,
									  	transfer_taskid_t* 	taskid);
xfer_errid_t 
transfer_mgr_get_bt_files_info(transfer_taskid_t taskid, torrent_seed_info_t* seed_info);

xfer_errid_t
transfer_mgr_get_kankan_buffer_percent(transfer_taskid_t taskid, int *percent);

xfer_errid_t
transfer_mgr_read_kankan_file(transfer_taskid_t taskid, u64_t start_pos, u64_t len, char *buf, int block_time);

/**
 * Check if the current task is creating tmp file. 
 * If this function doesn't return 0, it means this task is busy for creating tmp file.
 * We should close this task later.
 * @param taskid                    [in]    the task id
 * @return                                  error id. 
 */
xfer_errid_t
transfer_mgr_task_check_busy(transfer_taskid_t taskid);

#ifdef  __cplusplus
}
#endif

#endif
