#ifndef _PLAYER_UPDATE_H_
#define _PLAYER_UPDATE_H_

#include <player_type.h>
#include "player_priv.h"

unsigned int get_pts_pcrscr(void);
unsigned int get_pts_video(void);
int     update_playing_info(play_para_t *p_para);
int     set_media_info(play_para_t *p_para);
int     check_time_interrupt(unsigned long *old_msecond, int interval_ms);

#endif

