/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once

#include <../../include/clib/log_lib.h>
#include <stdbool.h>

void console_log_init(bool use_colors);
void console_log_shutdown(void);
void console_log_write(cl_log_level_t level, const char *message);
