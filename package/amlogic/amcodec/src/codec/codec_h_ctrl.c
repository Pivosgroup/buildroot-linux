/*
 * Amlogic  osd lib
 *
 * Copyright (C) 2009 Amlogic, Inc.
 *
 * author  : zhi.zhou@amlogic.com
 * version : 2.0
 */
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <codec_error.h>
#include <codec.h>
#include "codec_h_ctrl.h"

CODEC_HANDLE codec_h_open(const char *port_addr)
{
	int r;
    r=open(port_addr,O_WRONLY);
	if(r<0)
		{
			CODEC_PRINT("Init [%s] failed,ret = %d\n",port_addr,r);
			return r;
		}
	return (CODEC_HANDLE)r;
}

CODEC_HANDLE codec_h_open_rd(const char *port_addr)
{
	int r;
    r=open(port_addr,O_RDONLY);
	if(r<0)
		{
			CODEC_PRINT("Init [%s] failed,ret = %d\n",port_addr,r);
			return r;
		}
	return (CODEC_HANDLE)r;
}

int codec_h_close(CODEC_HANDLE h)
{
	if(h>=0)
		close(h);
	return 0;
}

int codec_h_control(CODEC_HANDLE h,int cmd,unsigned long paramter)
{
	int r;
	
	if(h<0)
		return -1;
	r=ioctl(h,cmd,paramter);		
	if(r<0)
	{
		CODEC_PRINT("send control failed,handle=%d,cmd=%x,paramter=%x, t=%x\n",h,cmd,paramter,r);
		return r;
	}
	return 0;	
}

int codec_h_read(CODEC_HANDLE handle,void *buffer,int size)
{
	int r;
	r=read(handle,buffer,size);    
	return r;
}

int codec_h_write(CODEC_HANDLE handle,void *buffer,int size)
{
	int r;
	r=write(handle,buffer,size);    
	return r;
}



