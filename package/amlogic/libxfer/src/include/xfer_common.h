/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#ifndef	_XFER_COMMON_H_
#define	_XFER_COMMON_H_

#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <malloc.h>


#define xfer_malloc      malloc
#define xfer_calloc      calloc
#define xfer_realloc     realloc
#define xfer_free        free

#define xfer_fileopen            open
#define xfer_fileclose(f)        close(f)
#define xfer_fileread(f,b,c)     read((f),(b),(c))
#define xfer_filewrite(f,b,c)    write((f),(b),(c))
#define xfer_filelseek(f,o,p)    lseek((f),(o),(p))
#define xfer_fileaccess(f,m)		access((f),(m))

#ifdef __WIN32__
#define xfer_mkdir(d, m)         mkdir((d))
#else
#define xfer_mkdir(d, m)         mkdir((d),(m))
#endif

#define XFER_O_RDONLY    O_RDONLY
#define XFER_O_RDWR      O_RDWR
#define XFER_O_WRONLY    O_WRONLY
#define XFER_O_CREAT     O_CREAT
#define XFER_O_TRUNC     O_TRUNC
#define XFER_O_APPEND    O_APPEND

#ifdef __WIN32__
#define XFER_O_BINARY    O_BINARY
#define XFER_S_IREAD     S_IREAD
#define XFER_S_IWRITE    S_IWRITE
#else
#define XFER_O_BINARY    0
#define XFER_S_IREAD     0
#define XFER_S_IWRITE    0
#endif

#define XFER_O_EXCL      O_EXCL


#endif  //_XFER_COMMON_H_

