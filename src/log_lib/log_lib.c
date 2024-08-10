/**
 * Created by jraynor on 8/3/2024.
 */

#include <clib/log_lib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LOG_MESSAGE_LENGTH 1024

// ANSI color codes
#define ANSI_RESET "\x1b[0m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_DIM "\x1b[2m"
#define ANSI_UNDERLINE "\x1b[4m"
#define ANSI_BLINK "\x1b[5m"
#define ANSI_BLACK "\x1b[30m"
#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"
#define ANSI_WHITE "\x1b[37m"

// Special characters for log decoration
#define LOG_PREFIX "│ "
#define LOG_SUFFIX " │"
#define LOG_SEPARATOR "─"
#define LOG_LINE_LEFT "├"
#define LOG_LINE_RIGHT "┤"
#define LOG_CORNER "╭"

#define LOG_CORNER_TOP "╮"
#define LOG_CORNER_BOTTOM_RIGHT "╰"
#define LOG_CORNER_BOTTOM_LEFT "╯"

#define MAX_TARGETS 10
#define MAX_LOG_MESSAGE_LENGTH 1024
#define MAX_LINE_LENGTH 90
// Forward declarations for target-specific functions
extern void console_log_write(cl_log_level_t level, const char *message);
extern void console_log_init(bool use_colors);
extern void console_log_shutdown(void);

#ifdef CL_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

typedef struct cl_log_target
{
    cl_log_target_type_t type;
    cl_log_level_t min_level;
    union
    {
        struct
        {
            bool use_colors;
        } console;
        // Add other target-specific data here in the future
    } config;
    void (*write)(cl_log_level_t level, const char *message);
    void (*init)(void *config);
    void (*shutdown)(void);
} cl_log_target_t;

static struct
{
    cl_log_config_t config;
    cl_log_target_t targets[MAX_TARGETS];
    int target_count;
} log_context;

static const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

int cl_log_init(const cl_log_config_t *config)
{
    if (config == null)
    {
        return -1;
    }
    memcpy(&log_context.config, config, sizeof(cl_log_config_t));
    log_context.target_count = 0;
#ifdef _WIN32
    // Sets the code page to UTF-8
    SetConsoleOutputCP(65001);
#endif
    return 0;
}

void cl_log_cleanup(void)
{
    for (int i = 0; i < log_context.target_count; i++)
    {
        if (log_context.targets[i].shutdown)
        {
            log_context.targets[i].shutdown();
        }
    }
    log_context.target_count = 0;
}


cl_log_target_t *cl_log_add_target(const cl_log_target_config_t *target_config)
{
    if (log_context.target_count >= MAX_TARGETS)
    {
        return null;
    }

    cl_log_target_t *target = &log_context.targets[log_context.target_count++];
    target->type = target_config->type;
    target->min_level = target_config->min_level;

    switch (target->type)
    {
    case CL_LOG_TARGET_CONSOLE:
        target->config.console.use_colors = target_config->config.console.use_colors;
        target->write = console_log_write;
        target->init = (void (*)(void *))console_log_init;
        target->shutdown = console_log_shutdown;
        break;
        // Add cases for other target types here
    default:
        log_context.target_count--;
        return null;
    }

    if (target->init)
    {
        target->init(&target->config);
    }

    return target;
}


void cl_log_remove_target(cl_log_target_t *target)
{
    for (int i = 0; i < log_context.target_count; i++)
    {
        if (&log_context.targets[i] == target)
        {
            if (target->shutdown)
            {
                target->shutdown();
            }
            memmove(&log_context.targets[i], &log_context.targets[i + 1],
                    (log_context.target_count - i - 1) * sizeof(cl_log_target_t));
            log_context.target_count--;
            break;
        }
    }
}

void cl_log_set_target_level(cl_log_target_t *target, cl_log_level_t level)
{
    if (target)
    {
        target->min_level = level;
    }
}
static const char *level_colors[] = {ANSI_DIM ANSI_CYAN, ANSI_BLUE, ANSI_GREEN,
                                     ANSI_YELLOW,        ANSI_RED,  ANSI_BOLD ANSI_RED};

static void format_log_message(char *buffer, u64 buffer_size, cl_log_level_t level, const char *file, int line,
                               const char *fmt, va_list args)
{
    u64 written = 0;
    char timestamp[20] = {0};
    char file_line_str[50] = {0};
    char message[MAX_LOG_MESSAGE_LENGTH];
    char wrapped_message[MAX_LOG_MESSAGE_LENGTH * 2] = {0}; // Allow space for line wrapping

    // Format timestamp
    if (log_context.config.include_timestamp)
    {
        time_t now = time(null);
        struct tm *tm_info = localtime(&now);
        if (log_context.config.use_short_time_format)
        {
            strftime(timestamp, sizeof(timestamp), "%I:%M:%S %p", tm_info);
        }
        else
        {
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
        }
    }

    // Format level string

    // Format file and line information
    if (log_context.config.include_file_line)
    {
        char formatted_file[50] = {0};
        const char *last_slash;
#ifdef CL_PLATFORM_WINDOWS
        last_slash = strrchr(file, '\\');
#else
        last_slash = strrchr(file, '/');
#endif
        if (last_slash)
        {
            snprintf(formatted_file, sizeof(formatted_file), "%s", last_slash + 1);
        }
        else
        {
            snprintf(formatted_file, sizeof(formatted_file), "%s", file);
        }
        snprintf(file_line_str, sizeof(file_line_str), "%s%s:%d%s", ANSI_DIM, formatted_file, line, ANSI_RESET);
    }

    // Format the actual message
    vsnprintf(message, sizeof(message), fmt, args);

    // Wrap long lines
    // Wrap long lines
    u64 prefix_length = strlen(timestamp) + strlen(file_line_str) + 5; // +5 for spaces and LOG_SUFFIX
    u64 first_line_max_length = MAX_LINE_LENGTH - prefix_length - strlen(LOG_PREFIX) - strlen(LOG_SUFFIX);
    u64 subsequent_line_max_length = MAX_LINE_LENGTH - strlen(LOG_PREFIX) - strlen(LOG_SUFFIX);
    u64 msg_length = strlen(message);
    u64 current_pos = 0;
    bool is_first_line = true;

    while (current_pos < msg_length)
    {
        u64 remaining = msg_length - current_pos;
        u64 max_length = is_first_line ? first_line_max_length : subsequent_line_max_length;
        u64 line_length = (remaining > max_length) ? max_length : remaining;

        // Find the last space in the line, if needed
        if (line_length < remaining && line_length == max_length)
        {
            u64 last_space = line_length;
            while (last_space > 0 && message[current_pos + last_space] != ' ')
            {
                last_space--;
            }
            if (last_space > 0)
            {
                line_length = last_space;
            }
        }

        // Append the line to the wrapped message
        if (!is_first_line)
        {
            strcat(wrapped_message, "\n");
            strcat(wrapped_message, level_colors[level]);
            strcat(wrapped_message, LOG_PREFIX ANSI_RESET);
        }
        else
        {
            strcat(wrapped_message, LOG_PREFIX);
        }

        strncat(wrapped_message, message + current_pos, line_length);

        // Add padding spaces and LOG_SUFFIX
        u64 padding = max_length - line_length;
        if (is_first_line)
        {

            for (u64 i = 0; i < padding; i++)
            {
                strcat(wrapped_message, " ");
            }
        }
        else
        {
            padding -= 9;
            for (u64 i = 0; i < padding; i++)
            {
                strcat(wrapped_message, " ");
            }
        }
        strcat(wrapped_message, level_colors[level]);
        strcat(wrapped_message, LOG_SUFFIX);
        strcat(wrapped_message, ANSI_RESET);

        current_pos += line_length;

        // Skip the space at the beginning of the next line
        if (message[current_pos] == ' ')
        {
            current_pos++;
        }

        is_first_line = false;
    }

    // Combine all parts
    written += snprintf(buffer + written, buffer_size - written, "%s%s%s%s%s%s%s%s", ANSI_BOLD, level_colors[level],
                        LOG_CORNER, ANSI_RESET, LOG_SEPARATOR, level_colors[level], level_strings[level], ANSI_RESET);
    for (int i = 0; i < MAX_LINE_LENGTH - 15 - strlen(level_strings[level]) - 1; i++)
    {
        written += snprintf(buffer + written, buffer_size - written, "%s", LOG_SEPARATOR);
    }
    written += snprintf(buffer + written, buffer_size - written, "%s%s%s%s\n", level_colors[level], LOG_CORNER_TOP,
                        level_colors[level], ANSI_RESET);
    written += snprintf(buffer + written, buffer_size - written, "%s%s", level_colors[level], LOG_PREFIX ANSI_RESET);

    if (log_context.config.include_timestamp)
    {
        written += snprintf(buffer + written, buffer_size - written, "%s%s%s ", ANSI_DIM, timestamp, ANSI_RESET);
    }

    if (log_context.config.include_file_line)
    {
        written += snprintf(buffer + written, buffer_size - written, "%s ", file_line_str);
    }

    written += snprintf(buffer + written, buffer_size - written, "%s\n", wrapped_message);
    written +=
        snprintf(buffer + written, buffer_size - written, "%s%s%s", level_colors[level], LOG_CORNER_BOTTOM_RIGHT);
    // Renders a straight line
    // Set the color to the level color
    written += snprintf(buffer + written, buffer_size - written, "%s", level_colors[level]);
    for (int i = 0; i < MAX_LINE_LENGTH - 15; i++)
    {
        written += snprintf(buffer + written, buffer_size - written, "%s", LOG_SEPARATOR);
    }
    // Reset the color
    written += snprintf(buffer + written, buffer_size - written, "%s%s\n", LOG_CORNER_BOTTOM_LEFT, ANSI_RESET);
}

void cl_logv(cl_log_level_t level, const char *file, int line, const char *fmt, va_list args)
{
    char message[MAX_LOG_MESSAGE_LENGTH];
    format_log_message(message, sizeof(message), level, file, line, fmt, args);

    for (int i = 0; i < log_context.target_count; i++)
    {
        if (level >= log_context.targets[i].min_level)
        {
            log_context.targets[i].write(level, message);
        }
    }
}

void cl_log_init_default(cl_log_level_t min_level)
{
    cl_log_config_t config = {
        .include_timestamp = true, .include_level = true, .include_file_line = true, .use_short_time_format = true};
    cl_log_init(&config);

    cl_log_target_config_t console_config = {
        .type = CL_LOG_TARGET_CONSOLE, .min_level = min_level, .config.console = {.use_colors = true}};
    cl_log_add_target(&console_config);
}

void cl_log(cl_log_level_t level, const char *file, int line, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    cl_logv(level, file, line, fmt, args);
    va_end(args);
}
