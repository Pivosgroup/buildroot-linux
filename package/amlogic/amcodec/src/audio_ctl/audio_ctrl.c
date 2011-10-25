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

#include<sys/types.h>
#include<sys/stat.h>
#include <errno.h>
#include <sys/ioctl.h>
#include<fcntl.h>
#include <codec_error.h>
#include <codec.h>
#include "audio_h_ctrl.h"

void audio_start(void )
{
	audio_trans_cmd("start");
}
void audio_stop(void )
{
	audio_trans_cmd("stop");
}
void audio_pause(void )	
{
	audio_trans_cmd("pause");
}
void audio_resume(void )	
{
	audio_trans_cmd("start");
}



int codec_get_mutesta(codec_para_t *p)	
{
    int ret;
	ret = audio_trans_cmd("getmute");    
    return ret;
}
int codec_set_mute(codec_para_t *p,int mute)
{
	int ret;
	if(mute)
		ret=audio_trans_cmd("mute");
	else
		ret=audio_trans_cmd("unmute");
	return ret;
}
int codec_get_volume_range(codec_para_t *p,int *min,int *max)
{	
	return -CODEC_ERROR_IO;
}
int codec_set_volume(codec_para_t *p,int val)
{
	int ret;
	char buf[16];
	sprintf(buf,"volset:%d",val);
	ret=audio_trans_cmd(buf);
	return ret;
}
int codec_get_volume(codec_para_t *p)
{
	int ret;
	ret=audio_trans_cmd("volget");
	return ret;
}

int codec_set_volume_balance(codec_para_t *p,int balance)
{
	return -CODEC_ERROR_IO;
}

int codec_swap_left_right(codec_para_t *p)
{
	int ret;
	ret=audio_trans_cmd("swap");
	return ret;
}

int codec_left_mono(codec_para_t *p)
{
	int ret;
	ret=audio_trans_cmd("leftmono");
	return ret;
}

int codec_right_mono(codec_para_t *p)
{
	int ret;
	ret=audio_trans_cmd("rightmono");
	return ret;
}

int codec_stereo(codec_para_t *p)
{
	int ret;
	ret=audio_trans_cmd("stereo");
	return ret;
}

int codec_audio_automute(int auto_mute)
{
    int ret;
    char buf[16];
    sprintf(buf,"automute:%d",auto_mute);
    ret=audio_trans_cmd(buf);
    return ret;
}

int codec_audio_spectrum_switch(codec_para_t *p,int isStart,int interval)
{
    int  ret;
    char cmd[32];
    
    if(isStart == 1)
   {
         snprintf(cmd,32,"spectrumon:%d",interval);
         ret=audio_trans_cmd(cmd);
    }
    else if(isStart == 0)
    {        
        ret=audio_trans_cmd("spectrumoff");
    }

    return ret;
}


