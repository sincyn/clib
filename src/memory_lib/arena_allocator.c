#include <stdlib.h>
#include <string.h>
#include "allocator_internal.h"
#include "clib/log_lib.h"
#include "clib/memory_lib.h"
#define ARENA_BLOCK_SIZE (64 * 1024) // 64 KB default block size

typedef struct arena_block
{
    void *memory;
    size_t size;
    size_t used;
    struct arena_block *next;
} arena_block_t;

typedef struct arena_allocator
{
    arena_block_t *current_block;
    size_t block_size;
} arena_allocator_t;

static void *arena_alloc(u64 size, void *user_data)
{
    arena_allocator_t *arena = (arena_allocator_t *)user_data;

    // Align size to 8 bytes
    size = (size + 7) & ~7;

    if (arena->current_block == null || arena->current_block->used + size > arena->current_block->size)
    {
        // Allocate a new block
        const u64 block_size = size > arena->block_size ? size : arena->block_size;
        arena_block_t *new_block = malloc(sizeof(arena_block_t));
        if (new_block == null)
            return null;

        new_block->memory = malloc(block_size);
        if (new_block->memory == null)
        {
            free(new_block);
            cl_log_warn("Failed to allocate memory for arena block");
            return null;
        }

        new_block->size = block_size;
        new_block->used = 0;
        new_block->next = arena->current_block;
        arena->current_block = new_block;
    }

    void *ptr = (char *)arena->current_block->memory + arena->current_block->used;
    arena->current_block->used += size;
    return ptr;
}

static void *arena_realloc(void *ptr, u64 new_size, void *user_data)
{
    // Arena allocator doesn't support reallocation, so we allocate new memory and copy
    arena_allocator_t *arena = (arena_allocator_t *)user_data;
    void *new_ptr = arena_alloc(new_size, arena);
    if (new_ptr && ptr)
    {
        memcpy(new_ptr, ptr, new_size); // This might copy more than necessary, but it's simple
    }
    return new_ptr;
}

static void arena_free(void *ptr, void *user_data)
{
    // Arena allocator doesn't free individual allocations
    (void)ptr;
    (void)user_data;
}

bool init_arena_allocator(cl_allocator_t *allocator, const cl_allocator_config_t *config)
{
    arena_allocator_t *arena = malloc(sizeof(arena_allocator_t));
    if (!arena)
        return false;
    if (config->config.arena.size < 0)
    {
        free(arena);
        return false;
    }

    arena->current_block = null;
    arena->block_size = config->config.arena.size > 0 ? config->config.arena.size : ARENA_BLOCK_SIZE;

    allocator->alloc = arena_alloc;
    allocator->realloc = arena_realloc;
    allocator->free = arena_free;
    allocator->type = CL_ALLOCATOR_TYPE_ARENA;
    allocator->flags = config->flags;
    allocator->user_data = arena;

    return true;
}

void deinit_arena_allocator(const cl_allocator_t *allocator)
{
    arena_allocator_t *arena = (arena_allocator_t *)allocator->user_data;
    arena_block_t *block = arena->current_block;

    while (block)
    {
        arena_block_t *next = block->next;
        free(block->memory);
        free(block);
        block = next;
    }

    free(arena);
}
