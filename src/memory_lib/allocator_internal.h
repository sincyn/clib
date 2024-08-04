/**
 * Created by jraynor on 8/3/2024.
 */
#pragma once

bool init_platform_allocator(cl_allocator_t *allocator, const cl_allocator_config_t *config);
void deinit_platform_allocator(const cl_allocator_t *allocator);
