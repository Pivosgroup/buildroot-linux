#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#include "taskmngr.h"
#include "mp_log.h"

int  MP_taskCreate(char name[],unsigned int pri, unsigned int  stksize,TASK_PROC_DEF pfEntry, void* arg)
{
    pthread_t taskId;
    pthread_attr_t attr;
    int detachstate;
    int stacksize;
    int ret = -1;
    pthread_attr_init(&attr);
    detachstate = PTHREAD_CREATE_DETACHED;
    stacksize = (size_t)stksize;

    if (stacksize <= 0)
        stacksize = TASK_DEFAULT_STACK_SIZE;

    pthread_attr_setdetachstate(&attr, detachstate);

    pthread_attr_setstacksize(&attr, stacksize);

    ret = pthread_create(&taskId, &attr, pfEntry,arg);

    if (ret == 0)
    {
        pthread_detach(taskId);
    }
    else
    {
        log_err("[%s] Error-create task %s failed, err: %s\n", __func__, name, strerror(errno));
        return -1;
    }

    pthread_attr_destroy(&attr);

    return (unsigned int)taskId;

}

int MP_taskBlockSignalPE()
{
    int ret = -1;
    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    ret = pthread_sigmask (SIG_BLOCK,&signal_mask, NULL);
    if (ret != 0)
    {
        log_err("[%s]block sigpipe error\n",__FUNCTION__);
        return -1;
    }
    return 0;
}

void  MP_taskDelete(unsigned int taskId)

{
    pthread_cancel((pthread_t)taskId);
}

void MP_taskSuspend(unsigned int taskId)

{
    pthread_kill((pthread_t)taskId, SIGSTOP);

}

void MP_taskResume(unsigned int taskId)

{
    pthread_kill((pthread_t)taskId, SIGCONT);
}

void MP_taskDelay(unsigned long usecs)
{
    usleep(usecs);
}

void MP_taskSetCancel()

{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
}



void MP_taskTestCancel()

{
    pthread_testcancel();
}
