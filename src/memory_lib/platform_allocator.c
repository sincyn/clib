/**
 * Created by jraynor on 8/3/2024.
 */
#include "clib/memory_lib.h"


#include "allocator_internal.h"

#include <stdbool.h>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>


#endif

static void *platform_alloc(size_t size, void *user_data)
{
#ifdef _WIN32
    static HANDLE g_process_heap = NULL;
    if (g_process_heap == NULL)
    {
        g_process_heap = GetProcessHeap();
    }
    return HeapAlloc(g_process_heap, 0, size);
#else
    (void)user_data; // Unused
    return malloc(size);
#endif
}

static void *platform_realloc(void *ptr, size_t new_size, void *user_data)
{
#ifdef _WIN32
  static HANDLE g_process_heap = NULL;
  if (g_process_heap == NULL)
  {
    g_process_heap = GetProcessHeap();
  }
    return HeapReAlloc(g_process_heap, 0, ptr, new_size);
#else
    (void)user_data; // Unused
    return realloc(ptr, new_size);
#endif
}

static void platform_free(void *ptr, void *user_data)
{
#ifdef _WIN32
  static HANDLE g_process_heap = NULL;
  if (g_process_heap == NULL)
  {
    g_process_heap = GetProcessHeap();
  }
    HeapFree(g_process_heap, 0, ptr);
#else
    (void)user_data; // Unused
    free(ptr);
#endif
}

bool init_platform_allocator(cl_allocator_t *allocator, const cl_allocator_config_t *config)
{
    allocator->alloc = platform_alloc;
    allocator->realloc = platform_realloc;
    allocator->free = platform_free;
    allocator->type = CL_ALLOCATOR_TYPE_PLATFORM;
    allocator->flags = config->flags;
    allocator->user_data = config->user_data;
    return true;
}

void deinit_platform_allocator(const cl_allocator_t *allocator)
{
    // Nothing to do for platform allocator
    (void)allocator;
}
