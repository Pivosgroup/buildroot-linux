/*******************************************************************
 * 
 *  Copyright C 2004 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: 
 *
 *  Created: 
 *
 *******************************************************************/

#ifndef TYPEDEFINE_H
#define TYPEDEFINE_H

/**
 * @file typedefine.h Provided by avioapi.lib
 */


#ifdef SAPI_INIT
#undef  SAPI_EXTERN
#define SAPI_EXTERN
#else
#undef  SAPI_EXTERN
#define SAPI_EXTERN  extern
#endif
#define BYTE_ORDER LITTLE_ENDIAN
#define STATIC 
#define INLINE 
typedef unsigned unsigned32 ;
typedef unsigned char boolean ;
#define AVFS_INLINE_ROUTINE
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
//typedef int  ssize_t;

//typedef unsigned int mode_t;
//typedef unsigned long long dev_t;

//typedef  int time_t;
//typedef unsigned int nlink_t;

typedef int avfs_filesystem_node_types_t;


typedef unsigned int Objects_Id;
typedef unsigned char unsigned8;
typedef char signed8;
typedef unsigned short unsigned16;
typedef  short signed16;
typedef unsigned int avfs_unsigned32;
typedef unsigned char avfs_boolean;
typedef unsigned int avfs_id;
typedef unsigned int uint32_t;
typedef unsigned short INT16U;
typedef unsigned char INT8U;
typedef char INT8S;
typedef unsigned int INT32U;
typedef int INT32S;

//typedef int pid_t;


#define O_NDELAY 1

#define LINK_MAX 8

//#include "seterr.h"



#ifndef assert
#define assert(ignore) ((void)0)
#endif
//#define assert(E) ((E)? (void)0 : _ASSERT(#E, __FILE__, __LINE__))
extern mode_t avfs_filesystem_umask;

#ifndef PRId64
#define PRId64	"ld"
#endif 
typedef unsigned long long u64;
#endif
