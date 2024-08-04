/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once
#include "defines.h"


#ifdef __cplusplus
extern "C" {
#endif

// Thread handle
typedef struct cl_thread cl_thread_t;

// Thread function prototype
typedef void *(*cl_thread_func_t)(void *arg);

// Thread creation flags
typedef enum cl_thread_flags
{
    CL_THREAD_FLAG_NONE = 0,
    CL_THREAD_FLAG_DETACHED = 1 << 0,
} cl_thread_flags_t;

// Thread priority
typedef enum cl_thread_priority
{
    CL_THREAD_PRIORITY_LOW,
    CL_THREAD_PRIORITY_NORMAL,
    CL_THREAD_PRIORITY_HIGH,
} cl_thread_priority_t;

// Mutex handle
typedef struct cl_mutex cl_mutex_t;

// Condition variable handle
typedef struct cl_cond cl_cond_t;

// Thread functions
cl_thread_t *cl_thread_create(cl_thread_func_t func, void *arg, cl_thread_flags_t flags);
void cl_thread_destroy(cl_thread_t *thread);
bool cl_thread_join(cl_thread_t *thread, void **result);
bool cl_thread_detach(cl_thread_t *thread);
bool cl_thread_set_priority(cl_thread_t *thread, cl_thread_priority_t priority);
void cl_thread_yield(void);
void cl_thread_sleep(uint32_t milliseconds);

// Mutex functions
cl_mutex_t *cl_mutex_create(void);
void cl_mutex_destroy(cl_mutex_t *mutex);
bool cl_mutex_lock(cl_mutex_t *mutex);
bool cl_mutex_trylock(cl_mutex_t *mutex);
bool cl_mutex_unlock(cl_mutex_t *mutex);

// Condition variable functions
cl_cond_t *cl_cond_create(void);
void cl_cond_destroy(cl_cond_t *cond);
bool cl_cond_wait(cl_cond_t *cond, cl_mutex_t *mutex);
bool cl_cond_timedwait(cl_cond_t *cond, cl_mutex_t *mutex, uint32_t milliseconds);
bool cl_cond_signal(cl_cond_t *cond);
bool cl_cond_broadcast(cl_cond_t *cond);

#ifdef __cplusplus
}
#endif
