/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once

#include <stddef.h>
#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// Test result enum
typedef enum
{
    CL_TEST_PASSED,
    CL_TEST_FAILED,
    CL_TEST_SKIPPED
} cl_test_result_t;

// Test function type
typedef void (*cl_test_func_t)(void);

// Test case structure
typedef struct
{
    const char *name;
    cl_test_func_t func;
} cl_test_case_t;

// Test suite structure
typedef struct
{
    const char *name;
    cl_test_case_t *tests;
    size_t test_count;
} cl_test_suite_t;


typedef struct
{
    cl_test_result_t result;
    const char *message;
    const char *file;
    int line;
} cl_test_assert_result_t;

// Test context structure
typedef struct
{
    const char *current_suite;
    const char *current_test;
    size_t assert_count;
    size_t assert_count_total;
    size_t pass_count;
    size_t pass_count_total;

    cl_test_assert_result_t pass_results[1024];
    size_t fail_count;
    size_t fail_count_total;
    cl_test_assert_result_t fail_results[1024];
    size_t skip_count;
    size_t skip_count_total;
    cl_test_assert_result_t skip_results[1024];

} cl_test_context_t;

// Initialize the test library
void cl_test_init(void);

// Cleanup the test library
void cl_test_cleanup(void);

// Run all registered test suites
void cl_test_run_all(void);

// Run a specific test suite
void cl_test_run_suite(const cl_test_suite_t *suite);

// Register a test suite
void cl_test_register_suite(const cl_test_suite_t *suite);

// Skip the current test
void cl_test_skip(const char *reason);

// Assertion macros
#define CL_ASSERT(condition) cl_test_assert(condition, #condition, __FILE__, __LINE__)

#define CL_ASSERT_EQUAL(actual, expected) cl_test_assert_equal(actual, expected, #actual, #expected, __FILE__, __LINE__)

#define CL_ASSERT_NOT_EQUAL(actual, expected)                                                                          \
    cl_test_assert_not_equal(actual, expected, #actual, #expected, __FILE__, __LINE__)

#define CL_ASSERT_NULL(value) cl_test_assert_null(value, #value, __FILE__, __LINE__)

#define CL_ASSERT_NOT_NULL(value) cl_test_assert_not_null(value, #value, __FILE__, __LINE__)

#define CL_ASSERT_STRING_EQUAL(actual, expected)                                                                       \
    cl_test_assert_string_equal(actual, expected, #actual, #expected, __FILE__, __LINE__)

#define CL_ASSERT_STRING_NOT_EQUAL(actual, expected)                                                                   \
    cl_test_assert_string_not_equal(actual, expected, #actual, #expected, __FILE__, __LINE__)

#define CL_ASSERT_FLOAT_EQUAL(actual, expected, epsilon)                                                               \
    cl_test_assert_float_equal(actual, expected, epsilon, #actual, #expected, __FILE__, __LINE__)

// Internal assertion functions (not to be called directly)
void cl_test_assert(bool condition, const char *condition_str, const char *file, int line);
void cl_test_assert_equal(long long actual, long long expected, const char *actual_str, const char *expected_str,
                          const char *file, int line);
void cl_test_assert_not_equal(long long actual, long long expected, const char *actual_str, const char *expected_str,
                              const char *file, int line);
void cl_test_assert_null(const void *value, const char *value_str, const char *file, int line);
void cl_test_assert_not_null(const void *value, const char *value_str, const char *file, int line);
void cl_test_assert_string_equal(const char *actual, const char *expected, const char *actual_str,
                                 const char *expected_str, const char *file, int line);
void cl_test_assert_string_not_equal(const char *actual, const char *expected, const char *actual_str,
                                     const char *expected_str, const char *file, int line);
void cl_test_assert_float_equal(double actual, double expected, double epsilon, const char *actual_str,
                                const char *expected_str, const char *file, int line);

// Get the current test context
cl_test_context_t *cl_test_get_context(void);

// Macros for creating and running test suites
#define CL_TEST(test_name) static void test_name(void)
#define CL_TEST_SUITE(suite_name) static cl_test_case_t suite_name[]

#define CL_TEST_SUITE_BEGIN(suite_name) CL_TEST_SUITE(suite_name) = {

#define CL_TEST_SUITE_TEST(test_name) {#test_name, test_name},

#define CL_TEST_SUITE_END                                                                                              \
    {                                                                                                                  \
        NULL, NULL                                                                                                     \
    }                                                                                                                  \
    }                                                                                                                  \
    ;

#define CL_RUN_TEST_SUITE(suite_name)                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        static cl_test_suite_t suite = {#suite_name, suite_name, sizeof(suite_name) / sizeof(suite_name[0]) - 1};      \
        cl_test_register_suite(&suite);                                                                                \
    }                                                                                                                  \
    while (0)

#define CL_RUN_ALL_TESTS()                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        cl_test_init();                                                                                                \
        cl_test_run_all();                                                                                             \
        cl_test_cleanup();                                                                                             \
    }                                                                                                                  \
    while (0)

#ifdef __cplusplus
}
#endif
