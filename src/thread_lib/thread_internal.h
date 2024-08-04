#pragma once

#include "../../include/clib/thread_lib.h"

#ifdef _WIN32
#include <process.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#else
#include <pthread.h>
#endif

struct cl_thread
{
    cl_thread_func_t func;
    void *arg;
    cl_thread_flags_t flags;
#ifdef _WIN32
    HANDLE handle;
    unsigned int id;
#else
    pthread_t handle;
#endif
};

struct cl_mutex
{
#ifdef _WIN32
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t mutex;
#endif
};

struct cl_cond
{
#ifdef _WIN32
    CONDITION_VARIABLE cond;
#else
    pthread_cond_t cond;
#endif
};

// Platform-specific function declarations
bool cl_thread_create_platform(cl_thread_t *thread);
void cl_thread_destroy_platform(cl_thread_t *thread);
bool cl_thread_join_platform(cl_thread_t *thread, void **result);
bool cl_thread_detach_platform(cl_thread_t *thread);
bool cl_thread_set_priority_platform(cl_thread_t *thread, cl_thread_priority_t priority);
void cl_thread_yield_platform(void);
void cl_thread_sleep_platform(uint32_t milliseconds);

bool cl_mutex_init_platform(cl_mutex_t *mutex);
void cl_mutex_destroy_platform(cl_mutex_t *mutex);
bool cl_mutex_lock_platform(cl_mutex_t *mutex);
bool cl_mutex_trylock_platform(cl_mutex_t *mutex);
bool cl_mutex_unlock_platform(cl_mutex_t *mutex);

bool cl_cond_init_platform(cl_cond_t *cond);
void cl_cond_destroy_platform(cl_cond_t *cond);
bool cl_cond_wait_platform(cl_cond_t *cond, cl_mutex_t *mutex);
bool cl_cond_timedwait_platform(cl_cond_t *cond, cl_mutex_t *mutex, uint32_t milliseconds);
bool cl_cond_signal_platform(cl_cond_t *cond);
bool cl_cond_broadcast_platform(cl_cond_t *cond);
