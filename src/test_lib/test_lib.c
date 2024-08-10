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
#define ANSI_COLOR_WHITE "\x1b[37m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_BACKGROUND_RED "\x1b[41m"
#define ANSI_BACKGROUND_GREEN "\x1b[42m"
#define ANSI_BACKGROUND_YELLOW "\x1b[43m"
#define ANSI_BACKGROUND_BLUE "\x1b[44m"
#define ANSI_BACKGROUND_MAGENTA "\x1b[45m"
#define ANSI_BACKGROUND_CYAN "\x1b[46m"
#define ANSI_BACKGROUND_WHITE "\x1b[47m"
#define ANSI_BACKGROUND_HEADER "\x1b[48;5;238m"

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
#define OUTPUT_WIDTH 82
#define TEST_NAME_WIDTH 30
#define RESULT_WIDTH 4
#define PROGRESS_BAR_WIDTH 20
#define MAX_PROGRESS_WIDTH 20 // Maximum width of the progress bar
#define INFO_WIDTH 14 // Width for percentage and total count

static cl_test_suite_t *g_suites[MAX_SUITES];
static u64 g_suite_count = 0;
static cl_test_context_t g_context = {0};
static void print_box_line(const char *start, const char *end, const char *fill, int width)
{
    printf("%s", start);
    for (int i = 0; i < width; i++)
    {
        printf("%s", fill);
    }
    printf("%s\n", end);
}

static void print_box_segment(const char *start, const char *end, const char *fill, int width)
{
    printf("%s", start);
    for (int i = 0; i < width; i++)
    {
        printf("%s", fill);
    }
    printf("%s", end);
}

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
        g_suites[i] = null;
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
static void print_progress_bar(u64 passed, u64 total, int width)
{
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


// Refactored run_test function
static void run_test(const char *suite_name, const char *test_name, cl_test_func_t test_func)
{
    g_context.current_suite = suite_name;
    g_context.current_test = test_name;
    u64 initial_assert_count = g_context.assert_count;
    u64 initial_pass_count = g_context.pass_count;

    test_func();

    u64 test_assert_count = g_context.assert_count - initial_assert_count;
    u64 test_pass_count = g_context.pass_count - initial_pass_count;
    u64 test_fail_count = test_assert_count - test_pass_count;

    // Start of test result line
    print_box_segment(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, "", " ", 1);

    // Print PASS/FAIL
    const char *result_str;
    char *color;
    if (test_assert_count == test_pass_count)
    {
        result_str = "PASS";
        color = ANSI_COLOR_GREEN;
    }
    else if (test_fail_count > 0)
    {
        result_str = "FAIL";
        color = ANSI_COLOR_RED;
    }
    else
    {
        result_str = "SKIP";
        color = ANSI_COLOR_YELLOW;
    }
    printf("%s%-*s" ANSI_COLOR_RESET " ", color, RESULT_WIDTH, result_str);

    // Print test name
    printf("%-*.*s %s ", TEST_NAME_WIDTH, TEST_NAME_WIDTH, test_name, ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_RESET);

    // Print progress bar with fixed width
    print_progress_bar(test_pass_count, test_assert_count, PROGRESS_BAR_WIDTH);

    // Print percentage
    double percentage = (test_assert_count > 0) ? ((double)test_pass_count / test_assert_count * 100) : 100.0;
    printf(" %s%6.2f%%%s", ANSI_COLOR_CYAN, percentage, ANSI_COLOR_RESET);

    // Print simplified results based on test outcome
    char count_str[50];
    if (test_assert_count == test_pass_count)
    {
        snprintf(count_str, sizeof(count_str), "%s%zu%s", ANSI_STYLE_UNDERLINE, test_assert_count, ANSI_STYLE_RESET);
    }
    else if (test_fail_count > 0)
    {
        snprintf(count_str, sizeof(count_str), "%s%zu%s", ANSI_STYLE_UNDERLINE, test_assert_count, ANSI_STYLE_RESET,
                 test_assert_count);
    }
    else
    {
        snprintf(count_str, sizeof(count_str), "%s%zu%s", ANSI_STYLE_UNDERLINE, test_assert_count - test_pass_count,
                 ANSI_STYLE_RESET);
    }

    int size = strlen(count_str);
    // Calculate remaining space and right-align the result string
    int remaining_space = INFO_WIDTH - size;
    printf(" (%s%s%s%s %sasserts%s)", ANSI_COLOR_WHITE, count_str, ANSI_STYLE_RESET, ANSI_COLOR_WHITE,
           ANSI_STYLE_DIM ANSI_STYLE_BOLD, ANSI_COLOR_RESET ANSI_STYLE_RESET);
    for (int i = 0; i < remaining_space - 2; i++)
    {
        printf(" ");
    }
    printf("%s%s\n", ANSI_COLOR_HEADER, BOX_VERTICAL);


    // Print detailed results if test failed
    if (test_assert_count != test_pass_count)
    {
        for (u64 i = initial_pass_count; i < g_context.pass_count; i++)
        {
            print_box_segment(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, "", " ", 3);
            printf(ANSI_COLOR_GREEN "✓ %s" ANSI_COLOR_RESET, g_context.pass_results[i].message);
            // Add right vertical line
            printf("%*s%s%s\n", OUTPUT_WIDTH - strlen(g_context.pass_results[i].message) - 5, "", ANSI_COLOR_HEADER,
                   BOX_VERTICAL);
        }
        for (u64 i = initial_assert_count; i < g_context.fail_count; i++)
        {
            print_box_segment(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, "", " ", 3);
            printf(ANSI_COLOR_RED "✗ %s" ANSI_COLOR_RESET, g_context.fail_results[i].message);
            // Add right vertical line
            printf("%*s%s%s\n", OUTPUT_WIDTH - strlen(g_context.fail_results[i].message) - 5, "", ANSI_COLOR_HEADER,
                   BOX_VERTICAL);
            print_box_segment(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, "", " ", 5);
            printf("%s:%d", g_context.fail_results[i].file, g_context.fail_results[i].line);
            // Add right vertical line
            int file_line_length =
                snprintf(null, 0, "%s:%d", g_context.fail_results[i].file, g_context.fail_results[i].line);
            printf("%*s%s%s\n", OUTPUT_WIDTH - file_line_length - 5, "", ANSI_COLOR_HEADER, BOX_VERTICAL);
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


// Update cl_test_run_suite to use the new functions
void cl_test_run_suite(const cl_test_suite_t *suite)
{
    print_box_line(ANSI_COLOR_HEADER BOX_TOP_LEFT ANSI_STYLE_RESET, BOX_TOP_RIGHT,
                   ANSI_COLOR_HEADER BOX_HORIZONTAL_DOWN, TEST_NAME_WIDTH);
    print_box_line(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, ANSI_COLOR_HEADER BOX_VERTICAL_LEFT,
                   ANSI_COLOR_CYAN BOX_HORIZONTAL_UP ANSI_COLOR_RESET, TEST_NAME_WIDTH);

    print_box_segment(ANSI_COLOR_HEADER BOX_VERTICAL ANSI_STYLE_RESET, "", " ", 1);
    int name_length = strlen(suite->name);
    int total_width = TEST_NAME_WIDTH - 1;
    int left_padding = (total_width - name_length) / 2;
    int right_padding = total_width - name_length - left_padding;
    printf("%s%*s%s%s%s%*s%s\n", ANSI_COLOR_HEADER, left_padding, " ",
           ANSI_COLOR_BLUE ANSI_STYLE_DIM ANSI_STYLE_BOLD ANSI_STYLE_UNDERLINE ANSI_STYLE_REVERSE, suite->name,
           ANSI_STYLE_RESET, right_padding, " ", ANSI_COLOR_HEADER BOX_VERTICAL);
    // printf("%s%-*s%s%-*s%s ", ANSI_STYLE_REVERSE, TEST_NAME_WIDTH / 4, " ", suite->name, TEST_NAME_WIDTH / 4, " ",
    //        ANSI_STYLE_RESET);


    print_box_line(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, ANSI_COLOR_HEADER BOX_VERTICAL_LEFT,
                   ANSI_COLOR_CYAN BOX_HORIZONTAL_DOWN ANSI_COLOR_RESET, TEST_NAME_WIDTH);
    print_box_segment(ANSI_COLOR_HEADER BOX_VERTICAL_RIGHT ANSI_STYLE_RESET, BOX_HORIZONTAL_UP,
                      ANSI_COLOR_HEADER BOX_HORIZONTAL_UP ANSI_COLOR_HEADER, TEST_NAME_WIDTH);
    print_box_line(BOX_HORIZONTAL, BOX_TOP_RIGHT, BOX_HORIZONTAL, OUTPUT_WIDTH - TEST_NAME_WIDTH - 2);

    for (u64 i = 0; i < suite->test_count; i++)
    {
        run_test(suite->name, suite->tests[i].name, suite->tests[i].func);
    }

    print_box_line(ANSI_COLOR_HEADER BOX_BOTTOM_LEFT, BOX_BOTTOM_RIGHT ANSI_COLOR_RESET, BOX_HORIZONTAL, OUTPUT_WIDTH);
}


void cl_test_run_all(void)
{
    for (u64 i = 0; i < g_suite_count; i++)
    {
        cl_test_run_suite(g_suites[i]);
    }

    // Calculate the width of the result strings
    int passed_width = snprintf(null, 0, "%zu / %zu", g_context.pass_count_total, g_context.assert_count_total);
    int failed_width = snprintf(null, 0, "%zu / %zu", g_context.fail_count_total, g_context.assert_count_total);
    int skipped_width = snprintf(null, 0, "%zu / %zu", g_context.skip_count_total, g_context.assert_count_total);

    // Find the maximum width
    int max_result_width = passed_width > failed_width ? passed_width : failed_width;
    max_result_width = max_result_width > skipped_width ? max_result_width : skipped_width;

    // Table dimensions
    const int col1_width = 20;
    const int col2_width = max_result_width + 4; // +2 for spacing
    const int table_width = col1_width + col2_width + 5; // +5 for separators and spacing

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

    // Print rows
    // Only show if any tests were passed
    if (g_context.pass_count_total > 0)
        printf("%s%s %-*s %s  \x1b[1m\x1b[4m%-*zu\x1b[0m / \x1b[1m\x1b[3m%-*zu %s%s\n", ANSI_COLOR_HEADER,
               BOX_VERTICAL ANSI_COLOR_RESET ANSI_STYLE_REVERSE, col1_width, "Passed",
               ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_GREEN, max_result_width / 2 - 1,
               g_context.pass_count_total, max_result_width / 2, g_context.assert_count_total,
               ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET);
    // Only show if any tests were failed
    if (g_context.fail_count_total > 0)
        printf("%s%s %-*s %s  \x1b[1m\x1b[4m%-*zu\x1b[0m / \x1b[1m\x1b[3m%-*zu %s%s\n", ANSI_COLOR_HEADER,
               BOX_VERTICAL ANSI_COLOR_RESET ANSI_STYLE_REVERSE, col1_width, "Failed",
               ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_RED, max_result_width / 2 - 1,
               g_context.fail_count_total, max_result_width / 2, g_context.assert_count_total,
               ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET);
    // Only show if any tests were skipped
    if (g_context.skip_count_total > 0)
        printf("%s%s %-*s %s \x1b[1m\x1b[4m%-*zu\x1b[0m / \x1b[1m\x1b[3m%-*zu %s%s\n", ANSI_COLOR_HEADER,
               BOX_VERTICAL ANSI_COLOR_RESET ANSI_STYLE_REVERSE, col1_width, "Skipped",
               ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL ANSI_COLOR_YELLOW, max_result_width / 2 - 1,
               g_context.skip_count_total, max_result_width / 2, g_context.assert_count_total,
               ANSI_STYLE_RESET ANSI_COLOR_HEADER BOX_VERTICAL, ANSI_COLOR_RESET);

    // Table bottom border
    printf("%s%s", ANSI_COLOR_HEADER, BOX_VERTICAL_RIGHT);
    for (int i = 0; i < col1_width + 2; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s", BOX_HORIZONTAL_UP);
    for (int i = 0; i < col2_width; i++)
        printf("%s", BOX_HORIZONTAL);
    printf("%s%s\n", BOX_VERTICAL_LEFT, ANSI_COLOR_RESET);

    // Progress bar row
    printf("%s%s", ANSI_COLOR_HEADER, BOX_BOTTOM_LEFT);
    printf("%s", BOX_HORIZONTAL);
    printf("%s ", BOX_VERTICAL_LEFT);
    print_progress_bar(g_context.pass_count_total, g_context.assert_count_total, 18);
    char progress_str[50];
    // printf(" %s%.1f%% ", ANSI_COLOR_CYAN, (double)g_context.pass_count_total / g_context.assert_count_total * 100);
    snprintf(progress_str, sizeof(progress_str), "%.1f%%",
             (double)g_context.pass_count_total / g_context.assert_count_total * 100);
    int progress_length = strlen(progress_str);
    int remaining_space = table_width - 25 - progress_length;
    printf(" %s%s%s", ANSI_COLOR_CYAN, progress_str, ANSI_COLOR_HEADER);
    printf("%s%s", ANSI_COLOR_HEADER, BOX_VERTICAL_RIGHT);
    for (int i = 0; i < remaining_space; i++)
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
    snprintf(output, 1024, "%s is null", value_str);
    return cl_test_assert(null == value, output, file, line);
}

void cl_test_assert_not_null(const void *value, const char *value_str, const char *file, int line)
{
    char *output = malloc(1024);
    snprintf(output, 1024, "%s is not null", value_str);
    return cl_test_assert(value != null, output, file, line);
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
