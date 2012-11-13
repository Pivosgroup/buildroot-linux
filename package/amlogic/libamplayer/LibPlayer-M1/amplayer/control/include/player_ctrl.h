

#ifndef PALYER_CTROL_H
#define PALYER_CTROL_H


#define  MALLOC(s)		malloc(s)
#define  FREE(d)		free(d)
#define  MEMCPY(d,s,l)	memcpy(d,s,l)
#define  MEMSET(d,v,l)		memset(d,v,l)
#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#include "message.h"

#ifdef  __cplusplus
extern "C" {
#endif

/*return can free cmd*/
int player_send_message(int pid,player_cmd_t *cmd);

#ifdef  __cplusplus
}
#endif

#define LOG_FILE		"/tmp/amplayer2.log"

#endif
