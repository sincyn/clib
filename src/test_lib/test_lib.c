/**
 * Created by jraynor on 8/3/2024.
 */
#include "clib/test_lib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef CL_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#define MAX_SUITES 100

// ANSI color codes
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Custom colors
#define ANSI_COLOR_HEADER "\x1b[38;5;208m"

#define ANSI_STYLE_BOLD "\x1b[1m"
#define ANSI_STYLE_DIM "\x1b[2m"
#define ANSI_STYLE_UNDERLINE "\x1b[4m"
#define ANSI_STYLE_BLINK "\x1b[5m"
#define ANSI_STYLE_REVERSE "\x1b[7m"
#define ANSI_STYLE_HIDDEN "\x1b[8m"
#define ANSI_STYLE_RESET "\x1b[0m"

// Progress bar characters
#define BLOCK_FULL "█"
#define BLOCK_EMPTY "░"

// Box drawing characters
#define BOX_HORIZONTAL "─"
#define BOX_VERTICAL "│"
#define BOX_TOP_LEFT "┌"
#define BOX_TOP_RIGHT "┐"
#define BOX_BOTTOM_LEFT "└"
#define BOX_BOTTOM_RIGHT "┘"
#define BOX_VERTICAL_RIGHT "├"
#define BOX_VERTICAL_LEFT "┤"
#define BOX_HORIZONTAL_DOWN "┬"
#define BOX_HORIZONTAL_UP "┴"
#define BOX_VERTICAL_HORIZONTAL "┼"


// Constants for formatting
#define OUTPUT_WIDTH 80
#define TEST_NAME_WIDTH 35
#define RESULT_WIDTH 6
#define PROGRESS_BAR_WIDTH 20

static cl_test_suite_t *g_suites[MAX_SUITES];
static u64 g_suite_count = 0;
static cl_test_context_t g_context = {0};


void cl_test_init(void)
{
    memset(&g_context, 0, sizeof(g_context));
#ifdef CL_PLATFORM_WINDOWS
    // Enable ANSI escape codes on Windows
    SetConsoleOutputCP(65001);
#endif
}

void cl_test_cleanup(void)
{
    for (u64 i = 0; i < g_suite_count; i++)
    {
        g_suites[i] = NULL;
    }
    g_suite_count = 0;
}

void cl_test_register_suite(const cl_test_suite_t *suite)
{
    if (g_suite_count < MAX_SUITES)
    {
        g_suites[g_suite_count++] = (cl_test_suite_t *)suite;
    }
    else
    {
        fprintf(stderr, ANSI_COLOR_RED "Error: Maximum number of test suites reached." ANSI_COLOR_RESET "\n");
    }
}

static void print_header(const char *title)
{
    u64 title_length = strlen(title);
    u64 box_width = title_length + 4; // 2 space padding on each side

    // Top border
    printf("\n%s%s", ANSI_COLOR_HEADER, BOX_TOP_LEFT);
    for (u64 i = 0; i < box_width; i++)
    {
        printf("%s", BOX_HORIZONTAL);
    }
    printf("%s\n", BOX_TOP_RIGHT);

    // Title with vertical borders
    printf("%s  %s%s%s  %s\n", BOX_VERTICAL, ANSI_COLOR_YELLOW, title, ANSI_COLOR_HEADER, BOX_VERTICAL);

    // Bottom border
    printf("%s", BOX_BOTTOM_LEFT);
    for (u64 i = 0; i < box_width; i++)
    {
        printf("%s", BOX_HORIZONTAL);
    }
    printf("%s%s\n\n", BOX_BOTTOM_RIGHT, ANSI_COLOR_RESET);
}


// Update the print_progress_bar function for better visual
static void print_progress_bar(u64 passed, u64 total)
{
    const int width = 20;
    int filled_width = (int)((double)passed / total * width);

    for (int i = 0; i < filled_width; i++)
    {
        printf("%s█", ANSI_COLOR_GREEN);
    }
    for (int i = filled_width; i < width; i++)
    {
        printf("%s░", ANSI_COLOR_RED);
    }
    printf(ANSI_COLOR_RESET);
}


static void run_test(const char *suite_name, const char *test_name, cl_test_func_t test_func)
{
    g_context.current_suite = suite_name;
    g_context.current_test = test_name;
    u64 initial_assert_count = g_context.assert_count;
    u64 initial_pass_count = g_context.pass_count;

    test_func();

    u64 test_assert_count = g_context.assert_count - initial_assert_count;
    u64 test_pass_count = g_context.pass_count - initial_pass_count;

    printf("%-*.*s ", TEST_NAME_WIDTH, TEST_NAME_WIDTH, test_name);

    if (test_assert_count == test_pass_count)
    {
        printf(ANSI_COLOR_GREEN "%-*s" ANSI_COLOR_RESET " ", RESULT_WIDTH, "PASS");
    }
    else
    {
        printf(ANSI_COLOR_RED "%-*s" ANSI_COLOR_RESET " ", RESULT_WIDTH, "FAIL");
    }

    print_progress_bar(test_pass_count, test_assert_count);
    printf(" %s%zu/%zu%s\n", ANSI_COLOR_BLUE, test_pass_count, test_assert_count, ANSI_COLOR_RESET);

    if (test_assert_count != test_pass_count)
    {
        for (u64 i = initial_pass_count; i < g_context.pass_count; i++)
        {
            printf("  " ANSI_COLOR_GREEN "✓ %s" ANSI_COLOR_RESET "\n", g_context.pass_results[i].message);
        }
        for (u64 i = initial_assert_count; i < g_context.fail_count; i++)
        {
            printf("  " ANSI_COLOR_RED "✗ %s" ANSI_COLOR_RESET "\n", g_context.fail_results[i].message);
            printf("    %s:%d\n", g_context.fail_results[i].file, g_context.fail_results[i].line);
        }
    }

    // Reset the context
    g_context.pass_count_total += test_pass_count;
    g_context.fail_count_total += test_assert_count - test_pass_count;
    g_context.assert_count_total += test_assert_count;
    g_context.pass_count = 0;
    g_context.fail_count = 0;
    g_context.assert_count = 0;
}

void cl_test_run_suite(const cl_test_suite_t *suite)
{
    print_header(suite->name);
    for (u64 i = 0; i < suite->test_count; i++)
    {
        run_test(suite->name, suite->tests[i].name, suite->tests[i].func);
    }
    printf("\n");
}
void print_row_separator(int col1_width, int col2_width)
{
    printf("%s%s", ANSI_COLOR_HEADER, BOX_VERTICAL_RIGHT);
    for (int i = 0; i < col1_width; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s", BOX_VERTICAL);
    for (int i = 0; i < col2_width; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s%s\n", BOX_VERTICAL_LEFT, ANSI_COLOR_RESET);
}

// Function to print a table row
#define PRINT_ROW(label, count, color)                                                                                 \
    printf("%s%s %-*s %s%*s%s%s%zu%s/%zu %s%s\n", ANSI_COLOR_HEADER, BOX_VERTICAL, col1_width, label,                  \
           ANSI_COLOR_HEADER BOX_VERTICAL, col2_width - 7, "", color ANSI_STYLE_BOLD ANSI_STYLE_UNDERLINE, count,      \
           ANSI_STYLE_RESET color, g_context.assert_count_total, ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET)

void cl_test_run_all(void)
{
    for (u64 i = 0; i < g_suite_count; i++)
    {
        cl_test_run_suite(g_suites[i]);
    }

    // Table dimensions
    const int table_width = 35;
    const int col1_width = 20;
    const int col2_width = table_width - col1_width - 3; // -3 for separators


    // Print header
    // Print header
    printf("\n%s%s %sTest Summary%s %s%s\n", ANSI_COLOR_HEADER, BOX_TOP_LEFT,
           ANSI_STYLE_BOLD ANSI_STYLE_UNDERLINE ANSI_COLOR_MAGENTA, ANSI_STYLE_RESET ANSI_COLOR_HEADER, BOX_TOP_RIGHT,
           ANSI_COLOR_RESET);
    printf("%s%s", ANSI_COLOR_HEADER, BOX_VERTICAL_RIGHT);
    // At the right of the Test summary, we put the left of the top right box
    for (int i = 0; i < 14; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s", BOX_HORIZONTAL_UP);
    for (int i = 13; i < col1_width; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s", BOX_HORIZONTAL_DOWN);
    for (int i = col1_width; i < table_width - 5; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s%s\n", BOX_TOP_RIGHT, ANSI_COLOR_RESET);


    printf("%s%s %-*s %s \x1b[1m\x1b[4m%-*zu / %-*zu\x1b[0m  %s%s\n", ANSI_COLOR_HEADER,
           BOX_VERTICAL ANSI_COLOR_RESET ANSI_STYLE_REVERSE, col1_width, "Passed",
           ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_GREEN, 2, g_context.pass_count_total, 2,
           g_context.assert_count_total, ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET);
    printf("%s%s %-*s %s \x1b[1m\x1b[4m%-*zu / %-*zu\x1b[0m  %s%s\n", ANSI_COLOR_HEADER,
           BOX_VERTICAL ANSI_COLOR_RESET ANSI_STYLE_REVERSE, col1_width, "Failed",
           ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_RED, 2, g_context.fail_count_total, 2,
           g_context.assert_count_total, ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET);
    printf("%s%s %-*s %s \x1b[1m\x1b[4m%-*zu / %-*zu\x1b[0m  %s%s\n", ANSI_COLOR_HEADER,
           BOX_VERTICAL ANSI_COLOR_RESET ANSI_STYLE_REVERSE, col1_width, "Skipped",
           ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_YELLOW, 2, g_context.skip_count_total, 2,
           g_context.assert_count_total, ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET);
    // Table bottom border
    printf("%s%s", ANSI_COLOR_HEADER, BOX_VERTICAL_RIGHT);
    for (int i = 0; i < col1_width + 2; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s", BOX_HORIZONTAL_UP);
    for (int i = 0; i < col2_width - 2; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s%s\n", BOX_VERTICAL_LEFT, ANSI_COLOR_RESET);

    // Progress bar row
    printf("%s%s", ANSI_COLOR_HEADER, BOX_BOTTOM_LEFT);
    for (int i = 0; i < 1; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s ", BOX_VERTICAL_LEFT);
    print_progress_bar(g_context.pass_count_total, g_context.assert_count_total);
    printf(" %s%.1f%% ", ANSI_COLOR_CYAN, (double)g_context.pass_count_total / g_context.assert_count_total * 100);
    printf("%s%s", ANSI_COLOR_HEADER, BOX_VERTICAL_RIGHT);
    // Fill the rest of the row with horizontal lines
    int remaining_width = table_width - 34; // Approximate width of progress bar and percentage
    for (int i = 0; i < remaining_width; i++)
        printf("%s", BOX_HORIZONTAL);

    printf("%s%s\n", ANSI_COLOR_HEADER, BOX_BOTTOM_RIGHT);
    printf("%s\n", ANSI_COLOR_RESET);

    // If it all passed, print a nice message
    if (g_context.fail_count_total == 0)
    {
        printf(ANSI_COLOR_GREEN "All tests passed!" ANSI_COLOR_RESET "\n");
    }
    else
    {
        printf(ANSI_COLOR_RED "Some tests failed." ANSI_COLOR_RESET "\n");
    }
}
void cl_test_skip(const char *reason)
{
    printf(ANSI_COLOR_YELLOW "SKIPPED: %s" ANSI_COLOR_RESET "\n", reason);
    g_context.skip_count++;
}

cl_test_context_t *cl_test_get_context(void) { return &g_context; }

// Assertion implementations

void cl_test_assert(bool condition, const char *condition_str, const char *file, int line)
{
    g_context.assert_count++;
    if (condition)
    {
        g_context.pass_results[g_context.pass_count].result = CL_TEST_PASSED;
        g_context.pass_results[g_context.pass_count].message = condition_str;
        g_context.pass_results[g_context.pass_count].file = file;
        g_context.pass_results[g_context.pass_count].line = line;
        g_context.pass_count++;
    }
    else
    {
        g_context.fail_results[g_context.fail_count].result = CL_TEST_FAILED;
        g_context.fail_results[g_context.fail_count].message = condition_str;
        g_context.fail_results[g_context.fail_count].file = file;
        g_context.fail_results[g_context.fail_count].line = line;
        g_context.fail_count++;
    }
}

void cl_test_assert_equal(long long actual, long long expected, const char *actual_str, const char *expected_str,
                          const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "%s == %s", actual_str, expected_str);
    return cl_test_assert(actual == expected, output, file, line);
}

void cl_test_assert_not_equal(long long actual, long long expected, const char *actual_str, const char *expected_str,
                              const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "%s != %s", actual_str, expected_str);
    return cl_test_assert(actual != expected, output, file, line);
}

void cl_test_assert_null(const void *value, const char *value_str, const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "%s is NULL", value_str);
    return cl_test_assert(value == NULL, output, file, line);
}

void cl_test_assert_not_null(const void *value, const char *value_str, const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "%s is not NULL", value_str);
    return cl_test_assert(value != NULL, output, file, line);
}

void cl_test_assert_string_equal(const char *actual, const char *expected, const char *actual_str,
                                 const char *expected_str, const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "strcmp(%s, %s) == 0", actual_str, expected_str);
    return cl_test_assert(strcmp(actual, expected) == 0, output, file, line);
}

void cl_test_assert_string_not_equal(const char *actual, const char *expected, const char *actual_str,
                                     const char *expected_str, const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "strcmp(%s, %s) != 0", actual_str, expected_str);
    return cl_test_assert(strcmp(actual, expected) != 0, output, file, line);
}

void cl_test_assert_float_equal(double actual, double expected, double epsilon, const char *actual_str,
                                const char *expected_str, const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "fabs(%s - %s) < %f", actual_str, expected_str, epsilon);
    return cl_test_assert(fabs(actual - expected) < epsilon, output, file, line);
}
