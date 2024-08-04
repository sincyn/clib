/**
 * Created by jraynor on 8/3/2024.
 */
#include <clib/log_lib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#endif

static bool use_colors = false;

#ifdef _WIN32
static WORD get_color_attribute(cl_log_level_t level)
{
    switch (level)
    {
    case CL_LOG_TRACE:
        return FOREGROUND_INTENSITY;
    case CL_LOG_DEBUG:
        return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case CL_LOG_INFO:
        return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case CL_LOG_WARN:
        return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    case CL_LOG_ERROR:
        return FOREGROUND_RED | FOREGROUND_INTENSITY;
    case CL_LOG_FATAL:
        return FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    default:
        return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    }
}
#else
static const char *get_color_code(cl_log_level_t level)
{
    switch (level)
    {
    case CL_LOG_TRACE:
        return "\033[37m"; // White
    case CL_LOG_DEBUG:
        return "\033[36m"; // Cyan
    case CL_LOG_INFO:
        return "\033[32m"; // Green
    case CL_LOG_WARN:
        return "\033[33m"; // Yellow
    case CL_LOG_ERROR:
        return "\033[31m"; // Red
    case CL_LOG_FATAL:
        return "\033[35m"; // Magenta
    default:
        return "\033[0m"; // Reset
    }
}
#endif
void console_log_init(bool use_colors_param)
{
    use_colors = use_colors_param;
#ifdef _WIN32
    // Enable ANSI escape codes on Windows 10 and later
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
#endif
}

void console_log_shutdown(void)
{
    // Nothing to do for console shutdown
}

void console_log_write(cl_log_level_t level, const char *message)
{
    FILE *output = (level >= CL_LOG_ERROR) ? stderr : stdout;

    if (use_colors)
    {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(level >= CL_LOG_ERROR ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(hConsole, &csbi);
        SetConsoleTextAttribute(hConsole, get_color_attribute(level));
        fprintf(output, "%s\n", message);
        SetConsoleTextAttribute(hConsole, csbi.wAttributes);
#else
        fprintf(output, "%s%s\033[0m\n", get_color_code(level), message);
#endif
    }
    else
    {
        fprintf(output, "%s\n", message);
    }
    fflush(output);
}
