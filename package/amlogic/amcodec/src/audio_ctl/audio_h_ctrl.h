#ifndef AUDIO_H_CTRL_H
#define AUDIO_H_CTRL_H
#define AMADECD_SOCKET_NAME "/tmp/amadec_socket"

#ifdef CODEC_DEBUG
 #define CODEC_PRINT(f,s...)	printf(f,##s)
#else
 #define CODEC_PRINT(f,s...)
#endif

int audio_trans_cmd(char * cmd);
#endif
