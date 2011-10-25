

#ifndef PALYER_CTROL_H
#define PALYER_CTROL_H


#define  MALLOC(s)		malloc(s)
#define  FREE(d)		free(d)
#define  MEMCPY(d,s,l)	memcpy(d,s,l)
#define  MEMSET(d,v,l)		memset(d,v,l)
#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#define LOG_FILE		"/tmp/amplayer2.log"

#endif
