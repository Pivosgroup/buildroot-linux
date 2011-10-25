/*******************************************************************
* 
*  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
*
*  Description: the interface of ABoxBase time functions
*
*  Author: Sympeer Software
*
*******************************************************************/


#ifndef _ABX_TIME_H_
#define	_ABX_TIME_H_


#ifdef AVOS
#include <sys/time.h>
#endif



#ifdef AVOS
    #define abx_sleep(ms)       AVTimeDly(ms)
    #define abx_tickcount()     AVTimeGet()
#else   /* AVOS */
    #define abx_sleep(ms)       Sleep(ms)
    #define abx_tickcount()     GetTickCount()
#endif



#endif
