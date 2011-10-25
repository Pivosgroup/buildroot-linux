/*
 * Amlogic codec ctrl lib
 *
 * Copyright (C) 2010 Amlogic, Inc.
 *
 * author  :  zhouzhi
 * version : 1.0
 */
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <codec_error.h>
#include <codec_type.h>
#include <codec.h>
#include <audio_priv.h>
#include "codec_h_ctrl.h"

#define SUBTITLE_EVENT
#define TS_PACKET_SIZE 188

static int codec_change_buf_size(codec_para_t *pcodec)
{
	int r;
	if(pcodec->abuf_size>0)
		{
		r=codec_h_control(pcodec->handle,AMSTREAM_IOC_AB_SIZE,pcodec->abuf_size);
		if(r<0)
			return system_error_to_codec_error(r);
		}
	if(pcodec->vbuf_size>0)
		{
		r=codec_h_control(pcodec->handle,AMSTREAM_IOC_VB_SIZE,pcodec->vbuf_size);
		if(r<0)
			return system_error_to_codec_error(r);
		}	
	return CODEC_ERROR_NONE;
}
static  int set_video_format( codec_para_t *pcodec)
{
	int format=pcodec->video_type;
	int r;
	
	if(format<0 || format>VFORMAT_MAX)
		return -CODEC_ERROR_VIDEO_TYPE_UNKNOW;
	
	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_VFORMAT,format);	
	if(pcodec->video_pid>=0)
	{
    	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_VID,pcodec->video_pid);	
    	if(r<0)
    		return system_error_to_codec_error(r);
	}
	if(r<0)
		return system_error_to_codec_error(r);
	return 0;
}

static  int set_video_codec_info( codec_para_t *pcodec)
{
	dec_sysinfo_t am_sysinfo = pcodec->am_sysinfo;
	int r;
	
	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_SYSINFO,(unsigned long)&am_sysinfo);			
	if(r<0)
		return system_error_to_codec_error(r);
	return 0;
}
static  int set_audio_format(codec_para_t *pcodec)
{
	int format=pcodec->audio_type;
	int r;
	int codec_r;
    
	if(format<0 || format>AFORMAT_MAX)
		return -CODEC_ERROR_AUDIO_TYPE_UNKNOW;
	
	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_AFORMAT,format);	
	if(r<0)
	{
        codec_r = system_error_to_codec_error(r);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;  
	}
	if(pcodec->audio_pid>=0)
	{
    	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_AID,pcodec->audio_pid);	
    	if(r<0)
    	{
            codec_r = system_error_to_codec_error(r);
            print_error_msg(codec_r, __FUNCTION__, __LINE__);
    		return codec_r;  
    	}
	}
	if(pcodec->audio_samplerate>0)
	{
    	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_SAMPLERATE,pcodec->audio_samplerate);	
    	if(r<0)
    	{
            codec_r = system_error_to_codec_error(r);
            print_error_msg(codec_r, __FUNCTION__, __LINE__);
    		return codec_r;  
    	}
	}
	if(pcodec->audio_channels>0)
	{
    	r=codec_h_control(pcodec->handle,AMSTREAM_IOC_ACHANNEL,pcodec->audio_channels);	
    	if(r<0)
    	{
            codec_r = system_error_to_codec_error(r);
            print_error_msg(codec_r, __FUNCTION__, __LINE__);
    		return codec_r;  
    	}
	}
	return 0;
}

static int set_audio_info(codec_para_t *pcodec)
{
    int r;
    int codec_r;
    audio_info_t *audio_info = &pcodec->audio_info;
    CODEC_PRINT("set_audio_info\n");
    r=codec_h_control(pcodec->handle,AMSTREAM_IOC_AUDIO_INFO,audio_info);	
	if(r<0)
	{
        codec_r = system_error_to_codec_error(r);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;  
	}  
    return 0;
}

static int set_sub_format(codec_para_t *pcodec)
{
    int r;

    if(pcodec->sub_pid>=0)
	{
        r=codec_h_control(pcodec->handle,AMSTREAM_IOC_SID,pcodec->sub_pid);	
        if(r<0)
            return system_error_to_codec_error(r);

        r=codec_h_control(pcodec->handle,AMSTREAM_IOC_SUB_TYPE,pcodec->sub_type);
        if(r<0)
            return system_error_to_codec_error(r);
    }
    
    return 0;
}

static int set_ts_skip_byte(codec_para_t *pcodec)
{
    int r, skip_byte;

    skip_byte = pcodec->packet_size - TS_PACKET_SIZE;

    if (skip_byte < 0)
        skip_byte = 0;
    
    r=codec_h_control(pcodec->handle, AMSTREAM_IOC_TS_SKIPBYTE, skip_byte);
    if(r<0)
        return system_error_to_codec_error(r);
    
    return 0;
}

static inline int codec_video_es_init(codec_para_t *pcodec)
{
	CODEC_HANDLE handle;
	int r;
    int codec_r;
	if(!pcodec->has_video)
		return CODEC_ERROR_NONE;
	
	handle=codec_h_open(CODEC_VIDEO_ES_DEVICE);	
	if(handle<0)
	{   
        codec_r = system_error_to_codec_error(handle);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;
	}
	pcodec->handle=handle;
	r=set_video_format(pcodec);	
	if(r<0)
	{
		codec_h_close(handle);
        codec_r = system_error_to_codec_error(r);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;
	}	
	r=set_video_codec_info(pcodec);	
	if(r<0)
	{
		codec_h_close(handle);
        codec_r = system_error_to_codec_error(r);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;        
	}
	return CODEC_ERROR_NONE;
}


static inline int codec_audio_es_init(codec_para_t *pcodec)
{
	CODEC_HANDLE handle;
	int r;
    int codec_r;
	if(!pcodec->has_audio)
		return CODEC_ERROR_NONE;
		
	handle=codec_h_open(CODEC_AUDIO_ES_DEVICE);
	if(handle<0)
	{
		codec_r = system_error_to_codec_error(handle);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;   
	}
	pcodec->handle=handle;
	r=set_audio_format(pcodec);
	if(r<0)
	{
		codec_h_close(handle);
		codec_r = system_error_to_codec_error(r);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;   
	}

	if((pcodec->audio_type == AFORMAT_ADPCM)||(pcodec->audio_type == AFORMAT_WMA))
		{
			r=set_audio_info(pcodec);
        		if(r<0)
        		{
            			codec_h_close(handle);
				codec_r = system_error_to_codec_error(r);
       			print_error_msg(codec_r, __FUNCTION__, __LINE__);
				return codec_r;
        		}
		}
	
	return CODEC_ERROR_NONE;
}

static inline int codec_sub_es_init(codec_para_t *pcodec)
{
#ifdef SUBTITLE_EVENT
    int r, codec_r;

    if(pcodec->has_sub)
    {        
        r=codec_init_sub(pcodec);
        if(r<0)
        {
            codec_r = system_error_to_codec_error(r);
            print_error_msg(codec_r, __FUNCTION__, __LINE__);
	        return codec_r;
        }
        pcodec->handle = pcodec->sub_handle;

        pcodec->sub_pid = 0xffff; // for es, sub id is identified for es parser
        r=set_sub_format(pcodec);
        if(r<0)
        {
            codec_r = system_error_to_codec_error(r);
            print_error_msg(codec_r, __FUNCTION__, __LINE__);
	        return codec_r;
        }

    }

#endif
	
	return CODEC_ERROR_NONE;
}

static inline int codec_ps_init(codec_para_t *pcodec)
{
	CODEC_HANDLE handle;
	int r;
    int codec_r;
	if(!((pcodec->has_video && IS_VALID_PID(pcodec->video_pid)) ||
	     (pcodec->has_audio&& IS_VALID_PID(pcodec->audio_pid))))
	     return -CODEC_ERROR_PARAMETER;
	
	handle=codec_h_open(CODEC_PS_DEVICE);
	if(handle<0)
	{
		codec_r = system_error_to_codec_error(handle);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;   
	}
	pcodec->handle=handle;
	if(pcodec->has_video)
	{
		r=set_video_format(pcodec);
		if(r<0)
		{
			goto error1;
		}
		if(pcodec->video_type == VFORMAT_H264)
		{
			r=set_video_codec_info(pcodec);	
			if(r<0)
			{
				/*codec_h_close(handle);
				codec_r = system_error_to_codec_error(r);
                print_error_msg(codec_r, __FUNCTION__, __LINE__);
        		return codec_r; */
        		goto error1;
			}	
		}
	}
	if(pcodec->has_audio)
	{
		r=set_audio_format(pcodec);
		if(r<0)
		{
			goto error1;
		}
	}
#ifdef SUBTITLE_EVENT
    if(pcodec->has_sub)
    {
        r=set_sub_format(pcodec);
        if(r<0)
            goto error1;
        
        r=codec_init_sub(pcodec);
        if(r<0)
            goto error1;
    }
#endif
	
	return CODEC_ERROR_NONE;
error1:
	codec_h_close(handle);
	codec_r = system_error_to_codec_error(r);
    print_error_msg(codec_r, __FUNCTION__, __LINE__);
	return codec_r;   
	
}


static inline int codec_ts_init(codec_para_t *pcodec)
{
	CODEC_HANDLE handle;
	int r;
    int codec_r;
	if(!((pcodec->has_video && IS_VALID_PID(pcodec->video_pid)) ||
	     (pcodec->has_audio&& IS_VALID_PID(pcodec->audio_pid))))
	     return -CODEC_ERROR_PARAMETER;
	
	handle=codec_h_open(CODEC_TS_DEVICE);
	if(handle<0)
	{
		codec_r = system_error_to_codec_error(handle);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;   
	}
	pcodec->handle=handle;
	if(pcodec->has_video)
	{
		r=set_video_format(pcodec);
		if(r<0)
		{
			goto error1;
		}
		if((pcodec->video_type == VFORMAT_H264) || (pcodec->video_type == VFORMAT_VC1))
		{
			r=set_video_codec_info(pcodec);	
			if(r<0)
			{
				codec_h_close(handle);
				codec_r = system_error_to_codec_error(r);
                print_error_msg(codec_r, __FUNCTION__, __LINE__);
        		return codec_r;   
			}	
		}
	}
	if(pcodec->has_audio)
	{
		r=set_audio_format(pcodec);
		if(r<0)
		{
			goto error1;
		}

		if(pcodec->audio_type == AFORMAT_ADPCM)
		{
			r=set_audio_info(pcodec);
        		if(r<0)
            			goto error1;
		}
	}
    
    r=set_ts_skip_byte(pcodec);
    if(r<0)
    {
        goto error1;
    }

#ifdef SUBTITLE_EVENT
    if(pcodec->has_sub)
    {
        r=set_sub_format(pcodec);
        if(r<0)
            goto error1;
        
        r=codec_init_sub(pcodec);
        if(r<0)
            goto error1;
    }
#endif
	return CODEC_ERROR_NONE;
error1:
	codec_h_close(handle);
	codec_r = system_error_to_codec_error(r);
    print_error_msg(codec_r, __FUNCTION__, __LINE__);
	return codec_r;   
	
}

static inline int codec_rm_init(codec_para_t *pcodec)
{
    CODEC_HANDLE handle;
    int r;
    int codec_r;
    if(!((pcodec->has_video && IS_VALID_PID(pcodec->video_pid)) ||
        (pcodec->has_audio&& IS_VALID_PID(pcodec->audio_pid))))
    {
        CODEC_PRINT("codec_rm_init failed! video=%d vpid=%d audio=%d apid=%d\n",pcodec->has_video,pcodec->video_pid,pcodec->has_audio,pcodec->audio_pid);
        return -CODEC_ERROR_PARAMETER;
    }

    handle=codec_h_open(CODEC_RM_DEVICE);
    if(handle<0)
    {
        codec_r = system_error_to_codec_error(handle);
        print_error_msg(codec_r, __FUNCTION__, __LINE__);
		return codec_r;   
    }
    
    pcodec->handle=handle;
    if(pcodec->has_video)
    {
        r=set_video_format(pcodec);
        if(r<0)
        {
            goto error1;
        }

        r=set_video_codec_info(pcodec);
        if(r<0)
        {
            goto error1;
        }
    }
    if(pcodec->has_audio)
    {
        r=set_audio_format(pcodec);
        if(r<0)
        {
            goto error1;
        }
        r=set_audio_info(pcodec);
        if(r<0)
            goto error1;
    }
    return CODEC_ERROR_NONE;

error1:
    codec_h_close(handle);
    codec_r = system_error_to_codec_error(r);
    print_error_msg(codec_r, __FUNCTION__, __LINE__);
	return codec_r;   
}

int codec_init(codec_para_t *pcodec)
{
	int ret;
	//if(pcodec->has_audio)
	//	audio_stop();
	CODEC_PRINT("[%s:%d]stream_type=%d\n",__FUNCTION__,__LINE__,pcodec->stream_type);
	switch(pcodec->stream_type)
	{
		case STREAM_TYPE_ES_VIDEO:
			ret=codec_video_es_init(pcodec);
			break;
		case STREAM_TYPE_ES_AUDIO:
			ret=codec_audio_es_init(pcodec);
			break;
        case STREAM_TYPE_ES_SUB:
            ret=codec_sub_es_init(pcodec);
            break;
		case STREAM_TYPE_PS:
			ret=codec_ps_init(pcodec);
			break;
		case STREAM_TYPE_TS:
			ret=codec_ts_init(pcodec);
			break;
		case STREAM_TYPE_RM:
			ret=codec_rm_init(pcodec);
			break;
		case STREAM_TYPE_UNKNOW:
		default:
			return -CODEC_ERROR_STREAM_TYPE_UNKNOW;
	}
	if(ret!=0)
	{
		return ret;
	}
	ret=codec_change_buf_size(pcodec);
	if(ret!=0)
	{
		return -CODEC_ERROR_SET_BUFSIZE_FAILED;
	}
	ret=codec_h_control(pcodec->handle,AMSTREAM_IOC_PORT_INIT,0);
	if(ret!=0)
	{
		return -CODEC_ERROR_INIT_FAILED;
	}
	if(pcodec->has_audio)
		audio_start();
	return ret;
}

int codec_write(codec_para_t *pcodec,void *buffer,int len)
{    
	return codec_h_write(pcodec->handle,buffer,len);    
}
int codec_read(codec_para_t *pcodec,void *buffer,int len)
{
	return codec_h_read(pcodec->handle,buffer,len);	
}

int codec_close(codec_para_t *pcodec)
{
	if(pcodec->has_audio)
		audio_stop();
	return codec_h_close(pcodec->handle);	
}

void codec_close_audio(codec_para_t *pcodec)
{
    if(pcodec)
        pcodec->has_audio = 0;  
    audio_stop();
    return;
}

void codec_resume_audio(codec_para_t *pcodec, unsigned int orig)
{
    pcodec->has_audio = orig;
    if (pcodec->has_audio)
        audio_start();
    return;
}

int codec_checkin_pts(codec_para_t *pcodec, unsigned long pts)
{
    //CODEC_PRINT("[%s:%d]pts=%x(%d)\n",__FUNCTION__,__LINE__,pts,pts/90000);
	return codec_h_control(pcodec->handle, AMSTREAM_IOC_TSTAMP, pts);
}

int codec_get_vbuf_state(codec_para_t *p,struct buf_status *buf)
{
    int r;    
    struct am_io_param am_io;
    r=codec_h_control(p->handle,AMSTREAM_IOC_VB_STATUS,(unsigned long)&am_io);
    memcpy(buf,&am_io.status,sizeof(*buf));
    return system_error_to_codec_error(r);
}
int codec_get_abuf_state(codec_para_t *p,struct buf_status *buf)
{
	int r;
	struct am_io_param am_io;
	r=codec_h_control(p->handle,AMSTREAM_IOC_AB_STATUS,(unsigned long)&am_io);
	memcpy(buf,&am_io.status,sizeof(*buf));
	return system_error_to_codec_error(r);
}
int codec_get_vdec_state(codec_para_t *p, struct vdec_status *vdec)
{
	int r;
	struct am_io_param am_io;
	r=codec_h_control(p->handle,AMSTREAM_IOC_VDECSTAT,(unsigned long)&am_io);
	if(r<0)
		CODEC_PRINT("[codec_get_vdec_state]error[%d]: %s\n",r,codec_error_msg(system_error_to_codec_error(r)));
	memcpy(vdec,&am_io.vstatus,sizeof(*vdec));
	return system_error_to_codec_error(r);	
}
int codec_get_adec_state(codec_para_t *p,struct adec_status *adec)
{
	int r;
	struct am_io_param am_io;
	r=codec_h_control(p->handle,AMSTREAM_IOC_ADECSTAT,(unsigned long)&am_io);
	memcpy(adec,&am_io.astatus,sizeof(*adec));
	return system_error_to_codec_error(r);
}

int codec_pause(codec_para_t *p)
{
    if (p)
    {
        codec_h_control(p->cntl_handle, AMSTREAM_IOC_VPAUSE, 1);
    }
    else
	{
	    audio_pause();
    }
}
int codec_resume(codec_para_t *p)
{	
    if (p)
    {
        codec_h_control(p->cntl_handle, AMSTREAM_IOC_VPAUSE, 0);
    }
    else
	{
	    audio_resume();
    }
}
int codec_reset(codec_para_t *p)
{
	int ret;    
	ret=codec_close(p);   
    if(ret!=0)
        return ret;
	ret=codec_init(p);    
    CODEC_PRINT("[%s:%d]ret=%x\n",__FUNCTION__, __LINE__,ret);
	return system_error_to_codec_error(ret); 
}

int codec_init_sub(codec_para_t *pcodec)
{
    CODEC_HANDLE sub_handle;

    sub_handle = codec_h_open(CODEC_SUB_DEVICE);
    if (sub_handle < 0)
    {
        CODEC_PRINT("get %s failed\n", CODEC_SUB_DEVICE);
        return system_error_to_codec_error(sub_handle);
    }

    pcodec->sub_handle = sub_handle;
    return CODEC_ERROR_NONE;
}

int codec_open_sub_read(void)
{
    CODEC_HANDLE sub_handle;

    sub_handle = codec_h_open_rd(CODEC_SUB_READ_DEVICE);
    if (sub_handle < 0)
    {
        CODEC_PRINT("get %s failed\n", CODEC_SUB_READ_DEVICE);
        return system_error_to_codec_error(sub_handle);
    }

    return sub_handle;
}

int codec_close_sub(codec_para_t *pcodec)
{
    if (pcodec)
        return codec_h_close(pcodec->sub_handle);
}

int codec_close_sub_fd(CODEC_HANDLE sub_fd)
{
    return codec_h_close(sub_fd);
}

int codec_poll_sub(codec_para_t *pcodec)
{
    struct pollfd sub_poll_fd[1];

    if (pcodec->sub_handle == 0)
    {
        return 0;
    }
    
    sub_poll_fd[0].fd = pcodec->sub_handle;
    sub_poll_fd[0].events = POLLOUT;

    return poll(sub_poll_fd, 1, 10);    
}

int codec_poll_sub_fd(CODEC_HANDLE sub_fd, int timeout)
{
    struct pollfd sub_poll_fd[1];

    if (sub_fd <= 0)
    {
        return 0;
    }
    
    sub_poll_fd[0].fd = sub_fd;
    sub_poll_fd[0].events = POLLOUT;

    return poll(sub_poll_fd, 1, timeout);    
}

int codec_get_sub_size(codec_para_t *pcodec)
{
    int sub_size, r;
    
    if (pcodec->sub_handle == 0)
    {
        CODEC_PRINT("no control handler\n");
        return 0;
    }

    r=codec_h_control(pcodec->sub_handle,AMSTREAM_IOC_SUB_LENGTH,(unsigned long)&sub_size);
    if(r<0)
        return system_error_to_codec_error(r);
    else
        return sub_size;
}

int codec_get_sub_size_fd(CODEC_HANDLE sub_fd)
{
    int sub_size, r;
    
    if (sub_fd <= 0)
    {
        CODEC_PRINT("no sub handler\n");
        return 0;
    }

    r=codec_h_control(sub_fd,AMSTREAM_IOC_SUB_LENGTH,(unsigned long)&sub_size);
    if(r<0)
        return system_error_to_codec_error(r);
    else
        return sub_size;
}

int codec_read_sub_data(codec_para_t *pcodec, char *buf, unsigned int length)
{
    int data_size=length, r, read_done=0;

    if (pcodec->sub_handle == 0)
    {
        CODEC_PRINT("no control handler\n");
        return 0;
    }

    while (data_size)
    {
        r = codec_h_read(pcodec->sub_handle, buf+read_done, data_size);
        if (r<0)
            return system_error_to_codec_error(r);
        else
        {
            data_size -= r;
            read_done += r;
        }
    }

    return 0;
}

int codec_read_sub_data_fd(CODEC_HANDLE sub_fd, char *buf, unsigned int length)
{
    int data_size=length, r, read_done=0;

    if (sub_fd <= 0)
    {
        CODEC_PRINT("no sub handler\n");
        return 0;
    }

    while (data_size)
    {
        r = codec_h_read(sub_fd, buf+read_done, data_size);
        if (r<0)
            return system_error_to_codec_error(r);
        else
        {
            data_size -= r;
            read_done += r;
        }
    }

    return 0;
}

int codec_write_sub_data(codec_para_t *pcodec, char *buf, unsigned int length)
{
    if (pcodec->sub_handle == 0)
    {
        CODEC_PRINT("no control handler\n");
        return 0;
    }

    return codec_h_write(pcodec->sub_handle, buf, length);
}

int codec_init_cntl(codec_para_t *pcodec)
{
    CODEC_HANDLE cntl;

    cntl = codec_h_open(CODEC_CNTL_DEVICE);
    if (cntl < 0)
    {
        CODEC_PRINT("get %s failed\n", CODEC_CNTL_DEVICE);
        return system_error_to_codec_error(cntl);
    }
    
    pcodec->cntl_handle = cntl;
    return CODEC_ERROR_NONE;
}

int codec_close_cntl(codec_para_t *pcodec)
{
    if (pcodec)
        return codec_h_close(pcodec->cntl_handle);
}

int codec_poll_cntl(codec_para_t *pcodec)
{
    struct pollfd codec_poll_fd[1];

    if (pcodec->cntl_handle == 0)
    {
        return 0;
    }
    
    codec_poll_fd[0].fd = pcodec->cntl_handle;
    codec_poll_fd[0].events = POLLOUT;

    return poll(codec_poll_fd, 1, 10);
}

int codec_get_cntl_state(codec_para_t *pcodec)
{
    int cntl_state, r;
    
    if (pcodec->cntl_handle == 0)
    {
        CODEC_PRINT("no control handler\n");
        return 0;
    }

    r=codec_h_control(pcodec->cntl_handle,AMSTREAM_IOC_TRICK_STAT,(unsigned long)&cntl_state);
    if(r<0)
        return system_error_to_codec_error(r);
    else
        return cntl_state;
}

int codec_set_cntl_mode(codec_para_t *pcodec, unsigned int mode)
{
    return codec_h_control(pcodec->cntl_handle, AMSTREAM_IOC_TRICKMODE, (unsigned long)mode);
}

int codec_set_cntl_avthresh(codec_para_t *pcodec, unsigned int avthresh)
{
    return codec_h_control(pcodec->cntl_handle, AMSTREAM_IOC_AVTHRESH, (unsigned long)avthresh);
}

int codec_set_cntl_syncthresh(codec_para_t *pcodec, unsigned int syncthresh)
{
    return codec_h_control(pcodec->cntl_handle, AMSTREAM_IOC_SYNCTHRESH, (unsigned long)syncthresh);
}

int codec_reset_audio(codec_para_t *pcodec)
{
    return codec_h_control(pcodec->handle, AMSTREAM_IOC_AUDIO_RESET, 0);
}

int codec_reset_subtile(codec_para_t *pcodec)
{
    return codec_h_control(pcodec->handle, AMSTREAM_IOC_SUB_RESET, 0);
}

int codec_set_audio_pid(codec_para_t *pcodec)
{
    return codec_h_control(pcodec->handle,AMSTREAM_IOC_AID,pcodec->audio_pid);
}

int codec_set_sub_id(codec_para_t *pcodec)
{
    return codec_h_control(pcodec->handle,AMSTREAM_IOC_SID,pcodec->sub_pid);
}

int codec_audio_reinit(codec_para_t *pcodec)
{
    return set_audio_format(pcodec);
}

int codec_set_dec_reset(codec_para_t *pcodec)
{
    return codec_h_control(pcodec->handle, AMSTREAM_IOC_SET_DEC_RESET, 0);
}
