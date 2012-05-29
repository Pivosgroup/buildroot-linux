/**
* @file codec_h_ctrl.c
* @brief functions of codec device handler operation
* @author Zhou Zhi <zhi.zhou@amlogic.com>
* @version 2.0.0
* @date 2011-02-24
*/
/* Copyright (C) 2007-2011, Amlogic Inc.
* All right reserved
* 
*/
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <codec_error.h>
#include <codec.h>
#include "codec_h_ctrl.h"

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_h_open  Open codec devices by file name 
*
* @param[in]  port_addr  File name of codec device
* @param[in]  flags      Open flags
*
* @return     The handler of codec device
*/
/* --------------------------------------------------------------------------*/
CODEC_HANDLE codec_h_open(const char *port_addr, int flags)
{
    int r;
    r = open(port_addr, flags);
    if (r < 0) {
        CODEC_PRINT("Init [%s] failed,ret = %d error=%d\n", port_addr, r, errno);
        return r;
    }
    return (CODEC_HANDLE)r;
}

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_h_open_rd  Open codec devices by file name in read_only mode
*
* @param[in]  port_addr  File name of codec device
*
* @return     THe handler of codec device
*/
/* --------------------------------------------------------------------------*/
CODEC_HANDLE codec_h_open_rd(const char *port_addr)
{
    int r;
    r = open(port_addr, O_RDONLY);
    if (r < 0) {
        CODEC_PRINT("Init [%s] failed,ret = %d errno=%d\n", port_addr, r, errno);
        return r;
    }
    return (CODEC_HANDLE)r;
}

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_h_close  Close codec devices
*
* @param[in]  h  Handler of codec device
*
* @return     0 for success 
*/
/* --------------------------------------------------------------------------*/
int codec_h_close(CODEC_HANDLE h)
{
	int r;
    if (h >= 0) {
        r = close(h);
		if (r < 0) {
        	CODEC_PRINT("close failed,handle=%d,ret=%d errno=%d\n", h, r, errno);  
    	}
    }
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_h_control  IOCTL commands for codec devices
*
* @param[in]  h         Codec device handler
* @param[in]  cmd       IOCTL commands
* @param[in]  paramter  IOCTL commands parameter
*
* @return     0 for success, non-0 for fail
*/
/* --------------------------------------------------------------------------*/
int codec_h_control(CODEC_HANDLE h, int cmd, unsigned long paramter)
{
    int r;

    if (h < 0) {
        return -1;
    }
    r = ioctl(h, cmd, paramter);
    if (r < 0) {
        CODEC_PRINT("send control failed,handle=%d,cmd=%x,paramter=%x, t=%x errno=%d\n", h, cmd, paramter, r, errno);
        return r;
    }
    return 0;
}

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_h_read  Read data from codec devices
*
* @param[in]   handle  Codec device handler
* @param[out]  buffer  Buffer for the data read from codec device
* @param[in]   size    Size of the data to be read
*
* @return      read length or fail if < 0
*/
/* --------------------------------------------------------------------------*/
int codec_h_read(CODEC_HANDLE handle, void *buffer, int size)
{
    int r;
    r = read(handle, buffer, size);
	if (r < 0) {
        CODEC_PRINT("read failed,handle=%d,ret=%d errno=%d\n", handle, r, errno);  
    }
    return r;
}

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_h_write  Write data to codec devices
*
* @param[in]   handle  Codec device handler
* @param[out]  buffer  Buffer for the data to be written to codec device
* @param[in]   size    Size of the data to be written
*
* @return      write length or fail if < 0
*/
/* --------------------------------------------------------------------------*/
int codec_h_write(CODEC_HANDLE handle, void *buffer, int size)
{
    int r;
    r = write(handle, buffer, size);
	if (r < 0 && errno != EAGAIN) {
        CODEC_PRINT("write failed,handle=%d,ret=%d errno=%d\n", handle, r, errno);  
    }
    return r;
}



