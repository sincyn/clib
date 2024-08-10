/**
 * Created by James Raynor on 8/6/24.
 */
/**
 * Hash Set Implementation
 * Created by Claude on 8/6/2024.
 */

#include "clib/containers_lib.h"
#include "clib/log_lib.h"

struct cl_hs
{
    cl_ht_t *table;
    cl_allocator_t *allocator;
};

cl_hs_t *cl_hs_init(cl_allocator_t *allocator)
{
    if (allocator == null)
    {
        cl_log_error("Null allocator provided to cl_hs_init");
        return null;
    }

    cl_hs_t *hs = cl_mem_alloc(allocator, sizeof(cl_hs_t));
    if (hs == null)
    {
        cl_log_error("Failed to allocate memory for hash set");
        return null;
    }

    hs->table = cl_ht_create(allocator);
    hs->allocator = allocator;
    if (hs->table == null)
    {
        cl_log_error("Failed to initialize hash table for hash set");
        cl_mem_free(allocator, hs);
        return null;
    }

    cl_log_debug("Hash set initialized");
    return hs;
}

void cl_hs_destroy(cl_hs_t *hs)
{
    if (hs)
    {

        cl_ht_destroy(hs->table);
        cl_mem_free(hs->allocator, hs);
        cl_log_debug("Hash set destroyed");
    }
}

bool cl_hs_insert(cl_hs_t *hs, const str_view *element)
{
    if (hs == null || element == null)
    {
        cl_log_error("Null hash set or element provided to cl_hs_insert");
        return false;
    }

    // We use the element as both key and value in the underlying hash table
    return cl_ht_put(hs->table, element->data, element->len, null, sizeof(u8), null);
}

bool cl_hs_contains(const cl_hs_t *hs, const str_view *element)
{
    if (hs == null || element == null)
    {
        cl_log_error("Null hash set or element provided to cl_hs_contains");
        return false;
    }

    return cl_ht_exists(hs->table, element->data, element->len);
}

bool cl_hs_remove(cl_hs_t *hs, const str_view *element)
{
    if (hs == null || element == null)
    {
        cl_log_error("Null hash set or element provided to cl_hs_remove");
        return false;
    }

    return cl_ht_remove(hs->table, element->data, element->len);
}

u64 cl_hs_size(const cl_hs_t *hs) { return (hs != null) ? cl_ht_size(hs->table) : 0; }

bool cl_hs_is_empty(const cl_hs_t *hs) { return (hs != null) ? cl_ht_is_empty(hs->table) : true; }

void cl_hs_clear(cl_hs_t *hs)
{
    if (hs != null)
    {
        cl_ht_clear(hs->table);
        cl_log_debug("Hash set cleared");
    }
}
static bool hs_foreach_wrapper(void *key, u64 key_size, void *data, u64 data_size, void *arg)
{
    void *wrapper_data[] = {arg, key};
    void (*callback)(const str_view *element, void *user_data) = wrapper_data[0];
    callback(key, wrapper_data[1]);
    return true;
}

void cl_hs_foreach(const cl_hs_t *hs, void (*callback)(const cl_hs_t *hs, const str_view *element, void *user_data),
                   void *user_data)
{
    if (hs != null && callback != null)
    {
        void *wrapper_data[] = {(void *)callback, (void *)hs->table, user_data};
        cl_ht_foreach(hs->table, hs_foreach_wrapper, wrapper_data);
    }
}
