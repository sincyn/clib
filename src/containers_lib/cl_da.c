/**
 * Dynamic Array Implementation
 * Created by Claude on 8/6/2024.
 */

#include <string.h>
#include "clib/containers_lib.h"
#include "clib/log_lib.h"

#define CL_DA_INITIAL_CAPACITY 16
#define CL_DA_GROWTH_FACTOR 2
#define CL_DA_SHRINK_FACTOR 0.25

struct cl_da
{
    void *data;
    u64 size;
    u64 capacity;
    size_t element_size;
    cl_allocator_t *allocator;
};

cl_da_t *cl_da_init(cl_allocator_t *allocator, const u64 element_size)
{
    if (allocator == null || element_size == 0)
    {
        cl_log_error("Invalid allocator or element size provided to cl_da_init");
        return null;
    }

    cl_da_t *da = cl_mem_alloc(allocator, sizeof(cl_da_t));
    if (da == null)
    {
        cl_log_error("Failed to allocate memory for dynamic array");
        return null;
    }

    da->data = cl_mem_alloc(allocator, CL_DA_INITIAL_CAPACITY * element_size);
    if (da->data == null)
    {
        cl_log_error("Failed to allocate memory for dynamic array data");
        cl_mem_free(allocator, da);
        return null;
    }

    da->size = 0;
    da->capacity = CL_DA_INITIAL_CAPACITY;
    da->element_size = element_size;
    da->allocator = allocator;

    cl_log_debug("Dynamic array initialized. Capacity: %llu, Element size: %zu", da->capacity, da->element_size);
    return da;
}

void cl_da_destroy(cl_da_t *da)
{
    if (da)
    {
        if (da->data)
        {
            cl_mem_free(da->allocator, da->data);
        }
        cl_mem_free(da->allocator, da);
        cl_log_debug("Dynamic array destroyed");
    }
}

static bool cl_da_resize(cl_da_t *da, u64 new_capacity)
{
    cl_log_debug("Resizing dynamic array from %llu to %llu", da->capacity, new_capacity);

    void *new_data = cl_mem_alloc(da->allocator, new_capacity * da->element_size);
    if (new_data == null)
    {
        cl_log_error("Failed to allocate memory for resizing");
        return false;
    }

    memcpy(new_data, da->data, da->size * da->element_size);
    cl_mem_free(da->allocator, da->data);
    da->data = new_data;
    da->capacity = new_capacity;

    cl_log_debug("Resize complete. New capacity: %llu", da->capacity);
    return true;
}

bool cl_da_push(cl_da_t *da, const void *element)
{
    if (da == null || element == null)
    {
        cl_log_error("Null dynamic array or element provided to cl_da_push");
        return false;
    }

    if (da->size == da->capacity)
    {
        if (!cl_da_resize(da, da->capacity * CL_DA_GROWTH_FACTOR))
        {
            cl_log_error("Resize failed during push");
            return false;
        }
    }

    memcpy((char *)da->data + da->size * da->element_size, element, da->element_size);
    da->size++;

    cl_log_debug("Element pushed. New size: %llu", da->size);
    return true;
}

void *cl_da_get(const cl_da_t *da, u64 index)
{
    if (da == null || index >= da->size)
    {
        cl_log_error("Invalid dynamic array or index provided to cl_da_get");
        return null;
    }

    return (char *)da->data + index * da->element_size;
}

bool cl_da_set(cl_da_t *da, u64 index, const void *element)
{
    if (da == null || element == null || index >= da->size)
    {
        cl_log_error("Invalid dynamic array, element, or index provided to cl_da_set");
        return false;
    }

    memcpy((char *)da->data + index * da->element_size, element, da->element_size);
    cl_log_debug("Element set at index %llu", index);
    return true;
}

bool cl_da_remove(cl_da_t *da, u64 index)
{
    if (da == null || index >= da->size)
    {
        cl_log_error("Invalid dynamic array or index provided to cl_da_remove");
        return false;
    }

    if (index < da->size - 1)
    {
        memmove((char *)da->data + index * da->element_size, (char *)da->data + (index + 1) * da->element_size,
                (da->size - index - 1) * da->element_size);
    }

    da->size--;

    if ((double)da->size / da->capacity < CL_DA_SHRINK_FACTOR && da->capacity > CL_DA_INITIAL_CAPACITY)
    {
        cl_da_resize(da, da->capacity / CL_DA_GROWTH_FACTOR);
    }

    cl_log_debug("Element removed at index %llu. New size: %llu", index, da->size);
    return true;
}

u64 cl_da_size(const cl_da_t *da) { return (da != null) ? da->size : 0; }

bool cl_da_is_empty(const cl_da_t *da) { return (da != null) ? (da->size == 0) : true; }

void cl_da_clear(cl_da_t *da)
{
    if (da != null)
    {
        da->size = 0;
        cl_log_debug("Dynamic array cleared");
    }
}

void cl_da_foreach(const cl_da_t *da, void (*callback)(void *element, void *user_data), void *user_data)
{
    if (da != null && callback != null)
    {
        for (u64 i = 0; i < da->size; i++)
        {
            callback((char *)da->data + i * da->element_size, user_data);
        }
    }
}
