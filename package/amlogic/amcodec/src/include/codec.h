#ifndef CODEC_CTRL_H_
#define CODEC_CTRL_H_

#include <codec_type.h>
#include <codec_error.h>


int codec_init(codec_para_t *);
int codec_close(codec_para_t *);
void codec_close_audio(codec_para_t *);
void codec_resume_audio(codec_para_t *, unsigned int);
int codec_reset(codec_para_t *);
int codec_init_sub(codec_para_t *);
int codec_open_sub_read(void);
int codec_close_sub(codec_para_t *);
int codec_close_sub_fd(CODEC_HANDLE);
int codec_reset_subtile(codec_para_t *pcodec);
int codec_poll_sub(codec_para_t *);
int codec_poll_sub_fd(CODEC_HANDLE, int);
int codec_get_sub_size(codec_para_t *);
int codec_get_sub_size_fd(CODEC_HANDLE);
int codec_read_sub_data(codec_para_t *, char *, unsigned int);
int codec_read_sub_data_fd(CODEC_HANDLE, char *, unsigned int);
int codec_write_sub_data(codec_para_t *, char *, unsigned int);
int codec_init_cntl(codec_para_t *);
int codec_close_cntl(codec_para_t *);
int codec_poll_cntl(codec_para_t *);
int codec_get_cntl_state(codec_para_t *);
int codec_set_cntl_mode(codec_para_t *, unsigned int);
int codec_set_cntl_avthresh(codec_para_t *, unsigned int);
int codec_set_cntl_syncthresh(codec_para_t *pcodec, unsigned int syncthresh);
int codec_reset_audio(codec_para_t *pcodec);
int codec_set_audio_pid(codec_para_t *pcodec);
int codec_set_sub_id(codec_para_t *pcodec);
int codec_audio_reinit(codec_para_t *pcodec);
int codec_set_dec_reset(codec_para_t *pcodec);

int codec_write(codec_para_t *pcodec,void *buffer,int len);
int codec_checkin_pts(codec_para_t *pcodec, unsigned long pts);
int codec_get_vbuf_state(codec_para_t *,struct buf_status *);
int codec_get_abuf_state(codec_para_t *,struct buf_status *);
int codec_get_vdec_state(codec_para_t *,struct vdec_status *);
int codec_get_adec_state(codec_para_t *,struct adec_status *);

int codec_pause(codec_para_t *);
int codec_resume(codec_para_t *);
int codec_audio_search(codec_para_t *p);
int codec_set_mute(codec_para_t *p,int mute);
int codec_get_volume_range(codec_para_t *,int *min,int *max);
int codec_set_volume(codec_para_t *,int val);
int codec_get_volume(codec_para_t *);
int codec_get_mutesta(codec_para_t *);
int codec_set_volume_balance(codec_para_t *,int);/*left£¨0-100)right*/
int codec_swap_left_right(codec_para_t *);
int codec_left_mono(codec_para_t *p);
int codec_right_mono(codec_para_t *p);
int codec_stereo(codec_para_t *p);
int codec_audio_automute(int auto_mute);
int codec_audio_spectrum_switch(codec_para_t *p,int isStart,int interval);
#endif
