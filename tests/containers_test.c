/**
 * Optimized Hash Table Tests
 * Created by Claude on 8/4/2024.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "clib/containers_lib.h"
#include "clib/log_lib.h"
#include "clib/memory_lib.h"
#include "clib/test_lib.h"

#define TEST_ITEMS 1000

static cl_ht_t *hash_table;
static cl_allocator_t *allocator;

// Helper function to create a string key
static char *create_key(int i)
{
    char *key = cl_mem_alloc(allocator, 20);
    snprintf(key, 20, "key_%d", i);
    return key;
}

// Helper function to create a string value
static char *create_value(int i)
{
    char *value = cl_mem_alloc(allocator, 20);
    snprintf(value, 20, "value_%d", i);
    return value;
}

CL_TEST(test_hash_table_create_destroy)
{
    cl_ht_t *ht = cl_ht_init(allocator);
    CL_ASSERT_NOT_NULL(ht);
    cl_ht_destroy(ht);
}

CL_TEST(test_hash_table_insert_get)
{
    cl_ht_t *ht = cl_ht_init(allocator);

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char *value = create_value(i);

        bool result = cl_ht_insert(ht, key, value);
        CL_ASSERT(result);
    }

    CL_ASSERT_EQUAL(cl_ht_size(ht), TEST_ITEMS);

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char expected_value[20];
        snprintf(expected_value, 20, "value_%d", i);

        char *value = cl_ht_get(ht, key);

        CL_ASSERT_NOT_NULL(value);
        CL_ASSERT_STRING_EQUAL(value, expected_value);

        cl_mem_free(allocator, key);
    }

    cl_ht_destroy(ht);
}

CL_TEST(test_hash_table_remove)
{
    cl_ht_t *ht = cl_ht_init(allocator);

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char *value = create_value(i);
        cl_ht_insert(ht, key, value);
    }

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);

        bool remove_result = cl_ht_remove(ht, key);
        CL_ASSERT(remove_result);

        void *retrieved_value = cl_ht_get(ht, key);
        CL_ASSERT_NULL(retrieved_value);

        cl_mem_free(allocator, key);
    }

    CL_ASSERT_EQUAL(cl_ht_size(ht), 0);
    CL_ASSERT(cl_ht_is_empty(ht));

    cl_ht_destroy(ht);
}

CL_TEST(test_hash_table_clear)
{
    cl_ht_t *ht = cl_ht_init(allocator);

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char *value = create_value(i);
        cl_ht_insert(ht, key, value);
    }

    CL_ASSERT_EQUAL(cl_ht_size(ht), TEST_ITEMS);

    cl_ht_clear(ht);
    CL_ASSERT_EQUAL(cl_ht_size(ht), 0);
    CL_ASSERT(cl_ht_is_empty(ht));

    cl_ht_destroy(ht);
}

CL_TEST(test_hash_table_update)
{
    cl_ht_t *ht = cl_ht_init(allocator);

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char *value = create_value(i);
        cl_ht_insert(ht, key, value);
    }

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char *new_value = cl_mem_alloc(allocator, 30);
        snprintf(new_value, 30, "updated_value_%d", i);

        bool update_result = cl_ht_insert(ht, key, new_value);
        CL_ASSERT(update_result);

        char *retrieved_value = cl_ht_get(ht, key);
        CL_ASSERT_NOT_NULL(retrieved_value);
        CL_ASSERT_STRING_EQUAL(retrieved_value, new_value);

        cl_mem_free(allocator, key);
    }

    CL_ASSERT_EQUAL(cl_ht_size(ht), TEST_ITEMS);

    cl_ht_destroy(ht);
}


static void count_entries(const void *key, void *value, void *user_data)
{
    int *count = (int *)user_data;
    (*count)++;
}

CL_TEST(test_hash_table_foreach)
{
    cl_ht_t *ht = cl_ht_init(allocator);

    for (int i = 0; i < TEST_ITEMS; i++)
    {
        char *key = create_key(i);
        char *value = create_value(i);
        cl_ht_insert(ht, key, value);
    }

    int count = 0;
    cl_ht_foreach(ht, count_entries, &count);

    CL_ASSERT_EQUAL(count, TEST_ITEMS);

    cl_ht_destroy(ht);
}

CL_TEST_SUITE_BEGIN(ContainerTests)
    CL_TEST_SUITE_TEST(test_hash_table_create_destroy)
    CL_TEST_SUITE_TEST(test_hash_table_insert_get)
    CL_TEST_SUITE_TEST(test_hash_table_remove)
    CL_TEST_SUITE_TEST(test_hash_table_clear)
    CL_TEST_SUITE_TEST(test_hash_table_update)
    CL_TEST_SUITE_TEST(test_hash_table_foreach)
CL_TEST_SUITE_END

int main()
{
    cl_log_init_default(CL_LOG_INFO);
    allocator = cl_allocator_create(&(cl_allocator_config_t){.type = CL_ALLOCATOR_TYPE_PLATFORM});

    CL_RUN_TEST_SUITE(ContainerTests);
    CL_RUN_ALL_TESTS();
    cl_allocator_destroy(allocator);
    cl_log_cleanup();
    return 0;
}