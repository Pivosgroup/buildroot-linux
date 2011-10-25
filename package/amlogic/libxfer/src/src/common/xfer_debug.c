/*******************************************************************
 * 
 *  Copyright (C) 2010 by Amlogic, Inc. All Rights Reserved.
 *
 *  Description: the interface of Transfer Manager 
 *
 *  Author: Peifu Jiang
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include "xfer_debug.h"

int xfer_log_level = 0;

/* 1: log into file; 0: just printf */
int xfer_log_type = 0;

FILE *xfer_log_fd = NULL;

void
xfer_log_init()
{
#if (XFER_DEBUG_ENABLE)
	char *env = NULL;

	env = getenv("XFER_LOG_LEVEL");
	printf("getenv(), XFER_LOG_LEVEL=%s\n", env);
	if(env == NULL)
		xfer_log_level = 0;
	else
		xfer_log_level = atoi(env);

	env = getenv("XFER_LOG_TYPE");
	printf("getenv(), XFER_LOG_TYPE=%s\n", env);
	if((env != NULL) && (strcmp(env, "1") == 0))
	{
		xfer_log_fd = fopen("xfer.log", "a");
		if(xfer_log_fd == NULL)
		{
			printf("XFER log file open ERROR!\n");
			xfer_log_type = 0;
			return;
		}
		xfer_log_type = 1;
	}
	else
		xfer_log_type = 0;

#endif
	return;
}

void
xfer_log_uninit()
{
#if (XFER_DEBUG_ENABLE)
	if(xfer_log_fd != NULL)
		fclose(xfer_log_fd);

	xfer_log_fd = NULL;
	xfer_log_type = 0;
	xfer_log_level = 0;
#endif
}

void xfer_log(const char *file, int line, unsigned int level, const char *format, ...)
{
#if (XFER_DEBUG_ENABLE)
	va_list arg_ptr;
	time_t now;
	struct tm *timenow;

	now = time(NULL);

	if (level & xfer_log_level)
	{
		va_start(arg_ptr, format);

		if ((xfer_log_type == 1) && xfer_log_fd)
		{
			if (xfer_log_level & 0x10)
				fprintf(xfer_log_fd, "[XFER][%ld]<%s:%d>", now, file, line);
			else
				fprintf(xfer_log_fd, "[XFER][%ld]", now);
			vfprintf(xfer_log_fd, format, arg_ptr);
		}
		else
		{
			if (xfer_log_level & 0x10)
				printf("[XFER][%ld]<%s:%d>", now, file, line);
			else
				printf("[XFER][%ld]", now);
			vprintf(format, arg_ptr);
		}

		va_end(arg_ptr);
	}
#endif
}

