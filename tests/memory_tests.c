#include <string.h>

#include "clib/log_lib.h"
#include "clib/memory_lib.h"
#include "clib/test_lib.h"

#define TEST_ALLOC_SIZE 100

static cl_allocator_t *test_allocator;
static cl_allocator_t *arena_allocator;

CL_TEST(test_allocator_create_and_destroy)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = null);
    CL_ASSERT_NOT_NULL(allocator);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_mem_alloc_and_free)
{
    void *ptr = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr);
    cl_mem_free(test_allocator, ptr);
}

CL_TEST(test_mem_realloc)
{
    void *ptr = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr);

    void *new_ptr = cl_mem_realloc(test_allocator, ptr, TEST_ALLOC_SIZE * 2);
    CL_ASSERT_NOT_NULL(new_ptr);

    cl_mem_free(test_allocator, new_ptr);
}


CL_TEST(test_mem_set)
{
    void *ptr = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr);

    int test_value = 0xAA;
    cl_mem_set(ptr, test_value, TEST_ALLOC_SIZE);

    for (u64 i = 0; i < TEST_ALLOC_SIZE; i++)
    {
        CL_ASSERT_EQUAL(((unsigned char *)ptr)[i], (unsigned char)test_value);
    }

    cl_mem_free(test_allocator, ptr);
}

CL_TEST(test_mem_copy)
{
    void *src = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    void *dest = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(src);
    CL_ASSERT_NOT_NULL(dest);

    for (u64 i = 0; i < TEST_ALLOC_SIZE; i++)
    {
        ((unsigned char *)src)[i] = (unsigned char)i;
    }

    cl_mem_copy(dest, src, TEST_ALLOC_SIZE);

    CL_ASSERT_EQUAL(memcmp(src, dest, TEST_ALLOC_SIZE), 0);

    cl_mem_free(test_allocator, src);
    cl_mem_free(test_allocator, dest);
}

CL_TEST(test_mem_move)
{
    unsigned char *buffer = (unsigned char *)cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE * 2);
    CL_ASSERT_NOT_NULL(buffer);

    for (u64 i = 0; i < TEST_ALLOC_SIZE; i++)
    {
        buffer[i] = (unsigned char)i;
    }

    cl_mem_move(buffer + TEST_ALLOC_SIZE / 2, buffer, TEST_ALLOC_SIZE);

    for (u64 i = 0; i < TEST_ALLOC_SIZE / 2; i++)
    {
        CL_ASSERT_EQUAL(buffer[TEST_ALLOC_SIZE / 2 + i], (unsigned char)i);
    }

    cl_mem_free(test_allocator, buffer);
}

CL_TEST(test_mem_compare)
{
    void *buf1 = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    void *buf2 = cl_mem_alloc(test_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(buf1);
    CL_ASSERT_NOT_NULL(buf2);

    cl_mem_set(buf1, 0xAA, TEST_ALLOC_SIZE);
    cl_mem_set(buf2, 0xAA, TEST_ALLOC_SIZE);

    CL_ASSERT_EQUAL(cl_mem_compare(buf1, buf2, TEST_ALLOC_SIZE), 0);

    ((unsigned char *)buf2)[TEST_ALLOC_SIZE - 1] = 0xBB;

    CL_ASSERT_NOT_EQUAL(cl_mem_compare(buf1, buf2, TEST_ALLOC_SIZE), 0);

    cl_mem_free(test_allocator, buf1);
    cl_mem_free(test_allocator, buf2);
}


CL_TEST(test_arena_allocator_create_and_destroy)
{
    cl_allocator_config_t config = {.type = CL_ALLOCATOR_TYPE_ARENA,
                                    .flags = CL_ALLOCATOR_FLAG_NONE,
                                    .config.arena = {
                                        .size = 1024 // 1KB block size
                                    }};
    cl_allocator_t *allocator = cl_allocator_create(&config);
    CL_ASSERT_NOT_NULL(allocator);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_arena_allocator_alloc_and_free)
{
    void *ptr1 = cl_mem_alloc(arena_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr1);

    void *ptr2 = cl_mem_alloc(arena_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr2);

    // In arena allocator, free doesn't actually free memory
    cl_mem_free(arena_allocator, ptr1);
    cl_mem_free(arena_allocator, ptr2);

    // We can still allocate and use memory after "freeing"
    void *ptr3 = cl_mem_alloc(arena_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr3);
    memset(ptr3, 0xAA, TEST_ALLOC_SIZE);
}

CL_TEST(test_arena_allocator_large_alloc)
{
    // Allocate a chunk larger than the default block size
    void *large_ptr = cl_mem_alloc(arena_allocator, 1024 * 1024); // 1MB
    CL_ASSERT_NOT_NULL(large_ptr);

    // Ensure we can use the entire allocated memory
    memset(large_ptr, 0xBB, 1024 * 1024);

    // Allocate another small chunk to ensure the allocator is still functional
    void *small_ptr = cl_mem_alloc(arena_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(small_ptr);
}

CL_TEST(test_arena_allocator_realloc)
{
    void *ptr1 = cl_mem_alloc(arena_allocator, TEST_ALLOC_SIZE);
    CL_ASSERT_NOT_NULL(ptr1);
    memset(ptr1, 0xCC, TEST_ALLOC_SIZE);

    // Realloc to a larger size
    void *ptr2 = cl_mem_realloc(arena_allocator, ptr1, TEST_ALLOC_SIZE * 2);
    CL_ASSERT_NOT_NULL(ptr2);

    // Check if the original data is preserved
    for (int i = 0; i < TEST_ALLOC_SIZE; i++)
    {
        CL_ASSERT_EQUAL(((unsigned char *)ptr2)[i], 0xCC);
    }

    // Fill the rest of the reallocated memory
    memset((unsigned char *)ptr2 + TEST_ALLOC_SIZE, 0xDD, TEST_ALLOC_SIZE);
}

CL_TEST_SUITE_BEGIN(PlatformMemoryTests)
CL_TEST_SUITE_TEST(test_allocator_create_and_destroy)
CL_TEST_SUITE_TEST(test_mem_alloc_and_free)
CL_TEST_SUITE_TEST(test_mem_realloc)
CL_TEST_SUITE_TEST(test_mem_set)
CL_TEST_SUITE_TEST(test_mem_copy)
CL_TEST_SUITE_TEST(test_mem_move)
CL_TEST_SUITE_TEST(test_mem_compare)
CL_TEST_SUITE_END

CL_TEST_SUITE_BEGIN(ArenaMemoryTests)
CL_TEST_SUITE_TEST(test_arena_allocator_create_and_destroy)
CL_TEST_SUITE_TEST(test_arena_allocator_alloc_and_free)
CL_TEST_SUITE_TEST(test_arena_allocator_large_alloc)
CL_TEST_SUITE_TEST(test_arena_allocator_realloc)
CL_TEST_SUITE_END


int main()
{
    test_allocator = cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = null);
    CL_RUN_TEST_SUITE(PlatformMemoryTests);
    CL_RUN_TEST_SUITE(ArenaMemoryTests);
    CL_RUN_ALL_TESTS();
    cl_allocator_destroy(test_allocator);
    return 0;
}
