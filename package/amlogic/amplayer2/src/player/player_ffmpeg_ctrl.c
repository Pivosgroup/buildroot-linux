#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <player.h>
#include <string.h>

#include <pthread.h>
#include "player_priv.h"

int ffmpeg_lock(void **pmutex, enum AVLockOp op)
{
	int r;
	pthread_mutex_t *mutex=*pmutex;
	switch(op)
		{
		case AV_LOCK_CREATE:  ///< Create a mutex
			  mutex=MALLOC(sizeof(pthread_mutex_t));
			  if(mutex==NULL)
			  		return -1;
			  r=pthread_mutex_init(mutex,NULL);
			  if(r!=0)
				{
				FREE(mutex); 
				mutex=NULL;
				}
			  *pmutex=mutex;
			  break;
		case AV_LOCK_OBTAIN:  ///< Lock the mutex
			  r=pthread_mutex_lock(mutex); 
			  break;
		case AV_LOCK_RELEASE: ///< Unlock the mutex
			  r=pthread_mutex_unlock(mutex); 
			  break;
		case AV_LOCK_DESTROY: ///< Free mutex resources
			  if(mutex)
			  	FREE(mutex); 
			  *pmutex=NULL;
			  break;
		}
	return r;
}


int ffmpeg_init(void)
{
	av_register_all();
	av_lockmgr_register(ffmpeg_lock);

	return 0;
}

int ffmpeg_parse_file(play_para_t *am_p)
{
	AVFormatContext *pFCtx ;
	int ret = -1;
    
	// Open video file
	if(am_p == NULL)
	{
		log_print("[ffmpeg_open_file] Empty pointer!\n");
        return FFMPEG_EMP_POINTER;
	}
	if(am_p->file_name != NULL)
	{	
        ret = av_open_input_file(&pFCtx, am_p->file_name, NULL, FILE_BUFFER_SIZE, NULL);
	    if(ret!=0)
	    {       
		 	log_print("ffmpeg error: Couldn't open input file! ret==%x\n",ret);
	        return FFMPEG_OPEN_FAILED; // Couldn't open file
	    }
        ret = av_find_stream_info(pFCtx);
		if(ret<0)
	    {       
		 	log_print("ERROR:Couldn't find stream information, ret=====%d\n",ret);
			av_close_input_file(pFCtx);
	        return FFMPEG_PARSE_FAILED; // Couldn't find stream information
	    }	
		am_p->pFormatCtx = pFCtx;		
		
		return FFMPEG_SUCCESS;
	}
	else
	{
		log_print("not assigned a file to play\n");
		return FFMPEG_NO_FILE;
	}
}

