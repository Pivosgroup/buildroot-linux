/**
 * \file adec-internal-mgt.c
 * \brief  Audio Dec Message Loop Thread
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
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>

#include <audio-dec.h>
#include <adec-pts-mgt.h>

/**
 * \brief set audio output mode.
 * \param cmd control commands
 * \return 0 on success otherwise -1
 */
static int audio_hardware_ctrl(hw_command_t cmd)
{
    int fd;

    fd = open(AUDIO_CTRL_DEVICE, O_RDONLY);
    if (fd < 0) {
        adec_print("Open Device %s Failed!", AUDIO_CTRL_DEVICE);
        return -1;
    }

    switch (cmd) {
    case HW_CHANNELS_SWAP:
        ioctl(fd, AMAUDIO_IOC_SET_CHANNEL_SWAP, 0);
        break;

    case HW_LEFT_CHANNEL_MONO:
        ioctl(fd, AMAUDIO_IOC_SET_LEFT_MONO, 0);
        break;

    case HW_RIGHT_CHANNEL_MONO:
        ioctl(fd, AMAUDIO_IOC_SET_RIGHT_MONO, 0);
        break;

    case HW_STEREO_MODE:
        ioctl(fd, AMAUDIO_IOC_SET_STEREO, 0);
        break;

    default:
        adec_print("Unknow Command %d!", cmd);
        break;

    };

    close(fd);

    return 0;

}

/**
 * \brief start audio dec when receive START command.
 * \param audec pointer to audec
 */
static void start_adec(aml_audio_dec_t *audec)
{
    int ret;
    audio_out_operations_t *aout_ops = &audec->aout_ops;
    dsp_operations_t *dsp_ops = &audec->adsp_ops;

    if (audec->state == INITTED) {
        audec->state = ACTIVE;

        while ((!audiodsp_get_first_pts_flag(dsp_ops)) && (!audec->need_stop)) {
            adec_print("wait first pts checkin complete !");
            usleep(100000);
        }

        /*start  the  the pts scr,...*/
        ret = adec_pts_start(audec);

        if (audec->auto_mute) {
            avsync_en(0);
            adec_pts_pause();

            while ((!audec->need_stop) && track_switch_pts(audec)) {
                usleep(1000);
            }

            avsync_en(1);
            adec_pts_resume();

            audec->auto_mute = 0;
        }

        aout_ops->start(audec);

    }
}

/**
 * \brief pause audio dec when receive PAUSE command.
 * \param audec pointer to audec
 */
static void pause_adec(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->state == ACTIVE) {
        audec->state = PAUSED;
        adec_pts_pause();
        aout_ops->pause(audec);
    }
}

/**
 * \brief resume audio dec when receive RESUME command.
 * \param audec pointer to audec
 */
static void resume_adec(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->state == PAUSED) {
        audec->state = ACTIVE;
        aout_ops->resume(audec);
        adec_pts_resume();
    }
}

/**
 * \brief stop audio dec when receive STOP command.
 * \param audec pointer to audec
 */
static void stop_adec(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->state > INITING) {
        audec->state = STOPPED;
        aout_ops->stop(audec);
        feeder_release(audec);
    }
}

/**
 * \brief release audio dec when receive RELEASE command.
 * \param audec pointer to audec
 */
static void release_adec(aml_audio_dec_t *audec)
{
    audec->state = TERMINATED;
}

/**
 * \brief mute audio dec when receive MUTE command.
 * \param audec pointer to audec
 * \param en 1 = mute, 0 = unmute
 */
static void mute_adec(aml_audio_dec_t *audec, int en)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (aout_ops->mute) {
        adec_print("%s the output !\n", (en ? "mute" : "unmute"));
        aout_ops->mute(audec, en);
        audec->muted = en;
    }
}

/**
 * \brief set volume to audio dec when receive SET_VOL command.
 * \param audec pointer to audec
 * \param vol volume value
 */
static void adec_set_volume(aml_audio_dec_t *audec, float vol)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (aout_ops->set_volume) {
        adec_print("set audio volume! vol = %f\n", vol);
        aout_ops->set_volume(audec, vol);
    }
}

static void adec_flag_check(aml_audio_dec_t *audec)
{
    audio_out_operations_t *aout_ops = &audec->aout_ops;

    if (audec->auto_mute && (audec->state > INITTED)) {
        aout_ops->pause(audec);
        while ((!audec->need_stop) && track_switch_pts(audec)) {
            usleep(1000);
        }
        aout_ops->resume(audec);
        audec->auto_mute = 0;
    }
}

/**
 * \brief adec main thread
 * \param args pointer to thread private data
 * \return NULL
 */
static void *adec_message_loop(void *args)
{
    int ret;
    aml_audio_dec_t *audec;
    audio_out_operations_t *aout_ops;
    adec_cmd_t *msg = NULL;

    audec = (aml_audio_dec_t *)args;
    aout_ops = &audec->aout_ops;

    while (!audec->need_stop) {
        audec->state = INITING;

        ret = feeder_init(audec);
        if (ret == 0) {
            ret = aout_ops->init(audec);
            if (ret) {
                adec_print("Audio out device init failed!");
                feeder_release(audec);
                continue;
            }
            audec->state = INITTED;
            break;
        }

	if(!audec->need_stop){
            usleep(100000);
	}
    }

    do {
        //if(message_pool_empty(audec))
        //{
        //adec_print("there is no message !\n");
        //  usleep(100000);
        //  continue;
        //}
        adec_flag_check(audec);

        msg = adec_get_message(audec);
        if (!msg) {
            usleep(100000);
            continue;
        }

        switch (msg->ctrl_cmd) {
        case CMD_START:

            adec_print("Receive START Command!\n");
            start_adec(audec);
            break;

        case CMD_PAUSE:

            adec_print("Receive PAUSE Command!");
            pause_adec(audec);
            break;

        case CMD_RESUME:

            adec_print("Receive RESUME Command!");
            resume_adec(audec);
            break;

        case CMD_STOP:

            adec_print("Receive STOP Command!");
            stop_adec(audec);
            break;

        case CMD_MUTE:

            adec_print("Receive Mute Command!");
            if (msg->has_arg) {
                mute_adec(audec, msg->value.en);
            }
            break;

        case CMD_SET_VOL:

            adec_print("Receive Set Vol Command!");
            if (msg->has_arg) {
                adec_set_volume(audec, msg->value.volume);
            }
            break;

        case CMD_CHANL_SWAP:

            adec_print("Receive Channels Swap Command!");
            audio_hardware_ctrl(HW_CHANNELS_SWAP);
            break;

        case CMD_LEFT_MONO:

            adec_print("Receive Left Mono Command!");
            audio_hardware_ctrl(HW_LEFT_CHANNEL_MONO);
            break;

        case CMD_RIGHT_MONO:

            adec_print("Receive Right Mono Command!");
            audio_hardware_ctrl(HW_RIGHT_CHANNEL_MONO);
            break;

        case CMD_STEREO:

            adec_print("Receive Stereo Command!");
            audio_hardware_ctrl(HW_STEREO_MODE);
            break;

        case CMD_RELEASE:

            adec_print("Receive RELEASE Command!");
            release_adec(audec);
            break;

        default:
            adec_print("Unknow Command!");
            break;

        }

        if (msg) {
            adec_message_free(msg);
            msg = NULL;
        }
    } while (audec->state != TERMINATED);

    adec_print("Exit Message Loop Thread!");
    pthread_exit(NULL);
    return NULL;
}

/**
 * \brief start audio dec
 * \param audec pointer to audec
 * \return 0 on success otherwise -1 if an error occurred
 */
int audiodec_init(aml_audio_dec_t *audec)
{
    int ret = 0;
    pthread_t    tid;

    memset(audec, 0, sizeof(aml_audio_dec_t));

    adec_message_pool_init(audec);
    get_output_func(audec);
    audec->adsp_ops.dsp_file_fd = -1;

    ret = pthread_create(&tid, NULL, (void *)adec_message_loop, (void *)audec);
    if (ret != 0) {
        adec_print("Create adec main thread failed!\n");
        return ret;
    }
    adec_print("Create adec main thread success! tid = %d\n", tid);

    audec->thread_pid = tid;

    return ret;
}
