#ifndef CLIB_LOG_LIB_H
#define CLIB_LOG_LIB_H

#include "defines.h"

#ifdef __cplusplus
extern "C" {
#endif

// Log levels
typedef enum
{
    CL_LOG_TRACE,
    CL_LOG_DEBUG,
    CL_LOG_INFO,
    CL_LOG_WARN,
    CL_LOG_ERROR,
    CL_LOG_FATAL
} cl_log_level_t;

// Log target types
typedef enum
{
    CL_LOG_TARGET_CONSOLE,
    CL_LOG_TARGET_FILE,
    CL_LOG_TARGET_REMOTE
} cl_log_target_type_t;

// Forward declaration of the log target structure
typedef struct cl_log_target cl_log_target_t;

// Log target configuration
typedef struct
{
    cl_log_target_type_t type;
    cl_log_level_t min_level;
    union
    {
        struct
        {
            bool use_colors;
        } console;
        struct
        {
            const char *filename;
            bool append;
        } file;
        struct
        {
            const char *host;
            uint16_t port;
            const char *pub_key_path;
        } remote;
    } config;
} cl_log_target_config_t;

// Log configuration
typedef struct
{
    bool include_timestamp;
    bool include_level;
    bool include_file_line;
    bool use_short_time_format;
} cl_log_config_t;

// Initialize the logging system
int cl_log_init(const cl_log_config_t *config);


// Shutdown the logging system
void cl_log_cleanup(void);

// Add a log target
cl_log_target_t *cl_log_add_target(const cl_log_target_config_t *target_config);

// Remove a log target
void cl_log_remove_target(cl_log_target_t *target);

// Set the minimum log level for a specific target
void cl_log_set_target_level(cl_log_target_t *target, cl_log_level_t level);

// Log a message
void cl_log(cl_log_level_t level, const char *file, int line, const char *fmt, ...);

// Log a message with va_list
void cl_logv(cl_log_level_t level, const char *file, int line, const char *fmt, va_list args);


inline void cl_log_init_default(cl_log_level_t min_level)
{
    cl_log_config_t config = {.include_timestamp = true,
                              .include_level = true,
                              .include_file_line = true,
                              .use_short_time_format = true};
    cl_log_init(&config);

    cl_log_target_config_t console_config = {
        .type = CL_LOG_TARGET_CONSOLE, .min_level = min_level, .config.console = {.use_colors = true}};
    cl_log_add_target(&console_config);
}

// Convenience macros for logging
#define CL_LOG_TRACE(...) cl_log(CL_LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define CL_LOG_DEBUG(...) cl_log(CL_LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define CL_LOG_INFO(...) cl_log(CL_LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define CL_LOG_WARN(...) cl_log(CL_LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define CL_LOG_ERROR(...) cl_log(CL_LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define CL_LOG_FATAL(...) cl_log(CL_LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // CLIB_LOG_LIB_H
