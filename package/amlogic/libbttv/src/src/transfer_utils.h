/*******************************************************************
 * 
 *  Copyright (C) 2008 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of transfer_mgr_manager lib
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/


#ifndef _TRANSFER_UTILS_H_
#define _TRANSFER_UTILS_H_

#include "net/ABoxBase/abx_error.h"
#include "transfer_def.h"

typedef struct _transfer_mem_buff{
 	char* 	buff;
	int 	capacity;
	int		length;
} transfer_mem_buff_t;


/**
 * append http header attribute 
 * @param taskid                   [in]    	task id
 * @param name                     [in]    	attribute name, e.g. "User-Agent"  
 * @param value                    [in]    	attribute value, e.g. "Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; "
 * @return                                  error id
 */
abx_errid_t
http_mgr_append_header(transfer_taskid_t taskid, const char* name, const char* value);

/**
 * trim http cache
 * @return                                  error id
 */
int
http_mgr_trim_cache();

/**
 * make directory recursively
 * @param path                    [in]    	path ending in '/', otherwise regarded as file name
 * @return                                  error id
 */
int 
make_dir_recursive(char* path);

/**
 * read whole file to buffer
 * @param buff                    [out]    	used to save data from file
 * @param file_name               [in]    	file name to read
 * @return                                  success, file length; otherwise -1
 */
int 
read_whole_file(char** buff, const char* file_name);

/**
 * copy a segment of src to dst 
 * @return                                  error id
 */
abx_errid_t 
pop_buff_safe(void* dst, char** src, int len, int* dst_len);

/**
 * push src to memory buffer
 * @return                                  error id
 */
abx_errid_t 
push_buff_safe(transfer_mem_buff_t* mem_buff, void* src, int len);

/**
 * write file safely, and used to save info, for example task info.
 * @param file                    [in]    	file name to read
 * @param buff                    [in]    	written data
 * @param len                     [in]    	written data length
 * @return                                  error id
 */
abx_errid_t 
write_file_safe(const char* file, const char* buff, int len);

abx_errid_t 
write_file_fd(int fd, const char* buff, int offset, int len);

#endif
