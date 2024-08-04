/**
 * Created by jraynor on 8/3/2024.
 */
#include <string.h>
#include "clib/test_lib.h"
#include "clib/thread_lib.h"
#include "clib/time_lib.h"

#define TEST_SLEEP_MS 100

static int shared_value = 0;
static cl_mutex_t *test_mutex;

void *test_thread_func(void *arg)
{
    int *value = (int *)arg;
    cl_mutex_lock(test_mutex);
    shared_value += *value;
    cl_mutex_unlock(test_mutex);
    return NULL;
}

CL_TEST(test_thread_create_and_join)
{
    int arg = 5;
    cl_thread_t *thread = cl_thread_create(test_thread_func, &arg, CL_THREAD_FLAG_NONE);
    CL_ASSERT_NOT_NULL(thread);

    cl_thread_join(thread, NULL);
    CL_ASSERT_EQUAL(shared_value, 5);

    cl_thread_destroy(thread);
}

CL_TEST(test_thread_mutex)
{
    test_mutex = cl_mutex_create();
    CL_ASSERT_NOT_NULL(test_mutex);

    int arg1 = 3, arg2 = 7;
    cl_thread_t *thread1 = cl_thread_create(test_thread_func, &arg1, CL_THREAD_FLAG_NONE);
    cl_thread_t *thread2 = cl_thread_create(test_thread_func, &arg2, CL_THREAD_FLAG_NONE);

    cl_thread_join(thread1, NULL);
    cl_thread_join(thread2, NULL);

    CL_ASSERT_EQUAL(shared_value, 15);

    cl_thread_destroy(thread1);
    cl_thread_destroy(thread2);
    cl_mutex_destroy(test_mutex);
}

CL_TEST(test_thread_sleep)
{
    cl_time_t start, end;
    cl_time_get_current(&start);
    cl_thread_sleep(TEST_SLEEP_MS);
    cl_time_get_current(&end);

    const cl_time_t diff = cl_time_diff(&end, &start);
    const i64 diff_ms = cl_time_to_ms(&diff);
    CL_ASSERT(diff_ms >= TEST_SLEEP_MS);
}

CL_TEST_SUITE_BEGIN(ThreadTests)
CL_TEST_SUITE_TEST(test_thread_create_and_join)
CL_TEST_SUITE_TEST(test_thread_mutex)
CL_TEST_SUITE_TEST(test_thread_sleep)
CL_TEST_SUITE_END

int main()
{
    CL_RUN_TEST_SUITE(ThreadTests);
    CL_RUN_ALL_TESTS();
    return 0;
}
