#ifndef ADEC_H
#define ADEC_H


typedef enum {
    AUDIO_FORMAT_UNKNOWN = 0,
    AUDIO_FORMAT_MPEG,
    AUDIO_FORMAT_PCM_S16LE,
    AUDIO_FORMAT_AAC,
    AUDIO_FORMAT_AC3,
    AUDIO_FORMAT_ALAW,
    AUDIO_FORMAT_MULAW,
    AUDIO_FORMAT_DTS,
    AUDIO_FORMAT_PCM_S16BE,
    AUDIO_FORMAT_FLAC,
    AUDIO_FORMAT_COOK,
    AUDIO_FORMAT_PCM_U8,
    AUDIO_FORMAT_ADPCM,
    AUDIO_FORMAT_AMR,
    AUDIO_FORMAT_RAAC,
    AUDIO_FORMAT_WMA,
   AUDIO_FORMAT_MAX,
} audio_format_t;
#define VALID_FMT(f)	((f>AUDIO_FORMAT_UNKNOWN)&& (f<AUDIO_FORMAT_MAX))

typedef struct adec_feeder{
    audio_format_t format;
    int channel_num;
    int sample_rate;
    int data_width;
    int ( *get_bits)(unsigned long *buf,int len);/*less than 16bites,and the buf is 4Bytes aligned*/
	int ( *get_bytes)(unsigned char *buf,int len);/*read a lot of bytes,return size!*/
    int ( *peek_bits)(unsigned long *buf,int len);//get the  bits witout pop the data
    int ( *reset_bits)(void);//reset to next 16bits bound;
    int ( *bits_left)(void);//reset to next 16bits bound;
    int dsp_on;
    int ( *dsp_read)(char *buffer,int len);//ream pcm stream from dsp
    unsigned long (*get_cur_pts)(struct adec_feeder *feeder);
} adec_feeder_t; 

typedef struct {
    int ( *play)(char *data, unsigned len);	
    int ( *get_buffersize)(void);
    int ( *get_space)(void);
    int (*get_delay)(void);/*uS*/	
    void ( *pause)(void);
    void ( *resume)(void);
    void ( *reset)(void);
    int (*init)(adec_feeder_t *);
    int ( *uninit)(void);	 
    int ( *set_data_width)(int);
    int ( *set_channel_num)(int);	
    int ( *set_sample_rate)(int);
} adec_out_t;

struct frame_fmt
{
    audio_format_t format;
    int channel_num;
    int sample_rate;
    int data_width;
};





typedef struct {
  char * name;
  audio_format_t format;
  int used;
  int (*init)(adec_feeder_t *);
  int (*release)(void);
  int (*decode_frame)(char * ,int,struct frame_fmt *);
}am_codec_struct;

extern int  adec_start();
extern void adec_stop();
extern void adec_pause();
extern void adec_resume();
extern void adec_reset();
extern int adec_init(void);
extern void adec_auto_mute(int);

typedef enum {
	E_OK=0,
	E_NOMEM,	
	E_PTR,
	E_FORMAT,
	E_REGISTERED,
	E_INUSED,
	E_NO,
}register_error_t;
void adec_thread_wait(int microseconds);
int adec_refresh_pts(void);
#endif /* ADEC_H */
