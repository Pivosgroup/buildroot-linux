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

#ifndef __win32_runtime_h
#define __win32_runtime_h

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN 1
#endif /* WIN32_LEAN_AND_MEAN */

#include <windows.h>

int createWindow(int uiWidth, int uiHeight);
int destroyWindow(void);
LRESULT CALLBACK processWindow(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
int checkWindow(void);

#endif /* __win32_runtime_h */

