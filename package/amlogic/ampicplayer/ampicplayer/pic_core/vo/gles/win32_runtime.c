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
#include <config.h>

#include "win32_runtime.h"
#include "statics.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif /* DMALLOC */

/* createWindow():	Set up Windows specific bits.
 *
 * uiWidth:	 Width of window to create.
 * uiHeight:	Height of window to create.
 *
 * Returns:	 Device specific window handle.
 */
int createWindow(int uiWidth, int uiHeight)
{
	WNDCLASS wc;
	RECT wRect;
	HWND sWindow;
	HINSTANCE hInstance;
	Statics *pStatics = getStatics();

	wRect.left = 0L;
	wRect.right = (long)uiWidth;
	wRect.top = 0L;
	wRect.bottom = (long)uiHeight;

	hInstance = GetModuleHandle(NULL);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)processWindow;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OGLES";

	RegisterClass(&wc);

	AdjustWindowRectEx(&wRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE);

	sWindow = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE, "OGLES", "main", WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, wRect.right - wRect.left, wRect.bottom - wRect.top, NULL, NULL, hInstance, NULL);

	ShowWindow(sWindow, SW_SHOW);
	SetForegroundWindow(sWindow);
	SetFocus(sWindow);

	pStatics->sWindow = sWindow;
	pStatics->sDC = GetDC(sWindow);

	return 0;
}



/* Destroy the window. */
int destroyWindow(void)
{
	Statics *pStatics = getStatics();

	ReleaseDC(pStatics->sWindow, pStatics->sDC);
	DestroyWindow(pStatics->sWindow);

	return 0;
}



/* processWindow(): This function handles Windows callbacks.
 */
LRESULT CALLBACK processWindow(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg) {
		case WM_CLOSE:
				PostQuitMessage(0);
				return 0;

		case WM_ACTIVATE:
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SIZE:
				return 0;
	}

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}



/* Check for quit message, returning 1 if quit, 0 otherwise. */
int checkWindow()
{
	Statics *pStatics = getStatics();

	/* Do Windows stuff. */
	if(PeekMessage(&pStatics->sMessage, NULL, 0, 0, PM_REMOVE)) {
		if(pStatics->sMessage.message == WM_QUIT) {
			return 1;
		} else {
			TranslateMessage(&pStatics->sMessage);
			DispatchMessage(&pStatics->sMessage);
		}
	}

	return 0;
}

