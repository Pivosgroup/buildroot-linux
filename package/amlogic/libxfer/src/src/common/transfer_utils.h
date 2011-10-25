/*******************************************************************
 * 
 *  Copyright (C) 2008 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Manager 
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/


#ifndef _TRANSFER_UTILS_H_
#define _TRANSFER_UTILS_H_

#include "xfer_error.h"
#include "transfer_def.h"

typedef struct _transfer_mem_buff{
 	char* 	buff;
	int 	capacity;
	int		length;
} transfer_mem_buff_t;

/**
 * examine the giving directory exist or not
 * @param path                    [in]    	
 * @return                                  error id
 */
int 
dir_exist(char *dir_name);

/**
 * make directory 
 * @param path                    [in]    	
 * @return                                  error id
 */
int 
xfer_mkdir_safe(char *dir_name);

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
xfer_errid_t 
pop_buff_safe(void* dst, char** src, int len, int* dst_len);

/**
 * push src to memory buffer
 * @return                                  error id
 */
xfer_errid_t 
push_buff_safe(transfer_mem_buff_t* mem_buff, void* src, int len);

/**
 * write file safely, and used to save info, for example task info.
 * @param file                    [in]    	file name to read
 * @param buff                    [in]    	written data
 * @param len                     [in]    	written data length
 * @return                                  error id
 */
xfer_errid_t 
write_file_safe(const char* file, const char* buff, int len);

xfer_errid_t 
write_file_fd(int fd, const char* buff, int offset, int len);

#endif
