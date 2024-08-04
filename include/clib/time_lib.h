/**
 * Created by jraynor on 8/4/2024.
 */
#pragma once

#include "defines.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    i64 seconds;
    i32 nanoseconds;
} cl_time_t;


// Get the current time
bool cl_time_get_current(cl_time_t *time);

// Calculate the difference between two times
cl_time_t cl_time_diff(const cl_time_t *end, const cl_time_t *start);

// Convert cl_time_t to milliseconds
int64_t cl_time_to_ms(const cl_time_t *time);

// Convert milliseconds to cl_time_t
cl_time_t cl_time_from_ms(i64 milliseconds);

// Sleep for the specified number of milliseconds
void cl_time_sleep(u32 milliseconds);

#ifdef __cplusplus
}
#endif
