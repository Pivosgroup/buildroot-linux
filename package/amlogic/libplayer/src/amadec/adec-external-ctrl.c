/**
 * \file adec-external-ctrl.c
 * \brief  Audio Dec Lib Functions
 * \version 1.0.0
 * \date 2011-03-08
 */
/* Copyright (C) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <audio-dec.h>


int audio_decode_basic_init(void)
{
#ifndef ALSA_OUT
    android_basic_init();
#endif
    return 0;
}

/**
 * \brief init audio dec
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_init(void **handle, arm_audio_info *a_ainfo)
{
    int ret;
    aml_audio_dec_t *audec;

    if (*handle) {
        adec_print("Existing an audio dec instance!Need not to create it !");
        return -1;
    }

    audec = (aml_audio_dec_t *)malloc(sizeof(aml_audio_dec_t));
    if (audec == NULL) {
        adec_print("malloc failed! not enough memory !");
        return -1;
    }
    //set param for arm audio decoder
    memset(audec, 0, sizeof(aml_audio_dec_t));
    audec->channels=a_ainfo->channels;
    audec->samplerate=a_ainfo->sample_rate;
    audec->format=a_ainfo->format;
    audec->adsp_ops.dsp_file_fd=a_ainfo->handle;
    audec->extradata_size=a_ainfo->extradata_size;
    if(a_ainfo->extradata_size>0&&a_ainfo->extradata_size<=AUDIO_EXTRA_DATA_SIZE)
        memcpy((char*)audec->extradata,(char*)a_ainfo->extradata,a_ainfo->extradata_size);
   
//	adec_print("audio_decode_init  pcodec = %d, pcodec->ctxCodec = %d!\n", pcodec, pcodec->ctxCodec);
    ret = audiodec_init(audec);
    if (ret) {
        adec_print("adec init failed!");
        return -1;
    }

    *handle = (void *)audec;

    return 0;
}

/**
 * \brief start audio dec
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_start(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_START;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief pause audio dec
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_pause(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_PAUSE;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief resume audio dec
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_resume(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_RESUME;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief stop audio dec
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_stop(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    audec->need_stop = 1;

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_STOP;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief release audio dec
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_release(void **handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *) * handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_RELEASE;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    ret = pthread_join(audec->thread_pid, NULL);

    free(*handle);
    *handle = NULL;

    return ret;

}

/**
 * \brief set auto-mute state in audio decode
 * \param handle pointer to player private data
 * \param stat 1 = enable automute, 0 = disable automute
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_automute(void *handle, int stat)
{
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    audec->auto_mute = 1;
    return 0;
}

/**
 * \brief mute audio output
 * \param handle pointer to player private data
 * \param en 1 = mute output, 0 = unmute output
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_set_mute(void *handle, int en)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_MUTE;
        cmd->value.en = en;
        cmd->has_arg = 1;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief set audio volume
 * \param handle pointer to player private data
 * \param vol volume value
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_set_volume(void *handle, float vol)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_SET_VOL;
        cmd->value.volume = vol;
	 audec->volume = vol;
        cmd->has_arg = 1;	
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief set audio volume
 * \param handle pointer to player private data
 * \param vol volume value
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_set_lrvolume(void *handle, float lvol,float rvol)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_SET_LRVOL;
        cmd->value.volume = lvol;
	 audec->volume = lvol;
        cmd->has_arg = 1;
	 cmd->value_ext.volume = rvol;
	 audec->volume_ext = rvol;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief set audio volume
 * \param handle pointer to player private data
 * \param vol volume value
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_get_volume(void *handle, float *vol)
{
    int ret = 0;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    *vol = audec->volume;

    return ret;
}

/**
 * \brief set audio volume
 * \param handle pointer to player private data
 * \param lvol: left volume value,rvol:right volume value
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_decode_get_lrvolume(void *handle, float *lvol,float* rvol)
{
    int ret = 0;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    *lvol = audec->volume;
    *rvol = audec->volume_ext;

    return ret;
}

/**
 * \brief swap audio left and right channels
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_channels_swap(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
	 audec->soundtrack = HW_CHANNELS_SWAP;	 
        cmd->ctrl_cmd = CMD_CHANL_SWAP;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief output left channel
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_channel_left_mono(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
	 audec->soundtrack = HW_LEFT_CHANNEL_MONO;
        cmd->ctrl_cmd = CMD_LEFT_MONO;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief output right channel
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_channel_right_mono(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
	 audec->soundtrack = HW_RIGHT_CHANNEL_MONO;
        cmd->ctrl_cmd = CMD_RIGHT_MONO;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief output left and right channels
 * \param handle pointer to player private data
 * \return 0 on success otherwise -1 if an error occurred
 */
int audio_channel_stereo(void *handle)
{
    int ret;
    adec_cmd_t *cmd;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    cmd = adec_message_alloc();
    if (cmd) {
	 audec->soundtrack = HW_STEREO_MODE;
        cmd->ctrl_cmd = CMD_STEREO;
        ret = adec_send_message(audec, cmd);
    } else {
        adec_print("message alloc failed, no memory!");
        ret = -1;
    }

    return ret;
}

/**
 * \brief check output mute or not
 * \param handle pointer to player private data
 * \return 1 = output is mute, 0 = output not mute
 */
int audio_output_muted(void *handle)
{
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    return audec->muted;
}

/**
 * \brief check audiodec ready or not
 * \param handle pointer to player private data
 * \return 1 = audiodec is ready, 0 = audiodec not ready
 */
int audio_dec_ready(void *handle)
{
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    if (audec->state > INITTED) {
        return 1;
    } else {
        return 0;
    }
}

/**
 * \brief get audio dsp decoded frame number
 * \param handle pointer to player private data
 * \return n = audiodec frame number, -1 = error
 */
int audio_get_decoded_nb_frames(void *handle)
{
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    audec->decoded_nb_frames = audiodsp_get_decoded_nb_frames(&audec->adsp_ops);
    //adec_print("audio_get_decoded_nb_frames:  %d!", audec->decoded_nb_frames);
    if (audec->decoded_nb_frames >= 0) {
        return audec->decoded_nb_frames;
    } else {
        return -2;
    }
}

/**
 * \brief set av sync threshold in ms.
 * \param handle pointer to player private data
 * \param threshold av sync time threshold in ms
 */
int audio_set_av_sync_threshold(void *handle, int threshold)
{
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    if ((threshold > 500) || (threshold < 60)) {
        adec_print("threshold %d id too small or too large.\n", threshold);
    }

    audec->avsync_threshold = threshold * 90;
    return 0;
}
int audio_get_soundtrack(void *handle, int* strack )
{
    int ret =0;
    aml_audio_dec_t *audec = (aml_audio_dec_t *)handle;

    if (!handle) {
        adec_print("audio handle is NULL !\n");
        return -1;
    }

    *strack= audec->soundtrack;

    return ret;    
}