/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/


#ifndef _ABX_COMMON_H_
#define _ABX_COMMON_H_

#include <includes.h>
#ifdef AVOS
    #include "Os_cfg.h"
    #include "Stdint.h"
    #include <sys/socket.h>
#endif


#ifndef INT32U
    #define INT32U unsigned int
#endif

#ifdef _MSC_VER /* MinGW already has these */
    typedef unsigned short      uint16_t;
    typedef unsigned int        uint32_t;
    typedef int ssize_t;
    typedef int socklen_t;
    typedef unsigned __int64 u_int64_t;
    typedef unsigned __int8 u_int8_t;
#endif /* _MSC_VER */


typedef int filesize_t;


#ifdef AVOS
#ifndef SOCKET
    typedef int SOCKET;
#endif  /* SOCKET */
#endif  /* AVOS */

#if !defined(__BOOL_DEFINED)
#ifndef QT
    typedef int bool;
#endif
    typedef int BOOL;
#endif

#ifndef true
    #define true 1
#endif

#ifndef false
    #define false 0
#endif

#endif
