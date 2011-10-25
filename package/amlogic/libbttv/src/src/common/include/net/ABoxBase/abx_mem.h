/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#ifndef	_ABX_MEM_H_
#define	_ABX_MEM_H_

#include "abx_common.h"
#include <malloc.h>

#ifdef AVOS
    #include "avmalloc.h"
#endif


void abx_mem_init(); //must be called before use abx_ memory functions
void abx_mem_exit();

#ifndef _WIN32
//#define		ABOX_MEM_CHECK_EN
#endif

#ifdef ABOX_MEM_CHECK_EN

void *abx_mem_check_malloc( size_t size, const char* file, INT16U line);
void *abx_mem_check_calloc( size_t nmemb, size_t size, const char* file, INT16U line);
void *abx_mem_check_realloc( void *buf, size_t size, const char* file, INT16U line);
void abx_mem_check_free( void *buf);
INT32U abx_mem_get_used();

#define abx_malloc(size)			abx_mem_check_malloc(size, __FILE__,__LINE__)
#define abx_calloc(n, size)		abx_mem_check_calloc(n, size, __FILE__,__LINE__)
#define abx_realloc(p, size) 		abx_mem_check_realloc(p, size, __FILE__,__LINE__)
#define abx_free(p)				abx_mem_check_free(p)


#else //ABOX_MEM_CHECK_EN



#ifdef ABOX_MEM_ALLOC
    void *abx_mem_malloc( size_t size);
    void *abx_mem_calloc( size_t nmemb, size_t size);
    void *abx_mem_realloc( void *buf, size_t size);
    void abx_mem_free( void *buf);

    #define	abx_malloc		abx_mem_malloc
    #define	abx_calloc		abx_mem_calloc
    #define	abx_realloc		abx_mem_realloc
    #define	abx_free		abx_mem_free
#else   // ABOX_MEM_ALLOC
#ifdef AVOS
    #define	abx_malloc		AVMem_kmalloc
    #define	abx_calloc		AVMem_kcalloc
    #define	abx_realloc     AVMem_krealloc
    #define	abx_free		AVMem_free
#else   // AVOS
    #define abx_malloc      malloc
    #define abx_calloc      calloc
    #define abx_realloc     realloc
    #define abx_free        free
#endif  // AVOS
#endif  // ABOX_MEM_ALLOC

#endif //ABOX_MEM_CHECK_EN

#endif  //ABX_MEM_H

