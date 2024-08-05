/**
 * Optimized Hash Table Implementation
 * Created by Claude on 8/4/2024.
 * Optimized on 8/4/2024.
 */

#include "clib/containers_lib.h"
#include <stdint.h>
#include <string.h>
#include "clib/log_lib.h"

#define CL_HT_INITIAL_CAPACITY 64
#define CL_HT_LOAD_FACTOR 0.75
#define CL_HT_GROWTH_FACTOR 2
#define CL_HT_SHRINK_FACTOR 0.25

// FNV-1a hash function constants
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct
{
    const void *key;
    void *value;
    uint64_t hash;
    bool is_deleted;
} cl_ht_entry_t;

struct cl_ht
{
    cl_ht_entry_t *entries;
    u64 capacity;
    u64 size;
    u64 deleted_count;
    u64 mask;
    cl_allocator_t *allocator;
};

static uint64_t hash_key(const void *key)
{
    const uint8_t *data = (const uint8_t *)key;
    uint64_t hash = FNV_OFFSET;
    size_t len = strlen((const char *)key);

    for (size_t i = 0; i < len; i++)
    {
        hash ^= (uint64_t)data[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

static u64 find_slot(const cl_ht_t *ht, const void *key, uint64_t hash)
{
    u64 index = hash & ht->mask;
    u64 step = 0;

    while (ht->entries[index].key != NULL &&
           (ht->entries[index].hash != hash || strcmp(ht->entries[index].key, key) != 0))
    {
        step++;
        index = (index + step) & ht->mask; // Linear probing
    }

    return index;
}

static bool should_resize(cl_ht_t *ht)
{
    return (double)(ht->size + ht->deleted_count) / ht->capacity > CL_HT_LOAD_FACTOR;
}

static bool resize(cl_ht_t *ht, u64 new_capacity)
{
    CL_LOG_DEBUG("Resizing hash table from %llu to %llu", ht->capacity, new_capacity);

    cl_ht_entry_t *new_entries = cl_mem_alloc(ht->allocator, new_capacity * sizeof(cl_ht_entry_t));
    if (new_entries == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for resizing");
        return false;
    }

    memset(new_entries, 0, new_capacity * sizeof(cl_ht_entry_t));

    u64 new_mask = new_capacity - 1;
    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key != NULL && !ht->entries[i].is_deleted)
        {
            u64 index = ht->entries[i].hash & new_mask;
            u64 step = 0;
            while (new_entries[index].key != NULL)
            {
                step++;
                index = (index + step) & new_mask;
            }
            new_entries[index] = ht->entries[i];
        }
    }

    cl_mem_free(ht->allocator, ht->entries);
    ht->entries = new_entries;
    ht->capacity = new_capacity;
    ht->mask = new_mask;
    ht->deleted_count = 0;

    CL_LOG_DEBUG("Resize complete. New capacity: %llu, New mask: %llx", ht->capacity, ht->mask);
    return true;
}

cl_ht_t *cl_ht_init(cl_allocator_t *allocator)
{
    if (allocator == NULL)
    {
        CL_LOG_ERROR("Null allocator provided to cl_ht_init");
        return NULL;
    }

    cl_ht_t *ht = cl_mem_alloc(allocator, sizeof(cl_ht_t));
    if (ht == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for hash table");
        return NULL;
    }

    ht->capacity = CL_HT_INITIAL_CAPACITY;
    ht->size = 0;
    ht->deleted_count = 0;
    ht->mask = ht->capacity - 1;
    ht->allocator = allocator;

    ht->entries = cl_mem_alloc(allocator, ht->capacity * sizeof(cl_ht_entry_t));
    if (ht->entries == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for hash table entries");
        cl_mem_free(allocator, ht);
        return NULL;
    }

    memset(ht->entries, 0, ht->capacity * sizeof(cl_ht_entry_t));

    CL_LOG_DEBUG("Hash table initialized. Capacity: %llu, Mask: %llx", ht->capacity, ht->mask);
    return ht;
}

void cl_ht_destroy(cl_ht_t *ht)
{
    if (ht)
    {
        if (ht->entries)
        {
            cl_mem_free(ht->allocator, ht->entries);
        }
        cl_mem_free(ht->allocator, ht);
        CL_LOG_DEBUG("Hash table destroyed");
    }
}

bool cl_ht_insert(cl_ht_t *ht, const void *key, void *value)
{
    if (ht == NULL || key == NULL)
    {
        CL_LOG_ERROR("Null hash table or key provided to cl_ht_insert");
        return false;
    }

    CL_LOG_DEBUG("Inserting key: %s, Current size: %llu, Capacity: %llu", (const char *)key, ht->size, ht->capacity);

    if (should_resize(ht))
    {
        if (!resize(ht, ht->capacity * CL_HT_GROWTH_FACTOR))
        {
            CL_LOG_ERROR("Resize failed during insert");
            return false;
        }
    }

    uint64_t hash = hash_key(key);
    u64 index = find_slot(ht, key, hash);

    if (ht->entries[index].key == NULL || ht->entries[index].is_deleted)
    {
        ht->entries[index].key = key;
        ht->entries[index].value = value;
        ht->entries[index].hash = hash;
        ht->entries[index].is_deleted = false;
        ht->size++;
        if (ht->entries[index].is_deleted)
        {
            ht->deleted_count--;
        }
        CL_LOG_DEBUG("Key inserted at index %llu, New size: %llu", index, ht->size);
        return true;
    }
    else
    {
        // Key already exists, update the value
        ht->entries[index].value = value;
        CL_LOG_DEBUG("Key updated at index %llu", index);
        return true;
    }
}

void *cl_ht_get(const cl_ht_t *ht, const void *key)
{
    if (ht == NULL || key == NULL)
    {
        CL_LOG_ERROR("Null hash table or key provided to cl_ht_get");
        return NULL;
    }

    uint64_t hash = hash_key(key);
    u64 index = find_slot(ht, key, hash);

    if (ht->entries[index].key != NULL && !ht->entries[index].is_deleted)
    {
        CL_LOG_DEBUG("Key found at index %llu", index);
        return ht->entries[index].value;
    }
    else
    {
        CL_LOG_DEBUG("Key not found: %s", (const char *)key);
        return NULL;
    }
}

bool cl_ht_remove(cl_ht_t *ht, const void *key)
{
    if (ht == NULL || key == NULL)
    {
        CL_LOG_ERROR("Null hash table or key provided to cl_ht_remove");
        return false;
    }

    uint64_t hash = hash_key(key);
    u64 index = find_slot(ht, key, hash);

    if (ht->entries[index].key == NULL || ht->entries[index].is_deleted)
    {
        CL_LOG_DEBUG("Key not found for removal: %s", (const char *)key);
        return false;
    }

    // Mark the slot as deleted
    ht->entries[index].is_deleted = true;
    ht->size--;
    ht->deleted_count++;

    CL_LOG_DEBUG("Key removed: %s, New size: %llu, Deleted count: %llu", (const char *)key, ht->size,
                 ht->deleted_count);

    // Check if we need to resize (shrink) the hash table
    if ((double)ht->size / ht->capacity < CL_HT_SHRINK_FACTOR && ht->capacity > CL_HT_INITIAL_CAPACITY)
    {
        resize(ht, ht->capacity / CL_HT_GROWTH_FACTOR);
    }

    return true;
}

u64 cl_ht_size(const cl_ht_t *ht) { return (ht != NULL) ? ht->size : 0; }

bool cl_ht_is_empty(const cl_ht_t *ht) { return (ht != NULL) ? (ht->size == 0) : true; }

void cl_ht_clear(cl_ht_t *ht)
{
    if (ht != NULL && ht->entries != NULL)
    {
        memset(ht->entries, 0, ht->capacity * sizeof(cl_ht_entry_t));
        ht->size = 0;
        ht->deleted_count = 0;
        CL_LOG_DEBUG("Hash table cleared");
    }
}

void cl_ht_foreach(const cl_ht_t *ht, void (*callback)(const void *key, void *value, void *user_data), void *user_data)
{
    if (ht != NULL && callback != NULL)
    {
        for (u64 i = 0; i < ht->capacity; i++)
        {
            if (ht->entries[i].key != NULL && !ht->entries[i].is_deleted)
            {
                callback(ht->entries[i].key, ht->entries[i].value, user_data);
            }
        }
    }
}


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
    if (allocator == NULL || element_size == 0)
    {
        CL_LOG_ERROR("Invalid allocator or element size provided to cl_da_init");
        return NULL;
    }

    cl_da_t *da = cl_mem_alloc(allocator, sizeof(cl_da_t));
    if (da == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for dynamic array");
        return NULL;
    }

    da->data = cl_mem_alloc(allocator, CL_DA_INITIAL_CAPACITY * element_size);
    if (da->data == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for dynamic array data");
        cl_mem_free(allocator, da);
        return NULL;
    }

    da->size = 0;
    da->capacity = CL_DA_INITIAL_CAPACITY;
    da->element_size = element_size;
    da->allocator = allocator;

    CL_LOG_DEBUG("Dynamic array initialized. Capacity: %llu, Element size: %zu", da->capacity, da->element_size);
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
        CL_LOG_DEBUG("Dynamic array destroyed");
    }
}

static bool cl_da_resize(cl_da_t *da, u64 new_capacity)
{
    CL_LOG_DEBUG("Resizing dynamic array from %llu to %llu", da->capacity, new_capacity);

    void *new_data = cl_mem_alloc(da->allocator, new_capacity * da->element_size);
    if (new_data == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for resizing");
        return false;
    }

    memcpy(new_data, da->data, da->size * da->element_size);
    cl_mem_free(da->allocator, da->data);
    da->data = new_data;
    da->capacity = new_capacity;

    CL_LOG_DEBUG("Resize complete. New capacity: %llu", da->capacity);
    return true;
}

bool cl_da_push(cl_da_t *da, const void *element)
{
    if (da == NULL || element == NULL)
    {
        CL_LOG_ERROR("Null dynamic array or element provided to cl_da_push");
        return false;
    }

    if (da->size == da->capacity)
    {
        if (!cl_da_resize(da, da->capacity * CL_DA_GROWTH_FACTOR))
        {
            CL_LOG_ERROR("Resize failed during push");
            return false;
        }
    }

    memcpy((char *)da->data + da->size * da->element_size, element, da->element_size);
    da->size++;

    CL_LOG_DEBUG("Element pushed. New size: %llu", da->size);
    return true;
}

void *cl_da_get(const cl_da_t *da, u64 index)
{
    if (da == NULL || index >= da->size)
    {
        CL_LOG_ERROR("Invalid dynamic array or index provided to cl_da_get");
        return NULL;
    }

    return (char *)da->data + index * da->element_size;
}

bool cl_da_set(cl_da_t *da, u64 index, const void *element)
{
    if (da == NULL || element == NULL || index >= da->size)
    {
        CL_LOG_ERROR("Invalid dynamic array, element, or index provided to cl_da_set");
        return false;
    }

    memcpy((char *)da->data + index * da->element_size, element, da->element_size);
    CL_LOG_DEBUG("Element set at index %llu", index);
    return true;
}

bool cl_da_remove(cl_da_t *da, u64 index)
{
    if (da == NULL || index >= da->size)
    {
        CL_LOG_ERROR("Invalid dynamic array or index provided to cl_da_remove");
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

    CL_LOG_DEBUG("Element removed at index %llu. New size: %llu", index, da->size);
    return true;
}

u64 cl_da_size(const cl_da_t *da) { return (da != NULL) ? da->size : 0; }

bool cl_da_is_empty(const cl_da_t *da) { return (da != NULL) ? (da->size == 0) : true; }

void cl_da_clear(cl_da_t *da)
{
    if (da != NULL)
    {
        da->size = 0;
        CL_LOG_DEBUG("Dynamic array cleared");
    }
}

void cl_da_foreach(const cl_da_t *da, void (*callback)(void *element, void *user_data), void *user_data)
{
    if (da != NULL && callback != NULL)
    {
        for (u64 i = 0; i < da->size; i++)
        {
            callback((char *)da->data + i * da->element_size, user_data);
        }
    }
}


struct cl_hs
{
    cl_ht_t *table;
};

cl_hs_t *cl_hs_init(cl_allocator_t *allocator)
{
    if (allocator == NULL)
    {
        CL_LOG_ERROR("Null allocator provided to cl_hs_init");
        return NULL;
    }

    cl_hs_t *hs = cl_mem_alloc(allocator, sizeof(cl_hs_t));
    if (hs == NULL)
    {
        CL_LOG_ERROR("Failed to allocate memory for hash set");
        return NULL;
    }

    hs->table = cl_ht_init(allocator);
    if (hs->table == NULL)
    {
        CL_LOG_ERROR("Failed to initialize hash table for hash set");
        cl_mem_free(allocator, hs);
        return NULL;
    }

    CL_LOG_DEBUG("Hash set initialized");
    return hs;
}

void cl_hs_destroy(cl_hs_t *hs)
{
    if (hs)
    {
        cl_ht_destroy(hs->table);
        cl_mem_free(hs->table->allocator, hs);
        CL_LOG_DEBUG("Hash set destroyed");
    }
}

bool cl_hs_insert(cl_hs_t *hs, const void *element)
{
    if (hs == NULL || element == NULL)
    {
        CL_LOG_ERROR("Null hash set or element provided to cl_hs_insert");
        return false;
    }

    // We use the element as both key and value in the underlying hash table
    return cl_ht_insert(hs->table, element, (void *)element);
}

bool cl_hs_contains(const cl_hs_t *hs, const void *element)
{
    if (hs == NULL || element == NULL)
    {
        CL_LOG_ERROR("Null hash set or element provided to cl_hs_contains");
        return false;
    }

    return cl_ht_get(hs->table, element) != NULL;
}

bool cl_hs_remove(cl_hs_t *hs, const void *element)
{
    if (hs == NULL || element == NULL)
    {
        CL_LOG_ERROR("Null hash set or element provided to cl_hs_remove");
        return false;
    }

    return cl_ht_remove(hs->table, element);
}

u64 cl_hs_size(const cl_hs_t *hs) { return (hs != NULL) ? cl_ht_size(hs->table) : 0; }

bool cl_hs_is_empty(const cl_hs_t *hs) { return (hs != NULL) ? cl_ht_is_empty(hs->table) : true; }

void cl_hs_clear(cl_hs_t *hs)
{
    if (hs != NULL)
    {
        cl_ht_clear(hs->table);
        CL_LOG_DEBUG("Hash set cleared");
    }
}

static void hs_foreach_wrapper(const void *key, void *value, void *user_data)
{
    void (*callback)(const void *element, void *user_data) = ((void **)user_data)[0];
    void *original_user_data = ((void **)user_data)[1];
    callback(key, original_user_data);
}

void cl_hs_foreach(const cl_hs_t *hs, void (*callback)(const void *element, void *user_data), void *user_data)
{
    if (hs != NULL && callback != NULL)
    {
        void *wrapper_data[2] = {(void *)callback, user_data};
        cl_ht_foreach(hs->table, hs_foreach_wrapper, wrapper_data);
    }
}
