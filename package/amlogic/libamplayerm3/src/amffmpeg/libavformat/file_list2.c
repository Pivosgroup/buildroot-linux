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
#include <fcntl.h>
#if HAVE_SETMODE
#include <io.h>
#endif
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include "os_support.h"
#include "file_list.h"
#include "amconfigutils.h"
#include "bandwidth_measure.h"
#include "hls_livesession.h"
#include "CacheHttp.h"
#include "libavutil/lfg.h"
#include "http.h"

#ifndef ANDROID
static char * strndup(const char* s, size_t n) {
  size_t l = strlen(s);
  char *r = NULL;

  if (l < n)
    return av_strdup(s);

  r = (char *) av_malloc(n+1);
  if (r == NULL)
    return NULL;

  av_strlcpy(r, s, n);
  r[n] ='\0';
  return r;
}
#endif

static struct list_demux *list_demux_list = NULL;
#define unused(x)   (x=x)

#define LIVE_LIST_MAX 120
#define SHRINK_LIVE_LIST_THRESHOLD (LIVE_LIST_MAX/3)
static int list_shrink_live_list(struct list_mgt *mgt);


#define SESSLEN 256
static int generate_playback_session_id(char * ssid, int size);
static int generate_segment_session_id(char * ssid, int size);

int generate_playback_session_id(char * ssid, int size)
{
    AVLFG rnd;
    char *ex_id = "314F1B49-AA3C-4715-950D-";
    unsigned int session_id = 0;
    char session_name[SESSLEN] = "";
    av_lfg_init(&rnd, 0xDEADC0DE);
    session_id =  av_lfg_get(&rnd);
    snprintf(session_name, SESSLEN, "%s%012u", ex_id, session_id);
    if (ssid != NULL) {
        snprintf(ssid, size, "%s", session_name);
        return 0;
    }
    return -1;
}

int generate_segment_session_id(char * ssid, int size)
{
    AVLFG rnd;
    char *ex_id = "E4499E08-D4C5-4DB0-A21C-";
    unsigned int session_id = 0;
    char session_name[SESSLEN] = "";
    av_lfg_init(&rnd, 0xDEADC0DE);
    session_id =  av_lfg_get(&rnd);
    snprintf(session_name, SESSLEN, "%s%012u", ex_id, session_id);
    if (ssid != NULL) {
        snprintf(ssid, size, "%s", session_name);
        return 0;
    }
    return -1;
}


int register_list_demux(struct list_demux *demux)
{
    list_demux_t **list = &list_demux_list;
    while (*list != NULL) {
        list = &(*list)->next;
    }
    *list = demux;
    demux->next = NULL;
    return 0;
}

struct list_demux * probe_demux(ByteIOContext  *s, const char *filename) {
    int ret;
    int max = 0;
    list_demux_t *demux;
    list_demux_t *best_demux = NULL;
    demux = list_demux_list;
    while (demux) {
        ret = demux->probe(s, filename);
        if (ret > max) {
            max = ret;
            best_demux = demux;
            if (ret >= 100) {
                return best_demux;
            }
        }
        demux = demux->next;
    }
    return best_demux;
}

int list_add_item(struct list_mgt *mgt, struct list_item*item)
{
    struct list_item**list;
    struct list_item*prev;
    list = &mgt->item_list;
    prev = NULL;
    while (*list != NULL) {
        prev = *list;
        list = &(*list)->next;
    }
    *list = item;
    item->prev = prev;
    item->next = NULL;
    mgt->item_num++;
    item->index = mgt->next_index;
    mgt->next_index++;
    av_log(NULL, AV_LOG_INFO, "list_add_item:seq=%d,index=%d\n", item->seq, item->index);
    return 0;
}
static struct list_item* list_test_find_samefile_item(struct list_mgt *mgt, struct list_item *item) {
    struct list_item **list;

    list = &mgt->item_list;
    if (item->file != NULL) {
        while (*list != NULL) {
            if ((item->seq >= 0  && (item->seq == (*list)->seq)) && (!mgt->have_list_end&& !strcmp((*list)->file, item->file))) {
                /*same seq or same file name*/
                return *list;
            }
            list = &(*list)->next;
        }
    }
    return NULL;
}

static struct list_item* list_find_item_by_index(struct list_mgt *mgt, int index) {
    struct list_item **list;

    list = &mgt->item_list;
    int next_index  = 0;
    if (index < mgt->item_num) {
        next_index = index + 1;
    } else {
        av_log(NULL, AV_LOG_INFO, "just return last item,index:%d\n", index);
        next_index = index;
    }
    if (index >= -1) {
        while (*list != NULL) {
            if ((*list)->index == next_index) {
                /*same index*/
                av_log(NULL, AV_LOG_INFO, "find next item,index:%d\n", index);
                return *list;
            }
            list = &(*list)->next;
        }
    }
    return NULL;
}
static struct list_item* list_find_item_by_seq(struct list_mgt *mgt, int seq) {
    struct list_item **list;
    int next_seq = seq + 1;
    list = &mgt->item_list;
    if (next_seq >= 0) {
        while (*list != NULL) {
            if ((*list)->seq == next_seq || (*list)->seq == (next_seq + 1) || (*list)->seq == (next_seq + 2) || (*list)->seq == (next_seq + 3)) {
                /*same seq*/
                av_log(NULL, AV_LOG_INFO, "find item,seq:%d\n", seq);
                return *list;
            }
            list = &(*list)->next;
        }
    }
    return NULL;
}
int list_test_and_add_item(struct list_mgt *mgt, struct list_item*item)
{
    struct list_item *sameitem;
    sameitem = list_test_find_samefile_item(mgt, item);
    if (sameitem) {
        av_log(NULL, AV_LOG_INFO, "list_test_and_add_item found same item\nold:%s[seq=%d]\nnew:%s[seq=%d]", sameitem->file, sameitem->seq, item->file, item->seq);
        return -1;/*found same item,drop it */
    }
    av_log(NULL,AV_LOG_INFO,"add this item,seq number:%d,start:%.4lf,duration:%.4lf\n",item->seq,item->start_time,item->duration);
    list_add_item(mgt, item);
    return 0;
}


static int list_del_item(struct list_mgt *mgt, struct list_item*item)
{

    struct list_item*tmp;
    if (!item || !mgt) {
        return -1;
    }
    if (mgt->item_list == item) {
        mgt->item_list = item->next;
        if (mgt->item_list) {
            mgt->item_list->prev = NULL;
        }
    } else {
        tmp = item->prev;
        tmp->next = item->next;
        if (item->next) {
            item->next->prev = tmp;
        }
    }
    mgt->item_num--;
    if (item->ktype == KEY_AES_128) {
        av_free(item->key_ctx);
    }

    av_free(item);
    item = NULL;
    return 0;
}

static int list_shrink_live_list(struct list_mgt *mgt)
{
    struct list_item **tmp = NULL;
    if (NULL == mgt) {
        return -1;
    }
    tmp = &mgt->item_list;
    if (!mgt->have_list_end) { //not have list end,just refer to live streaming
        while (mgt->item_num > SHRINK_LIVE_LIST_THRESHOLD && *tmp != mgt->current_item) {
            list_del_item(mgt, *tmp);
            tmp = &mgt->item_list;
        }
    }
    av_log(NULL, AV_LOG_INFO, "shrink live item from list,total:%d\n", mgt->item_num);
    return 0;
}

static int list_delall_item(struct list_mgt *mgt)
{
    struct list_item *p = NULL;
    struct list_item *t = NULL;
    if (NULL == mgt) {
        return -1;
    }

    p = mgt->item_list;

    while (p != NULL) {
        t = p->next;
        mgt->item_num--;
        if (p->ktype == KEY_AES_128) {           
            av_free(p->key_ctx);
        }

        av_free(p);

        p = NULL;

        p = t;
    }
    mgt->item_list = NULL;
    av_log(NULL, AV_LOG_INFO, "delete all items from list,total:%d\n", mgt->item_num);
    return 0;

}

/*=======================================================================================*/
int url_is_file_list(ByteIOContext *s, const char *filename)
{
    int ret;
    list_demux_t *demux;
    ByteIOContext *lio = s;
    int64_t    *oldpos = 0;
    if (am_getconfig_bool("media.amplayer.usedm3udemux")) {
        return 0;    /*if used m3u demux,always failed;*/
    }
    if (!lio) {
        ret = url_fopen(&lio, filename, AVIO_FLAG_READ | URL_MINI_BUFFER|URL_NO_LP_BUFFER);
        if (ret != 0) {
            return AVERROR(EIO);
        }
    } else {
        oldpos = url_ftell(lio);
    }
    demux = probe_demux(lio, filename);
    if (lio != s) {
        url_fclose(lio);
    } else {
        url_fseek(lio, oldpos, SEEK_SET);
    }
    return demux != NULL ? 100 : 0;
}

static int list_open_internet(ByteIOContext **pbio, struct list_mgt *mgt, const char *filename, int flags)
{
    list_demux_t *demux;
    int ret =0;
    ByteIOContext *bio = *pbio;
    char* url = filename;
reload:
    
    if (url_interrupt_cb()) {
         ret = -1;       
         goto error;   
    }
    
    if(bio==NULL){
        ret = avio_open_h(&bio, url, flags,mgt->ipad_ex_headers);
        av_log(NULL,AV_LOG_INFO,"http open,return value: %d\n",ret);
        
    }else{
        avio_reset(bio,AVIO_FLAG_READ); 
        URLContext* last = (URLContext*)bio->opaque;            
        ret = ff_http_do_new_request(last,NULL);
        av_log(NULL,AV_LOG_INFO,"do http request,return value: %d\n",ret);
    }
    
    if (ret != 0) {      
        if(bio!=NULL){
            avio_close(bio);
            bio = NULL;
        }
        return AVERROR(EIO);
    }
    mgt->location = bio->reallocation;
    if (NULL == mgt->location && mgt->n_variants > 0) { //set location for multibandwidth streaming,such youtube,etc.
        mgt->location = url;
    }
    demux = probe_demux(bio, url);
    if (!demux) {
        ret = -1;       
        goto error;
    }
    ret = demux->parser(mgt, bio);
    if (ret <0 && mgt->n_variants == 0) {
        ret = -1;
        
        goto error;
    } else {
       
        if (mgt->item_num == 0 && mgt->n_variants > 0) { //simplely choose server,mabye need sort variants when got it from server.
            if (bio) {
                url_fclose(bio);
                bio = NULL;
            }


            float value = 0.0;
            ret = am_getconfig_float("libplayer.hls.level", &value);
            if (ret < 0 || value <= 0) {
                struct variant *v = mgt->variants[0];
                url = v->url;
                mgt->playing_variant = v;
                av_log(NULL, AV_LOG_INFO, "[%d]reload playlist,url:%s\n", __LINE__, url);
                goto reload;
            } else if (value < mgt->n_variants) {
                int iValue = (int)(value + 0.5);
                struct variant *v = mgt->variants[iValue];
                url = v->url;
                mgt->playing_variant = v;
                av_log(NULL, AV_LOG_INFO, "[%d]reload playlist,url:%s,%d\n", __LINE__, url, iValue);
                goto reload;

            }

        }

    }
   
    *pbio = bio;
    return 0;
error:
    if (bio) {       
        url_fclose(bio);
        *pbio  = NULL;
    }
    return ret;
}

static struct list_mgt* gListMgt = NULL;
static int list_open(URLContext *h, const char *filename, int flags)
{
    struct list_mgt *mgt;
    int ret;   
    mgt = av_malloc(sizeof(struct list_mgt));
    if (!mgt) {
        return AVERROR(ENOMEM);
    }
    memset(mgt, 0, sizeof(struct list_mgt));
    mgt->key_tmp = NULL;
    mgt->start_seq = -1;
    mgt->next_seq = -1;
    mgt->filename = filename + 5;
    mgt->flags = flags;
    mgt->next_index = 0;
    mgt->playing_item_index = 0;
    mgt->playing_item_seq = 0;
    mgt->strategy_up_counts = 0;
    mgt->strategy_down_counts = 0;
    mgt->listclose = 0;
    mgt->cur_uio = NULL;


    char headers[1024];
    char sess_id[40];
    memset(headers, 0, sizeof(headers));
    memset(sess_id, 0, sizeof(sess_id));
    generate_segment_session_id(sess_id, 37);
    snprintf(headers, sizeof(headers),
             "Connection: keep-alive\r\n"
             /*"Range: bytes=0- \r\n"*/
             "X-Playback-Session-Id: %s\r\n%s", sess_id,h!=NULL&&h->headers!=NULL?h->headers:"");
    av_log(NULL, AV_LOG_INFO, "Generate ipad http request headers,\r\n%s\n", headers);
    mgt->ipad_ex_headers = strndup(headers, 1024);
   
    memset(headers, 0, sizeof(headers));
    generate_playback_session_id(sess_id, 37);
    snprintf(headers, sizeof(headers),
             /*"Connection: keep-alive\r\n"*/
             "X-Playback-Session-Id: %s\r\n%s", sess_id,h!=NULL&&h->headers!=NULL?h->headers:"");
    av_log(NULL, AV_LOG_INFO, "Generate ipad http request media headers,\r\n%s\n", headers);
    
    mgt->ipad_req_media_headers = strndup(headers, 1024);

    gListMgt = mgt;
    if ((ret = list_open_internet(&mgt->cur_uio, mgt, mgt->filename, flags | URL_MINI_BUFFER | URL_NO_LP_BUFFER)) != 0) {
        av_free(mgt);
        return ret;
    }
    lp_lock_init(&mgt->mutex, NULL);
	
    float value = 0.0;
    ret = am_getconfig_float("libplayer.hls.stpos", &value);
    if (ret < 0 || value <= 0) {	
        if (!mgt->have_list_end) {
            int itemindex =0;
            if(mgt->item_num<10&&mgt->target_duration<5){
                itemindex = mgt->item_num / 2+1; /*for live streaming ,choose the middle item.*/

            }else if(mgt->item_num>=10){
                itemindex = mgt->item_num-3; //last item
            }else if(mgt->item_num<10&&mgt->target_duration>5){
                itemindex =  mgt->item_num -1;               
            }
            mgt->current_item = list_find_item_by_index(mgt,itemindex-1);
        } else {
            mgt->current_item = mgt->item_list;
        }		
    }else{
    	mgt->current_item = list_find_item_by_index(mgt,(int)value-1);
    }	
   
    h->is_streamed = 1;
    h->is_slowmedia = 1;
    if (mgt->full_time > 0 && mgt->have_list_end) {
        h->support_time_seek = 1;
    }
    h->priv_data = mgt;
    mgt->cache_http_handle = NULL;

    ret = CacheHttp_Open(&mgt->cache_http_handle,mgt->ipad_req_media_headers);
  

    return 0;
}

#define ADAPTATION_PROFILE_DEFAULT 1  //AGREESSIVE_ADAPTIVE
static int get_adaptation_profile(){
    float value = 0.0;
    int ret = -1;
    ret = am_getconfig_float("libplayer.hls.profile", &value);
    if(ret < 0 || value <0||value>3){
        ret = ADAPTATION_PROFILE_DEFAULT;
    }else{
        ret= (int)value;
    }
    av_log(NULL, AV_LOG_INFO,"just get adaptation profile: %d\n",ret);
    return ret;

}

static float get_adaptation_ex_para(int type){
    float value = 0.0;
    int ret = -1;
    if(type==0){
        ret = am_getconfig_float("libplayer.hls.sensitivity", &value);
    }else if(type == 1){
        ret = am_getconfig_float("libplayer.hls.uplevel", &value);
    }else{
        ret = am_getconfig_float("libplayer.hls.downlevel", &value);
    }
    if(ret < 0 || value <0){
        ret = -1;
    }else{
        ret= value;
    }
    av_log(NULL, AV_LOG_INFO,"just get adaptation extend parameter: %f\n",ret);
    return ret;

}
#define AUDIO_BANDWIDTH_MAX 100000  //100k
static int select_best_variant(struct list_mgt *c)
{
    int i;
    int best_index = -1, best_band = -1;
    int min_index = 0, min_band = -1;
    int max_index = 0, max_band = -1;
    int m, f, a;
    AdaptationProfile vf;
    // Consider only 80% of the available bandwidth usable.
    CacheHttp_GetSpeed(c->cache_http_handle, &f, &m, &a);
    vf = get_adaptation_profile();
    
    //int bandwidthBps = (a * 8) / 10;
    if (m < 1||vf ==CONSTANT_ADAPTIVE) {
        int org_playing_index = -1;
        for (i = 0; i < c->n_variants; i++) {
            struct variant *v = c->variants[i];
            if (v->bandwidth == c->playing_variant->bandwidth) {
                org_playing_index = i;
                break;
            }
        }
        return org_playing_index;
    }
    
    int bandwidthBps = m;
    if(vf == CONSERVATIVE_ADAPTIVE){
        float sensitivity =get_adaptation_ex_para(0);
        if(sensitivity!=-1&&sensitivity>0.5&&sensitivity<1){
            bandwidthBps*=sensitivity;
        }else{
            bandwidthBps*=0.8;
        }
    }else if(vf ==MEAN_ADAPTIVE){

    }

    int up_counts = get_adaptation_ex_para(1);
    if(up_counts == -1){
        up_counts = 1;
    }
    int down_counts = get_adaptation_ex_para(2);
    if(down_counts == -1){
        down_counts = 1;
    }
    for (i = 0; i < c->n_variants; i++) {
        struct variant *v = c->variants[i];
        if (v->bandwidth <= bandwidthBps && v->bandwidth > best_band && v->bandwidth > AUDIO_BANDWIDTH_MAX) {
            best_band = v->bandwidth;
            best_index = i;
        }
        if (v->bandwidth > AUDIO_BANDWIDTH_MAX && (v->bandwidth < min_band || min_band < 0)) {
            min_band = v->bandwidth;
            min_index = i;
        }
        if (v->bandwidth > max_band || max_band < 0) {
            max_band = v->bandwidth;
            max_index = i;
        }
    }
    if (best_index < 0) { /*no low rate streaming found,used the lowlest*/
        best_index = min_index;
    }

    if (c->playing_variant->bandwidth > c->variants[best_index]->bandwidth) {
        c->strategy_down_counts++;
        if (c->strategy_down_counts == down_counts) {
            c->strategy_down_counts = 0;
            return best_index;
        } else {
            int playing_index = -1;
            for (i = 0; i < c->n_variants; i++) {
                struct variant *v = c->variants[i];
                if (v->bandwidth == c->playing_variant->bandwidth) {
                    playing_index = i;
                    break;
                }
            }
            return playing_index;
        }

    } else if (c->playing_variant->bandwidth < c->variants[best_index]->bandwidth) {
        c->strategy_up_counts++;
        if (c->strategy_up_counts == up_counts) {
            c->strategy_up_counts = 0;
            return best_index;
        } else {
            int playing_index = -1;
            for (i = 0; i < c->n_variants; i++) {
                struct variant *v = c->variants[i];
                if (v->bandwidth == c->playing_variant->bandwidth) {
                    playing_index = i;
                    break;
                }
            }
            return playing_index;
        }
    }
    av_log(NULL, AV_LOG_INFO, "select best bandwidth,index:%d,bandwidth:%d\n", best_index, c->variants[best_index]->bandwidth);
    return best_index;
}
static struct list_item * switchto_next_item(struct list_mgt *mgt) {
    struct list_item *next = NULL;
    struct list_item *current = NULL;
    if (!mgt) {
        return NULL;
    }

    if (mgt->n_variants > 0) { //vod,have mulit-bandwidth streams
        mgt->playing_item_index = mgt->current_item->index;
        mgt->playing_item_seq = mgt->current_item->seq;
        av_log(NULL, AV_LOG_INFO, "current playing item index: %d,current playing seq:%d\n", mgt->playing_item_index, mgt->playing_item_seq);
        int estimate_index = select_best_variant(mgt);
        if (mgt->playing_variant->bandwidth != mgt->variants[estimate_index]->bandwidth) { //will remove this tricks.

            if (mgt->item_num > 0) {
                list_delall_item(mgt);
            }

            mgt->current_item = mgt->current_item->next = NULL;
            mgt->start_seq = -1;
            mgt->next_seq = -1;
            mgt->next_index = 0;
            mgt->item_num = 0;
            mgt->full_time = 0;
            mgt->playing_variant = mgt->variants[estimate_index];

            if(mgt->cur_uio){
                url_fclose(mgt->cur_uio);
                mgt->cur_uio = NULL;
            }
        }

        av_log(NULL, AV_LOG_INFO, "select best variant,bandwidth: %d\n", mgt->playing_variant->bandwidth);

    }
    int64_t reload_interval = mgt->item_num > 0 && mgt->current_item != NULL ?
                              mgt->current_item->duration :
                              mgt->target_duration;
    reload_interval *= 1000000;
    int isNeedFetch = 1;

reload:
    if (mgt->current_item == NULL || mgt->current_item->next == NULL) {
        /*new refresh this mgtlist now*/
        ByteIOContext *bio = mgt->cur_uio;

        int ret;
        char* url = NULL;

        if (mgt->n_variants > 0 && NULL != mgt->playing_variant) {
            url = mgt->playing_variant->url;
            av_log(NULL, AV_LOG_INFO, "list open variant url:%s,bandwidth:%d\n", url, mgt->playing_variant->bandwidth);
        } else {
            url = mgt->filename;
            av_log(NULL, AV_LOG_INFO, "list open url:%s\n", url);
        }

        if (!mgt->have_list_end && (av_gettime() - mgt->last_load_time < reload_interval)) {
            av_log(NULL, AV_LOG_INFO, "drop fetch playlist from server\n");
            isNeedFetch = 0;
        }
        if (url_interrupt_cb()) {
            return NULL;
        }
        if (isNeedFetch == 0 || (ret = list_open_internet(&bio, mgt, url, mgt->flags | URL_MINI_BUFFER | URL_NO_LP_BUFFER)) != 0) {
            goto switchnext;
        }
      
        if (mgt->current_item && mgt->current_item->file) { /*current item,switch to next*/
            current = mgt->current_item;
            next = mgt->current_item->next;
            for (; next != NULL; next = next->next) {
                if (next->file && strcmp(current->file, next->file) == 0) {
                    /*found the same item,switch to the next*/
                    current = next;
                    break;
                }
            }
#if 0
            while (current != mgt->item_list) {
                /*del the old item,lest current,and then play current->next*/
                list_del_item(mgt, mgt->item_list);
            }
#endif

            mgt->current_item = current; /*switch to new current;*/
            if (!mgt->have_list_end && mgt->item_num > LIVE_LIST_MAX) {
                list_shrink_live_list(mgt);
            }
        }
    }
switchnext:
    if (mgt->current_item) {
        next = mgt->current_item->next;

    } else {
        if (mgt->n_variants < 1 || !mgt->have_list_end) {
            if (!mgt->have_list_end && mgt->playing_item_seq > 0) {
                next = list_find_item_by_seq(mgt, mgt->playing_item_seq);
                if (next == NULL) {
                    av_log(NULL, AV_LOG_WARNING, "can't find same seq item,just featch first node\n");
                    next = mgt->item_list;
                }
            } else {
                next = mgt->item_list;
            }
        } else {
            next = list_find_item_by_index(mgt, mgt->playing_item_index);
        }

    }
    if (mgt->listclose)
	return NULL;
    if (next)
        av_log(NULL, AV_LOG_INFO, "switch to new file=%s,total=%d,start=%.4lf,duration=%.4lf,index:%d,seq:%d\n",
               next->file, mgt->item_num, next->start_time, next->duration,next->index,next->seq);
    else {
        av_log(NULL, AV_LOG_INFO, "switch to new file=NULL,total=%d\n", mgt->item_num);
        if (!mgt->have_list_end) {


            if (url_interrupt_cb()) {
                return NULL;
            }
            usleep(100 * 1000);
            reload_interval = mgt->target_duration * 250000;
            isNeedFetch = 1;
            goto reload;
        }
    }
    return next;
}



static int list_read(URLContext *h, unsigned char *buf, int size)
{
    struct list_mgt *mgt = h->priv_data;
    int len = AVERROR(EIO);
    int counts = 200;
    do {
        if (url_interrupt_cb()) {
            av_log(NULL, AV_LOG_ERROR, " url_interrupt_cb\n");
            len = AVERROR(EINTR);
            break;
        }
        len = CacheHttp_Read(mgt->cache_http_handle, buf, size);
        /*
        if (len == AVERROR(EAGAIN)) {
            usleep(1000 * 10);
            break;
        }*/
        if (len <0) {
            usleep(1000 * 10);
        }
        if (len >= 0) {
            break;
        }
    } while (counts-- > 0);
    
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
    struct list_item *item;
    
    int fulltime;

    if (whence == AVSEEK_BUFFERED_TIME) {
        int64_t buffed_time = 0;

        buffed_time = CacheHttp_GetBuffedTime(mgt->cache_http_handle);
        //av_log(NULL, AV_LOG_INFO, "list current buffed_time=%lld\n",buffed_time);
        if (buffed_time >= 0) {
            return buffed_time;

        } else {
            av_log(NULL, AV_LOG_DEBUG, "Can't get buffer time: %lld\n", buffed_time);
            return 0;
        }
    }

    if (whence == AVSEEK_SIZE) {
        return mgt->file_size;
    }
    av_log(NULL, AV_LOG_INFO, "list_seek pos=%lld,whence=%x\n", pos, whence);
    if (whence == AVSEEK_FULLTIME) {
        if (mgt->have_list_end) {
            av_log(NULL, AV_LOG_INFO, "return mgt->full_timet=%d\n", mgt->full_time);
            return (int64_t)mgt->full_time;
        } else {
            return -1;
        }
    }

    if (whence == AVSEEK_TO_TIME) {
        av_log(NULL, AV_LOG_INFO, "list_seek to Time =%lld,whence=%x\n", pos, whence);

        if (pos >= 0 && pos < mgt->full_time) {
            for (item = mgt->item_list; item; item = item->next) {

                if (item->start_time <= pos && pos < item->start_time + item->duration) {
                    mgt->current_item = NULL;
                    CacheHttp_Reset(mgt->cache_http_handle);
                    mgt->current_item = item;
                    mgt->playing_item_index = item->index - 1;
                    if (!mgt->have_list_end) {
                        mgt->playing_item_seq = item->seq - 1;
                    }
                    av_log(NULL, AV_LOG_INFO, "list_seek to item->file =%s\n", item->file);
                    //CacheHttp_Reset(mgt->cache_http_handle);
                    return (int64_t)(item->start_time);/*pos=0;*/
                }
            }


        }


    }
    av_log(NULL, AV_LOG_INFO, "list_seek failed\n");
    return -1;
}
static int list_close(URLContext *h)
{
    struct list_mgt *mgt = h->priv_data;

    if (!mgt) {
        return 0;
    }
    mgt->listclose = 1;
    CacheHttp_Close(mgt->cache_http_handle);
    list_delall_item(mgt);
    int i;
    for (i = 0; i < mgt->n_variants; i++) {
        struct variant *var = mgt->variants[i];
        av_free(var);
    }
    av_freep(&mgt->variants);
    mgt->n_variants = 0;
    if (NULL != mgt->prefix) {
        av_free(mgt->prefix);
        mgt->prefix = NULL;
    }

    if(NULL!=mgt->ipad_ex_headers){
        av_free(mgt->ipad_ex_headers);
    }
    if(NULL!= mgt->ipad_req_media_headers){
        av_free(mgt->ipad_req_media_headers);
    }

    if (mgt->cur_uio != NULL) {
        url_fclose(mgt->cur_uio);
        mgt->cur_uio = NULL;
    }
    av_free(mgt);
    unused(h);
    gListMgt = NULL;
    return 0;
}
static int list_get_handle(URLContext *h)
{
    return (intptr_t) h->priv_data;
}

URLProtocol file_list_protocol = {
    .name = "list",
    .url_open = list_open,
    .url_read  = list_read,
    .url_write = list_write,
    .url_seek   = list_seek,
    .url_close = list_close,
    .url_exseek = list_seek, /*same as seek is ok*/
    .url_get_file_handle = list_get_handle,
};
list_item_t* getCurrentSegment(void* hSession)
{
    struct list_item *item = gListMgt->current_item;
    if(item)
        item->have_list_end = gListMgt->have_list_end;
    return item;

}
const char* getCurrentSegmentUrl(void* hSession)
{
    if (gListMgt == NULL) {
        return NULL;
    }

    const char* url = getCurrentSegment(gListMgt)->file;

    return url;
}

long long getTotalDuration(void* hSession)
{
    if (gListMgt == NULL) {
        return NULL;
    }
    return gListMgt->full_time;
}

int switchNextSegment(void* hSession)
{
    gListMgt->current_item = switchto_next_item(gListMgt);
    return 0;
}
URLProtocol *get_file_list_protocol(void)
{
    return &file_list_protocol;
}
int register_list_demux_all(void)
{
    static int registered_all = 0;
    if (registered_all) {
        return;
    }
    registered_all++;
    extern struct list_demux m3u_demux;
    av_register_protocol(&file_list_protocol);
    register_list_demux(&m3u_demux);
    return 0;
}

