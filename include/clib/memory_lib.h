/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once

#include "defines.h"

// Forward declarations
typedef struct cl_allocator cl_allocator_t;

// Memory alignment
#define CL_MEMORY_ALIGN(size, alignment) (((size) + ((alignment)-1)) & ~((alignment)-1))

// Allocator function types
typedef void *(*cl_alloc_func)(u64 size, void *user_data);
typedef void *(*cl_realloc_func)(void *ptr, u64 new_size, void *user_data);
typedef void (*cl_free_func)(void *ptr, void *user_data);

// Allocator structure
typedef enum cl_allocator_type
{
    CL_ALLOCATOR_TYPE_PLATFORM, // malloc, heapalloc, etc
    CL_ALLOCATOR_TYPE_ARENA, // arena allocator
    CL_ALLOCATOR_TYPE_POOL, // pool allocator (fixed-size blocks)
    CL_ALLOCATOR_TYPE_LINEAR, // linear allocator
    CL_ALLOCATOR_TYPE_STACK, // stack allocator
    CL_ALLOCATOR_TYPE_FREE_LIST, // free list allocator
    CL_ALLOCATOR_TYPE_PROXY, // proxy allocator
    CL_ALLOCATOR_TYPE_COUNT
} cl_allocator_type_t;

typedef enum cl_allocator_flags
{
    CL_ALLOCATOR_FLAG_NONE = 0,
    CL_ALLOCATOR_FLAG_CLEAR = 1 << 0, // clear memory to zero
    CL_ALLOCATOR_FLAG_ALIGN = 1 << 1, // align memory to a specific boundary
    CL_ALLOCATOR_FLAG_COUNT
} cl_allocator_flags_t;

typedef struct cl_allocator_config
{
    cl_allocator_type_t type;
    cl_allocator_flags_t flags;
    union
    {
        struct
        {
            u64 block_size;
            u64 block_count;
        } pool;
        struct
        {
            u64 size;
            void *start;
            void *current;
            void *end;
        } linear;
        struct
        {
            u64 size;
            void *start;
            void *current;
            void *end;
        } stack;
        struct
        {
            u64 block_size;
            u64 block_count;
            void *free_list;
        } free_list;
        struct
        {
            u64 size;
        } arena;
        struct
        {
            cl_allocator_t *allocator;
        } proxy;
    } config;
    void *user_data;
} cl_allocator_config_t;


struct cl_allocator
{
    cl_allocator_type_t type;
    cl_allocator_flags_t flags;
    cl_alloc_func alloc;
    cl_realloc_func realloc;
    cl_free_func free;
    void *user_data;
};

// Memory system initialization

// Allocator management
cl_allocator_t *cl_allocator_create(const cl_allocator_config_t *config);
#define cl_allocator_new(alloc_type, ...) cl_allocator_create(&(cl_allocator_config_t){.type = alloc_type, __VA_ARGS__})
void cl_allocator_destroy(cl_allocator_t *allocator);

// General memory functions
void *cl_mem_alloc(const cl_allocator_t *allocator, u64 size);
void *cl_mem_realloc(const cl_allocator_t *allocator, void *ptr, u64 new_size);
void cl_mem_free(const cl_allocator_t *allocator, void *ptr);
void *cl_mem_aligned_alloc(cl_allocator_t *allocator, u64 alignment, u64 size);
void cl_mem_aligned_free(const cl_allocator_t *allocator, void *ptr);

// Utility functions
void cl_mem_set(void *ptr, int value, u64 num);
void cl_mem_copy(void *dest, const void *src, u64 num);
void cl_mem_move(void *dest, const void *src, u64 num);
int cl_mem_compare(const void *ptr1, const void *ptr2, u64 num);

#ifdef __cplusplus
}
#endif
