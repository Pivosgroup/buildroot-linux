/*******************************************************************
 * 
 *  Copyright (C) 2007 by Sympeer, Inc. All Rights Reserved.
 *
 *  Description: the interface of ABoxBase threads
 *
 *  Author: Sympeer Software
 *
 *******************************************************************/

#ifndef _ABX_THREAD_H_
#define _ABX_THREAD_H_


#ifdef AVOS

    #include <includes.h>
    #include <OS_API.h>

    typedef INT16U              abx_thread_t;
    typedef OS_EVENT*           abx_sema_t;
    #define ABX_INVALID_THREAD  0xFFFF
    #define ABX_INVALID_SEMA    NULL
    #define ABX_STDCALL

    #define abx_sem_new(count)  AVSemCreate(count)
    #define abx_sem_free(sem)   do { INT8U err; AVSemDel(sem, OS_DEL_ALWAYS, &err); } while(0)
    #define abx_sem_signal(sem) AVSemPost(sem)
    #define abx_sem_wait(sem)   do { INT8U err; AVSemPend(sem, 0, &err); } while(0)

    #define abx_init_critical_section()
    #define abx_declare_critical_section()  DECLARE_CPU_SR
    #define abx_enter_critical_section()    OS_ENTER_CRITICAL()
    #define abx_leave_critical_section()    OS_EXIT_CRITICAL()
    #define abx_fini_critical_section()


#else   // AVOS
#ifdef QT
#include <QtCore>
    typedef QSemaphore *        abx_sema_t;
    typedef char                OS_STK;
    #define ABX_INVALID_THREAD  NULL
    #define ABX_INVALID_SEMA    NULL
    #define ABX_STDCALL         __stdcall


    #define abx_sem_new(count)  new QSemaphore(count)
    #define abx_sem_free(sem)   delete sem
    #define abx_sem_signal(sem) sem->release();
    #define abx_sem_wait(sem)   sem->acquire();
#else
    #include <Windows.h>

    typedef HANDLE              abx_thread_t;
    typedef HANDLE              abx_sema_t;
    typedef char                OS_STK;
    #define ABX_INVALID_THREAD  NULL
    #define ABX_INVALID_SEMA    NULL
    #define ABX_STDCALL         __stdcall


    #define abx_sem_new(count)  CreateSemaphore(NULL, count, count, NULL)
    #define abx_sem_free(sem)   CloseHandle(sem)
    #define abx_sem_signal(sem) ReleaseSemaphore(sem, 1, NULL)
    #define abx_sem_wait(sem)   WaitForSingleObject(sem, INFINITE)

    extern CRITICAL_SECTION CriticalSection;
    #define abx_init_critical_section()     InitializeCriticalSection(&CriticalSection)
    #define abx_declare_critical_section()  typedef int abx_for_cpu_sr_t
    #define abx_enter_critical_section()    EnterCriticalSection(&CriticalSection)
    #define abx_leave_critical_section()    LeaveCriticalSection(&CriticalSection)
    #define abx_fini_critical_section()     DeleteCriticalSection(&CriticalSection)
#endif
#endif  // AVOS

#ifndef QT
abx_thread_t abx_thread_new(int (ABX_STDCALL * thread)(void *), void *arg, void * ptos, int stksize, int prio);
void abx_wait_thread(abx_thread_t id);
#endif


#endif
