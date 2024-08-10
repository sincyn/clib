/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once
#include "clib/memory_lib.h"

bool init_platform_allocator(cl_allocator_t *allocator, const cl_allocator_config_t *config);
void deinit_platform_allocator(const cl_allocator_t *allocator);

bool init_arena_allocator(cl_allocator_t *allocator, const cl_allocator_config_t *config);
void deinit_arena_allocator(const cl_allocator_t *allocator);
