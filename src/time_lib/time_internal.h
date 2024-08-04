/**
 * Created by jraynor on 8/4/2024.
 */
#ifndef CLIB_TIME_INTERNAL_H
#define CLIB_TIME_INTERNAL_H

#include "clib/time_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

// Platform-specific initialization
bool cl_time_platform_init(void);

// Platform-specific cleanup
void cl_time_platform_cleanup(void);

// Platform-specific implementation to get current time
bool cl_time_platform_get_current(cl_time_t *time);

// Platform-specific sleep implementation
void cl_time_platform_sleep(u32 milliseconds);

#ifdef __cplusplus
}
#endif

#endif // CLIB_TIME_INTERNAL_H
