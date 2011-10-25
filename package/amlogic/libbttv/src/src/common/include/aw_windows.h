/*****************************************************************
**                                                             	**
**  Copyright (C) 2004 Amlogic,Inc.                            	**
**  All rights reserved                                        	**
**        Filename : aw_windows.h /Project:AVOS  				** 
**        Revision : 1.0                                       	**
**                                                             	**
*****************************************************************/
#ifndef _AW_WINDOWS_H
#define _AW_WINDOWS_H

/**
 * @file aw_windows.h
 */

/*@{*/
 
#define AVMem_ufree 	AVMem_free
/*
#include "includes.h"
#include <listop.h>
#include "aw_define.h"
#include "aw_error.h"
#include "aw_typedef.h"
#include "aw_msg.h"
#include "aw_resource.h"
#include "aw_app.h"
#include "aw_command.h"
#include "aw_control.h"
#include "aw_input.h"
#include "aw_playlist.h"

#ifndef __ROM_
	#define ASSERT(a,b)	{\
    					OS_CPU_SR  cpu_sr;\
	                    OS_ENTER_CRITICAL(); \
	                    if(a) printf("ASSERT Failed, At %s \n", b); \
	                    OS_EXIT_CRITICAL();} 
	#define WARNNING(a)	{\
    					OS_CPU_SR  cpu_sr;\
	                    OS_ENTER_CRITICAL(); \
	                    printf("Warning : %s \n", a); \
	                    OS_EXIT_CRITICAL();}
	#define WARNNING_CATCH(a,b) {\
    					OS_CPU_SR  cpu_sr;\
	                    OS_ENTER_CRITICAL(); \
	                    if(a)	printf("Warning : %s \n", b); \
	                    OS_EXIT_CRITICAL();}
#else
	#define ASSERT(a,b)	
	#define WARNNING(a)
	#define WARNNING_CATCH(a,b)
#endif

void winmain(void *arg) ;
*/

/*@}*/
#endif //end of _AW_WINDOWS_H
