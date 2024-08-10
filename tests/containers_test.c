// table_tests.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clib/containers_lib.h"
#include "clib/test_lib.h"
#include "clib/time_lib.h"

#define TEST_ALLOCATOR null // Replace with your actual test allocator if needed

// Helper function to generate random strings
char *generate_random_string(int length)
{
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    char *str = malloc(length + 1);
    for (int i = 0; i < length; i++)
    {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[length] = '\0';
    return str;
}

// Helper function to print benchmark results
void print_benchmark(const char *test_name, cl_time_t duration, int operations)
{
    double ms = cl_time_to_ms(&duration) / 1000.0;
    printf("%s: %.3f ms (%.2f ops/ms)\n", test_name, ms, operations / ms);
}

CL_TEST(test_ht_basic_operations)
{
    cl_ht_t *ht = cl_ht_create(TEST_ALLOCATOR);
    CL_ASSERT(ht != null);

    // Test insertion
    CL_ASSERT(cl_ht_put(ht, "key1", 4, "value1", 7, null));
    CL_ASSERT(cl_ht_put(ht, "key2", 4, "value2", 7, null));

    // Test retrieval
    void *value;
    u64 value_size;
    CL_ASSERT(cl_ht_get(ht, "key1", 4, &value, &value_size));
    CL_ASSERT(strcmp(value, "value1") == 0);
    CL_ASSERT(value_size == 7);

    // Test update
    CL_ASSERT(cl_ht_put(ht, "key1", 4, "new_value1", 11, null));
    CL_ASSERT(cl_ht_get(ht, "key1", 4, &value, &value_size));
    CL_ASSERT(strcmp(value, "new_value1") == 0);
    CL_ASSERT(value_size == 11);

    // Test removal
    CL_ASSERT(cl_ht_remove(ht, "key1", 4) != null);
    CL_ASSERT(!cl_ht_get(ht, "key1", 4, &value, &value_size));

    // Test size
    CL_ASSERT(cl_ht_size(ht) == 1);

    cl_ht_destroy(ht);
}

CL_TEST(test_ht_collision_handling)
{
    cl_ht_t *ht = cl_ht_create_with_size(TEST_ALLOCATOR, 2); // Force collisions with small initial size
    const char *keys[] = {"key1", "key2", "key3", "key4", "key5"};
    int values[] = {1, 2, 3, 4, 5};
    bool all_valid = true;

    for (int i = 0; i < 5; i++)
    {
        all_valid &= cl_ht_put(ht, keys[i], strlen(keys[i]), &values[i], sizeof(int), null);
    }

    CL_ASSERT(cl_ht_size(ht) == 5);

    for (int i = 0; i < 5 && all_valid; i++)
    {
        void *data;
        u64 data_size;
        all_valid &= cl_ht_get(ht, keys[i], strlen(keys[i]), &data, &data_size);
        all_valid &= (*(int *)data == values[i]);
    }

    CL_ASSERT(all_valid);
    cl_ht_destroy(ht);
}


CL_TEST(test_ht_resize)
{
    cl_ht_t *ht = cl_ht_create_with_size(TEST_ALLOCATOR, 2);
    const int num_entries = 1000;
    bool all_valid = true;

    for (int i = 0; i < num_entries; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        int *value = malloc(sizeof(int));
        *value = i;
        all_valid &= cl_ht_put(ht, key, strlen(key), value, sizeof(int), null);
    }

    CL_ASSERT(cl_ht_size(ht) == num_entries);

    for (int i = 0; i < num_entries && all_valid; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        void *data;
        u64 data_size;
        all_valid &= cl_ht_get(ht, key, strlen(key), &data, &data_size);
        all_valid &= (*(int *)data == i);
    }

    CL_ASSERT(all_valid);
    cl_ht_destroy(ht);
}

CL_TEST(test_ht_string_specific_functions)
{
    cl_ht_t *ht = cl_ht_create(TEST_ALLOCATOR);

    // Test put_str and get_str
    cl_ht_put_str(ht, "string_key", "string_value");
    char *value = cl_ht_get_str(ht, "string_key");
    CL_ASSERT(strcmp(value, "string_value") == 0);

    // Test exists_str
    CL_ASSERT(cl_ht_exists_str(ht, "string_key"));
    CL_ASSERT(!cl_ht_exists_str(ht, "nonexistent_key"));

    // Test remove_str
    value = cl_ht_remove_str(ht, "string_key");
    CL_ASSERT(strcmp(value, "string_value") == 0);
    CL_ASSERT(!cl_ht_exists_str(ht, "string_key"));

    cl_ht_destroy(ht);
}

CL_TEST(test_ht_iteration)
{
    cl_ht_t *ht = cl_ht_create(TEST_ALLOCATOR);
    const int num_entries = 100;
    bool all_valid = true;

    for (int i = 0; i < num_entries; i++)
    {
        char key[16], value[16];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);
        cl_ht_put_str(ht, key, strdup(value));
    }

    int count = -1;
    char *key;
    void *data;
    if (cl_ht_iter_init_str(ht, &key, &data))
    {
        do
        {
            count++;
            all_valid &= (strncmp(key, "key", 3) == 0);
            all_valid &= (strncmp(data, "value", 5) == 0);
        }
        while (cl_ht_iter_next_str(ht, &key, &data));
    }

    CL_ASSERT(count == num_entries);
    CL_ASSERT(all_valid);

    cl_ht_destroy(ht);
}

typedef struct
{
    int count;
    bool all_valid;
} foreach_data_t;


static bool foreach_callback(void *key, u64 key_size, void *data, u64 data_size, void *arg)
{
    foreach_data_t *fd = (foreach_data_t *)arg;
    fd->count++;

    // Verify that the key starts with "key" and the data starts with "value"
    if (key_size < 3 || memcmp(key, "key", 3) != 0 || data_size < 5 || memcmp(data, "value", 5) != 0)
    {
        fd->all_valid = false;
    }

    return true;
}

CL_TEST(test_ht_foreach)
{
    cl_ht_t *ht = cl_ht_create(TEST_ALLOCATOR);
    const int num_entries = 100;

    for (int i = 0; i < num_entries; i++)
    {
        char key[16], value[16];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);
        cl_ht_put(ht, key, strlen(key), strdup(value), strlen(value), null);
    }
    char *value;
    u64 value_size;
    CL_ASSERT(cl_ht_get(ht, "key0", 4, (void **)&value, &value_size));

    char* value2 = cl_ht_get_str(ht, "key5");
    CL_ASSERT(strcmp(value2, "value5") == 0);

    foreach_data_t fd = {0, true};
    u64 visited = cl_ht_foreach(ht, foreach_callback, &fd);

    CL_ASSERT(visited == num_entries);
    CL_ASSERT(fd.count == num_entries);
    CL_ASSERT(fd.all_valid);

    cl_ht_destroy(ht);
}

CL_TEST(test_ht_edge_cases)
{
    cl_ht_t *ht = cl_ht_create(TEST_ALLOCATOR);

    // Test empty key
    CL_ASSERT(cl_ht_put(ht, "", 0, "empty_key_value", 16, null));
    void *value;
    u64 value_size;
    CL_ASSERT(cl_ht_get(ht, "", 0, &value, &value_size));
    CL_ASSERT(strcmp(value, "empty_key_value") == 0);

    // Test null value
    CL_ASSERT(cl_ht_put(ht, "null_value_key", 14, null, 0, null));
    CL_ASSERT(cl_ht_get(ht, "null_value_key", 14, &value, &value_size));
    CL_ASSERT(value == null);
    CL_ASSERT(value_size == 0);

    // Test large key and value
    char *large_key = generate_random_string(1000);
    char *large_value = generate_random_string(10000);
    CL_ASSERT(cl_ht_put(ht, large_key, 1000, large_value, 10000, null));
    CL_ASSERT(cl_ht_get(ht, large_key, 1000, &value, &value_size));
    CL_ASSERT(memcmp(value, large_value, 10000) == 0);
    free(large_key);
    free(large_value);

    cl_ht_destroy(ht);
}

CL_TEST(test_ht_performance)
{
    cl_ht_t *ht = cl_ht_create(TEST_ALLOCATOR);
    const int num_entries = 1000000;
    cl_time_t start, end, duration;
    bool all_valid = true;

    // Benchmark insertion
    cl_time_get_current(&start);
    for (int i = 0; i < num_entries; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        int *value = malloc(sizeof(int));
        *value = i;
        all_valid &= cl_ht_put(ht, key, strlen(key), value, sizeof(int), null);
        if (!all_valid)
        {
            printf("Insertion failed at i=%d\n", i);
            break;
        }
    }
    cl_time_get_current(&end);
    duration = cl_time_diff(&end, &start);
    print_benchmark("Insertion", duration, num_entries);

    // Benchmark retrieval
    cl_time_get_current(&start);
    for (int i = 0; i < num_entries && all_valid; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        int *value;
        all_valid &= cl_ht_get(ht, key, strlen(key), (void **)&value, null);
        all_valid &= (value != null && *value == i);
        if (!all_valid)
        {
            printf("Retrieval failed at i=%d, value=%d\n", i, (value ? *value : -1));
            break;
        }
    }
    cl_time_get_current(&end);
    duration = cl_time_diff(&end, &start);
    print_benchmark("Retrieval", duration, num_entries);

    // Benchmark deletion
    cl_time_get_current(&start);
    for (int i = 0; i < num_entries && all_valid; i++)
    {
        char key[16];
        snprintf(key, sizeof(key), "key%d", i);
        int *value = cl_ht_remove_str(ht, key);
        all_valid &= (value != null && *value == i);
        free(value);
        if (!all_valid)
        {
            printf("Deletion failed at i=%d, value=%d\n", i, (value ? *value : -1));
            break;
        }
    }
    cl_time_get_current(&end);
    duration = cl_time_diff(&end, &start);
    print_benchmark("Deletion", duration, num_entries);

    CL_ASSERT(cl_ht_size(ht) == 0);
    CL_ASSERT(all_valid);

    cl_ht_destroy(ht);
}


CL_TEST_SUITE_BEGIN(HashTableTests)
CL_TEST_SUITE_TEST(test_ht_basic_operations)
CL_TEST_SUITE_TEST(test_ht_collision_handling)
CL_TEST_SUITE_TEST(test_ht_resize)
CL_TEST_SUITE_TEST(test_ht_string_specific_functions)
CL_TEST_SUITE_TEST(test_ht_iteration)
CL_TEST_SUITE_TEST(test_ht_foreach)
CL_TEST_SUITE_TEST(test_ht_edge_cases)
CL_TEST_SUITE_TEST(test_ht_performance)
CL_TEST_SUITE_END

int main()
{
    CL_RUN_TEST_SUITE(HashTableTests);
    CL_RUN_ALL_TESTS();
    return 0;
}
