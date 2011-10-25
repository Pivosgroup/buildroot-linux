/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/

#ifndef _ABX_DBG_H_
#define	_ABX_DBG_H_

#include "abx_common.h"

#ifdef AVOS
    #include "Os_cfg.h"
    void do_nothing(...);
    #define abx_printf AVOS_printf
    #define	ABX_LOG						do_nothing
    #define	ABX_LOG3(s,p1,p2,p3)    abx_printf("%s %d : " s, __FILE__, __LINE__, p1,p2,p3)
#else
    #include <assert.h>
    #define abx_printf printf
    #define	ABX_LOG
    #define	ABX_LOG3(s,p1,p2,p3)    abx_printf("%s %d : " s, __FILE__, __LINE__, p1,p2,p3)
#endif



#ifdef AVOS
    #define	ABX_ASSERT(b)		assert(b)	//while(!(b)){ printf("ASSERT: %s %d : " #b "\n", __FILE__, __LINE__ ); AVTimeDly(OS_MILLIS_TO_TICKS(60 * 1000));}
#else
    #define ABX_ASSERT(b)       assert(b)
#endif

#endif //_ABX_DBG_H_
