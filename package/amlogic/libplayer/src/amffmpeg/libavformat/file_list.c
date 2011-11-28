/*
 * filelist io for ffmpeg system
 * Copyright (c) 2001 Fabrice Bellard
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/avstring.h"
#include "avformat.h"
#include "url.h"
#include <fcntl.h>
#if HAVE_SETMODE
#include <io.h>
#endif
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include "os_support.h"
#include "file_list.h"
static struct list_demux *list_demux_list=NULL;
#define unused(x)	(x=x)

int register_list_demux(struct list_demux *demux)
{
	list_demux_t **list=&list_demux_list;
    while (*list != NULL) list = &(*list)->next;
    *list = demux;
    demux->next = NULL;
	return 0;
}

struct list_demux * probe_demux(AVIOContext *s,const char *filename)
{
	int ret;
	int max=0;
	list_demux_t *demux;
	list_demux_t *best_demux=NULL;
	demux=list_demux_list;
	while(demux)
	{
		ret=demux->probe(s,filename);
		if(ret>max)
		{
			max=ret;
			best_demux=demux;
			if(ret>=100)
			{
				return best_demux;
			}
		}
		demux=demux->next;
	}
	return best_demux;
}

int list_add_item(struct list_mgt *mgt,struct list_item*item)
{
	struct list_item**list;
	struct list_item*prev;
	list=&mgt->item_list;
	prev=NULL;
	 while (*list != NULL) 
	 {
	 	prev=*list;
	 	list = &(*list)->next;
	 }
	*list = item;
	item->prev=prev;
       item->next = NULL;
	mgt->item_num++;
	return 0;
}

int list_test_and_add_item(struct list_mgt *mgt,struct list_item*item)
{
	struct list_item**list;
	struct list_item*prev;
	list=&mgt->item_list;
	prev=NULL;
	//test
	if(item->file!=NULL){
		while (*list != NULL) 
		{	
			if(strcmp((*list)->file,item->file)==0){//found the same item,drop it.
				av_log(NULL, AV_LOG_INFO, "hit the same item,drop it\n");
				return -1;
			}
			list = &(*list)->next;
		}

	}

	while (*list != NULL) 
	{
		prev=*list;
		list = &(*list)->next;
	}
	*list = item;
	item->prev=prev;
	item->next = NULL;
	mgt->item_num++;
	return 0;
}
static int list_del_item(struct list_mgt *mgt,struct list_item*item)
{
	struct list_item*tmp;
	if(!item || !mgt)
		return -1;
	if(mgt->item_list==item){
		mgt->item_list=item->next;
		if(mgt->item_list)
			mgt->item_list->prev=NULL;
	}
	else {
		tmp=item->prev;
		tmp->next=item->next;
		if(item->next) 
			item->next->prev=tmp;
	}
	mgt->item_num--;
	av_free(item);
	item = NULL;
	return 0;		
}



/*=======================================================================================*/
int url_is_file_list(AVIOContext *s,const char *filename)
{
	int ret;
	list_demux_t *demux;
	AVIOContext *lio=s;
	int64_t	   *oldpos=0;
	if(!lio)
	{
		ret=url_fopen(&lio,filename,AVIO_FLAG_READ);
		if(ret!=0)
		{ 
		return AVERROR(EIO); 
		}
	}
	else{
		oldpos=url_ftell(lio);
	}
	demux=probe_demux(lio,filename);
	if(lio!=s)
	{
		url_fclose(lio);
	}
	else
	{
		url_fseek(lio, oldpos, SEEK_SET);
	}
	return demux!=NULL?1:0;
}

static int list_open_internet(AVIOContext **pbio,struct list_mgt *mgt,const char *filename, int flags)
{
	list_demux_t *demux;
	int ret;
	AVIOContext *bio;
	ret=url_fopen(&bio,filename,flags);
	if(ret!=0)
		{
			return AVERROR(EIO); 
		}
	mgt->location=bio->reallocation;
	demux=probe_demux(bio,filename);
	if(!demux)
	{
		ret=-1;
		goto error;
	}
	ret=demux->parser(mgt,bio);
	if(ret<=0)
	{
		ret=-1;
		goto error;
	}
	*pbio=bio;
	return 0;
error:
	if(bio)
		url_fclose(bio);
	return ret;
}

static int list_open(URLContext *h, const char *filename, int flags)
{
	struct list_mgt *mgt;
	int ret;
	AVIOContext *bio;
	mgt=av_malloc(sizeof(struct list_mgt));
	if(!mgt)
		return AVERROR(ENOMEM);
	memset(mgt,0,sizeof(*mgt));
	mgt->seq = -1;
	mgt->filename=filename+5;
	mgt->flags=flags;
	if((ret=list_open_internet(&bio,mgt,mgt->filename,flags| URL_MINI_BUFFER | URL_NO_LP_BUFFER))!=0)
	{
		av_free(mgt);
		return ret;
	}
	lp_lock_init(&mgt->mutex,NULL);
	mgt->current_item=mgt->item_list;
	mgt->cur_uio=NULL;
 	h->is_streamed=1;
	h->is_slowmedia=1;
	if(mgt->full_time>0 && mgt->have_list_end)
		h->support_time_seek=1;
	h->priv_data = mgt;
	
	url_fclose(bio);
	return 0;
}


static struct list_item * switchto_next_item(struct list_mgt *mgt)
{
	struct list_item *next=NULL;
	struct list_item *current = NULL;
	if(!mgt)
		return NULL;
	if(mgt->current_item==NULL || mgt->current_item->next==NULL){
			/*new refresh this mgtlist now*/
			AVIOContext *bio;
			
			int ret;
			if((ret=list_open_internet(&bio,mgt,mgt->filename,mgt->flags| URL_MINI_BUFFER | URL_NO_LP_BUFFER))!=0)
			{
				goto switchnext;
			}
			url_fclose(bio);
			if(mgt->current_item && mgt->current_item->file){/*current item,switch to next*/
				current=mgt->current_item;
				next=mgt->current_item->next;
				for(;next!=NULL;next=next->next){
					if(next->file && strcmp(current->file,next->file)==0){
						/*found the same item,switch to the next*/	
						current=next;
						break;
					}
				}
				while(current!=mgt->item_list){
					/*del the old item,lest current,and then play current->next*/
					list_del_item(mgt,mgt->item_list);
				}
				mgt->current_item=current;/*switch to new current;*/
			}
	}
switchnext:
	if(mgt->current_item)
		next=mgt->current_item->next;
	else
		next=mgt->item_list;
	if(next)
		av_log(NULL, AV_LOG_INFO, "switch to new file=%s,total=%d,start=%d,duration=%d\n",
			next->file,mgt->item_num,next->start_time,next->duration);
	else
		av_log(NULL, AV_LOG_INFO, "switch to new file=NULL,total=%d\n",mgt->item_num);
	return next;
}

static void fresh_item_list(struct list_mgt *mgt){	
	int retries = 5;
	do{	
		if((switchto_next_item(mgt))!=NULL)
		{
			av_log(NULL, AV_LOG_INFO, "refresh the Playlist,item num:%d,filename:%s\n",mgt->item_num,mgt->filename);		
			break;
		}	
		else{			
			usleep(50000); //50ms
			av_log(NULL, AV_LOG_INFO, "no new item,wait next refresh,current item num:%d\n",mgt->item_num);	
		}

	}while(retries-->0);//50ms*5

}

static int list_read(URLContext *h, unsigned char *buf, int size)
{   
	struct list_mgt *mgt = h->priv_data;
    int len=AVERROR(EIO);
	struct list_item *item=mgt->current_item;
	int retries = 10;
	
retry:	
	if (url_interrupt_cb()){     
		av_log(NULL, AV_LOG_ERROR," url_interrupt_cb\n");	
            	return AVERROR(EINTR);

	}
	//av_log(NULL, AV_LOG_INFO, "list_read start buf=%x,size=%d\n",buf,size);
	if(!mgt->cur_uio )
	{
		if(item && item->file)
		{
			AVIOContext *bio;
			av_log(NULL, AV_LOG_INFO, "list_read switch to new file=%s\n",item->file);
			len=url_fopen(&bio,item->file,AVIO_FLAG_READ | URL_MINI_BUFFER | URL_NO_LP_BUFFER);
			if(len!=0)
			{
				av_log(NULL, AV_LOG_INFO, "list url_fopen failed =%d\n",len);
				return len;
			}
			if(url_is_file_list(bio,item->file))
			{
				/*have 32 bytes space at the end..*/
				memmove(item->file+5,item->file,strlen(item->file)+1);
				memcpy(item->file,"list:",5);
				url_fclose(bio);
				len=url_fopen(&bio,item->file,mgt->flags | URL_MINI_BUFFER | URL_NO_LP_BUFFER);
				if(len!=0)
				{
					av_log(NULL, AV_LOG_INFO, "file list url_fopen failed =%d\n",len);
					return len;
				}
			}
			mgt->cur_uio=bio;
		}
	}
	if(mgt->cur_uio){
		len=get_buffer(mgt->cur_uio,buf,size);
		//av_log(NULL, AV_LOG_INFO, "list_read get_buffer=%d\n",len);
	}
	if(len==AVERROR(EAGAIN))
		 return AVERROR(EAGAIN);/*not end,bug need to*/
	else if((len<=0)&& mgt->current_item!=NULL)
	{/*end of the file*/
		av_log(NULL, AV_LOG_INFO, "try switchto_next_item buf=%x,size=%d,len=%d\n",buf,size,len);

		if(item && (item->flags & ENDLIST_FLAG))
			return 0;
		item=switchto_next_item(mgt);
		if(!item){
			if(mgt->flags&REAL_STREAMING_FLAG){
				fresh_item_list(mgt);	
				if(retries>0){
					retries --;
					goto retry;
				}
			}

			av_log(NULL, AV_LOG_INFO, "Need more retry by player logic\n");
			return AVERROR(EAGAIN);/*not end,but need to refresh the list later*/
		}
		if(mgt->cur_uio)
			url_fclose(mgt->cur_uio);
		mgt->cur_uio=NULL;
		mgt->current_item=item;
		if(item->flags & ENDLIST_FLAG){
			av_log(NULL, AV_LOG_INFO, "reach list end now!,item=%x\n",item);
			return 0;/*endof file*/
		}
		else if(item->flags & DISCONTINUE_FLAG){
			av_log(NULL, AV_LOG_INFO, "Discontiue item \n");
			//1 TODO:need to notify uper level stream is changed
			goto retry;
		}
		else{	
			goto retry;
		}
	}
	if(mgt->flags&REAL_STREAMING_FLAG&&mgt->item_num<4){ //force refresh items
		fresh_item_list(mgt);	
	}

	av_log(NULL, AV_LOG_INFO, "list_read end buf=%x,size=%d\n",buf,size);
    return len;
}

static int list_write(URLContext *h, unsigned char *buf, int size)
{    
	unused(h);
	unused(buf);
	unused(size);
	return -1;
}
/* XXX: use llseek */
static int64_t list_seek(URLContext *h, int64_t pos, int whence)
{
	struct list_mgt *mgt = h->priv_data;
	struct list_item *item,*item1;
	if (whence == AVSEEK_BUFFERED_TIME)
	{
		int64_t buffed_time=0;
		if(mgt->current_item ){
			//av_log(NULL, AV_LOG_INFO, "list_seek uui=%ld,size=%lld\n",mgt->cur_uio,url_fsize(mgt->cur_uio));
			if(mgt->cur_uio && url_fsize(mgt->cur_uio)>0 && mgt->current_item->duration>=0){
				//av_log(NULL, AV_LOG_INFO, "list_seek start_time=%ld,pos=%lld\n",mgt->current_item->start_time,url_buffed_pos(mgt->cur_uio));
				buffed_time=mgt->current_item->start_time+url_buffed_pos(mgt->cur_uio)*mgt->current_item->duration/url_fsize(mgt->cur_uio);
			}
			else{
				buffed_time=mgt->current_item->start_time;
				if(mgt->current_item && (mgt->current_item->flags & ENDLIST_FLAG))
					buffed_time=mgt->full_time;/*read to end list, show full bufferd*/
			}
		}
		av_log(NULL, AV_LOG_INFO, "list current buffed_time=%lld\n",buffed_time);
		return buffed_time;
	}
	
	if (whence == AVSEEK_SIZE)
        return mgt->file_size;
	av_log(NULL, AV_LOG_INFO, "list_seek pos=%lld,whence=%x\n",pos,whence);
	if (whence == AVSEEK_FULLTIME)
	{
		if(mgt->have_list_end)
			return mgt->full_time;
		else
			return -1;
	}
	
	if(whence == AVSEEK_TO_TIME && pos>=0 && pos<mgt->full_time)
	{
		av_log(NULL, AV_LOG_INFO, "list_seek to Time =%lld,whence=%x\n",pos,whence);
		for(item=mgt->item_list;item;item=item->next)
		{
			if(item->start_time<=pos && pos <item->start_time+item->duration)
			{	
				if(mgt->cur_uio)
					url_fclose(mgt->cur_uio);
				mgt->cur_uio=NULL;
				mgt->current_item=item;
				av_log(NULL, AV_LOG_INFO, "list_seek to item->file =%s\n",item->file);
				return item->start_time;/*pos=0;*/
			}
		}

	}
	av_log(NULL, AV_LOG_INFO, "list_seek failed\n");
	return -1;
}
static int list_close(URLContext *h)
{
	struct list_mgt *mgt = h->priv_data;
	struct list_item *item,*item1;
	if(!mgt)return 0;
	item=mgt->item_list;
	if(mgt->cur_uio!=NULL)
		url_fclose(mgt->cur_uio);
	while(item!=NULL)
		{
		item1=item;
		item=item->next;
		av_free(item1);
	
		}
	av_free(mgt);
	unused(h);
	return 0;
}
static int list_get_handle(URLContext *h)	
{    
return (intptr_t) h->priv_data;
}

URLProtocol file_list_protocol = {
    "list",
    list_open,
    list_read,
    list_write,
    list_seek,
    list_close,
    .url_exseek=list_seek,/*same as seek is ok*/ 
    .url_get_file_handle = list_get_handle,
};

URLProtocol *get_file_list_protocol(void)
{
	return &file_list_protocol;
}
int register_list_demux_all(void)
{
	static int registered_all=0;
	if(registered_all)	
		return;
	registered_all++;
	extern struct list_demux m3u_demux;
	av_register_protocol(&file_list_protocol); 
	register_list_demux(&m3u_demux); 
	return 0;
}

