/**
 * Optimized Hash Table Implementation
 * Created by Claude on 8/4/2024.
 */

#pragma once

#include "defines.h"
#include "memory_lib.h"

// Hash table handle
typedef struct cl_ht cl_ht_t;

// Create a new default hash table
cl_ht_t *cl_ht_init(cl_allocator_t *allocator);

// Destroy a hash table
void cl_ht_destroy(cl_ht_t *ht);

// Insert a key-value pair into the hash table
bool cl_ht_insert(cl_ht_t *ht, const void *key, void *value);

// Get a value from the hash table
void *cl_ht_get(const cl_ht_t *ht, const void *key);

// Remove a key-value pair from the hash table
bool cl_ht_remove(cl_ht_t *ht, const void *key);

// Get the number of elements in the hash table
u64 cl_ht_size(const cl_ht_t *ht);

// Check if the hash table is empty
bool cl_ht_is_empty(const cl_ht_t *ht);

// Clear all elements from the hash table
void cl_ht_clear(cl_ht_t *ht);

// Iterate over the hash table
void cl_ht_foreach(const cl_ht_t *ht, void (*callback)(const void *key, void *value, void *user_data), void *user_data);

typedef struct cl_da cl_da_t;
// Dynamic array functions
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


// Hash set handle
typedef struct cl_hs cl_hs_t;
// Hash set functions
cl_hs_t *cl_hs_init(cl_allocator_t *allocator);
void cl_hs_destroy(cl_hs_t *hs);
bool cl_hs_insert(cl_hs_t *hs, const void *element);
bool cl_hs_contains(const cl_hs_t *hs, const void *element);
bool cl_hs_remove(cl_hs_t *hs, const void *element);
u64 cl_hs_size(const cl_hs_t *hs);
bool cl_hs_is_empty(const cl_hs_t *hs);
void cl_hs_clear(cl_hs_t *hs);
void cl_hs_foreach(const cl_hs_t *hs, void (*callback)(const void *element, void *user_data), void *user_data);
