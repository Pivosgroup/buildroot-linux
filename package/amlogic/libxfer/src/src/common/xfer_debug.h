/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Manager 
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#ifndef _XFER_DEBUG_H_
#define _XFER_DEBUG_H_


/* Log level, could be following defines */
extern int xfer_log_level;
void xfer_log_init();
void xfer_log_uninit();

#define XFER_LOG_MAIN 1
#define XFER_LOG_HTTP 2
#define XFER_LOG_THUNDER 4
#define XFER_LOG_DETAIL 8

#define XFER_TYPE_PRINTF 0
#define XFER_TYPE_FILE 1

#ifdef XFER_DEBUG
#define XFER_DEBUG_ENABLE 1
#else
#define XFER_DEBUG_ENABLE 0
#endif

#if (XFER_DEBUG_ENABLE)
#define debugf(...) xfer_log(__FILE__, __LINE__, __VA_ARGS__)
#else
#define debugf(arg,...)
#endif

#endif //_XFER_DEBUG_H_

