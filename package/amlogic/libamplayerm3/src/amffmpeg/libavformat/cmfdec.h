#ifndef AVFORMAT_CMFDEMUX_H
#define AVFORMAT_CMFDEMUX_H

#include "avio.h"
#include "internal.h"
#include "cmfvpb.h"




typedef struct cmf {
    struct cmfvpb *cmfvpb;
    AVFormatContext *sctx;
    AVPacket pkt;
    int64_t parsering_index;
    int64_t calc_startpts;
} cmf_t;




#endif






