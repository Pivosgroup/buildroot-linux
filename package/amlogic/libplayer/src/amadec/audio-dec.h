/**
 * \file audio-dec.h
 * \brief  Definitiond Of Audio Dec Types And Structures
 * \version 1.0.0
 * \date 2011-03-08
 */
/* Copyright (C) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */
#ifndef AUDIO_DEC_H
#define AUDIO_DEC_H

#include<pthread.h>

#include <audio-out.h>
#include <audiodsp.h>
#include <adec-types.h>
#include <adec-message.h>
#include <log-print.h>
#include <adec-armdec-mgt.h>
ADEC_BEGIN_DECLS

#define  AUDIO_CTRL_DEVICE    "/dev/amaudio_ctl"

#define AMAUDIO_IOC_MAGIC  'A'
#define AMAUDIO_IOC_SET_LEFT_MONO               _IOW(AMAUDIO_IOC_MAGIC, 0x0e, int)
#define AMAUDIO_IOC_SET_RIGHT_MONO              _IOW(AMAUDIO_IOC_MAGIC, 0x0f, int)
#define AMAUDIO_IOC_SET_STEREO               _IOW(AMAUDIO_IOC_MAGIC, 0x10, int)
#define AMAUDIO_IOC_SET_CHANNEL_SWAP            _IOW(AMAUDIO_IOC_MAGIC, 0x11, int)

//for ffmpeg audio decode
#define AMSTREAM_IOC_MAGIC  'S'
#define AMSTREAM_IOC_APTS_LOOKUP    _IOR(AMSTREAM_IOC_MAGIC, 0x81,unsigned long)   
#define GET_FIRST_APTS_FLAG			_IOR(AMSTREAM_IOC_MAGIC, 0x82, long)

/*******************************************************************************************/

typedef struct aml_audio_dec    aml_audio_dec_t;
typedef enum {
    HW_STEREO_MODE = 0,
    HW_LEFT_CHANNEL_MONO,
    HW_RIGHT_CHANNEL_MONO,
    HW_CHANNELS_SWAP,
} hw_command_t;

struct aml_audio_dec {
    adec_state_t  state;
    pthread_t       thread_pid;
    adec_audio_format_t  format;
    int channels;
    int samplerate;
    int data_width;
    int need_stop;
    int auto_mute;
    int muted;
    int decoded_nb_frames;
    int avsync_threshold;
    float volume; //left or main volume
    float volume_ext; //right	
    //codec_para_t *pcodec;
    hw_command_t soundtrack;
    audio_out_operations_t aout_ops;
    dsp_operations_t adsp_ops;
    message_pool_t message_pool;
    audio_decoder_operations_t *adec_ops;//non audiodsp decoder operations
    int extradata_size;      ///< extra data size
    char extradata[AUDIO_EXTRA_DATA_SIZE];
};

//from amcodec
typedef struct {
    int sample_rate;         ///< audio stream sample rate
    int channels;            ///< audio stream channels
    int format;            ///< codec format id
    int handle;        ///< codec device handler
    int extradata_size;      ///< extra data size
    char extradata[AUDIO_EXTRA_DATA_SIZE];
} arm_audio_info;
//status check
struct adec_status {
    unsigned int channels;
    unsigned int sample_rate;
    unsigned int resolution;
    unsigned int error_count;
    unsigned int status;
};

//audio decoder type, default arc
#define AUDIO_ARC_DECODER 0
#define AUDIO_ARM_DECODER 1
#define AUDIO_FFMPEG_DECODER 2

#define    ACODEC_FMT_NULL   -1
#define    ACODEC_FMT_MPEG   0
#define    ACODEC_FMT_PCM_S16LE  1
#define    ACODEC_FMT_AAC   2
#define    ACODEC_FMT_AC3    3
#define    ACODEC_FMT_ALAW  4
#define    ACODEC_FMT_MULAW  5
#define    ACODEC_FMT_DTS  6
#define    ACODEC_FMT_PCM_S16BE  7
#define    ACODEC_FMT_FLAC  8
#define    ACODEC_FMT_COOK  9
#define    ACODEC_FMT_PCM_U8  10
#define    ACODEC_FMT_ADPCM  11
#define    ACODEC_FMT_AMR   12
#define    ACODEC_FMT_RAAC   13
#define    ACODEC_FMT_WMA   14
#define    ACODEC_FMT_WMAPRO    15
#define    ACODEC_FMT_PCM_BLURAY   16
#define    ACODEC_FMT_ALAC   17
#define    ACODEC_FMT_VORBIS     18
#define    ACODEC_FMT_AAC_LATM    19
#define    ACODEC_FMT_APE    20

/***********************************************************************************************/
extern void android_basic_init(void);
int audiodec_init(aml_audio_dec_t *aml_audio_dec);
int adec_message_pool_init(aml_audio_dec_t *);
adec_cmd_t *adec_message_alloc(void);
int adec_message_free(adec_cmd_t *);
int adec_send_message(aml_audio_dec_t *, adec_cmd_t *);
adec_cmd_t *adec_get_message(aml_audio_dec_t *);
int feeder_init(aml_audio_dec_t *);
int feeder_release(aml_audio_dec_t *);
void *adec_armdec_loop(void *args);
ADEC_END_DECLS
#endif
