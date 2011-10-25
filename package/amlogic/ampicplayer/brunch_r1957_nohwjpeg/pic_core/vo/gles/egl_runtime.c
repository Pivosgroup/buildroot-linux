/* vim: set sts=4 ts=4 noexpandtab: */
/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2009 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifdef WIN32
#include "win32_runtime.h"
#endif /* WIN32 */

#include <EGL/egl.h>

#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#include "egl_runtime.h"
#include "statics.h"
#include "gles_common.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif /* DMALLOC */



EGLint g_aEGLAttributes[] =
{
	EGL_SAMPLES,			0,
	EGL_RED_SIZE,			WINDOW_RED_SIZE,
	EGL_GREEN_SIZE,			WINDOW_GREEN_SIZE,
	EGL_BLUE_SIZE,			WINDOW_BLUE_SIZE,
	EGL_ALPHA_SIZE,			0,
	EGL_BUFFER_SIZE,		WINDOW_BUFFER_SIZE,
	EGL_DEPTH_SIZE,			16,
	EGL_STENCIL_SIZE,		0,
	EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
	EGL_RENDERABLE_TYPE,	EGL_OPENGL_ES2_BIT,
	EGL_NONE
};



int initializeEGL()
{
	Statics *pStatics = getStatics();
	EGLConfig *pEGLConfig = NULL;
	EGLint cEGLConfigs = 0;
	int iEGLConfig = 0;
	EGLBoolean bResult = EGL_FALSE;

#ifdef WIN32
    pStatics->sEGLInfo.sEGLDisplay = eglGetDisplay(pStatics->sDC);
#else /* WIN32 */
	pStatics->sEGLInfo.sEGLDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
#endif /* WIN32 */

	if(pStatics->sEGLInfo.sEGLDisplay == EGL_NO_DISPLAY)
	{
		fprintf(stderr, "Error: No EGL Display available at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Initialize EGL. */
    bResult = eglInitialize(pStatics->sEGLInfo.sEGLDisplay, NULL, NULL);
	if(bResult != EGL_TRUE)
	{
		EGLint iError = eglGetError();
		fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		fprintf(stderr, "Error: Failed to initialize EGL at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Enumerate available EGL configurations which match or exceed our required attribute list. */
	bResult = eglChooseConfig(pStatics->sEGLInfo.sEGLDisplay, g_aEGLAttributes, NULL, 0, &cEGLConfigs);
	if(bResult != EGL_TRUE)
	{
		EGLint iError = eglGetError();
		fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		fprintf(stderr, "Error: Failed to enumerate EGL configs at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Allocate space for all EGL configs available and get them. */
	pEGLConfig = (EGLConfig *)calloc(cEGLConfigs, sizeof(EGLConfig));
	if(pEGLConfig == NULL)
	{
		fprintf(stderr, "Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}
	bResult = eglChooseConfig(pStatics->sEGLInfo.sEGLDisplay, g_aEGLAttributes, pEGLConfig, cEGLConfigs, &cEGLConfigs);
	if(bResult != EGL_TRUE)
	{
		EGLint iError = eglGetError();
		fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		fprintf(stderr, "Error: Failed to enumerate EGL configs at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Loop through the EGL configs to find a color depth match.
	 * NB This is necessary, since EGL considers a higher color depth than requested to be 'better'
	 * even though this may force the driver to use a slow color conversion blitting routine. */
	for(iEGLConfig = 0; iEGLConfig < cEGLConfigs; iEGLConfig ++)
	{
		EGLint iEGLValue = 0;

		bResult = eglGetConfigAttrib(pStatics->sEGLInfo.sEGLDisplay, pEGLConfig[iEGLConfig], EGL_RED_SIZE, &iEGLValue);
		if(bResult != EGL_TRUE)
		{
			EGLint iError = eglGetError();
			fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
			fprintf(stderr, "Error: Failed to get EGL attribute at %s:%i\n", __FILE__, __LINE__);
			exit(1);
		}

		if(iEGLValue == WINDOW_RED_SIZE)
		{
			bResult = eglGetConfigAttrib(pStatics->sEGLInfo.sEGLDisplay, pEGLConfig[iEGLConfig], EGL_GREEN_SIZE, &iEGLValue);
			if(bResult != EGL_TRUE)
			{
				EGLint iError = eglGetError();
				fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
				fprintf(stderr, "Error: Failed to get EGL attribute at %s:%i\n", __FILE__, __LINE__);
				exit(1);
			}

			if(iEGLValue == WINDOW_GREEN_SIZE)
			{
				bResult = eglGetConfigAttrib(pStatics->sEGLInfo.sEGLDisplay, pEGLConfig[iEGLConfig], EGL_BLUE_SIZE, &iEGLValue);
				if(bResult != EGL_TRUE)
				{
					EGLint iError = eglGetError();
					fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
					fprintf(stderr, "Error: Failed to get EGL attribute at %s:%i\n", __FILE__, __LINE__);
					exit(1);
				}

				if(iEGLValue == WINDOW_BLUE_SIZE) break;
			}
		}
	}
	if(iEGLConfig >= cEGLConfigs)
	{
		fprintf(stderr, "Error: Failed to find matching EGL config at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Create a surface. */
	pStatics->sEGLInfo.sEGLSurface = eglCreateWindowSurface(pStatics->sEGLInfo.sEGLDisplay, pEGLConfig[iEGLConfig], (EGLNativeWindowType)(pStatics->sWindow), NULL);
	if(pStatics->sEGLInfo.sEGLSurface == EGL_NO_SURFACE)
	{
		EGLint iError = eglGetError();
		fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		fprintf(stderr, "Error: Failed to create EGL surface at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Create context. */
	pStatics->sEGLInfo.sEGLContext = eglCreateContext(pStatics->sEGLInfo.sEGLDisplay, pEGLConfig[iEGLConfig], EGL_NO_CONTEXT, aEGLContextAttributes);
	if(pStatics->sEGLInfo.sEGLContext == EGL_NO_CONTEXT)
	{
		EGLint iError = eglGetError();
		fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		fprintf(stderr, "Error: Failed to create EGL context at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	/* Make context current. */
	bResult = eglMakeCurrent(pStatics->sEGLInfo.sEGLDisplay, pStatics->sEGLInfo.sEGLSurface, pStatics->sEGLInfo.sEGLSurface, pStatics->sEGLInfo.sEGLContext);
	if(bResult == EGL_FALSE)
	{
		EGLint iError = eglGetError();
		fprintf(stderr, "eglGetError(): %i (0x%.4x)\n", (int)iError, (int)iError);
		fprintf(stderr, "Error: Failed to make context current at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}

	free(pEGLConfig);
	pEGLConfig = NULL;

	return 0;
}

int terminateEGL()
{
	Statics *pStatics = getStatics();

	/* Shut down OpenGL-ES. */
	eglMakeCurrent(pStatics->sEGLInfo.sEGLDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, pStatics->sEGLInfo.sEGLContext);
	eglDestroyContext(pStatics->sEGLInfo.sEGLDisplay, pStatics->sEGLInfo.sEGLContext);
	eglDestroySurface(pStatics->sEGLInfo.sEGLDisplay, pStatics->sEGLInfo.sEGLSurface);
    eglTerminate(pStatics->sEGLInfo.sEGLDisplay);

	return 0;
}

