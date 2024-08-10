/**
 * Created by jraynor on 8/3/2024.
 */
#include <stdio.h>
#include <stdlib.h>
#include "clib/log_lib.h"
#include "clib/thread_lib.h"

#define NUM_THREADS 5
#define ITERATIONS 10

cl_mutex_t *mutex;
cl_cond_t *cond;
int shared_counter = 0;

void *thread_func(void *arg)
{
    int thread_id = *(int *)arg;

    for (int i = 0; i < ITERATIONS; ++i)
    {
        cl_mutex_lock(mutex);

        // Wait for our turn
        while (shared_counter % NUM_THREADS != thread_id)
        {
            cl_cond_wait(cond, mutex);
        }

        // Do some work
        cl_log_info("Thread %d: Counter = %d", thread_id, shared_counter);
        shared_counter++;

        // Signal the next thread
        cl_cond_broadcast(cond);
        cl_mutex_unlock(mutex);

        // Simulate some work
        cl_thread_sleep(10);
    }

    free(arg);
    return null;
}

int main()
{
    // Initialize the logging system
    cl_log_config_t log_config = {.include_timestamp = true, .include_level = true, .include_file_line = true};
    cl_log_init(&log_config);

    // Configure and add the console target
    cl_log_target_config_t console_config = {
        .type = CL_LOG_TARGET_CONSOLE, .min_level = CL_LOG_INFO, .config.console = {.use_colors = true}};
    cl_log_add_target(&console_config);

    // Create mutex and condition variable
    mutex = cl_mutex_create();
    cond = cl_cond_create();

    cl_thread_t *threads[NUM_THREADS];

    // Create threads
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        int *thread_id = malloc(sizeof(int));
        *thread_id = i;
        threads[i] = cl_thread_create(thread_func, thread_id, CL_THREAD_FLAG_NONE);
        if (threads[i] == null)
        {
            cl_log_error("Failed to create thread %d", i);
            return 1;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; ++i)
    {
        cl_thread_join(threads[i], null);
        cl_thread_destroy(threads[i]);
    }

    // Clean up
    cl_mutex_destroy(mutex);
    cl_cond_destroy(cond);

    cl_log_info("All threads completed. Final counter value: %d", shared_counter);

    // Shutdown the logging system
    cl_log_cleanup();

    return 0;
}
