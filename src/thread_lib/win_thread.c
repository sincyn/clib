/**
 * Created by jraynor on 8/3/2024.
 */
#ifdef _WIN32

#include "thread_internal.h"

static unsigned __stdcall win32_thread_func(void *arg)
{
    cl_thread_t *thread = (cl_thread_t *)arg;
    thread->func(thread->arg);
    return 0;
}

bool cl_thread_create_platform(cl_thread_t *thread)
{
    thread->handle =
        (HANDLE)_beginthreadex(NULL, 0, win32_thread_func, thread,
                               (thread->flags & CL_THREAD_FLAG_DETACHED) ? CREATE_SUSPENDED : 0, &thread->id);
    if (thread->handle == NULL)
    {
        return false;
    }
    if (thread->flags & CL_THREAD_FLAG_DETACHED)
    {
        CloseHandle(thread->handle);
    }
    return true;
}

void cl_thread_destroy_platform(cl_thread_t *thread)
{
    if (!(thread->flags & CL_THREAD_FLAG_DETACHED))
    {
        CloseHandle(thread->handle);
    }
}

bool cl_thread_join_platform(cl_thread_t *thread, void **result)
{
    DWORD wait_result = WaitForSingleObject(thread->handle, INFINITE);
    if (wait_result != WAIT_OBJECT_0)
        return false;
    if (result != NULL)
    {
        DWORD exit_code;
        GetExitCodeThread(thread->handle, &exit_code);
        *result = (void *)(uintptr_t)exit_code;
    }
    return true;
}

bool cl_thread_detach_platform(cl_thread_t *thread) { return CloseHandle(thread->handle) != 0; }

bool cl_thread_set_priority_platform(cl_thread_t *thread, cl_thread_priority_t priority)
{
    int win_priority;
    switch (priority)
    {
    case CL_THREAD_PRIORITY_LOW:
        win_priority = THREAD_PRIORITY_BELOW_NORMAL;
        break;
    case CL_THREAD_PRIORITY_NORMAL:
        win_priority = THREAD_PRIORITY_NORMAL;
        break;
    case CL_THREAD_PRIORITY_HIGH:
        win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
        break;
    default:
        return false;
    }
    return SetThreadPriority(thread->handle, win_priority) != 0;
}

void cl_thread_yield_platform(void) { SwitchToThread(); }

void cl_thread_sleep_platform(uint32_t milliseconds) { Sleep(milliseconds); }

bool cl_mutex_init_platform(cl_mutex_t *mutex)
{
    InitializeCriticalSection(&mutex->cs);
    return true;
}

void cl_mutex_destroy_platform(cl_mutex_t *mutex) { DeleteCriticalSection(&mutex->cs); }

bool cl_mutex_lock_platform(cl_mutex_t *mutex)
{
    EnterCriticalSection(&mutex->cs);
    return true;
}

bool cl_mutex_trylock_platform(cl_mutex_t *mutex) { return TryEnterCriticalSection(&mutex->cs) != 0; }

bool cl_mutex_unlock_platform(cl_mutex_t *mutex)
{
    LeaveCriticalSection(&mutex->cs);
    return true;
}

bool cl_cond_init_platform(cl_cond_t *cond)
{
    InitializeConditionVariable(&cond->cond);
    return true;
}

void cl_cond_destroy_platform(cl_cond_t *cond) { (void)cond; }


bool cl_cond_wait_platform(cl_cond_t *cond, cl_mutex_t *mutex)
{
    return SleepConditionVariableCS(&cond->cond, &mutex->cs, INFINITE) != 0;
}
bool cl_cond_timedwait_platform(cl_cond_t *cond, cl_mutex_t *mutex, uint32_t milliseconds)
{
    return SleepConditionVariableCS(&cond->cond, &mutex->cs, milliseconds) != 0;
}
bool cl_cond_signal_platform(cl_cond_t *cond)
{
    WakeConditionVariable(&cond->cond);
    return true;
}
bool cl_cond_broadcast_platform(cl_cond_t *cond)
{
    WakeAllConditionVariable(&cond->cond);
    return true;
}

#endif
