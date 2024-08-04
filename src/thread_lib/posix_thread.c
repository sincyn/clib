/**
 * Created by jraynor on 8/3/2024.
 */
#include "clib/defines.h"
#if defined(CL_PLATFORM_APPLE) || defined(CL_PLATFORM_LINUX)

#include <time.h>
#include <unistd.h>
#include "thread_internal.h"

bool cl_thread_create_platform(cl_thread_t *thread)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (thread->flags & CL_THREAD_FLAG_DETACHED)
    {
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    int result = pthread_create(&thread->handle, &attr, thread->func, thread->arg);
    pthread_attr_destroy(&attr);
    return result == 0;
}

void cl_thread_destroy_platform(cl_thread_t *thread)
{
    // Nothing to do for POSIX threads
    (void)thread;
}

bool cl_thread_join_platform(cl_thread_t *thread, void **result) { return pthread_join(thread->handle, result) == 0; }

bool cl_thread_detach_platform(cl_thread_t *thread) { return pthread_detach(thread->handle) == 0; }

bool cl_thread_set_priority_platform(cl_thread_t *thread, cl_thread_priority_t priority)
{
    int policy;
    struct sched_param param;
    if (pthread_getschedparam(thread->handle, &policy, &param) != 0)
        return false;
    switch (priority)
    {
    case CL_THREAD_PRIORITY_LOW:
        param.sched_priority = sched_get_priority_min(policy);
        break;
    case CL_THREAD_PRIORITY_NORMAL:
        param.sched_priority = (sched_get_priority_max(policy) + sched_get_priority_min(policy)) / 2;
        break;
    case CL_THREAD_PRIORITY_HIGH:
        param.sched_priority = sched_get_priority_max(policy);
        break;
    default:
        return false;
    }
    return pthread_setschedparam(thread->handle, policy, &param) == 0;
}

void cl_thread_yield_platform(void) { sched_yield(); }

void cl_thread_sleep_platform(uint32_t milliseconds) { usleep(milliseconds * 1000); }

bool cl_mutex_init_platform(cl_mutex_t *mutex) { return pthread_mutex_init(&mutex->mutex, NULL) == 0; }

void cl_mutex_destroy_platform(cl_mutex_t *mutex) { pthread_mutex_destroy(&mutex->mutex); }

bool cl_mutex_lock_platform(cl_mutex_t *mutex) { return pthread_mutex_lock(&mutex->mutex) == 0; }

bool cl_mutex_trylock_platform(cl_mutex_t *mutex) { return pthread_mutex_trylock(&mutex->mutex) == 0; }

bool cl_mutex_unlock_platform(cl_mutex_t *mutex) { return pthread_mutex_unlock(&mutex->mutex) == 0; }

bool cl_cond_init_platform(cl_cond_t *cond) { return pthread_cond_init(&cond->cond, NULL) == 0; }

void cl_cond_destroy_platform(cl_cond_t *cond) { pthread_cond_destroy(&cond->cond); }

bool cl_cond_wait_platform(cl_cond_t *cond, cl_mutex_t *mutex)
{
    return pthread_cond_wait(&cond->cond, &mutex->mutex) == 0;
}

bool cl_cond_timedwait_platform(cl_cond_t *cond, cl_mutex_t *mutex, uint32_t milliseconds)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += milliseconds / 1000;
    ts.tv_nsec += (milliseconds % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(&cond->cond, &mutex->mutex, &ts) == 0;
}

bool cl_cond_signal_platform(cl_cond_t *cond) { return pthread_cond_signal(&cond->cond) == 0; }

bool cl_cond_broadcast_platform(cl_cond_t *cond) { return pthread_cond_broadcast(&cond->cond) == 0; }

#endif

