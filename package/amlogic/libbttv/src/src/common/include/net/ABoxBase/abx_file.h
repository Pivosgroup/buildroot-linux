/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase files
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#ifndef _ABX_FILE_H_
#define	_ABX_FILE_H_


#include "abx_common.h"
#include "abx_error.h"


#ifdef AVOS

#include <io.h>
#include <Ioapi.h>
#include <Sysdefine.h>

#define abx_fileopen            open
#define abx_fileclose(f)        close(f)
#define abx_fileread(f,b,c)     read((f),(b),(c))
#define abx_filewrite(f,b,c)    write((f),(b),(c))
#define abx_filelseek(f,o,p)    lseek((f),(o),(p))
#define abx_fileaccess(f,m)		access((f),(m))
#define abx_mkdir(d, m)         mkdir((d),(m))

#define ABX_O_RDONLY    O_RDONLY
#define ABX_O_RDWR      O_RDWR
#define ABX_O_WRONLY    O_WRONLY
#define ABX_O_CREAT     O_CREAT
#define ABX_O_TRUNC     O_TRUNC
#define ABX_O_APPEND    O_APPEND
#define ABX_O_BINARY    0
#define ABX_O_EXCL      O_EXCL

// do NOT use S_IREAD & S_IWRITE since they are not defined correctly
#define ABX_S_IREAD     0
#define ABX_S_IWRITE    0


#else   // AVOS

#include <sys/stat.h>
//#include <direct.h>
//#include <io.h>
#include <fcntl.h>

#define abx_fileopen            _open
#define abx_fileclose(f)        _close(f)
#define abx_fileread(f,b,c)     _read((f),(b),(c))
#define abx_filewrite(f,b,c)    _write((f),(b),(c))
#define abx_filelseek(f,o,p)    _lseek((f),(o),(p))
#define abx_fileaccess(f,m)		_access((f),(m))
#define abx_mkdir(d, m)         mkdir(d)

#define ABX_O_RDONLY    _O_RDONLY
#define ABX_O_RDWR      _O_RDWR
#define ABX_O_WRONLY    _O_WRONLY
#define ABX_O_CREAT     _O_CREAT
#define ABX_O_TRUNC     _O_TRUNC
#define ABX_O_APPEND    _O_APPEND
#define ABX_O_BINARY    O_BINARY
#define ABX_O_EXCL      _O_EXCL
#define ABX_S_IREAD     _S_IREAD
#define ABX_S_IWRITE    _S_IWRITE

// todo get file size (maybe AVOS lacks _tell); flags have not been defined

#endif  //AVOS


abx_errid_t abx_copyfile(const char * src, const char * dst, bool fail_if_exist);

#endif
