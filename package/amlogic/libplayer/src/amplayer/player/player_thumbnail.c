#include <stdio.h>
#include <stdlib.h>
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
    
    

    if(av_open_input_file(&stream->pFormatCtx, filename, NULL, 0, NULL) != 0) {
        log_print("Coundn't open file %s !\n", filename);
        goto err;
    }

    if(av_find_stream_info(stream->pFormatCtx) < 0) {
        log_print("Coundn't find stream information !\n");
        goto err1;
    }

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

    calc_aspect_ratio(&frame->displayAspectRatio, stream);

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
    int frameFinished;
    int count;
    struct video_frame *frame = (struct video_frame *)handle;
    struct stream *stream = &frame->stream;
    AVPacket        packet;

    while(av_read_frame(stream->pFormatCtx, &packet) >= 0) {
        if(packet.stream_index==stream->videoStream){
            if(count >= 10){
                log_print("exceed count, cann't get frame!\n");
		  av_free_packet(&packet);
		  break;
            }
			
            avcodec_decode_video2(stream->pCodecCtx, stream->pFrameYUV, &frameFinished, &packet);
	     if(frameFinished && stream->pFrameYUV->key_frame){
		  count++;
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

    tag = av_dict_get(stream->pFormatCtx->metadata, key, tag, AV_METADATA_MATCH_CASE);
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
