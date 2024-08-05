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
