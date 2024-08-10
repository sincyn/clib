// table_lib.c

#include <string.h>
#include "clib/containers_lib.h"

#define CL_HT_INITIAL_SIZE 16
#define CL_HT_LOAD_FACTOR_LOW 0.25f
#define CL_HT_LOAD_FACTOR_HIGH 0.75f
#define CL_HT_INITIAL_SIZE 16
#define CL_HT_LOAD_FACTOR 0.75f

typedef struct cl_ht_entry
{
    u64 hash;
    u64 key_size;
    void *key;
    void *data;
    u64 data_size;
} cl_ht_entry_t;

struct cl_ht
{
    cl_ht_entry_t *entries;
    u64 capacity;
    u64 size;
    u64 mask; // For fast modulo operation
    cl_ht_flags_t flags;
    cl_ht_hash_func_t hash_func;
    cl_ht_free_func_t free_func;
    cl_mutex_t *mutex;
    const cl_allocator_t *allocator;
};

static inline u64 cl_ht_default_hash(const void *input, u64 length)
{
    const u64 PRIME64_1 = 11400714785074694791ULL;
    const u64 PRIME64_2 = 14029467366897019727ULL;
    const u64 PRIME64_3 = 1609587929392839161ULL;
    const u64 PRIME64_4 = 9650029242287828579ULL;
    const u64 PRIME64_5 = 2870177450012600261ULL;

    const u8 *p = (const u8 *)input;
    const u8 *end = p + length;
    u64 h64;

    if (length >= 32)
    {
        const u8 *limit = end - 32;
        u64 v1 = PRIME64_1 + PRIME64_2;
        u64 v2 = PRIME64_2;
        u64 v3 = 0;
        u64 v4 = -PRIME64_1;

        do
        {
            v1 += (*(u64 *)p) * PRIME64_2;
            v1 = (v1 << 31) | (v1 >> 33);
            v1 *= PRIME64_1;
            p += 8;

            v2 += (*(u64 *)p) * PRIME64_2;
            v2 = (v2 << 31) | (v2 >> 33);
            v2 *= PRIME64_1;
            p += 8;

            v3 += (*(u64 *)p) * PRIME64_2;
            v3 = (v3 << 31) | (v3 >> 33);
            v3 *= PRIME64_1;
            p += 8;

            v4 += (*(u64 *)p) * PRIME64_2;
            v4 = (v4 << 31) | (v4 >> 33);
            v4 *= PRIME64_1;
            p += 8;
        }
        while (p <= limit);

        h64 =
            ((v1 << 1) | (v1 >> 63)) + ((v2 << 7) | (v2 >> 57)) + ((v3 << 12) | (v3 >> 52)) + ((v4 << 18) | (v4 >> 46));

        v1 *= PRIME64_2;
        v1 = (v1 << 31) | (v1 >> 33);
        v1 *= PRIME64_1;
        h64 ^= v1;
        h64 = h64 * PRIME64_1 + PRIME64_4;

        v2 *= PRIME64_2;
        v2 = (v2 << 31) | (v2 >> 33);
        v2 *= PRIME64_1;
        h64 ^= v2;
        h64 = h64 * PRIME64_1 + PRIME64_4;

        v3 *= PRIME64_2;
        v3 = (v3 << 31) | (v3 >> 33);
        v3 *= PRIME64_1;
        h64 ^= v3;
        h64 = h64 * PRIME64_1 + PRIME64_4;

        v4 *= PRIME64_2;
        v4 = (v4 << 31) | (v4 >> 33);
        v4 *= PRIME64_1;
        h64 ^= v4;
        h64 = h64 * PRIME64_1 + PRIME64_4;
    }
    else
    {
        h64 = PRIME64_5;
    }

    h64 += (u64)length;

    while (p + 8 <= end)
    {
        u64 k1 = *(u64 *)p;
        k1 *= PRIME64_2;
        k1 = (k1 << 31) | (k1 >> 33);
        k1 *= PRIME64_1;
        h64 ^= k1;
        h64 = ((h64 << 27) | (h64 >> 37)) * PRIME64_1 + PRIME64_4;
        p += 8;
    }

    if (p + 4 <= end)
    {
        h64 ^= (u64)(*(u32 *)p) * PRIME64_1;
        h64 = ((h64 << 23) | (h64 >> 41)) * PRIME64_2 + PRIME64_3;
        p += 4;
    }

    while (p < end)
    {
        h64 ^= (*p) * PRIME64_5;
        h64 = ((h64 << 11) | (h64 >> 53)) * PRIME64_1;
        p++;
    }

    h64 ^= h64 >> 33;
    h64 *= PRIME64_2;
    h64 ^= h64 >> 29;
    h64 *= PRIME64_3;
    h64 ^= h64 >> 32;

    return h64;
}

static inline u64 cl_ht_probe_distance(const cl_ht_t *ht, u64 hash, u64 slot_index)
{
    return (slot_index + ht->capacity - (hash & ht->mask)) & ht->mask;
}

static cl_ht_t *cl_ht_create_internal(const cl_allocator_t *allocator, u64 size, cl_ht_flags_t flags)
{
    cl_ht_t *ht = cl_mem_alloc(allocator, sizeof(cl_ht_t));
    if (!ht)
        return null;

    ht->capacity = size > 0 ? size : CL_HT_INITIAL_SIZE;
    // Ensure capacity is a power of 2
    ht->capacity = 1ULL << (64 - __builtin_clzll(ht->capacity - 1));
    ht->entries = cl_mem_alloc(allocator, ht->capacity * sizeof(cl_ht_entry_t));
    if (!ht->entries)
    {
        cl_mem_free(allocator, ht);
        return null;
    }
    memset(ht->entries, 0, ht->capacity * sizeof(cl_ht_entry_t));

    ht->size = 0;
    ht->mask = ht->capacity - 1;
    ht->flags = flags;
    ht->hash_func = cl_ht_default_hash;
    ht->free_func = null;
    ht->allocator = allocator;

    if (!(flags & CL_HT_FLAG_NO_LOCKING))
    {
        ht->mutex = cl_mutex_create();
        if (!ht->mutex)
        {
            cl_mem_free(allocator, ht->entries);
            cl_mem_free(allocator, ht);
            return null;
        }
    }
    else
    {
        ht->mutex = null;
    }

    return ht;
}

cl_ht_t *cl_ht_create(const cl_allocator_t *allocator) { return cl_ht_create_internal(allocator, 0, CL_HT_FLAG_NONE); }
cl_ht_t *cl_ht_create_with_size(const cl_allocator_t *allocator, u64 size)
{
    return cl_ht_create_internal(allocator, size, CL_HT_FLAG_NONE);
}

cl_ht_t *cl_ht_create_with_flags(const cl_allocator_t *allocator, cl_ht_flags_t flags)
{
    return cl_ht_create_internal(allocator, 0, flags);
}

cl_ht_t *cl_ht_create_with_free_func(const cl_allocator_t *allocator, cl_ht_free_func_t ff)
{
    cl_ht_t *ht = cl_ht_create_internal(allocator, 0, CL_HT_FLAG_NONE);
    if (ht)
    {
        ht->free_func = ff;
    }
    return ht;
}

void cl_ht_destroy(cl_ht_t *ht)
{
    if (!ht)
        return;
    cl_ht_clear(ht);
    if (ht->mutex)
    {
        cl_mutex_destroy(ht->mutex);
    }
    cl_mem_free(ht->allocator, ht->entries);
    cl_mem_free(ht->allocator, ht);
}

void cl_ht_destroy_with_free_func(cl_ht_t *ht, cl_ht_free_func_t ff)
{
    cl_ht_free_func_t old_ff = ht->free_func;
    ht->free_func = ff;
    cl_ht_destroy(ht);
    ht->free_func = old_ff;
}

bool cl_ht_set_hash_function(cl_ht_t *ht, cl_ht_hash_func_t hf)
{
    if (!ht || !hf)
        return false;
    ht->hash_func = hf;
    return true;
}


bool cl_ht_set_free_function(cl_ht_t *ht, cl_ht_free_func_t ff)
{
    if (!ht)
        return false;
    ht->free_func = ff;
    return true;
}

cl_ht_flags_t cl_ht_get_flags(cl_ht_t *ht) { return ht ? ht->flags : CL_HT_FLAG_NONE; }

cl_ht_flags_t cl_ht_set_flag(cl_ht_t *ht, cl_ht_flags_t flag)
{
    if (!ht)
        return CL_HT_FLAG_NONE;
    ht->flags |= flag;
    return ht->flags;
}

cl_ht_flags_t cl_ht_clear_flag(cl_ht_t *ht, cl_ht_flags_t flag)
{
    if (!ht)
        return CL_HT_FLAG_NONE;
    ht->flags &= ~flag;
    return ht->flags;
}


static bool cl_ht_keys_equal(const cl_ht_t *ht, const void *key1, u64 key1_size, const void *key2, u64 key2_size)
{
    if (key1_size != key2_size)
        return false;

    if (ht->flags & CL_HT_FLAG_IGNORE_CASE)
    {
        return strncasecmp(key1, key2, key1_size) == 0;
    }
    else
    {
        return memcmp(key1, key2, key1_size) == 0;
    }
}


bool cl_ht_get(cl_ht_t *ht, const void *key, u64 key_size, void **data, u64 *data_size)
{
    if (!ht || !key)
        return false;

    u64 hash = ht->hash_func(key, key_size);
    u64 index = hash & ht->mask;
    u64 dist = 0;

    while (true)
    {
        if (!ht->entries[index].key)
        {
            return false; // Key not found
        }

        if (ht->entries[index].hash == hash &&
            cl_ht_keys_equal(ht, ht->entries[index].key, ht->entries[index].key_size, key, key_size))
        {
            if (data)
                *data = ht->entries[index].data;
            if (data_size)
                *data_size = ht->entries[index].data_size;
            return true;
        }

        u64 probe_dist = cl_ht_probe_distance(ht, ht->entries[index].hash, index);
        if (probe_dist < dist)
        {
            return false; // Key not found
        }

        index = (index + 1) & ht->mask;
        dist++;
    }
}


bool cl_ht_exists(cl_ht_t *ht, const void *key, u64 key_size)
{
    void *data;
    u64 data_size;
    return cl_ht_get(ht, key, key_size, &data, &data_size);
}

static bool cl_ht_resize(cl_ht_t *ht, u64 new_capacity)
{
    cl_ht_entry_t *new_entries = cl_mem_alloc(ht->allocator, new_capacity * sizeof(cl_ht_entry_t));
    if (!new_entries)
        return false;

    memset(new_entries, 0, new_capacity * sizeof(cl_ht_entry_t));

    u64 new_mask = new_capacity - 1;
    for (u64 i = 0; i < ht->capacity; i++)
    {
        cl_ht_entry_t *entry = &ht->entries[i];
        if (entry->key)
        {
            u64 index = entry->hash & new_mask;
            while (new_entries[index].key)
            {
                index = (index + 1) & new_mask;
            }
            new_entries[index] = *entry;
        }
    }

    cl_mem_free(ht->allocator, ht->entries);
    ht->entries = new_entries;
    ht->capacity = new_capacity;
    ht->mask = new_mask;

    return true;
}

bool cl_ht_put(cl_ht_t *ht, const void *key, u64 key_size, void *data, u64 data_size, void **old_data)
{
    if (!ht || !key)
        return false;

    if ((float)ht->size / ht->capacity > CL_HT_LOAD_FACTOR)
    {
        if (!(ht->flags & CL_HT_FLAG_FROZEN) && !cl_ht_resize(ht, ht->capacity * 2))
        {
            return false;
        }
    }

    u64 hash = ht->hash_func(key, key_size);
    u64 index = hash & ht->mask;
    u64 dist = 0;

    while (true)
    {
        if (!ht->entries[index].key)
        {
            // Insert new entry
            void *new_key;
            if (!(ht->flags & CL_HT_FLAG_NOCOPY_KEYS))
            {
                new_key = cl_mem_alloc(ht->allocator, key_size);
                if (!new_key)
                    return false;
                cl_mem_copy(new_key, key, key_size);
            }
            else
            {
                new_key = (void *)key;
            }

            ht->entries[index].key = new_key;
            ht->entries[index].key_size = key_size;
            ht->entries[index].data = data;
            ht->entries[index].data_size = data_size;
            ht->entries[index].hash = hash;
            ht->size++;

            if (old_data)
                *old_data = null;
            return true;
        }

        if (ht->entries[index].hash == hash &&
            cl_ht_keys_equal(ht, ht->entries[index].key, ht->entries[index].key_size, key, key_size))
        {
            // Update existing entry
            if (old_data)
                *old_data = ht->entries[index].data;
            if (ht->free_func && !(ht->flags & CL_HT_FLAG_FREE_DATA))
            {
                ht->free_func(ht->entries[index].data);
            }
            ht->entries[index].data = data;
            ht->entries[index].data_size = data_size;
            return true;
        }

        u64 probe_dist = cl_ht_probe_distance(ht, ht->entries[index].hash, index);
        if (probe_dist < dist)
        {
            // Robin Hood hashing: swap with the current entry
            cl_ht_entry_t temp = ht->entries[index];

            // Allocate new key for the entry being displaced
            void *new_key;
            if (!(ht->flags & CL_HT_FLAG_NOCOPY_KEYS))
            {
                new_key = cl_mem_alloc(ht->allocator, key_size);
                if (!new_key)
                    return false;
                cl_mem_copy(new_key, key, key_size);
            }
            else
            {
                new_key = (void *)key;
            }

            ht->entries[index] = (cl_ht_entry_t){hash, key_size, new_key, data, data_size};

            key = temp.key;
            key_size = temp.key_size;
            data = temp.data;
            data_size = temp.data_size;
            hash = temp.hash;
            dist = probe_dist;
        }

        index = (index + 1) & ht->mask;
        dist++;
    }
}


void cl_ht_clear(cl_ht_t *ht)
{
    if (!ht)
        return;

    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            if (!(ht->flags & CL_HT_FLAG_NOCOPY_KEYS))
            {
                cl_mem_free(ht->allocator, ht->entries[i].key);
            }
            if (ht->free_func && !(ht->flags & CL_HT_FLAG_FREE_DATA))
            {
                ht->free_func(ht->entries[i].data);
            }
            ht->entries[i].key = null;
            ht->entries[i].data = null;
        }
    }
    ht->size = 0;
}


void **cl_ht_values(cl_ht_t *ht, u64 *num_values, u64 **value_sizes)
{
    if (!ht || !num_values)
        return null;

    void **values = cl_mem_alloc(ht->allocator, ht->size * sizeof(void *));
    if (!values)
        return null;

    if (value_sizes)
    {
        *value_sizes = cl_mem_alloc(ht->allocator, ht->size * sizeof(u64));
        if (!*value_sizes)
        {
            cl_mem_free(ht->allocator, values);
            return null;
        }
    }

    u64 index = 0;
    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            values[index] = ht->entries[i].data;
            if (value_sizes)
            {
                (*value_sizes)[index] = ht->entries[i].data_size;
            }
            index++;
        }
    }

    *num_values = ht->size;
    return values;
}
void *cl_ht_remove(cl_ht_t *ht, const void *key, u64 key_size)
{
    if (!ht || !key)
        return null;

    u64 hash = ht->hash_func(key, key_size);
    u64 index = hash & ht->mask;
    u64 dist = 0;

    while (true)
    {
        if (!ht->entries[index].key)
        {
            return null; // Key not found
        }

        if (ht->entries[index].hash == hash &&
            cl_ht_keys_equal(ht, ht->entries[index].key, ht->entries[index].key_size, key, key_size))
        {
            void *data = ht->entries[index].data;

            if (!(ht->flags & CL_HT_FLAG_NOCOPY_KEYS))
            {
                cl_mem_free(ht->allocator, ht->entries[index].key);
            }

            // Backward-shift deletion
            u64 next_index = (index + 1) & ht->mask;
            while (true)
            {
                if (!ht->entries[next_index].key ||
                    cl_ht_probe_distance(ht, ht->entries[next_index].hash, next_index) == 0)
                {
                    break;
                }
                ht->entries[index] = ht->entries[next_index];
                index = next_index;
                next_index = (next_index + 1) & ht->mask;
            }

            memset(&ht->entries[index], 0, sizeof(cl_ht_entry_t));
            ht->size--;

            return data;
        }

        u64 probe_dist = cl_ht_probe_distance(ht, ht->entries[index].hash, index);
        if (probe_dist < dist)
        {
            return null; // Key not found
        }

        index = (index + 1) & ht->mask;
        dist++;
    }
}
void **cl_ht_keys(cl_ht_t *ht, u64 *num_keys, u64 **key_sizes, bool fast)
{
    if (!ht || !num_keys)
        return null;

    void **keys = cl_mem_alloc(ht->allocator, ht->size * sizeof(void *));
    if (!keys)
        return null;

    if (key_sizes)
    {
        *key_sizes = cl_mem_alloc(ht->allocator, ht->size * sizeof(u64));
        if (!*key_sizes)
        {
            cl_mem_free(ht->allocator, keys);
            return null;
        }
    }

    u64 index = 0;
    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            if (fast)
            {
                keys[index] = ht->entries[i].key;
            }
            else
            {
                keys[index] = cl_mem_alloc(ht->allocator, ht->entries[i].key_size);
                if (keys[index])
                {
                    cl_mem_copy(keys[index], ht->entries[i].key, ht->entries[i].key_size);
                }
            }
            if (key_sizes)
            {
                (*key_sizes)[index] = ht->entries[i].key_size;
            }
            index++;
        }
    }

    *num_keys = ht->size;
    return keys;
}

bool cl_ht_iter_init(cl_ht_t *ht, void **key, u64 *key_size, void **data, u64 *data_size)
{
    if (!ht)
        return false;

    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            *key = ht->entries[i].key;
            *key_size = ht->entries[i].key_size;
            *data = ht->entries[i].data;
            *data_size = ht->entries[i].data_size;
            return true;
        }
    }

    return false;
}


bool cl_ht_iter_next(cl_ht_t *ht, void **key, u64 *key_size, void **data, u64 *data_size)
{
    if (!ht)
        return false;

    static u64 current_index = 0;

    for (u64 i = current_index + 1; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            current_index = i;
            *key = ht->entries[i].key;
            *key_size = ht->entries[i].key_size;
            *data = ht->entries[i].data;
            *data_size = ht->entries[i].data_size;
            return true;
        }
    }

    current_index = 0; // Reset for next iteration
    return false;
}


u64 cl_ht_foreach_remove(cl_ht_t *ht, cl_ht_remove_func_t r_fn, cl_ht_free_func_t ff, void *arg)
{
    if (!ht || !r_fn)
        return 0;

    u64 removed = 0;
    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            if (r_fn(ht->entries[i].key, ht->entries[i].key_size, ht->entries[i].data, ht->entries[i].data_size, arg))
            {
                if (!(ht->flags & CL_HT_FLAG_NOCOPY_KEYS))
                {
                    cl_mem_free(ht->allocator, ht->entries[i].key);
                }
                if (ff)
                {
                    ff(ht->entries[i].data);
                }
                else if (ht->free_func)
                {
                    ht->free_func(ht->entries[i].data);
                }
                memset(&ht->entries[i], 0, sizeof(cl_ht_entry_t));
                ht->size--;
                removed++;
            }
        }
    }

    return removed;
}
u64 cl_ht_foreach(cl_ht_t *ht, cl_ht_foreach_func_t fe_fn, void *arg)
{
    if (!ht || !fe_fn)
        return 0;

    u64 count = 0;
    for (u64 i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key)
        {
            if (!fe_fn(ht->entries[i].key, ht->entries[i].key_size, ht->entries[i].data, ht->entries[i].data_size, arg))
            {
                return count;
            }
            count++;
        }
    }

    return count;
}

bool cl_ht_rehash(cl_ht_t *ht) { return cl_ht_resize(ht, ht->capacity); }

u64 cl_ht_size(cl_ht_t *ht) { return ht ? ht->size : 0; }

bool cl_ht_lock(cl_ht_t *ht)
{
    if (!ht || !ht->mutex)
        return false;
    return cl_mutex_lock(ht->mutex);
}

bool cl_ht_unlock(cl_ht_t *ht)
{
    if (!ht || !ht->mutex)
        return false;
    return cl_mutex_unlock(ht->mutex);
}

// String-specific functions
void *cl_ht_get_str(cl_ht_t *ht, const char *key)
{
    void *data;
    u64 data_size;
    if (cl_ht_get(ht, key, strlen(key), &data, &data_size))
    {
        return data;
    }
    return null;
}

bool cl_ht_exists_str(cl_ht_t *ht, const char *key) { return cl_ht_exists(ht, key, strlen(key)); }
bool cl_ht_is_empty(cl_ht_t *ht) { return ht ? (ht->size == 0) : true; }
void *cl_ht_put_str(cl_ht_t *ht, const char *key, void *data)
{
    void *old_data;
    cl_ht_put(ht, key, strlen(key), data, sizeof(void *), &old_data);
    return old_data;
}

void *cl_ht_remove_str(cl_ht_t *ht, const char *key) { return cl_ht_remove(ht, key, strlen(key)); }
float cl_ht_load_factor(cl_ht_t *ht) { return ht ? ((float)ht->size / ht->capacity) : 0.0f; }
bool cl_ht_iter_init_str(cl_ht_t *ht, char **key, void **data)
{
    u64 key_size, data_size;
    return cl_ht_iter_init(ht, (void **)key, &key_size, data, &data_size);
}

bool cl_ht_iter_next_str(cl_ht_t *ht, char **key, void **data)
{
    u64 key_size, data_size;
    return cl_ht_iter_next(ht, (void **)key, &key_size, data, &data_size);
}
