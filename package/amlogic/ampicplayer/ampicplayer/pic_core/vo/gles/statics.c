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

#include <stdlib.h>
#include <stdio.h>
#include <config.h>

#include "statics.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif /* DMALLOC */



/* The single global variable, pointing to a structure of all other global data. */
static void *f_pStatics = NULL;



void terminateStatics()
{
	Statics *pStatic = getStatics();
	free(pStatic);
	setStatics(NULL);
}



void initializeStatics()
{
	Statics *pStatics = (Statics *)calloc(1, sizeof(Statics));
	if(pStatics == NULL)
	{
		fprintf(stderr, "Error: Out of memory at %s:%i\n", __FILE__, __LINE__);
		exit(1);
	}
	setStatics(pStatics);
}



Statics *getStatics()
{
	return f_pStatics;
}



void setStatics(void *pStatics)
{
	f_pStatics = pStatics;
}

