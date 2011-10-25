#include "q_sem.h"


/**
 * Checks the semaphore to see if a resource is available or, if an event occurred. Unlike
 * AVSemPend(), AVSemAccept() does not suspend the calling task if the resource is not available
 * or the event did not occur. The resource counter is decremented by 1 like OSSemPend().
 *
 * @param[in] pevent is a pointer to the semaphore
 * @return >0       if the resource is available
 * @return 0        if the semaphore is not available or the pointer actually doesn't point to a semaphore
 */
INT16U        AVSemAccept(OS_EVENT *pevent)
{
    if (!pevent->tryAcquire())
        return 0;
    return 1;
}

/**
 * Create a semaphore.
 *
 * @param[in] cnt is a initial resource counter. If the value is 0, no resource is available.
 * @return !0        The pointer to the created semaphore object.
 * @return 0         The creation failed. No free event control blocks are available.
 */
OS_EVENT     *AVSemCreate(INT16U cnt)
{
    return new QSemaphore(cnt);
}

/**
 * Delete a semaphore and make all tasks pending on the semaphore ready.
 *
 * @param[in] pevent pointer to the semaphore.
 * @param[in] opt    OS_DEL_NO_PEND delete the semaphore ONLY if no task pending
 * @param[in] opt    OS_DEL_ALWAYS Deletes the semaphore even if tasks are waiting.
 *                   In this case, all the tasks pending will be readied.
 *
 * @param[out] err   OS_NO_ERR            The call was successful and the semaphore was deleted
 * @param[out] err   OS_ERR_DEL_ISR       If you attempted to delete the semaphore from an ISR
 * @param[out] err   OS_ERR_INVALID_OPT   An invalid option was specified
 * @param[out] err   OS_ERR_TASK_WAITING  One or more tasks were waiting on the semaphore
 * @param[out] err   OS_ERR_EVENT_TYPE    If you didn't pass a pointer to a semaphore
 * @param[out] err   OS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer.
 *
 * @return 0        if the semaphore is deleted
 * @return !0       if the semaphore is not deleted, the input pevent is returned.
 */
OS_EVENT     *AVSemDel(OS_EVENT *pevent, INT8U opt, INT8U *err)
{
    if (!pevent)
        return 0;

    //todo: opts, errs

    delete pevent;
    return 0;
}

/**
 * Checks one semaphore resource. The resource counter is decremented by 1 if the resource is available.
 * If there is no resource (i.e. the counter is 0), then the task will be in waiting status.
 *
 * @param[in] pevent    is a pointer to the semaphore
 * @param[in] timeout   is an optional timeout period (in clock ticks).  If non-zero, your task will
 *                      wait for the resource up to the amount of time specified by this argument.
 *                      If you specify 0, however, your task will wait forever at the specified
 *                      semaphore or, until the resource becomes available (or the event occurs).
 * @param[out] err      is a pointer to where an error message will be deposited.
 * @param[out] err      OS_NO_ERR           The call was successful and your task owns the resource
 *                                          or, the event you are waiting for occurred.
 * @param[out] err      OS_TIMEOUT          The semaphore was not received within the specified
 *                                          timeout.
 * @param[out] err      OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore.
 * @param[out] err      OS_ERR_PEND_ISR     If you called this function from an ISR and the result
 *                                          would lead to a suspension.
 * @param[out] err      OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
 */
void          AVSemPend(OS_EVENT *pevent, INT16U timeout, INT8U *err)
{
    if (timeout == 0)
    {
        pevent->acquire();
        return;
    }

    if (!pevent->tryAcquire(1, timeout))
        *err = 10 /*OS_TIMEOUT*/;
    else
        *err = 0 /*OS_NO_ERR*/;
}

/**
 * Signal the semaphore. The resource counter is incremented by 1.
 *
 * @param[in] pevent    is a pointer to the semaphore
 * @return     OS_NO_ERR           The call was successful and your task owns the resource
 *                                 or, the event you are waiting for occurred.
 * @return     OS_SEM_OVF          If the semaphore count exceeded its limit.  In other words, you have
 *                                 signalled the semaphore more often than you waited on it with either
 *                                 OSSemAccept() or OSSemPend().
 * @return     OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore
 * @return     OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
 */
INT8U         AVSemPost(OS_EVENT *pevent)
{
    pevent->release();
    return 0;
}

/**
 * Obtain information about a semaphore
 *
 * @param[in]   pevent    is a pointer to the semaphore
 * @param[out]  pdata     is a pointer to a structure that will contain information about the
 *                        semaphore.
 * @return     OS_NO_ERR           The call was successful.
 * @return     OS_ERR_EVENT_TYPE   If you didn't pass a pointer to a semaphore
 * @return     OS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
 */
//INT8U         AVSemQuery(OS_EVENT *pevent, OS_SEM_DATA *pdata)
//{
//}

