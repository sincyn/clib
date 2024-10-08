#include "clib/thread_lib.h"
#include "clib/memory_lib.h"
#include "thread_internal.h"

cl_thread_t *cl_thread_create(const cl_thread_func_t func, void *arg, const cl_thread_flags_t flags) {
    cl_thread_t *thread = cl_mem_alloc(null, sizeof(cl_thread_t));
    if (thread == null) return null;

    thread->func = func;
    thread->arg = arg;
    thread->flags = flags;

    if (!cl_thread_create_platform(thread)) {
        cl_mem_free(null, thread);
        return null;
    }

    return thread;
}

void cl_thread_destroy(cl_thread_t *thread) {
    if (thread == null) return;
    cl_thread_destroy_platform(thread);
    cl_mem_free(null, thread);
}

bool cl_thread_join(cl_thread_t *thread, void **result) {
    if (thread == null) return false;
    return cl_thread_join_platform(thread, result);
}

bool cl_thread_detach(cl_thread_t *thread) {
    if (thread == null) return false;
    return cl_thread_detach_platform(thread);
}

bool cl_thread_set_priority(cl_thread_t *thread, const cl_thread_priority_t priority) {
    if (thread == null) return false;
    return cl_thread_set_priority_platform(thread, priority);
}

void cl_thread_yield(void) {
    cl_thread_yield_platform();
}

void cl_thread_sleep(const uint32_t milliseconds) {
    cl_thread_sleep_platform(milliseconds);
}

cl_mutex_t *cl_mutex_create(void) {
    cl_mutex_t *mutex = cl_mem_alloc(null, sizeof(cl_mutex_t));
    if (mutex == null) return null;

    if (!cl_mutex_init_platform(mutex)) {
        cl_mem_free(null, mutex);
        return null;
    }

    return mutex;
}

void cl_mutex_destroy(cl_mutex_t *mutex) {
    if (mutex == null) return;
    cl_mutex_destroy_platform(mutex);
    cl_mem_free(null, mutex);
}

bool cl_mutex_lock(cl_mutex_t *mutex) {
    if (mutex == null) return false;
    return cl_mutex_lock_platform(mutex);
}

bool cl_mutex_trylock(cl_mutex_t *mutex) {
    if (mutex == null) return false;
    return cl_mutex_trylock_platform(mutex);
}

bool cl_mutex_unlock(cl_mutex_t *mutex) {
    if (mutex == null) return false;
    return cl_mutex_unlock_platform(mutex);
}

cl_cond_t *cl_cond_create(void) {
    cl_cond_t *cond = cl_mem_alloc(null, sizeof(cl_cond_t));
    if (cond == null) return null;

    if (!cl_cond_init_platform(cond)) {
        cl_mem_free(null, cond);
        return null;
    }

    return cond;
}

void cl_cond_destroy(cl_cond_t *cond) {
    if (cond == null) return;
    cl_cond_destroy_platform(cond);
    cl_mem_free(null, cond);
}

bool cl_cond_wait(cl_cond_t *cond, cl_mutex_t *mutex) {
    if (cond == null || mutex == null) return false;
    return cl_cond_wait_platform(cond, mutex);
}

bool cl_cond_timedwait(cl_cond_t *cond, cl_mutex_t *mutex, const uint32_t milliseconds) {
    if (cond == null || mutex == null) return false;
    return cl_cond_timedwait_platform(cond, mutex, milliseconds);
}

bool cl_cond_signal(cl_cond_t *cond) {
    if (cond == null) return false;
    return cl_cond_signal_platform(cond);
}

bool cl_cond_broadcast(cl_cond_t *cond) {
    if (cond == null) return false;
    return cl_cond_broadcast_platform(cond);
}