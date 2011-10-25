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

#include <stdio.h>
#include <stdlib.h>
#include <config.h>

#include "linux_runtime.h"
#include "statics.h"

/* createWindow():	Set up Linux specific bits.
 *
 * uiWidth:	 Width of window to create.
 * uiHeight:	Height of window to create.
 */
int createWindow(int uiWidth, int uiHeight)
{
	Statics *pStatics = getStatics();

	pStatics->sWindow = (fbdev_window *)calloc(1, sizeof(fbdev_window));
	if(pStatics->sWindow == NULL)
	{
		fprintf(stderr, "Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}
	pStatics->sWindow->width = uiWidth;
	pStatics->sWindow->height = uiHeight;

	return 0;
}



/* Destroy the window. */
int destroyWindow(void)
{
	Statics *pStatics = getStatics();

	free(pStatics->sWindow);

	return 0;
}

