#include "clib/time_lib.h"
#include <time.h>
#include "time_internal.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif


bool cl_time_get_current(cl_time_t *time)
{
    return cl_time_platform_get_current(time);
}
cl_time_t cl_time_diff(const cl_time_t *end, const cl_time_t *start)
{
    cl_time_t diff;
    diff.seconds = end->seconds - start->seconds;
    diff.nanoseconds = end->nanoseconds - start->nanoseconds;

    if (diff.nanoseconds < 0) {
        diff.seconds--;
        diff.nanoseconds += 1000000000;
    }

    return diff;
}

int64_t cl_time_to_ms(const cl_time_t *time)
{
    return time->seconds * 1000 + time->nanoseconds / 1000000;
}

cl_time_t cl_time_from_ms(int64_t milliseconds)
{
    cl_time_t time;
    time.seconds = milliseconds / 1000;
    time.nanoseconds = (milliseconds % 1000) * 1000000;
    return time;
}

void cl_time_sleep(uint32_t milliseconds)
{
    cl_time_platform_sleep(milliseconds);
}