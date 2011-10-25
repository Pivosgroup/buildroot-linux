#ifndef _MP_LOG_H__
#define _MP_LOG_H__

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum{
    DBG_LVL_OFF = 0,
    DBG_LVL_ERR, 
    DBG_LVL_WRN,
    DBG_LVL_TRC,
    DBG_LVL_INF,
    DBG_LVL_CON,  /* Output details content */
    DBG_LVL_ALL = 10,
}MP_DebugLevel;

#define DBG_LVL_DEF DBG_LVL_WRN

#define DBG_PRINT(lvl, lvlstr, str, args...)  \
do{ \
    if (lvl <= DBG_LVL_DEF) { \
        printf("[%u]:"lvlstr"%09ld:", (unsigned)getpid(), time(NULL)); \
        printf(str, ## args); \
    } \
}while(0);



#define log_err(str, args...)  DBG_PRINT(DBG_LVL_ERR, "ERR:", str, ## args)
#define log_wrn(str, args...)  DBG_PRINT(DBG_LVL_WRN, "WRN:", str, ## args)
#define log_trc(str, args...)  DBG_PRINT(DBG_LVL_TRC, "TRC:", str, ## args)
#define log_info(str, args...)  DBG_PRINT(DBG_LVL_INF, "INF:", str, ## args)
#define log_con(str, args...)  DBG_PRINT(DBG_LVL_CON, "CON:", str, ## args)


#define DBG_B2A_PRINT(bytes, len) \
{ \
	unsigned int       i;\
	printf("-------------------------------------------\n");\
	for (i=0; i<(len); i++) {\
		printf("%02X", (bytes)[i]);\
		if ((i+1)%16 == 0)\
			printf("\n");\
		}\
		printf("\n-------------------------------------------\n");\
}


#ifdef  __cplusplus
}
#endif

#endif 
