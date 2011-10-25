/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the realization of ABoxBase thread functions
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/

#include "abx_thread.h"
#include "abx_dbg.h"
#include "abx_mem.h"
#include "abx_time.h"


#ifdef AVOS


struct _abx_thread_func_arg_t {
    int (* thread)(void *);
    void * arg;
};

void _abx_thead_func(void * arg)
{
    struct _abx_thread_func_arg_t * thread_func_arg = (struct _abx_thread_func_arg_t *)arg;
    ABX_ASSERT(thread_func_arg && thread_func_arg->thread);
    if (thread_func_arg && thread_func_arg->thread) {
        thread_func_arg->thread(thread_func_arg->arg);
    }
    abx_free(thread_func_arg);
    AVTaskDel(OS_ID_SELF);
}

abx_thread_t abx_thread_new(int (ABX_STDCALL * thread)(void *), void *arg, void * ptos, int stksize, int prio)
{
    abx_thread_t id;
    struct _abx_thread_func_arg_t * thread_func_arg = (struct _abx_thread_func_arg_t *)abx_malloc(sizeof(struct _abx_thread_func_arg_t));
    thread_func_arg->thread = thread;
    thread_func_arg->arg = arg;
    if (AVTaskCreate(_abx_thead_func, thread_func_arg, (OS_STK *)ptos, prio, &id) != OS_NO_ERR)
        return ABX_INVALID_THREAD;
    else
    {
	 #if OS_TASK_STACKCHK_EN > 0
	    AVTaskSetStackSize(id,stksize);
	#endif
        return id;
    }
}


void abx_wait_thread(abx_thread_t id)
{
    OS_TCB tcb;
    ABX_ASSERT(id != ABX_INVALID_THREAD && id != OS_ID_SELF);
    while (AVTaskQuery(id, &tcb) != OS_ID_ERR) {
        abx_sleep(200);
    }
}



unsigned int abx_tickcount();



void do_nothing(...)		// for ABX_LOG
{
}


#else   // AVOS



abx_thread_t abx_thread_new(int (ABX_STDCALL * thread)(void *), void *arg, void * ptos,int stksize, int prio)
{
    DWORD threadid;
    return CreateThread( NULL, 0, thread, arg, 0, &threadid);
}



void abx_wait_thread(abx_thread_t id)
{
    ABX_ASSERT(id != ABX_INVALID_THREAD);
    WaitForSingleObject(id, INFINITE);
}



CRITICAL_SECTION CriticalSection;


#endif  // AVOS

