/**
 * Created by jraynor on 8/3/2024.
 */
#include "clib/memory_lib.h"
#include <stdlib.h>
#include <string.h>
#include "allocator_internal.h"

cl_allocator_t *cl_allocator_create(const cl_allocator_config_t *config)
{
    if (config == null)
        return null;

    cl_allocator_t *allocator = malloc(sizeof(cl_allocator_t));
    if (allocator == null)
        return null;

    switch (config->type)
    {
    case CL_ALLOCATOR_TYPE_PLATFORM:
        if (!init_platform_allocator(allocator, config))
        {
            free(allocator);
            return null;
        }
        break;
    case CL_ALLOCATOR_TYPE_ARENA:
        if (!init_arena_allocator(allocator, config))
        {
            free(allocator);
            return null;
        }
        break;
    default:
        free(allocator);
        return null;
    }

    return allocator;
}

void cl_allocator_destroy(cl_allocator_t *allocator)
{
    if (allocator == null)
        return;
    switch (allocator->type)
    {
    case CL_ALLOCATOR_TYPE_PLATFORM:
        deinit_platform_allocator(allocator);
        break;
    case CL_ALLOCATOR_TYPE_ARENA:
        deinit_arena_allocator(allocator);
    default:
        break;
    }

    free(allocator);
}

void *cl_mem_alloc(const cl_allocator_t *allocator, const u64 size)
{
    if (allocator == null || allocator->alloc == null)
        return malloc(size);
    return allocator->alloc(size, allocator->user_data);
}

void *cl_mem_realloc(const cl_allocator_t *allocator, void *ptr, const u64 new_size)
{
    if (allocator == null || allocator->realloc == null)
        return realloc(ptr, new_size);
    return allocator->realloc(ptr, new_size, allocator->user_data);
}

void cl_mem_free(const cl_allocator_t *allocator, void *ptr)
{
    if (allocator == null || allocator->free == null)
        return free(ptr);
    allocator->free(ptr, allocator->user_data);
}
void *cl_mem_aligned_alloc(cl_allocator_t *allocator, u64 alignment, u64 size)
{
    // Aligns the memory allocation to the specified alignment
    // The alignment must be a power of 2
    // The size must be a multiple of the alignment

    if (allocator == null || allocator->alloc == null)
        return null;

    if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        return null;

    if (size % alignment != 0)
        return null;

    // Allocate memory with extra space for alignment
    u64 total_size = size + alignment;
    void *ptr = allocator->alloc(total_size, allocator->user_data);
    if (ptr == null)
        return null;

        // Calculate the aligned memory address
#ifdef _WIN32
    void *aligned_ptr = (void *)(((uintptr_t)ptr + alignment - 1) & ~(alignment - 1));
#else
    void *aligned_ptr = null;
    if (posix_memalign(&aligned_ptr, alignment, size) != 0)
    {
        allocator->free(ptr, allocator->user_data);
        return null;
    }
#endif

    // Store the original pointer before the aligned address
    void **store_ptr = (void **)aligned_ptr - 1;
    *store_ptr = ptr;

    return aligned_ptr;
}

void cl_mem_aligned_free(const cl_allocator_t *allocator, void *ptr)
{
    if (allocator == null || allocator->free == null)
        return;

    if (ptr == null)
        return;

    // Retrieve the original pointer before the aligned address
    void **store_ptr = (void **)ptr - 1;
    void *original_ptr = *store_ptr;

    allocator->free(original_ptr, allocator->user_data);
}


void cl_mem_set(void *ptr, const int value, const u64 num) { memset(ptr, value, num); }

void cl_mem_copy(void *dest, const void *src, const u64 num) { memcpy(dest, src, num); }

void cl_mem_move(void *dest, const void *src, const u64 num) { memmove(dest, src, num); }

int cl_mem_compare(const void *ptr1, const void *ptr2, const u64 num) { return memcmp(ptr1, ptr2, num); }
