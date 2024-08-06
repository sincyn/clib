#include <stdio.h>
#include <string.h>
#include "clib/log_lib.h"
#include "clib/string_lib.h"
#include "clib/test_lib.h"

CL_TEST(test_str_view_create)
{
    const str_view sv = str_view_create("Hello, World!", 13);
    CL_ASSERT_EQUAL(sv.len, 13);
    CL_ASSERT_EQUAL(strncmp(sv.data, "Hello, World!", 13), 0);
}

CL_TEST(test_str_view_lit)
{
    const str_view sv = str_view_lit("Hello, World!");
    CL_ASSERT_EQUAL(sv.len, 13);
    CL_ASSERT_EQUAL(strncmp(sv.data, "Hello, World!", 13), 0);
}

CL_TEST(test_str_view_substr)
{
    const str_view sv = str_view_lit("Hello, World!");
    const str_view sub = str_view_substr(&sv, 7, 12);
    CL_ASSERT_EQUAL(sub.len, 5);
    CL_ASSERT_EQUAL(strncmp(sub.data, "World", 5), 0);
}

CL_TEST(test_str_view_equals)
{
    const str_view sv1 = str_view_lit("Hello");
    const str_view sv2 = str_view_lit("Hello");
    const str_view sv3 = str_view_lit("World");
    CL_ASSERT(str_view_equals(&sv1, &sv2));
    CL_ASSERT(!str_view_equals(&sv1, &sv3));
}

CL_TEST(test_str_view_contains)
{
    const str_view sv = str_view_lit("Hello, World!");
    const str_view needle1 = str_view_lit("World");
    const str_view needle2 = str_view_lit("Universe");
    CL_ASSERT(str_view_contains(&sv, &needle1));
    CL_ASSERT(!str_view_contains(&sv, &needle2));
}

CL_TEST(test_str_view_index_of)
{
    const str_view sv = str_view_lit("Hello, World!");
    const str_view needle1 = str_view_lit("World");
    const str_view needle2 = str_view_lit("Universe");
    CL_ASSERT_EQUAL(str_view_index_of(&sv, &needle1), 7);
    CL_ASSERT_EQUAL(str_view_index_of(&sv, &needle2), -1);
}

CL_TEST(test_str_view_starts_with)
{
    const str_view sv = str_view_lit("Hello, World!");
    const str_view prefix1 = str_view_lit("Hello");
    const str_view prefix2 = str_view_lit("World");
    CL_ASSERT(str_view_starts_with(&sv, &prefix1));
    CL_ASSERT(!str_view_starts_with(&sv, &prefix2));
}

CL_TEST(test_str_view_ends_with)
{
    const str_view sv = str_view_lit("Hello, World!");
    const str_view suffix1 = str_view_lit("World!");
    const str_view suffix2 = str_view_lit("Hello");
    CL_ASSERT(str_view_ends_with(&sv, &suffix1));
    CL_ASSERT(!str_view_ends_with(&sv, &suffix2));
}

CL_TEST(test_str_view_trim)
{
    const str_view sv = str_view_lit("  Hello, World!  ");
    const str_view trimmed = str_view_trim(&sv);
    CL_ASSERT_EQUAL(trimmed.len, 13);
    CL_ASSERT_EQUAL(strncmp(trimmed.data, "Hello, World!", 13), 0);
}

CL_TEST(test_str_create)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    str s = str_create(allocator, "Hello, World!", 13);
    CL_ASSERT_EQUAL(s.len, 13);
    CL_ASSERT_EQUAL(strcmp(s.data, "Hello, World!"), 0);
    str_destroy(allocator, &s);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_lit)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    str s = str_lit(allocator, "Hello, World!");
    CL_ASSERT_EQUAL(s.len, 13);
    CL_ASSERT_EQUAL(strcmp(s.data, "Hello, World!"), 0);
    str_destroy(allocator, &s);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_from_view)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    const str_view sv = str_view_lit("Hello, World!");
    str s = str_from_view(allocator, &sv);
    CL_ASSERT_EQUAL(s.len, 13);
    CL_ASSERT_EQUAL(strcmp(s.data, "Hello, World!"), 0);
    str_destroy(allocator, &s);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_concat)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    const str_view sv1 = str_view_lit("Hello, ");
    const str_view sv2 = str_view_lit("World!");
    str s = str_concat(allocator, &sv1, &sv2);
    CL_ASSERT_EQUAL(s.len, 13);
    CL_ASSERT_EQUAL(strcmp(s.data, "Hello, World!"), 0);
    str_destroy(allocator, &s);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_split)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    const str_view sv = str_view_lit("Hello,World,Test");
    str *split = str_split(allocator, &sv, ',');
    CL_ASSERT_EQUAL(split[0].len, 5);
    CL_ASSERT_EQUAL(strcmp(split[0].data, "Hello"), 0);
    CL_ASSERT_EQUAL(split[1].len, 5);
    CL_ASSERT_EQUAL(strcmp(split[1].data, "World"), 0);
    CL_ASSERT_EQUAL(split[2].len, 4);
    CL_ASSERT_EQUAL(strcmp(split[2].data, "Test"), 0);
    CL_ASSERT_EQUAL(split[3].len, 0);
    CL_ASSERT_NULL(split[3].data);
    for (int i = 0; i < 3; i++)
    {
        str_destroy(allocator, &split[i]);
    }
    cl_mem_free(allocator, split);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_join)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    const str_view strings[] = {str_view_lit("Hello"), str_view_lit("World"), str_view_lit("Test")};
    const str_view delimiter = str_view_lit(", ");
    str joined = str_join(allocator, strings, 3, &delimiter);
    CL_ASSERT_EQUAL(joined.len, 18);
    CL_ASSERT_EQUAL(strcmp(joined.data, "Hello, World, Test"), 0);
    str_destroy(allocator, &joined);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_as_view)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    str s = str_lit(allocator, "Hello, World!");
    const str_view sv = str_as_view(&s);
    CL_ASSERT_EQUAL(sv.len, 13);
    CL_ASSERT_EQUAL(strncmp(sv.data, "Hello, World!", 13), 0);
    str_destroy(allocator, &s);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_str_clear)
{
    cl_allocator_t *allocator =
        cl_allocator_new(CL_ALLOCATOR_TYPE_PLATFORM, .flags = CL_ALLOCATOR_FLAG_NONE, .user_data = NULL);
    str s = str_lit(allocator, "Hello, World!");
    str_clear(&s);
    CL_ASSERT_EQUAL(s.len, 0);
    CL_ASSERT_EQUAL(s.data[0], '\0');
    str_destroy(allocator, &s);
    cl_allocator_destroy(allocator);
}

CL_TEST_SUITE_BEGIN(StringTests)
CL_TEST_SUITE_TEST(test_str_view_create)
CL_TEST_SUITE_TEST(test_str_view_lit)
CL_TEST_SUITE_TEST(test_str_view_substr)
CL_TEST_SUITE_TEST(test_str_view_equals)
CL_TEST_SUITE_TEST(test_str_view_contains)
CL_TEST_SUITE_TEST(test_str_view_index_of)
CL_TEST_SUITE_TEST(test_str_view_starts_with)
CL_TEST_SUITE_TEST(test_str_view_ends_with)
CL_TEST_SUITE_TEST(test_str_view_trim)
CL_TEST_SUITE_TEST(test_str_create)
CL_TEST_SUITE_TEST(test_str_lit)
CL_TEST_SUITE_TEST(test_str_from_view)
CL_TEST_SUITE_TEST(test_str_concat)
CL_TEST_SUITE_TEST(test_str_split)
CL_TEST_SUITE_TEST(test_str_join)
CL_TEST_SUITE_TEST(test_str_as_view)
CL_TEST_SUITE_TEST(test_str_clear)
CL_TEST_SUITE_END

int main()
{
    CL_RUN_TEST_SUITE(StringTests);
    CL_RUN_ALL_TESTS();
    return 0;
}
