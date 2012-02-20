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

ADEC_BEGIN_DECLS

#define  AUDIO_CTRL_DEVICE    "/dev/amaudio_ctl"

#define AMAUDIO_IOC_MAGIC  'A'
#define AMAUDIO_IOC_SET_LEFT_MONO               _IOW(AMAUDIO_IOC_MAGIC, 0x0e, int)
#define AMAUDIO_IOC_SET_RIGHT_MONO              _IOW(AMAUDIO_IOC_MAGIC, 0x0f, int)
#define AMAUDIO_IOC_SET_STEREO               _IOW(AMAUDIO_IOC_MAGIC, 0x10, int)
#define AMAUDIO_IOC_SET_CHANNEL_SWAP            _IOW(AMAUDIO_IOC_MAGIC, 0x11, int)

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
    audio_format_t  format;
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
    hw_command_t soundtrack;
    audio_out_operations_t aout_ops;
    dsp_operations_t adsp_ops;
    message_pool_t message_pool;
};


/***********************************************************************************************/
extern void android_basic_init(void);
int audiodec_init(aml_audio_dec_t *);
int adec_message_pool_init(aml_audio_dec_t *);
adec_cmd_t *adec_message_alloc(void);
int adec_message_free(adec_cmd_t *);
int adec_send_message(aml_audio_dec_t *, adec_cmd_t *);
adec_cmd_t *adec_get_message(aml_audio_dec_t *);
int feeder_init(aml_audio_dec_t *);
int feeder_release(aml_audio_dec_t *);

ADEC_END_DECLS

#endif
