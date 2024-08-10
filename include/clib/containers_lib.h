/**
 * Containers Library Header
 * Updated by Claude on 8/6/2024.
 */

#pragma once

#include "defines.h"
#include "memory_lib.h"
#include "string_lib.h"

// Hash table
// table_lib.h

#pragma once

#include "defines.h"
#include "memory_lib.h"
#include "string_lib.h"
#include "thread_lib.h"

// Hash table structure
typedef struct cl_ht cl_ht_t;

// Hash function prototype
typedef u64 (*cl_ht_hash_func_t)(const void *key, u64 length);

// Free function prototype
typedef void (*cl_ht_free_func_t)(void *data);

// Remove function prototype
typedef bool (*cl_ht_remove_func_t)(void *key, u64 key_size, void *data, u64 data_size, void *arg);

// Foreach function prototype
typedef bool (*cl_ht_foreach_func_t)(void *key, u64 key_size, void *data, u64 data_size, void *arg);

// Hash table flags
typedef enum cl_ht_flags
{
    CL_HT_FLAG_NONE = 0,
    CL_HT_FLAG_NOCOPY_KEYS = 1 << 0,
    CL_HT_FLAG_NO_LOCKING = 1 << 1,
    CL_HT_FLAG_FROZEN = 1 << 2,
    CL_HT_FLAG_FROZEN_UNTIL_GROWS = 1 << 3,
    CL_HT_FLAG_FREE_DATA = 1 << 4,
    CL_HT_FLAG_IGNORE_CASE = 1 << 5
} cl_ht_flags_t;

// Hash table functions
cl_ht_t *cl_ht_create(const cl_allocator_t *allocator);
cl_ht_t *cl_ht_create_with_size(const cl_allocator_t *allocator, u64 size);
cl_ht_t *cl_ht_create_with_flags(const cl_allocator_t *allocator, cl_ht_flags_t flags);
cl_ht_t *cl_ht_create_with_free_func(const cl_allocator_t *allocator, cl_ht_free_func_t ff);

void cl_ht_destroy(cl_ht_t *ht);
void cl_ht_destroy_with_free_func(cl_ht_t *ht, cl_ht_free_func_t ff);

bool cl_ht_set_hash_function(cl_ht_t *ht, cl_ht_hash_func_t hf);
bool cl_ht_set_free_function(cl_ht_t *ht, cl_ht_free_func_t ff);

cl_ht_flags_t cl_ht_get_flags(cl_ht_t *ht);
cl_ht_flags_t cl_ht_set_flag(cl_ht_t *ht, cl_ht_flags_t flag);
cl_ht_flags_t cl_ht_clear_flag(cl_ht_t *ht, cl_ht_flags_t flag);

bool cl_ht_get(cl_ht_t *ht, const void *key, u64 key_size, void **data, u64 *data_size);
bool cl_ht_exists(cl_ht_t *ht, const void *key, u64 key_size);
bool cl_ht_put(cl_ht_t *ht, const void *key, u64 key_size, void *data, u64 data_size, void **old_data);
void cl_ht_clear(cl_ht_t *ht);
void *cl_ht_remove(cl_ht_t *ht, const void *key, u64 key_size);

void **cl_ht_keys(cl_ht_t *ht, u64 *num_keys, u64 **key_sizes, bool fast);
bool cl_ht_iter_init(cl_ht_t *ht, void **key, u64 *key_size, void **data, u64 *data_size);
bool cl_ht_iter_next(cl_ht_t *ht, void **key, u64 *key_size, void **data, u64 *data_size);

u64 cl_ht_foreach_remove(cl_ht_t *ht, cl_ht_remove_func_t r_fn, cl_ht_free_func_t ff, void *arg);
u64 cl_ht_foreach(cl_ht_t *ht, cl_ht_foreach_func_t fe_fn, void *arg);

bool cl_ht_rehash(cl_ht_t *ht);
u64 cl_ht_size(cl_ht_t *ht);
bool cl_ht_is_empty(cl_ht_t *ht);


bool cl_ht_lock(cl_ht_t *ht);
bool cl_ht_unlock(cl_ht_t *ht);

// String-specific functions
void *cl_ht_get_str(cl_ht_t *ht, const char *key);
bool cl_ht_exists_str(cl_ht_t *ht, const char *key);
void *cl_ht_put_str(cl_ht_t *ht, const char *key, void *data);
void *cl_ht_remove_str(cl_ht_t *ht, const char *key);
bool cl_ht_iter_init_str(cl_ht_t *ht, char **key, void **data);
bool cl_ht_iter_next_str(cl_ht_t *ht, char **key, void **data);
// Dynamic array
typedef struct cl_da cl_da_t;

cl_da_t *cl_da_init(cl_allocator_t *allocator, u64 element_size);
void cl_da_destroy(cl_da_t *da);
bool cl_da_push(cl_da_t *da, const void *element);
void *cl_da_get(const cl_da_t *da, u64 index);
bool cl_da_set(cl_da_t *da, u64 index, const void *element);
bool cl_da_remove(cl_da_t *da, u64 index);
u64 cl_da_size(const cl_da_t *da);
bool cl_da_is_empty(const cl_da_t *da);
void cl_da_clear(cl_da_t *da);
void cl_da_foreach(const cl_da_t *da, void (*callback)(void *element, void *user_data), void *user_data);

// Hash set
typedef struct cl_hs cl_hs_t;

cl_hs_t *cl_hs_init(cl_allocator_t *allocator);
void cl_hs_destroy(cl_hs_t *hs);
bool cl_hs_insert(cl_hs_t *hs, const str_view *element);
bool cl_hs_contains(const cl_hs_t *hs, const str_view *element);
bool cl_hs_remove(cl_hs_t *hs, const str_view *element);
u64 cl_hs_size(const cl_hs_t *hs);
bool cl_hs_is_empty(const cl_hs_t *hs);
void cl_hs_clear(cl_hs_t *hs);
void cl_hs_foreach(const cl_hs_t *hs, void (*callback)(const cl_hs_t *hs, const str_view *element, void *user_data),
                   void *user_data);
