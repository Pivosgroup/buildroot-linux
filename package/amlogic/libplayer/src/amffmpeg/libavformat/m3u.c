/*
 *m3u for ffmpeg system
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
#include <fcntl.h>
#if HAVE_SETMODE
#include <io.h>
#endif
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include "os_support.h"
#include "file_list.h"

#define EXTM3U						"#EXTM3U"
#define EXTINF						"#EXTINF"
#define EXT_X_TARGETDURATION		"#EXT-X-TARGETDURATION"

#define EXT_X_MEDIA_SEQUENCE		"#EXT-X-MEDIA-SEQUENCE"
#define EXT_X_KEY					"#EXT-X-KEY"
#define EXT_X_PROGRAM_DATE_TIME	"#EXT-X-PROGRAM-DATE-TIME"
#define EXT_X_ALLOW_CACHE			"#EXT-X-ALLOW-CACHE"
#define EXT_X_ENDLIST				"#EXT-X-ENDLIST"
#define EXT_X_STREAM_INF			"#EXT-X-STREAM-INF"

#define EXT_X_DISCONTINUITY		"#EXT-X-DISCONTINUITY"

#define is_TAG(l,tag)	(!strncmp(l,tag,strlen(tag)))

struct m3u_info{
	int duration;
	int sequence;
	int allow_cache;
	int endlist;
	char *key;
	char *data_time;
	char *streaminfo;
	char *file;
};

int m3u_format_get_line(ByteIOContext *s,char *line,int line_size)
{
    int ch;
    char *q;

    q = line;
    for(;;) {
        ch = get_byte(s);
        if (ch < 0)
            return AVERROR(EIO);
        if (ch == '\n' || ch == '\0') {
            /* process line */
            if (q > line && q[-1] == '\r')
                q--;
            *q = '\0';
			av_log(NULL, AV_LOG_INFO, "m3u_format_get_line line =%s\n",line);
            return q-line;
        } else {
            if ((q - line) < line_size - 1)
                *q++ = ch;
        }
    }
	return 0;
}

static int m3u_parser_line(struct list_mgt *mgt,unsigned char *line,struct list_item*item)
{
	unsigned char *p=line; 
	int enditem=0;
		
	p=line;
	while(*p==' ' && p!='\0' && p-line<1024) p++;
	if(*p!='#' && strlen(p)>0)
	{

		item->file=p; 
		enditem=1;
	}else if(is_TAG(p,EXT_X_ENDLIST)){
		item->flags=ENDLIST_FLAG;
		enditem=1;
	}else if(is_TAG(p,EXTINF)){
		int duration=0;
		sscanf(p+8,"%d",&duration);//skip strlen("#EXTINF:")
		if(duration>0){
			item->flags|=DURATION_FLAG;
			item->duration=duration;
			
		}
	}else if(is_TAG(p,EXT_X_ALLOW_CACHE)){
		item->flags|=ALLOW_CACHE_FLAG;
	}else{
		return 0;
	}
	return enditem;
}


static int m3u_format_parser(struct list_mgt *mgt,ByteIOContext *s)
{ 
	unsigned  char line[1024];
	int ret;
	unsigned char *p; 
	int getnum=0;
	struct list_item tmpitem;
 	char prefix[1024]="";
	int prefix_len=0;
	int start_time=mgt->full_time;

	
	if(mgt->filename){
		char *tail;
		tail=strrchr(mgt->filename,'/');
		if(tail!=NULL){
			prefix_len=tail-mgt->filename;
			memcpy(prefix,mgt->filename,prefix_len);
			prefix[prefix_len]='\0';
		}
	}
	memset(&tmpitem,0,sizeof(tmpitem));
	av_log(NULL, AV_LOG_INFO, "m3u_format_parser get prefix=%s\n",prefix);
	while(m3u_format_get_line(s,line,1024)>0)
	{
		if(m3u_parser_line(mgt,line,&tmpitem))
		{
			struct list_item*item;
			int need_prefix=0;
			int size_file=tmpitem.file?(strlen(tmpitem.file)+32):4;
			tmpitem.start_time=start_time;
			start_time+=tmpitem.duration;
			if(tmpitem.file && 
				((!strncmp(prefix,"http",4) || !strncmp(prefix,"shttp",5)) && 
				(strncmp(tmpitem.file,"http",4)!=0) ))
			{/*if m3u is http,item is not http,add prefix*/
				need_prefix=1;
				size_file+=prefix_len;
			}
			item=av_malloc(sizeof(struct list_item)+size_file);
			if(!item)
				return AVERROR(ENOMEM);
			memcpy(item,&tmpitem,sizeof(tmpitem));
			item->file=NULL;
			if(tmpitem.file)
			{
				item->file=&item[1];
				if(need_prefix){
					strcpy(item->file,prefix);
					strcpy(item->file+prefix_len,tmpitem.file);
				}
				else{
					strcpy(item->file,tmpitem.file);
				}
			}
			list_add_item(mgt,item);
			if(item->flags &ENDLIST_FLAG)
			{
				mgt->have_list_end=1;
				break;
			}
			else
			{
				memset(&tmpitem,0,sizeof(tmpitem));
				getnum++;
			}
		}else{
			if(tmpitem.flags&ALLOW_CACHE_FLAG)
				mgt->flags|=ALLOW_CACHE_FLAG;
		}
		
	}

	mgt->full_time=start_time;
	av_log(NULL, AV_LOG_INFO, "m3u_format_parser end num =%d,fulltime=%d\n",getnum,start_time);
	return getnum;
}


static int m3u_probe(ByteIOContext *s,const char *file)
{
	if(s)
	{
		char line[1024];
		if(m3u_format_get_line(s,line,1024)>0)
		{

			if(memcmp(line,EXTM3U,strlen(EXTM3U))==0)
			{
				av_log(NULL, AV_LOG_INFO, "get m3u flags!!\n");
				return 100;
			}
		}	
	}
	else
	{
		if((match_ext(file, "m3u"))||(match_ext(file, "m3u8"))) 
		{
			return 50;
		}
	}
	return 0;
}
 
 struct list_demux m3u_demux = {
	"m3u",
    m3u_probe,
	m3u_format_parser,
};



