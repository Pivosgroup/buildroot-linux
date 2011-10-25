#ifndef DSP_CTL_HEADER_H
#define DSP_CTL_HEADER_H

int audiodsp_start(adec_feeder_t *feeder);
int audiodsp_init(void);
int audiodsp_release();
int audiodsp_stream_read(char *buffer,int size);
int audiodsp_stop();
unsigned long  audiodsp_get_pts(void );
#endif

