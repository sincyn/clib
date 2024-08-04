#include "clib/defines.h"
#ifdef CL_PLATFORM_APPLE || CL_PLATFORM_LINUX
/**
 * Created by jraynor on 8/4/2024.
 */
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "time_internal.h"

bool cl_time_platform_get_current(cl_time_t *time)
{
    if (!time)
        return false;

    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
    {
        return false;
    }

    time->seconds = ts.tv_sec;
    time->nanoseconds = ts.tv_nsec;
    return true;
}

void cl_time_platform_sleep(uint32_t milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;

    while (nanosleep(&ts, &ts) == -1)
    {
        if (errno != EINTR)
        {
            break;
        }
    }
}
#endif
