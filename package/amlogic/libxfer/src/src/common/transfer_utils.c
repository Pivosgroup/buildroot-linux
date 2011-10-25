/*******************************************************************
 * 
 *  Copyright (C) 2008 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Manager 
 *
 *  Author: Amlogic Software
 *
 *******************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#endif
#include "transfer_utils.h"
#include "xfer_common.h"
#include "../http/http_manager.h"
#include "xfer_debug.h"

//#include <dirent.h>
#ifndef NULL
#define NULL ((void *)0)
#endif


#define XFER_TASKINFO_SIZE_MAX	(1024*1024)

int dir_exist(char *dir_name) 
{
	int ret;
	int len;
#ifndef _MSC_VER
	DIR* dir;
#endif
	char dir_path[DL_MAX_PATH+1];

	if((dir_name == NULL) || ((len = strlen(dir_name)) <= 0))
		return -1;

	if(len > DL_MAX_PATH)
		len = DL_MAX_PATH;
	
	memset(dir_path, 0, sizeof(dir_path));		
	strncpy(dir_path, dir_name, len);
	if(dir_path[len-1] != '/')
	{
		dir_path[len++] = '/';
		dir_path[len] = 0;
	}	

#ifdef _MSC_VER
	if(_access(dir_path,0) == 0)
#else
	if( (dir = opendir(dir_path)) == NULL)
#endif
	{
		ret = 0;
	}
	else
	{
#ifndef _MSC_VER
		closedir(dir);
#endif
		ret = 1;
	}
	
	return ret;
}

int xfer_mkdir_safe(char *dir_name)
{
	int ret = 0;
	int len = 0;
	char dir_path[DL_MAX_PATH+1];

	if((dir_name == NULL) || ((len = strlen(dir_name)) <= 0))
		return -1;

	if(len > DL_MAX_PATH)
		len = DL_MAX_PATH;
	
	memset(dir_path, 0, sizeof(dir_path));	
	strncpy(dir_path, dir_name, len);
	/*
	if((dir_path[len-1] == '/' )||(dir_path[len-1] == '\\'))
	{
		dir_path[len-1] = 0;
	}
	dir_path[len] = 0;
	*/
	
	//ret = xfer_mkdir(dir_path, 0777); 
	ret = make_dir_recursive(dir_path);

	return ret;
}

int make_dir_recursive(char* path)
{
	int		res;
	int		len;
	char*	sep = NULL;
	char	subdir[DL_MAX_PATH];
	sep = path;
	sep = strchr(sep+1, XFER_PATH_SP);

	while(sep) {
		len = (int)(sep - path);
		memcpy( subdir, path, len);
		subdir[len] = 0;
		if (subdir[len-1] != ':')
		{
			res = xfer_mkdir(subdir, 0777);
			if (res != 0)
			{
				if (errno == EEXIST) {
					/* check for existing directory */
					int mderr = errno;
					int serr;
					struct stat sbuf;
					serr = stat(subdir, &sbuf);
					if (!serr)
						res = 0;
					errno = mderr;
				}
			}	
			if (res)
			{
				return XFER_ERROR_STORAGE_CREATE_PATH_ERR;
			}
		}
		
		sep = strchr(sep+1, XFER_PATH_SP);
	}
	return XFER_ERROR_STORAGE_SUCCESS;
}

static int create_file(const char* file_name, int len)
{
	int fd = -1;
	if ((fd = xfer_fileopen(file_name, XFER_O_WRONLY | XFER_O_CREAT | XFER_O_TRUNC,
		XFER_S_IREAD | XFER_S_IWRITE)) == -1)
	{
		return -1;
	}

	if (xfer_filelseek(fd, len-1, SEEK_SET) == -1)
	{
		xfer_fileclose(fd);
		return -1;
	}
	
	if (xfer_filewrite(fd, "0", 1) == -1)
	{
		xfer_fileclose(fd);
		return -1;
	}

	xfer_fileclose(fd);
	return 0;
}

static unsigned short xfer_checksum(const char* buffer, int size)
{
    unsigned long cksum = 0;
    while(size>1)
    {
        cksum += *buffer++;
        size -= sizeof(char);
    }
    if(size) {
        cksum += *buffer;
    }
    cksum = (cksum>>16) + (cksum&0xffff);
    cksum += (cksum>>16);             
    return (unsigned short)(~cksum);
}

int read_whole_file(char** buff, const char* file_name)
{
	int 	fd = -1;
	int 	read_len = 0;
	int 	curr_read_len = 0;
	int		need_read = 0;
	struct stat stat_buf;
	char    version[5];
	int		has_version = 0;
	unsigned short checksum = 0;

	if (file_name == NULL || strlen(file_name) < 4)
		return -1;

	if ((fd = xfer_fileopen(file_name, XFER_O_RDONLY | XFER_O_BINARY, XFER_S_IREAD)) == -1
		|| fstat(fd, &stat_buf) == -1 || stat_buf.st_size < XFER_TASKINFO_SIZE_MAX)
	{
		if (fd != -1)
			xfer_fileclose(fd);
		create_file(file_name, XFER_TASKINFO_SIZE_MAX);
		write_file_safe(file_name, "1.00", 4);
		
		return -1;
	}
	
	// parse version begin
	if(xfer_fileread(fd, (void*)version, 4) != 4)
	{
		xfer_fileclose(fd);
		return -1;
	}
	
	if (memcmp(version, "1.01", 4) != 0)
	{
		if (xfer_filelseek(fd, 0, SEEK_SET) == -1)
		{
			xfer_fileclose(fd);
			return -1;
		}
		has_version = 0;
	}
	else
	{
		if(xfer_fileread(fd, (void*)&checksum, 2) != 2)
		{
			xfer_fileclose(fd);
			return -1;
		}
		has_version = 1;
	}
	// parse version end

	if((curr_read_len = xfer_fileread(fd, (void*)&need_read, sizeof(int))) != sizeof(int)
		|| need_read < 20 || need_read > stat_buf.st_size)
	{
		xfer_fileclose(fd);
		return -1;
	}
	
	//need_read = stat_buf.st_size;
	if ((*buff = (char*)xfer_malloc(need_read+1)) == NULL)
	{
		xfer_fileclose(fd);
		return -1;
	}

	curr_read_len = 0;
	do{
		curr_read_len = xfer_fileread(fd, *buff+read_len, need_read);
		if (curr_read_len <= 0)
		{
			xfer_free(*buff);
			*buff = NULL;
			xfer_fileclose(fd);
			return -1;
		}
		read_len += curr_read_len;
		need_read -= curr_read_len;
	}while(need_read);
	
	xfer_fileclose(fd);

	if (has_version && checksum != xfer_checksum(*buff, read_len))
	{
		xfer_free(*buff);
		*buff = NULL;
		return -1;
	}
	
	return read_len;
}

xfer_errid_t pop_buff_safe(void* dst, char** src, int len, int* dst_len)
{
	if (len > *dst_len)
		return XFER_ERROR_HTTP_FILE_CONTENT_ERROR;
	
	memcpy(dst, *src, len); 
	*src 	 += len; 
	*dst_len -= len;	

	return XFER_ERROR_OK;
}

xfer_errid_t push_buff_safe(transfer_mem_buff_t* mem_buff, void* src, int len)
{
	if (mem_buff == NULL)
		return XFER_ERROR_HTTP_MALLOC_FAILURE;
	
	if (mem_buff->capacity - mem_buff->length < len)
	{
		if (mem_buff->capacity == 0)
		{
			mem_buff->capacity = (len < 512) ? 512 : len;
			mem_buff->buff = xfer_malloc(mem_buff->capacity);
		}
		else
		{
			mem_buff->capacity *= 2;
			if (mem_buff->capacity < len + mem_buff->length)
				mem_buff->capacity = (len + mem_buff->length) * 2;
			mem_buff->buff = xfer_realloc(mem_buff->buff, mem_buff->capacity);
		}
		if (mem_buff->buff == NULL)
		{
			return XFER_ERROR_HTTP_MALLOC_FAILURE;
		}
	}
	
	memcpy(mem_buff->buff+mem_buff->length, src, len); 
	mem_buff->length += len; 

	return XFER_ERROR_OK;
}

xfer_errid_t write_file_safe(const char* file, const char* buff, int len)
{
	int		fd = -1;
	xfer_errid_t res = XFER_ERROR_OK;
    unsigned short checksum = 0;
	int		offset = 0;

	if (len == 0)
		return XFER_ERROR_OK;

	if (file == NULL || strlen(file) < 4)
		return XFER_ERROR_HTTP_IN_PARAM_NULL;

	if ((fd = xfer_fileopen(file, XFER_O_WRONLY | XFER_O_BINARY, XFER_S_IREAD | XFER_S_IWRITE)) == -1)
	{
		return XFER_ERROR_HTTP_OPEN_FILE_ERR;
	}

	// added version number begin
	if (xfer_filewrite(fd, "1.01", 4) != 4)
	{
		xfer_fileclose(fd); 
		return XFER_ERROR_HTTP_WRITE_FILE_ERR;
	}
	offset += 4;
		
	checksum = xfer_checksum(buff, len);
	if (xfer_filewrite(fd, (void*)&checksum, 2) != 2)
	{
		xfer_fileclose(fd); 
		return XFER_ERROR_HTTP_WRITE_FILE_ERR;
	}
	offset += 2;
	// added version number end

	if (xfer_filewrite(fd, (void*)&len, sizeof(int)) != sizeof(int))
	{
		xfer_fileclose(fd); 
		return XFER_ERROR_HTTP_WRITE_FILE_ERR;
	}
	offset += sizeof(int);

	if ((res = write_file_fd(fd, buff, offset, len)) != XFER_ERROR_OK)
	{
			xfer_fileclose(fd);
			return XFER_ERROR_HTTP_WRITE_FILE_ERR;
	}
	
	xfer_fileclose(fd); 
	return XFER_ERROR_OK;
}

xfer_errid_t 
write_file_fd(int fd, const char* buff, int offset, int len)
{
	int written 		= 0;
	int curr_written 	= 0;
	int need_write 		= len;
#ifdef __ROM_
	int write_0_byte_c 	= 0;
#endif

	if (len == 0)
		return XFER_ERROR_OK;

	if (xfer_filelseek(fd, offset, SEEK_SET) == -1)
		return XFER_ERROR_HTTP_SEEK_FILE_ERR;
	
	do{
		if ((curr_written = xfer_filewrite(fd, buff+written, need_write)) < 0)
		{
			return XFER_ERROR_HTTP_WRITE_FILE_ERR;
		}

#ifdef __ROM_
		if (0 == curr_written)
		{
			if (10 == ++write_0_byte_c)
				return XFER_ERROR_HTTP_WRITE_0BYTE_ERR;
		}
		else 
			write_0_byte_c = 0;
#endif
		
		written += curr_written;
		need_write -= curr_written;
	}while(written < len);

	return XFER_ERROR_OK;
}


