/*******************************************************************
 * 
 *  Copyright (C) 2008 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of transfer_manager lib
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/


#ifndef _TRANSFER_DEF_H_
#define _TRANSFER_DEF_H_

#include "net/ABoxBase/abx_common.h"
#include "net/ABoxBase/abx_error.h"
#include "net/ABoxBase/abx_dbg.h"
#include "embed_thunder.h"

/////< the major version
//extern const int VERSION_MAJOR;
//
///// the minor version
//extern const int VERSION_MINOR;
//
///// the build version
//extern const int VERSION_BUILD;

#define DL_MAX_PATH			260		///< max path line
#define DL_DOWNLOADING_MAX	2		///< max downloading tasks at the same time
#define DL_BT_FILES_MAX			50		///< max download bt task files

#define TASKID_INVALID		(-1)
#define TASKID_BT_BEGIN		0
#define TASKID_BT_END		1023
#define TASKID_HTTP_BEGIN	1024
#define TASKID_HTTP_END		8192
#define TASKID_THUNDER_BEGIN 8193
#define TASKID_THUNDER_END 	 (8193+8192)

typedef int transfer_taskid_t;       ///< task id type      

/**
 * task completion callback function pointer.
 * @param taskid                    [in]    completed task id
 * @param result                    [in]    if the (mode&XFER_BUFF) of added task is true, result is buff memory pointer, else is file name
 * @param opt                       [in]    if the (mode&XFER_BUFF) of added task is true, len is memory buff length, else is file length.
 * @param type                      [in]    defined as abx_errid_t
 */
typedef void (*transfer_complete_cb)(transfer_taskid_t taskid, const char* result, int len, int errno);

/**
 * task completion callback function pointer.
 * @param stat                    [in]    thunder download status
 */
typedef void (*transfer_status_cb)(int stat);

/**
 * task completion callback function pointer.
 * @param arg                    	[in]    canceled task id
 */
typedef void (*transfer_cancel_cb)(int arg);

typedef enum{
	XFER_TASK_HTTP	= 1,	///< http task
	XFER_TASK_BT	= 2,	///< bt task
	XFER_TASK_FTP	= 3,	///< ftp task
	XFER_TASK_XLURL = 4,	///< xunlei http task
	XFER_TASK_XLTCID= 5,	///< xunlei http task
	XFER_TASK_XLBT	= 6,	///< xunlei bt task
	XFER_TASK_XLGCID= 7,	///< xunlei gcid task
	XFER_TASK_ERROR = 1024	///< ERROR task
}transfer_type_e;

typedef enum{
	XFER_NULL 		= 0,	///< null
	XFER_SHOW		= 1,	///< HTTP ONLY. http task that is visible in downloading list. 
	XFER_POST  		= 2,	///< HTTP ONLY. http task that post data. 
	XFER_NOCACHE	= 4,	///< HTTP ONLY. don't cache content to file. 
	XFER_BUFF		= 8,	///< HTTP ONLY. complete_callback return memory buffer, instead of saving file. 
	XFER_CRYPT		= 16,	///< HTTP ONLY. crypt data.
	XFER_COOKIE		= 32,	///< HTTP ONLY. use cookie. 
	XFER_IMG		= 64,	///< HTTP ONLY. use to download img file. 
	XFER_IMGEX 		= 128,	///< HTTP ONLY. download img file
	XFER_ERROR 		= 1024	///< error download, unused
}transfer_mode_e;

/**
 * transfer task status
 */
typedef enum {
   	DLST_STOPPED,                    ///< task not started
    DLST_DOWNLOADING,                ///< downloading
    DLST_FINISHED,                   ///< download finished
    DLST_ERROR,                      ///< critical error occurred
    DLST_PENDING,					 ///< pending to download
    DLST_CREATING,					 ///< creating task
    DLST_NONE,						 ///< initialized
} transfer_task_stat_e;

/**
 *peer status
 */
typedef enum {
	BT_PEER_STATUS_USABLE = 0,
	BT_PEER_STATUS_CONNECTING,
	BT_PEER_STATUS_CONNECTED,
	BT_PEER_STATUS_BANED,
		
} bt_task_peer_status_e;

/**
 *peer info
 */
 typedef struct _transfer_bt_task_peer_info{
	unsigned long ip;
	unsigned short port;
	bt_task_peer_status_e statusflag; 
	char source;
	unsigned short healthindex;
	unsigned int download_speed;
	unsigned int upload_speed;
} transfer_bt_task_peer_info_t;

/**
 *peers info
 */
 typedef struct _transfer_bt_task_peers_info{
	int count;
	transfer_bt_task_peer_info_t *pbt_task_peer_info;
} transfer_bt_task_peers_info_t;
 
/**
 * transfer task info
 */
typedef struct _transfer_task_info{
    transfer_taskid_t 		task_id;        ///< task id
    char *              	task_name;      ///< task name, in GB2312
    char *              	task_path;      ///< task saved path, in GB2312
    char *              	task_src_url;   ///< task source url, in GB2312
    transfer_task_stat_e	task_stat;      ///< task status
    unsigned int        	speed;          ///< downloading speed, in Bytes/second
    filesize_t         		total_size;     ///< total size, in bytes
    filesize_t                 mem_size;     //downloaded_memory_size
    filesize_t         		downloaded_size;///< downloaded size, in bytes
    abx_errid_t         	error_id;       ///< current error ID, only valid in ABXBTST_ERROR stat
    int						file_created;
    transfer_taskid_t 		thunder_task_id;///< thunder task id

    transfer_bt_task_peers_info_t   bt_task_peers_info;
    int     b_get_index ;	
} transfer_task_info_t;


/**
 * transfer task list
 */
typedef struct _transfer_task_list{
    int             	task_count;         ///< the task count
    int					capacity;			///< the capacity of task_ids
    transfer_taskid_t * task_ids;           ///< task IDs as an array, whose element count is task_count
} transfer_task_list_t;


//#define MAX_TASK_COUNT  10              	///< the maximal concurrent task count


/**
 * transfer option types
 */
typedef enum {
    DLOPT_RATELIMIT_DOWNLOAD,        	///< the downloading rate limit
    DLOPT_RATELIMIT_UPLOAD,         	///< the uploading rate limit
    DLOPT_SETUP_DEMO,					///< setup demo
    DLOPT_RATELIMIT_MAX_TASKS,        	///< the downloading rate limit
    DLOPT_RATELIMIT_MAX_CONNS,         	///< the uploading rate limit
    DLOPT_RATELIMIT_SEED_SWITCH,        ///< the uploading rate limit
} transfer_options_e;

typedef struct _torrent_file_info
{
	u32 index;
	char file_name[256];
	u64 file_size;

	u32 file_percent;
	u32 file_status;
	u32 sub_task_err_code;
} torrent_file_info_t;

typedef struct _torrent_seed_info
{
	u64 	total_size;
	char name[256];
	u32 file_num;
	torrent_file_info_t *torrent_file_list;

	/* Just for bt task */
	char seed_file_path[256];
	char saved_path[256];
	char src_url[256];
	u32 seed_file_path_len;
	u32 saved_path_len;	
	u32 src_url_len;
	u32  download_file_index_array[DL_BT_FILES_MAX];
	u32  download_num;
} torrent_seed_info_t;


/**
 * set tmp path for thunder kankan
 * @param path                    [in]    the path writable for thunder kankan
 */
void thunder_mgr_set_tmp_path(const char * path);

#ifdef WIN32
    #define XFER_PATH_SP '\\'
	#define XFER_PATH_SP_STR "\\"
#else
    #define XFER_PATH_SP '/'
	#define XFER_PATH_SP_STR "/"
#endif


#endif

