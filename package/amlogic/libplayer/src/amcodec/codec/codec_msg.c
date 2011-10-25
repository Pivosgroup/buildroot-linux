/**
* @file codec_msg.c
* @brief  Codec message covertion functions
* @author Zhang Chen <chen.zhang@amlogic.com>
* @version 1.0.0
* @date 2011-02-24
*/
/* Copyright (C) 2007-2011, Amlogic Inc.
* All right reserved
* 
*/
#include <stdio.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <codec_error.h>
#include <codec.h>
#include "codec_h_ctrl.h"


/* --------------------------------------------------------------------------*/
/**
* @brief  system_error_to_codec_error  Convert system error to codec error types
*
* @param[in]  error  System error to be converted
*
* @return     Codec error type
*/
/* --------------------------------------------------------------------------*/
int system_error_to_codec_error(int error)
{
    switch (error) {
    case 0:
        return CODEC_ERROR_NONE;
    case EBUSY:
        return -CODEC_ERROR_BUSY;
    case ENOMEM:
        return -CODEC_ERROR_NOMEM;
    case ENODEV:
        return -CODEC_ERROR_IO;
    default:
        return -CODEC_ERROR_INVAL;
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief  codec_error_msg  Convert codec type to error message
*
* @param[in]  error  Codec error type
*
* @return     Error message string
*/
/* --------------------------------------------------------------------------*/
const char * codec_error_msg(int error)
{
    switch (error) {
    case CODEC_ERROR_NONE:
        return "codec no errors";
    case -CODEC_ERROR_INVAL:
        return "invalid handle or parameter";
    case -CODEC_ERROR_BUSY:
        return "codec is busy";
    case -CODEC_ERROR_NOMEM:
        return "no enough memory for codec";
    case -CODEC_ERROR_IO:
        return "codec io error";
    case -CODEC_ERROR_PARAMETER:
        return "Parameters error";
    case -CODEC_ERROR_AUDIO_TYPE_UNKNOW:
        return "Audio Type error";
    case -CODEC_ERROR_VIDEO_TYPE_UNKNOW:
        return "Video Type error";
    case -CODEC_ERROR_STREAM_TYPE_UNKNOW:
        return "Stream Type error";
    case -CODEC_ERROR_INIT_FAILED:
        return "Codec init failed";
    case -CODEC_ERROR_SET_BUFSIZE_FAILED:
        return "Codec change buffer size failed";
    default:
        return "invalid operate";
    }
}

/* --------------------------------------------------------------------------*/
/**
* @brief  print_error_msg  Print error message in uniform format
*
* @param[in]  error  Codec error type
* @param[in]  func   Function where error happens
* @param[in]  line   Line where error happens
*/
/* --------------------------------------------------------------------------*/
void print_error_msg(int error, char *func, int line)
{
    CODEC_PRINT("Error=%x,%s,func=%s,line=%d\n", error, codec_error_msg(error), func, line);
}


