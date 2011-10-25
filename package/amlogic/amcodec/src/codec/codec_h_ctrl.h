
#ifndef CODEC_HEADER_H_H
#define CODEC_HEADER_H_H
#include <codec_type.h>
#include <codec_error.h>

#define CODEC_DEBUG

#ifdef CODEC_DEBUG
 #define CODEC_PRINT(f,s...)	fprintf(stderr,f,##s)
#else
 #define CODEC_PRINT(f,s...)
#endif

#define CODEC_VIDEO_ES_DEVICE		"/dev/amstream_vbuf"
#define CODEC_AUDIO_ES_DEVICE		"/dev/amstream_abuf"
#define CODEC_TS_DEVICE			"/dev/amstream_mpts"
#define CODEC_PS_DEVICE			"/dev/amstream_mpps"
#define CODEC_RM_DEVICE			"/dev/amstream_rm"
#define CODEC_CNTL_DEVICE         "/dev/amvideo"
#define CODEC_SUB_DEVICE          "/dev/amstream_sub"
#define CODEC_SUB_READ_DEVICE    "/dev/amstream_sub_read"

CODEC_HANDLE codec_h_open(const char *port_addr);
int codec_h_close(CODEC_HANDLE h);
int codec_h_write(CODEC_HANDLE ,void *,int);
int codec_h_read(CODEC_HANDLE,void *,int);
int codec_h_control(CODEC_HANDLE h,int cmd,unsigned long paramter);
 


#endif
