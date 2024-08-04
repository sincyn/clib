#include "clib/defines.h"
#ifdef CL_PLATFORM_WINDOWS
/**
 * Created by jraynor on 8/4/2024.
 */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "time_internal.h"

bool cl_time_platform_get_current(cl_time_t *time)
{
    if (time == null)
        return false;

    FILETIME ft;
    ULARGE_INTEGER uli;

    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;

    // Convert to UNIX epoch (January 1, 1970)
    uli.QuadPart -= 116444736000000000ULL;
    time->seconds = uli.QuadPart / 10000000;
    time->nanoseconds = (uli.QuadPart % 10000000) * 100;
    return true;
}

void cl_time_platform_sleep(const uint32_t milliseconds) { Sleep(milliseconds); }

#endif // CL_PLATFORM_WINDOWS
