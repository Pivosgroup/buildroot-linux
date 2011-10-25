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

#ifndef __egl_runtime_h
#define __egl_runtime_h

#ifdef WIN32
#include "win32_runtime.h"
#endif /* WIN32 */

#include <EGL/egl.h>



#ifdef WIN32
#	define WINDOW_RED_SIZE		8
#	define WINDOW_GREEN_SIZE	8
#	define WINDOW_BLUE_SIZE		8
#	define WINDOW_BUFFER_SIZE	32
#else /* WIN32 */
#	define WINDOW_RED_SIZE		5
#	define WINDOW_GREEN_SIZE	6
#	define WINDOW_BLUE_SIZE		5
#	define WINDOW_BUFFER_SIZE	16
#endif /* WIN32 */



#define EGL_CHECK(x) \
	x; \
	{ \
		EGLint eglError = eglGetError(); \
		if(eglError != EGL_SUCCESS) { \
			fprintf(stderr, "eglGetError() = %i (0x%.8x) at %s:%i\n", eglError, eglError, __FILE__, __LINE__); \
			exit(1); \
		} \
	}



extern EGLint g_aEGLAttributes[];

static const EGLint aEGLContextAttributes[] =
{
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};



typedef struct
{
    EGLDisplay sEGLDisplay;
	EGLContext sEGLContext;
	EGLSurface sEGLSurface;
} EGLInfo;



int initializeEGL(void);
int terminateEGL(void);



#endif /* __egl_runtime_h */

