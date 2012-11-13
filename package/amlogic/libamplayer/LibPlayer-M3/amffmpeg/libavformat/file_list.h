/*
 * File list 
 * Copyright (C) 2009 Justin Ruggles
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

#ifndef AVFORMAT_FILE_LIST_H
#define AVFORMAT_FILE_LIST_H

#include <libavformat/avio.h>

#include <pthread.h>
#include "internal.h"

#define lock_t			pthread_mutex_t
#define lp_lock_init(x,v) 	pthread_mutex_init(x,v)
#define lp_lock(x)		pthread_mutex_lock(x)
#define lp_unlock(x)   	pthread_mutex_unlock(x)



#define DISCONTINUE_FLAG			(1<<0)
#define DURATION_FLAG				(1<<1)
#define SEEK_SUPPORT_FLAG			(1<<2)
#define ENDLIST_FLAG				(1<<3)
#define KEY_FLAG					(1<<4)
#define EXT_INFO					(1<<5)
#define READ_END_FLAG				(1<<6)
#define ALLOW_CACHE_FLAG			(1<<7)
#define REAL_STREAMING_FLAG		(1<<8)


struct list_mgt;
struct list_demux;

enum KeyType {
    KEY_NONE = 0,
    KEY_AES_128,
};

struct encrypt_key_priv_t{
	enum KeyType key_type;
	char key_from[MAX_URL_SIZE];
	uint8_t key[16];
	uint8_t iv[16];
	int is_have_key_file; //just get file from server 
};

#define MAX_BUFFER_BLOCKS 150
#define BLOCKSIZE 16

struct AES128KeyContext{	
	uint8_t key[16];
	uint8_t iv[16];
};

struct AESCryptoContext{
	uint8_t inbuffer [BLOCKSIZE*MAX_BUFFER_BLOCKS],
		outbuffer[BLOCKSIZE*MAX_BUFFER_BLOCKS];
	uint8_t *outptr;
	int indata, indata_used, outdata;
	int eof;	
	struct AVAES *aes;
	int have_init;
};
typedef struct list_item
{
	const char *file;
	int 	   flags;	  
	int 		start_time;
	int 		duration;
	enum KeyType ktype;
	struct AES128KeyContext* key_ctx; //just store key info.
	struct AESCryptoContext* crypto;
	struct list_item * prev;
	struct list_item * next;
}list_item_t;
struct variant{
	char url[MAX_URL_SIZE];
	int bandwidth;
};

typedef enum _ClarityType{
	LOW_BANDWIDTH,
	MIDDLE_BANDWIDTH,//default
	HIGH_BANDWIDTH,
	
}ClarityType;

typedef struct list_mgt
{
	char *filename;
	char *location;
	int flags;
	lock_t mutex;
	struct list_item *item_list;
	int item_num;
	struct list_item *current_item;
	int64_t file_size;
	int 	full_time;
	int 	have_list_end;
	int  seq;  
	//added for Playlist file with encrypted media segments
	ClarityType ctype; //default is HIGH_BANDWIDTH
	int n_variants;
	struct variant ** variants;
	int is_variant;
	int has_iv;
	int bandwidth;
	char* prefix; //	
	struct encrypt_key_priv_t* key_tmp; //just for parsing using,if ended parsing,just free this pointer.
	//end.
	ByteIOContext	*cur_uio;
	struct list_demux *demux;
	int 	have_sub_list;
}list_mgt_t;

typedef struct list_demux
{
	const char * name;
	int (*probe)(ByteIOContext *s,const char *file);
	int (*parser)(struct list_mgt *mgt,ByteIOContext *s);
	struct list_demux *next;
}list_demux_t;
URLProtocol *get_file_list_protocol(void);
int register_list_demux_all(void);
int register_list_demux(struct list_demux *demux);
struct list_demux * probe_demux(ByteIOContext  *s,const char *filename);
int list_add_item(struct list_mgt *mgt,struct list_item*item);
int list_test_and_add_item(struct list_mgt *mgt,struct list_item*item);
int url_is_file_list(ByteIOContext *s,const char *filename);


#endif /* AVFORMAT_FILE_LIST_H */

