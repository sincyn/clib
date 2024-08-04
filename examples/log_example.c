/**
 * Created by jraynor on 8/3/2024.
 */
#include <clib/log_lib.h>

int main()
{
    // Initialize the logging system
    cl_log_config_t config = {.include_timestamp = true, .include_level = true, .include_file_line = true};
    cl_log_init(&config);

    // Configure and add the console target
    cl_log_target_config_t console_config = {.type = CL_LOG_TARGET_CONSOLE,
                                             .min_level = CL_LOG_INFO, // Set minimum level for console to INFO
                                             .config.console = {.use_colors = true}};
    cl_log_target_t *console_target = cl_log_add_target(&console_config);

    // Log some messages
    CL_LOG_TRACE("This is a trace message"); // This won't be displayed (below min_level)
    CL_LOG_DEBUG("This is a debug message"); // This won't be displayed (below min_level)
    CL_LOG_INFO("This is an info message");
    CL_LOG_WARN("This is a warning message");
    CL_LOG_ERROR("This is an error message");
    CL_LOG_FATAL("This is a fatal error message");

    // Change the console target's log level
    cl_log_set_target_level(console_target, CL_LOG_DEBUG);

    // Log more messages
    CL_LOG_DEBUG("This debug message will now be displayed");
    CL_LOG_INFO("Another info message");

    // Shutdown the logging system
    cl_log_cleanup();

    return 0;
}
