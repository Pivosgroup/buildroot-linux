#ifndef _TASKMNGR_H_

#define _TASKMNGR_H_

#ifdef  __cplusplus

extern "C" {

#endif


#define TASK_DEFAULT_STACK_SIZE  64*1024

typedef void* (*TASK_PROC_DEF)(void*);


int   MP_taskCreate(char name[],unsigned int pri, unsigned int stksize,TASK_PROC_DEF pfEntry,void* arg);

int MP_taskBlockSignalPE();

void   MP_taskDelete(unsigned int taskId);

void  MP_taskSuspend(unsigned int taskId);

void   MP_taskResume(unsigned int taskId);

void MP_taskDelay(unsigned long usecs);

void MP_taskTestCancel();

void MP_taskSetCancel();

#ifdef  __cplusplus
}
#endif



#endif



