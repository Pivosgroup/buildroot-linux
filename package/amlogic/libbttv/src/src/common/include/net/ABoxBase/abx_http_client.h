/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABox Nonblocking HTTP Client
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/

#ifndef _ABX_HTTP_CLIENT_H_
#define _ABX_HTTP_CLIENT_H_


#include "abx_error.h"


/**
 * the callback function to notify the downloading progress
 * return 1 if cancellation is needed
 */
typedef int (* abx_http_checkprogress_cb_t)(filesize_t total_size, filesize_t downloaded);


/**
 * init HTTP client
 * @param task_count                [in]    the task count allocated to HTTP client
 * @param task_prios                [in]    the priorities allocated for HTTP client, the array size should equal to task_count
 * @param cache_path                [in]    the path of the cache directory
 * @return                                  error id
 */
abx_errid_t abx_http_init(int task_count, int task_prios[], const char * cache_path);

/**
 * get the information contained in the torrent file
 * @param torrent_path              [in]    the name of the torrent file
 * @param torrent_info              [out]   the returned torrent information
 * @return                                  error id
 */
abx_errid_t abx_http_get(const char * url, const char * filepath, const char * filename, abx_http_checkprogress_cb_t cb);

abx_errid_t abx_http_openfile();

abx_errid_t abx_http_closefile();




#endif
