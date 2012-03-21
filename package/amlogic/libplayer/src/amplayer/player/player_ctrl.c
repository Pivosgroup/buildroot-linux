/**
 * @file        player_ctrl.c
 * @brief
 * @author      Xu Hui <hui.xu@amlogic.com>
 * @version     1.0.0
 * @date        2011-02-21
 */

/* Copyright (c) 2007-2011, Amlogic Inc.
 * All right reserved
 *
 */

#include <pthread.h>
#include <player.h>
#include <player_set_sys.h>

#include "player_ts.h"
#include "player_es.h"
#include "player_rm.h"
#include "player_ps.h"
#include "player_video.h"
#include "player_audio.h"

#include "player_update.h"
#include "thread_mgt.h"
#include "player_ffmpeg_ctrl.h"
#include "player_cache_mgt.h"


#ifndef FBIOPUT_OSD_SRCCOLORKEY
#define  FBIOPUT_OSD_SRCCOLORKEY    0x46fb
#endif

#ifndef FBIOPUT_OSD_SRCKEY_ENABLE
#define  FBIOPUT_OSD_SRCKEY_ENABLE  0x46fa
#endif

/* --------------------------------------------------------------------------*/
/**
 * @brief  player_init
 *
 * @return PLAYER_SUCCESS   success
 *
 * @details Amlogic player initilization. Make sure call it once when
 *               setup amlogic player every time
 */
/* --------------------------------------------------------------------------*/
extern void print_version_info();

int player_init(void)
{
    /*register all formats and codecs*/
	print_version_info();
	
    ffmpeg_init();

    player_id_pool_init();

    codec_audio_basic_init();
    /*register all support decoder */
    ts_register_stream_decoder();
    es_register_stream_decoder();
    ps_register_stream_decoder();
    rm_register_stream_decoder();
    audio_register_stream_decoder();
    video_register_stream_decoder();

    set_stb_source_hiu();
    set_stb_demux_source_hiu();
    return PLAYER_SUCCESS;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_start
 *
 * @param[in]   ctrl_p  player control parameters structure pointer
 * @param[in]   priv    Player unique identification
 *
 * @return pid  current player tag
 *
 * @details Amlogic player start to play a specified path streaming file.
 *               If need_start in ctrl_p is setting 1, it need call
 *               player_start_play to playback, else playback immediately
 */
/* --------------------------------------------------------------------------*/
int player_start(play_control_t *ctrl_p, unsigned long  priv)
{
    int ret;
    int pid = -1;
    play_para_t *p_para;
	print_version_info();
    log_print("[player_start:enter]p= %p \n", ctrl_p);

    if (ctrl_p == NULL) {
        return PLAYER_EMPTY_P;
    }
    /* set output black policy to black out--default */
    set_black_policy(0);
	  if (!ctrl_p->displast_frame) {
		  set_black_policy(1);
	  } else if (!check_file_same(ctrl_p->file_name)) {
		  set_black_policy(1);
	  }
	
    pid = player_request_pid();
    if (pid < 0) {
        return PLAYER_NOT_VALID_PID;
    }

    p_para = MALLOC(sizeof(play_para_t));
    if (p_para == NULL) {
        return PLAYER_NOMEM;
    }

    MEMSET(p_para, 0, sizeof(play_para_t));

    /* init time_point to a invalid value */
    p_para->playctrl_info.time_point = -1;

    player_init_pid_data(pid, p_para);

    message_pool_init(p_para);

    p_para->start_param = ctrl_p;
    p_para->player_id = pid;
    p_para->extern_priv = priv;
    log_debug1("[player_start]player_para=%p,start_param=%p pid=%d\n", p_para, p_para->start_param, pid);

    ret = player_thread_create(p_para) ;
    if (ret != PLAYER_SUCCESS) {
        FREE(p_para);
        player_release_pid(pid);
        return PLAYER_CAN_NOT_CREAT_THREADS;
    }
    log_print("[player_start:exit]pid = %d \n", pid);

    return pid;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_start_play
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return PLAYER_NOT_VALID_PID playet tag invalid
 *          PLAYER_NOMEM        alloc memory failed
 *          PLAYER_SUCCESS      success
 *
 * @details if need_start set 1, need call player_start_play to start
 *               playback
 */
/* --------------------------------------------------------------------------*/
int player_start_play(int pid)
{
    player_cmd_t *cmd;
    int r = PLAYER_SUCCESS;
    play_para_t *player_para;

    log_print("[player_start_play:enter]pid=%d\n", pid);

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;
    }

    cmd = message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_START;
        r = send_message(player_para, cmd);
    } else {
        r = PLAYER_NOMEM;
    }

    player_close_pid_data(pid);
    log_print("[player_start_play:exit]pid = %d\n", pid);

    return r;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_stop
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send stop command to player (synchronous)
 */
/* --------------------------------------------------------------------------*/
int player_stop(int pid)
{
    player_cmd_t *cmd;
    int r = PLAYER_SUCCESS;
    play_para_t *player_para;

    log_print("[player_stop:enter]pid=%d\n", pid);

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;
    }

    log_print("[player_stop]player_status=0x%x\n", get_player_state(player_para));
    if ((get_player_state(player_para) & 0x30000) == 1) {
        return PLAYER_SUCCESS;
    }

    /*if (player_para->pFormatCtx) {
        av_ioctrl(player_para->pFormatCtx, AVIOCTL_STOP, 0, 0);
    }*/

    cmd = message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_STOP;
        ffmpeg_interrupt(player_para->thread_mgt.pthread_id);
        r = send_message(player_para, cmd);
        r = player_thread_wait_exit(player_para);
        log_print("[player_stop:%d]wait player_theadpid[%d] r = %d\n", __LINE__, player_para->player_id, r);
        clear_all_message(player_para);
        ffmpeg_uninterrupt(player_para->thread_mgt.pthread_id);
    } else {
        r = PLAYER_NOMEM;
    }

    player_close_pid_data(pid);
    log_print("[player_stop:exit]pid=%d\n", pid);

    return r;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_stop
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send stop command to player (asynchronous)
 */
/* --------------------------------------------------------------------------*/
int player_stop_async(int pid)
{
    player_cmd_t *cmd;
    int r = PLAYER_SUCCESS;
    play_para_t *player_para;
    player_para = player_open_pid_data(pid);

    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;
    }

    if (player_para->pFormatCtx) {
        ///av_ioctrl(player_para->pFormatCtx, AVIOCTL_STOP, 0, 0);
    }

    cmd = message_alloc();
    if (cmd) {
        cmd->ctrl_cmd = CMD_STOP;
        ffmpeg_interrupt(player_para->thread_mgt.pthread_id);
        r = send_message(player_para, cmd);
    } else {
        r = PLAYER_NOMEM;
    }

    player_close_pid_data(pid);

    return r;
}




/* --------------------------------------------------------------------------*/
/**
 * @brief   player_exit
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details player_exit must with pairs of player_play
 */
/* --------------------------------------------------------------------------*/
int player_exit(int pid)
{
    int ret = PLAYER_SUCCESS;
    play_para_t *para;

    log_print("[player_exit:enter]pid=%d\n", pid);

    para = player_open_pid_data(pid);
    if (para != NULL) {
        log_print("[player_exit]player_state=0x%x\n", get_player_state(para));
        if (get_player_state(para) != PLAYER_EXIT) {
            player_stop(pid);
        }

        ret = player_thread_wait_exit(para);
        log_print("[player_exit]player thread already exit: %d\n", ret);
        ffmpeg_uninterrupt(para->thread_mgt.pthread_id);
        FREE(para);
        para = NULL;
    }
    player_close_pid_data(pid);
    player_release_pid(pid);
    log_print("[player_exit:exit]pid=%d\n", pid);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_pause
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send pause command to player
 */
/* --------------------------------------------------------------------------*/
int player_pause(int pid)
{
    player_cmd_t cmd;
    int ret = PLAYER_SUCCESS;

    log_print("[player_pause:enter]pid=%d\n", pid);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_PAUSE;

    ret = player_send_message(pid, &cmd);
    log_print("[player_pause:exit]pid=%d ret=%d\n", pid, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_resume
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send resume command to player
 */
/* --------------------------------------------------------------------------*/
int player_resume(int pid)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_resume:enter]pid=%d\n", pid);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_RESUME;

    ret = player_send_message(pid, &cmd);
    log_print("[player_resume:exit]pid=%d ret=%d\n", pid, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_loop
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send loop command to set loop play current file
 */
/* --------------------------------------------------------------------------*/
int player_loop(int pid)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_loop:enter]pid=%d\n", pid);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.set_mode = CMD_LOOP;

    ret = player_send_message(pid, &cmd);
    log_print("[player_loop:exit]pid=%d ret=%d\n", pid, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_loop
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send noloop command to cancle loop play
 */
/* --------------------------------------------------------------------------*/

int player_noloop(int pid)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_loop:enter]pid=%d\n", pid);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.set_mode = CMD_NOLOOP;

    ret = player_send_message(pid, &cmd);
    log_print("[player_loop:exit]pid=%d ret=%d\n", pid, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_timesearch
 *
 * @param[in]   pid player tag which get from player_start return value
 * @param[in]   s_time target time, unit is second
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details seek to designated time point to play.
 *          After time search, player playback from a key frame
 */
/* --------------------------------------------------------------------------*/
int player_timesearch(int pid, int s_time)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_timesearch:enter]pid=%d s_time=%d\n", pid, s_time);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_SEARCH;
    cmd.param = s_time;

    ret = player_send_message(pid, &cmd);
    log_print("[player_timesearch:exit]pid=%d ret=%d\n", pid, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_forward
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   speed   fast forward step
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send ff command to player.
 *          After ff, player playback from a key frame
 */
/* --------------------------------------------------------------------------*/
int player_forward(int pid, int speed)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_forward:enter]pid=%d speed=%d\n", pid, speed);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_FF;
    cmd.param = speed;

    ret = player_send_message(pid, &cmd);
    log_print("[player_forward:exit]pid=%d ret=%d\n", pid, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_backward
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   speed   fast backward step
 *
 * @return PLAYER_NOT_VALID_PID playet tag invalid
 *          PLAYER_NOMEM        alloc memory failed
 *          PLAYER_SUCCESS      success
 *
 * @details send fb command to player.
 *          After fb, player playback from a key frame
 */
/* --------------------------------------------------------------------------*/
int player_backward(int pid, int speed)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_backward:enter]pid=%d speed=%d\n", pid, speed);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_FB;
    cmd.param = speed;

    ret = player_send_message(pid, &cmd);
    log_print("[player_backward]cmd=%x param=%d ret=%d\n", cmd.ctrl_cmd, cmd.param, ret);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_aid
 *
 * @param[in]   pid         player tag which get from player_start return value
 * @param[in]   audio_id    target audio stream id,
 *                          can find through media_info command
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send switch audio id command to player.
 */
/* --------------------------------------------------------------------------*/
int player_aid(int pid, int audio_id)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_aid:enter]pid=%d aid=%d\n", pid, audio_id);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_SWITCH_AID;
    cmd.param = audio_id;

    ret = player_send_message(pid, &cmd);
    log_print("[player_aid:exit]pid=%d ret=%d\n", pid, ret);

    return ret;

}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_sid
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   sub_id  target subtitle stream id,
 *                      can find through media_info command
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send switch subtitle id command to player.
 */
/* --------------------------------------------------------------------------*/
int player_sid(int pid, int sub_id)
{
    player_cmd_t cmd;
    int ret;

    log_print("[player_sid:enter]pid=%d sub_id=%d\n", pid, sub_id);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.ctrl_cmd = CMD_SWITCH_SID;
    cmd.param = sub_id;

    ret = player_send_message(pid, &cmd);
    log_print("[player_sid:exit]pid=%d sub_id=%d\n", pid, sub_id);

    return ret;

}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_enable_autobuffer
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   enable  enable/disable auto buffer function
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details enable/disable auto buffering
 */
/* --------------------------------------------------------------------------*/
int player_enable_autobuffer(int pid, int enable)
{
    player_cmd_t cmd;
    int ret;

    log_print("[%s:enter]pid=%d enable=%d\n", __FUNCTION__, pid, enable);

    MEMSET(&cmd, 0, sizeof(player_cmd_t));

    cmd.set_mode = CMD_EN_AUTOBUF;
    cmd.param = enable;

    ret = player_send_message(pid, &cmd);
    log_print("[%s:exit]pid=%d enable=%d\n", __FUNCTION__, pid, enable);

    return ret;

}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_set_autobuffer_level
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   min     buffer min percent (less than min, enter buffering, av pause)
 * @param[in]   middle  buffer middle percent(more than middler, exit buffering, av resume)
 * @param[in]   max     buffer max percent(more than max, do not feed data)
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details enable/disable auto buffering
 */
/* --------------------------------------------------------------------------*/
int player_set_autobuffer_level(int pid, float min, float middle, float max)
{
    player_cmd_t cmd;
    int ret;

    log_print("[%s:enter]pid=%d min=%.3f middle=%.3f max=%.3f\n", __FUNCTION__, pid, min, middle, max);

    if (min <  middle && middle < max && max < 1) {
        MEMSET(&cmd, 0, sizeof(player_cmd_t));

        cmd.set_mode = CMD_SET_AUTOBUF_LEV;
        cmd.f_param = min;
        cmd.f_param1 = middle;
        cmd.f_param2 = max;

        ret = player_send_message(pid, &cmd);
    } else {
        ret = -1;
        log_error("[%s]invalid param, please check!\n", __FUNCTION__);
    }
    log_print("[%s:exit]pid=%d min=%.3f middle=%.3f max=%.3f\n", __FUNCTION__, pid, min, middle, max);

    return ret;

}


/* --------------------------------------------------------------------------*/
/**
 * @brief   player_send_message
 *
 * @param[in]   pid player tag which get from player_start return value
 * @param[in]   cmd player control command
 *
 * @return  PLAYER_NOT_VALID_PID    playet tag invalid
 *          PLAYER_NOMEM            alloc memory failed
 *          PLAYER_SUCCESS          success
 *
 * @details send player control message
 */
/* --------------------------------------------------------------------------*/
int player_send_message(int pid, player_cmd_t *cmd)
{
    player_cmd_t *mycmd;
    int r = -1;
    play_para_t *player_para;
    char buf[512];

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;
    }

    if (player_get_state(pid) == PLAYER_EXIT) {
		player_close_pid_data(pid);
        return PLAYER_SUCCESS;
    }

    mycmd = message_alloc();
    if (mycmd) {
        memcpy(mycmd, cmd, sizeof(*cmd));
        r = send_message_by_pid(pid, mycmd);
        if (cmd2str(cmd, buf) != -1) {
            log_print("[%s]cmd = %s\n", __FUNCTION__, buf);
        }
    } else {
        r = PLAYER_NOMEM;
    }
	player_close_pid_data(pid);
    return r;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_register_update_callback
 *
 * @param[in]   cb          callback structure point
 * @param[in]   up_fn       update function
 * @param[in]   interval_s  update interval
 *
 * @return  PLAYER_EMPTY_P          invalid pointer
 *          PLAYER_ERROR_CALLBACK   up_fn invalid
 *          PLAYER_SUCCESS          success
 *
 * @details App register a callback function in player to notify message
 */
/* --------------------------------------------------------------------------*/
int player_register_update_callback(callback_t *cb, update_state_fun_t up_fn, int interval_s)
{
    int ret;
    if (!cb) {
        log_error("[player_register_update_callback]empty callback pointer!\n");
        return PLAYER_EMPTY_P;
    }

    ret = register_update_callback(cb, up_fn, interval_s);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_get_state
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  status  player current status
 *          PLAYER_NOT_VALID_PID error,invalid pid
 *
 * @details get player current status
 */
/* --------------------------------------------------------------------------*/
player_status player_get_state(int pid)
{
    player_status status;
    play_para_t *player_para;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;
    }

    status = get_player_state(player_para);
    player_close_pid_data(pid);

    return status;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_get_extern_priv
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  externed                player's unique identification
 *          PLAYER_NOT_VALID_PID    error,invalid pid
 *
 * @details get current player's unique identification
 */
/* --------------------------------------------------------------------------*/
unsigned int player_get_extern_priv(int pid)
{
    unsigned long externed;
    play_para_t *player_para;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;    /*this data is 0 for default!*/
    }

    externed = player_para->extern_priv;
    player_close_pid_data(pid);

    return externed;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief   player_get_play_info
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[out]  info    play info structure pointer
 *
 * @return  PLAYER_SUCCESS          success
 *          PLAYER_NOT_VALID_PID    error,invalid pid
 *
 * @details get player's information
 */
/* --------------------------------------------------------------------------*/
int player_get_play_info(int pid, player_info_t *info)
{
    play_para_t *player_para;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;    /*this data is 0 for default!*/
    }

    MEMSET(info, 0, sizeof(player_info_t));
    MEMCPY(info, &player_para->state, sizeof(player_info_t));
    player_close_pid_data(pid);

    return PLAYER_SUCCESS;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_get_media_info
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[out]  minfo   media info structure pointer
 *
 * @return  PLAYER_SUCCESS          success
 *          PLAYER_NOT_VALID_PID    error,invalid pid
 *
 * @details get file media information
 */
/* --------------------------------------------------------------------------*/
int player_get_media_info(int pid, media_info_t *minfo)
{
    play_para_t *player_para;
	player_status sta;
	
    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return PLAYER_NOT_VALID_PID;    /*this data is 0 for default!*/
    }
	
	sta = get_player_state(player_para);
	if (sta >= PLAYER_ERROR && sta <= PLAYER_EXIT) {
		return PLAYER_INVALID_CMD;
	}
	
    MEMSET(minfo, 0, sizeof(media_info_t));
    MEMCPY(minfo, &player_para->media_info, sizeof(media_info_t));

    log_print("[player_get_media_info]video_num=%d vidx=%d\n", minfo->stream_info.total_video_num, minfo->stream_info.cur_video_index);
    player_close_pid_data(pid);

    return PLAYER_SUCCESS;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_video_overlay_en
 *
 * @param[in]   enable  osd colorkey enable flag
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details enable osd colorkey
 */
/* --------------------------------------------------------------------------*/
int player_video_overlay_en(unsigned enable)
{
    int fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd >= 0) {
        unsigned myKeyColor = 0;
        unsigned myKeyColor_en = enable;

        if (myKeyColor_en) {
            myKeyColor = 0xff;/*set another value to solved the bug in kernel..remove later*/
            ioctl(fd, FBIOPUT_OSD_SRCCOLORKEY, &myKeyColor);
            myKeyColor = 0;
            ioctl(fd, FBIOPUT_OSD_SRCCOLORKEY, &myKeyColor);
            ioctl(fd, FBIOPUT_OSD_SRCKEY_ENABLE, &myKeyColor_en);
        } else {
            ioctl(fd, FBIOPUT_OSD_SRCKEY_ENABLE, &myKeyColor_en);
        }
        close(fd);
        return PLAYER_SUCCESS;
    }
    return PLAYER_FAILED;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_set_mute
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   mute_on volume mute flag 1:mute 0:inmute
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details volume mute switch
 */
/* --------------------------------------------------------------------------*/

int audio_set_mute(int pid, int mute_on)
{

    int ret = PLAYER_FAILED;
    play_para_t *player_para;
    codec_para_t *p;

    player_para = player_open_pid_data(pid);
    if (player_para != NULL) {
        player_para->playctrl_info.audio_mute = mute_on & 0x1;
        log_print("[audio_set_mute:%d]muteon=%d audio_mute=%d\n", __LINE__, mute_on, player_para->playctrl_info.audio_mute);

        p = get_audio_codec(player_para);
        if (p != NULL) {
            ret = codec_set_mute(p, mute_on);
        }
        player_close_pid_data(pid);
    } else {
        ret = codec_set_mute(NULL, mute_on);
    }

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_get_volume_range
 *
 * @param[in]   pid player tag which get from player_start return value
 * @param[out]  min volume minimum
 * @param[out]  max volume maximum
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details get volume range
 */
/* --------------------------------------------------------------------------*/
int audio_get_volume_range(int pid, int *min, int *max)
{
    return codec_get_volume_range(NULL, min, max);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_set_volume
 *
 * @param[in]   pid player tag which get from player_start return value
 * @param[in]   val volume value
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details set volume to val
 */
/* --------------------------------------------------------------------------*/
int audio_set_volume(int pid, float val)
{
    return codec_set_volume(NULL, val);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_get_volume
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  r   current volume
 *
 * @details get volume
 */
/* --------------------------------------------------------------------------*/
int audio_get_volume(int pid, float *vol)
{
    int r;

    r = codec_get_volume(NULL, vol);
    log_print("[audio_get_volume:%d]r=%d\n", __LINE__, r);

    return r;//codec_get_volume(NULL);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_set_lrvolume
 *
 * @param[in]   pid player tag which get from player_start return value
 * @param[in]   lval: left volume value
 * @param[in]   rval: right volume value
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details set volume to val
 */
/* --------------------------------------------------------------------------*/
int audio_set_lrvolume(int pid, float lvol,float rvol)
{
    return codec_set_lrvolume(NULL, lvol,rvol );
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_get_volume
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  r   current volume
 *
 * @details get volume
 */
/* --------------------------------------------------------------------------*/
int audio_get_lrvolume(int pid, float *lvol,float* rvol)
{
    int r;

    r = codec_get_lrvolume(NULL, lvol,rvol);
    log_print("[audio_get_volume:%d]r=%d\n", __LINE__, r);

    return r;//codec_get_volume(NULL);
}



/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_set_volume_balance
 *
 * @param[in]   pid     player tag which get from player_start return value
 * @param[in]   balance balance flag    1:set balance 0:cancel balance
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details balance switch
 */
/* --------------------------------------------------------------------------*/
int audio_set_volume_balance(int pid, int balance)
{
    return codec_set_volume_balance(NULL, balance);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_swap_left_right
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details swap left and right channel
 */
/* --------------------------------------------------------------------------*/
int audio_swap_left_right(int pid)
{
    return codec_swap_left_right(NULL);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_left_mono
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 */
/* --------------------------------------------------------------------------*/

int audio_left_mono(int pid)
{
    int ret = -1;
    play_para_t *player_para;
    codec_para_t *p;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return 0;    /*this data is 0 for default!*/
    }

    p = get_audio_codec(player_para);
    if (p != NULL) {
        ret = codec_left_mono(p);
    }
    player_close_pid_data(pid);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_right_mono
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 */
/* --------------------------------------------------------------------------*/
int audio_right_mono(int pid)
{
    int ret = -1;
    play_para_t *player_para;
    codec_para_t *p;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return 0;    /*this data is 0 for default!*/
    }

    p = get_audio_codec(player_para);
    if (p != NULL) {
        ret = codec_right_mono(p);
    }
    player_close_pid_data(pid);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_stereo
 *
 * @param[in]   pid player tag which get from player_start return value
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 */
/* --------------------------------------------------------------------------*/
int audio_stereo(int pid)
{
    int ret = -1;
    play_para_t *player_para;
    codec_para_t *p;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return 0;    /*this data is 0 for default!*/
    }

    p = get_audio_codec(player_para);
    if (p != NULL) {
        ret = codec_stereo(p);
    }
    player_close_pid_data(pid);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   audio_set_spectrum_switch
 *
 * @param[in]   pid         player tag which get from player_start return value
 * @param[in]   isStart     open/close spectrum switch function
 * @param[in]   interval    swtich interval
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 */
/* --------------------------------------------------------------------------*/
int audio_set_spectrum_switch(int pid, int isStart, int interval)
{
    int ret = -1;
    play_para_t *player_para;
    codec_para_t *p;

    player_para = player_open_pid_data(pid);
    if (player_para == NULL) {
        return 0;    /*this data is 0 for default!*/
    }

    p = get_audio_codec(player_para);
    if (p != NULL) {
        ret = codec_audio_spectrum_switch(p, isStart, interval);
    }
    player_close_pid_data(pid);

    return ret;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_progress_exit
 *
 * @return  PLAYER_SUCCESS  success
 *
 * @details used for all exit,please only call at this process fatal error.
 *          Do not wait any things in this function
 */
/* --------------------------------------------------------------------------*/
int player_progress_exit(void)
{
    codec_close_audio(NULL);

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_list_allpid
 *
 * @param[out]  pid pid list structure pointer
 *
 * @return  PLAYER_SUCCESS  success
 *          PLAYER_FAILED   failed
 *
 * @details list all alived player pid
 */
/* --------------------------------------------------------------------------*/

int player_list_allpid(pid_info_t *pid)
{
    char buf[MAX_PLAYER_THREADS];
    int pnum = 0;
    int i;

    pnum = player_list_pid(buf, MAX_PLAYER_THREADS);
    pid->num = pnum;

    for (i = 0; i < pnum; i++) {
        pid->pid[i] = buf[i];
    }

    return 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_cache_system_init
 *
 * @param[in]   enable dir max_size block_size
 *
 * @return  0;
 */
/* --------------------------------------------------------------------------*/


int player_cache_system_init(int enable,const char*dir,int max_size,int block_size)
{
	return cache_system_init(enable,dir,max_size,block_size);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   player_status2str
 *
 * @param[in]   status  player status
 *
 * @return  player status details strings
 */
/* --------------------------------------------------------------------------*/
char *player_status2str(player_status status)
{
    switch (status) {
    case PLAYER_INITING:
        return "BEGIN_INIT";

	case PLAYER_TYPE_REDY:
		return "TYPE_READY";
		
    case PLAYER_INITOK:
        return "INIT_OK";

    case PLAYER_RUNNING:
        return "PLAYING";

    case PLAYER_BUFFERING:
        return "BUFFERING";

	case PLAYER_BUFFER_OK:
        return "BUFFEROK";

    case PLAYER_PAUSE:
        return "PAUSE";

    case PLAYER_SEARCHING:
        return "SEARCH_FFFB";

    case PLAYER_SEARCHOK:
        return "SEARCH_OK";

    case PLAYER_START:
        return "START_PLAY";

    case PLAYER_FF_END:
        return "FF_END";

    case PLAYER_FB_END:
        return "FB_END";

    case PLAYER_ERROR:
        return "ERROR";

    case PLAYER_PLAYEND:
        return "PLAY_END";

    case PLAYER_STOPED:
        return "STOPED";

    case PLAYER_EXIT:
        return "EXIT";

    case PLAYER_PLAY_NEXT:
        return "PLAY_NEXT";
		
	case PLAYER_FOUND_SUB:
		return "NEW_SUB";
		
    case PLAYER_DIVX_AUTHORERR:
        return "DIVX_AUTHORERR";
    case PLAYER_DIVX_RENTAL_VIEW:
        return "DIVX_RENTAL";
    case PLAYER_DIVX_RENTAL_EXPIRED:
        return "DIVX_EXPIRED";

    default:
        return "UNKNOW_STATE";
    }
}

