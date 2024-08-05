#include <string.h>
#include "clib/test_lib.h"
#include "clib/time_lib.h"

#define TEST_SLEEP_MS 100
#define EPSILON_MS 10 // Allow for small variations in timing

CL_TEST(test_time_get_current)
{
    cl_time_t time1, time2;
    CL_ASSERT(cl_time_get_current(&time1));
    
    // Sleep for a short duration to ensure time has passed
    cl_time_sleep(10);
    
    CL_ASSERT(cl_time_get_current(&time2));
    
    // Ensure time2 is greater than time1
    CL_ASSERT(time2.seconds > time1.seconds || 
              (time2.seconds == time1.seconds && time2.nanoseconds > time1.nanoseconds));
}

CL_TEST(test_time_diff)
{
    cl_time_t start = {.seconds = 10, .nanoseconds = 500000000};
    cl_time_t end = {.seconds = 11, .nanoseconds = 700000000};
    
    cl_time_t diff = cl_time_diff(&end, &start);
    
    CL_ASSERT_EQUAL(diff.seconds, 1);
    CL_ASSERT_EQUAL(diff.nanoseconds, 200000000);
    
    // Test case where nanoseconds wrap around
    start.nanoseconds = 800000000;
    end.seconds = 12;
    end.nanoseconds = 300000000;
    
    diff = cl_time_diff(&end, &start);
    
    CL_ASSERT_EQUAL(diff.seconds, 1);
    CL_ASSERT_EQUAL(diff.nanoseconds, 500000000);
}

CL_TEST(test_time_to_ms)
{
    cl_time_t time = {.seconds = 5, .nanoseconds = 500000000};
    int64_t ms = cl_time_to_ms(&time);
    CL_ASSERT_EQUAL(ms, 5500);
    
    time.seconds = 0;
    time.nanoseconds = 1500000;
    ms = cl_time_to_ms(&time);
    CL_ASSERT_EQUAL(ms, 1);
}

CL_TEST(test_time_from_ms)
{
    cl_time_t time = cl_time_from_ms(5500);
    CL_ASSERT_EQUAL(time.seconds, 5);
    CL_ASSERT_EQUAL(time.nanoseconds, 500000000);
    
    time = cl_time_from_ms(1);
    CL_ASSERT_EQUAL(time.seconds, 0);
    CL_ASSERT_EQUAL(time.nanoseconds, 1000000);
}

CL_TEST(test_time_sleep)
{
    cl_time_t start, end;
    CL_ASSERT(cl_time_get_current(&start));
    
    cl_time_sleep(TEST_SLEEP_MS);
    
    CL_ASSERT(cl_time_get_current(&end));
    
    cl_time_t diff = cl_time_diff(&end, &start);
    int64_t diff_ms = cl_time_to_ms(&diff);
    
    // Allow for some small variation in sleep time
    CL_ASSERT(diff_ms >= TEST_SLEEP_MS);
    CL_ASSERT(diff_ms < TEST_SLEEP_MS + EPSILON_MS);
}

CL_TEST(test_time_operations_consistency)
{
    int64_t original_ms = 12345;
    cl_time_t time = cl_time_from_ms(original_ms);
    int64_t converted_ms = cl_time_to_ms(&time);
    
    CL_ASSERT_EQUAL(original_ms, converted_ms);

}

CL_TEST_SUITE_BEGIN(TimeTests)
    CL_TEST_SUITE_TEST(test_time_get_current)
    CL_TEST_SUITE_TEST(test_time_diff)
    CL_TEST_SUITE_TEST(test_time_to_ms)
    CL_TEST_SUITE_TEST(test_time_from_ms)
    CL_TEST_SUITE_TEST(test_time_sleep)
    CL_TEST_SUITE_TEST(test_time_operations_consistency)
CL_TEST_SUITE_END

int main()
{
    CL_RUN_TEST_SUITE(TimeTests);
    CL_RUN_ALL_TESTS();
    return 0;
}