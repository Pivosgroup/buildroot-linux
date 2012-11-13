#include <stdio.h>
#include <stdlib.h>
#include <player_priv.h>
#include <log_print.h>
#include "thumbnail_type.h"

static inline void calc_aspect_ratio(rational *ratio, struct stream *stream)
{
    int num, den;
	
    av_reduce(&num, &den, 
			stream->pCodecCtx->width * stream->pCodecCtx->sample_aspect_ratio.num, 
			stream->pCodecCtx->height * stream->pCodecCtx->sample_aspect_ratio.den,
			1024*1024);
    ratio->num = num;
    ratio->den = den;
}

static void find_best_keyframe(AVFormatContext *pFormatCtx, int video_index, int count, int64_t *time, int64_t *offset)
{
    int i = 0;
    int j = 0;
    int maxFrameSize = 0;
    int64_t thumbTime = 0;
    int64_t thumbOffset = 0;
    AVPacket packet;
    int r = 0;

    r = av_read_frame(pFormatCtx, &packet);
    while((i < count) && (r >= 0) && (j < count*10)){
        //log_print("[%s]j=%d i=%d r=%d pktidx=%d[%d] flags=%x\n", __FUNCTION__,j, i,r,packet.stream_index,video_index, packet.flags);
        if(packet.stream_index == video_index) {    //find 100 video packets
            j ++;
            if(packet.flags & AV_PKT_FLAG_KEY){ // find key frame
                ++i;
                //log_print("[%s]packet.size=%d maxFrameSize=%d\n", __FUNCTION__,packet.size, maxFrameSize);

                if(packet.size > maxFrameSize){
                    maxFrameSize = packet.size;
                    thumbTime = packet.pts;                
                    thumbOffset = avio_tell(pFormatCtx->pb);  
                    //log_print("[%s]maxFrameSize=%d thumbTime=%lld thumbOffset=%lld\n", __FUNCTION__,maxFrameSize, thumbTime, thumbOffset);
                }
                av_free_packet(&packet);           
            }
        }
        r = av_read_frame(pFormatCtx, &packet);        
    }
    //log_print("[%s]j=%d i=%d r=%d\n", __FUNCTION__,j, i,r);
    if (i == count)
        av_free_packet(&packet);
    *time = thumbTime;
    *offset = thumbOffset;    
}

static void find_thumbnail_frame(AVFormatContext *pFormatCtx, int video_index, int64_t *thumb_time, int64_t *thumb_offset)
{    
    int64_t thumbTime = 0;
    int64_t thumbOffset = 0;
    AVPacket packet;
    AVStream *st = pFormatCtx->streams[video_index];
    int duration = pFormatCtx->duration/AV_TIME_BASE;
    int64_t init_seek_time = (duration>0) ? MIN(10, duration>>1) : 10;
    int ret = 0;
    
    init_seek_time = av_rescale_q(init_seek_time*AV_TIME_BASE, st->time_base, AV_TIME_BASE_Q);
    log_print("[find_thumbnail_frame]duration=%lld init_seek_time=%lld timebase=%d:%d\n",pFormatCtx->duration,init_seek_time,st->time_base.num,st->time_base.den);
    ret = av_seek_frame(pFormatCtx, video_index, init_seek_time, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) {
        avio_seek(pFormatCtx->pb, 0, SEEK_SET);
        log_error("[%s]seek error, reset offset to 0\n", __FUNCTION__);
        return;
    }
    find_best_keyframe(pFormatCtx, video_index, 5, &thumbTime, &thumbOffset);
    
    if(thumbTime != AV_NOPTS_VALUE)
        *thumb_time = av_rescale_q(thumbTime, st->time_base, AV_TIME_BASE_Q);
    else
        *thumb_time = AV_NOPTS_VALUE;
    *thumb_offset = thumbOffset;
}

void * thumbnail_res_alloc(void)
{
    struct video_frame * frame;

    frame = (struct video_frame *)malloc(sizeof(struct video_frame));
    if(frame == NULL)
        return NULL;
    memset(frame, 0, sizeof(struct video_frame));

    av_register_all();

    return (void *)frame;
}

int thumbnail_find_stream_info(void *handle, const char* filename)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;

    if(av_open_input_file(&stream->pFormatCtx, filename, NULL, 0, NULL) != 0) {
        log_print("Coundn't open file %s !\n", filename);
        goto err;
    }

    if(av_find_stream_info(stream->pFormatCtx) < 0) {
        log_print("Coundn't find stream information !\n");
        goto err1;
    }

    return 0;

err1:
    av_close_input_file(stream->pFormatCtx);
err:
    memset(&frame->stream, 0, sizeof(struct stream));

    return -1;   
}

int thumbnail_find_stream_info_end(void *handle)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;

    av_close_input_file(stream->pFormatCtx);

    return 0;
}

int thumbnail_decoder_open(void *handle, const char* filename)
{
    int i;
    int video_index = -1;
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
    
    log_print("thumbnail open file:%s\n", filename);

    if(av_open_input_file(&stream->pFormatCtx, filename, NULL, 0, NULL) != 0) {
        log_print("Coundn't open file %s !\n", filename);
        goto err;
    }

    if(av_find_stream_info(stream->pFormatCtx) < 0) {
        log_print("Coundn't find stream information !\n");
        goto err1;
    }
    
    //dump_format(stream->pFormatCtx, 0, filename, 0);
    for(i=0; i < stream->pFormatCtx->nb_streams; i++){
        if(stream->pFormatCtx->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO){
            video_index = i;
            break;
        }
    }

    if(video_index == -1){
        log_print("Didn't find a video stream!\n");
	 goto err1;
    }

   find_thumbnail_frame(stream->pFormatCtx, video_index, &frame->thumbNailTime, &frame->thumbNailOffset); 
	
    stream->videoStream = video_index;
    stream->pCodecCtx = stream->pFormatCtx->streams[video_index]->codec;
    if(stream->pCodecCtx == NULL)
        log_print("pCodecCtx is NULL !\n");

    frame->width = stream->pCodecCtx->width;
    frame->height = stream->pCodecCtx->height;

    stream->pCodec = avcodec_find_decoder(stream->pCodecCtx->codec_id);
    if(stream->pCodec == NULL){
        log_print("Didn't find codec!\n");
	 goto err1;
    }

    if(avcodec_open(stream->pCodecCtx, stream->pCodec)<0){
        log_print("Couldn't open codec!\n");
	 goto err1;
    }
	
    frame->duration = stream->pFormatCtx->duration;

    stream->pFrameYUV = avcodec_alloc_frame();
    if(stream->pFrameYUV == NULL) {
        log_print("alloc YUV frame failed!\n");
        goto err2;
    }
	
    stream->pFrameRGB=avcodec_alloc_frame();
    if(stream->pFrameRGB==NULL){
        log_print("alloc RGB frame failed!\n");
	 goto err3;
    }

    frame->DataSize = avpicture_get_size(DEST_FMT, frame->width, frame->height);
    frame->data = (char *)malloc(frame->DataSize);
    if(frame->data == NULL){
        log_print("alloc buffer failed!\n");
	 goto err4;
    }

    avpicture_fill((AVPicture *)stream->pFrameRGB, frame->data, DEST_FMT, frame->width, frame->height);

    return 0;
	
err4:
    av_free(stream->pFrameRGB);
err3:
    av_free(stream->pFrameYUV);
err2:
    avcodec_close(stream->pCodecCtx);
err1:
    av_close_input_file(stream->pFormatCtx);
err:
    memset(&frame->stream, 0, sizeof(struct stream));

    return -1;
}

int thumbnail_extract_video_frame(void *handle, int64_t time, int flag)
{
    int frameFinished = 0;
    int count = 0;
    int tryNum = 0;
    int i = 0;
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
    AVFormatContext *pFormatCtx = stream->pFormatCtx;
    AVPacket        packet;
    AVCodecContext *pCodecCtx = pFormatCtx->streams[stream->videoStream]->codec;

    if (time >= 0) {
        //thumbTime = av_rescale_q(time, AV_TIME_BASE_Q, stream->pFormatCtx->streams[stream->videoStream]->time_base);
        av_seek_frame(pFormatCtx, stream->videoStream, time, AVSEEK_FLAG_BACKWARD);
        find_best_keyframe(pFormatCtx, stream->videoStream, 5, &frame->thumbNailTime, &frame->thumbNailOffset);
        log_print("[thumbnail_extract_video_frame]time=%lld time=%lld offset=%lld!\n", time, frame->thumbNailTime, frame->thumbNailOffset);
    }
    
    if(frame->thumbNailTime != AV_NOPTS_VALUE) {
        log_print("seek to thumbnail frame by timestamp(0x%llx)!\n", frame->thumbNailTime);
        av_seek_frame(pFormatCtx, stream->videoStream, frame->thumbNailTime, AVSEEK_FLAG_BACKWARD);
	 }else{
	     log_print("seek to thumbnail frame by offset(%lld)!\n", frame->thumbNailOffset);
            avio_seek(pFormatCtx->pb, frame->thumbNailOffset, SEEK_SET);
    }	 	
    
	avcodec_flush_buffers(stream->pCodecCtx);
	
    while(av_read_frame(pFormatCtx, &packet) >= 0) {
        if(packet.stream_index==stream->videoStream){
            if(tryNum > 30){
                log_print("exceed count, cann't get frame!\n");
		  av_free_packet(&packet);
		  break;
            }
			
            avcodec_decode_video2(stream->pCodecCtx, stream->pFrameYUV, &frameFinished, &packet);
	     tryNum++;
            //log_print("[%s]decode a video frame, finish=%d key=%d count==%d\n", __FUNCTION__, frameFinished, stream->pFrameYUV->key_frame,count);
	     if(frameFinished && stream->pFrameYUV->key_frame){
		  count++;
                 //log_print("[%s]pCodecCtx->codec_id=%x count==%d\n", __FUNCTION__, pCodecCtx->codec_id,count);
                 if ((pCodecCtx->codec_id == CODEC_ID_MPEG1VIDEO)
                        || (pCodecCtx->codec_id == CODEC_ID_MPEG2VIDEO)
                        || (pCodecCtx->codec_id == CODEC_ID_MPEG2VIDEO_XVMC)) {
                        if(count < 6) {
                        log_print("mpeg video: decoder %d keyframe\n", count);
                        av_free_packet(&packet);
                        continue;
    		        }
                    }	
                    struct SwsContext *img_convert_ctx;
                    img_convert_ctx = sws_getContext(stream->pCodecCtx->width, stream->pCodecCtx->height, 
                    					stream->pCodecCtx->pix_fmt, 
                    					frame->width, frame->height, DEST_FMT, SWS_BICUBIC,
                    					NULL, NULL, NULL);
                    if(img_convert_ctx == NULL) {
                    log_print("can not initialize the coversion context!\n");
                    av_free_packet(&packet);
                    break;
                    }

                    sws_scale(img_convert_ctx, stream->pFrameYUV->data, stream->pFrameYUV->linesize, 0, 
                            frame->height, stream->pFrameRGB->data, stream->pFrameRGB->linesize);
                    av_free_packet(&packet);
                    goto ret;
            }
        }

        av_free_packet(&packet);
    }

    if(frame->data)
        free(frame->data);
    av_free(stream->pFrameRGB);
    av_free(stream->pFrameYUV);
    avcodec_close(stream->pCodecCtx);
    av_close_input_file(stream->pFormatCtx);
    memset(&frame->stream, 0, sizeof(struct stream));
    return -1;

ret:
    return 0;
}

int thumbnail_read_frame(void *handle, char* buffer)
{
    int i;
    int index = 0;
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
	
    for(i = 0; i < frame->height; i++){
        memcpy(buffer + index, stream->pFrameRGB->data[0] + i*stream->pFrameRGB->linesize[0], frame->width*2);
        index += frame->width * 2;
    }

    return 0;
}

void thumbnail_get_video_size(void *handle, int* width, int* height)
{
    struct video_frame *frame = (struct video_frame *)handle;

    *width = frame->width;
    *height = frame->height;
}

float thumbnail_get_aspect_ratio(void *handle)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;

    calc_aspect_ratio(&frame->displayAspectRatio, stream);

    if( !frame->displayAspectRatio.num || !frame->displayAspectRatio.den)
        return (float)frame->width / frame->height;
    else
        return (float)frame->displayAspectRatio.num / frame->displayAspectRatio.den;
}

void thumbnail_get_duration(void *handle, int64_t *duration)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;

    *duration = stream->pFormatCtx->duration;
}

int thumbnail_get_key_metadata(void* handle, char* key, const char** value)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
    AVDictionaryEntry *tag=NULL;

    if( !stream->pFormatCtx->metadata)
        return 0;

    tag = av_dict_get(stream->pFormatCtx->metadata, key, tag, 0);
    if(tag) {
        *value = tag->value;
        return 1;
    }

    return 0;
}

int thumbnail_get_key_data(void* handle, char* key, const void** data, int* data_size)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
    AVDictionaryEntry *tag=NULL;
	
    if( !stream->pFormatCtx->metadata)
        return 0;

    if(av_dict_get(stream->pFormatCtx->metadata, key, tag, AV_METADATA_IGNORE_SUFFIX)) {
        *data = stream->pFormatCtx->cover_data;
        *data_size = stream->pFormatCtx->cover_data_len;
        return 1;
    }

    return 0;
}

void thumbnail_get_video_rotation(void *handle, int* rotation)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
    int stream_rotation = stream->pFormatCtx->streams[stream->videoStream]->rotation_degree;

    switch (stream_rotation) {
        case 1:
            *rotation = 90;
            break;

        case 2:
            *rotation = 180;
            break;

        case 3:
            *rotation = 270;
            break;

        default:
            *rotation = 0;
            break;
    }

    return;
}

int thumbnail_decoder_close(void *handle)
{
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;

    if(frame->data)
        free(frame->data);
    if(stream->pFrameRGB)
        av_free(stream->pFrameRGB);
    if(stream->pFrameYUV)
	 av_free(stream->pFrameYUV);

    avcodec_close(stream->pCodecCtx);
    av_close_input_file(stream->pFormatCtx);

    return 0;
}

void thumbnail_res_free(void* handle)
{
    struct video_frame *frame = (struct video_frame *)handle;

    if(frame)
        free(frame);
}
