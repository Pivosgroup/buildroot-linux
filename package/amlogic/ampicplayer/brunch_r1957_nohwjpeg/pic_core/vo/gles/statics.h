/* vim:set sts=4 ts=4 noexpandtab: */
/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef __statics_h
#define __statics_h

#include "egl_runtime.h"



typedef struct
{
	EGLInfo sEGLInfo;
#ifdef WIN32
	HWND sWindow;
	MSG sMessage;
	HDC sDC;
#else /* WIN32 */
	fbdev_window *sWindow;
#endif /* WIN32 */
} Statics;



void terminateStatics();
void initializeStatics();
Statics *getStatics();
void setStatics(void *pStatics);



#endif /* __statics_h */

