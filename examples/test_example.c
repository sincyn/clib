/**
 * Created by jraynor on 8/3/2024.
 */

#include <string.h>
#include "clib/test_lib.h"

CL_TEST(test_addition)
{
    CL_ASSERT_EQUAL(2 + 2, 4);
    CL_ASSERT_NOT_EQUAL(2 + 2, 5);
}

CL_TEST(test_subtraction)
{
    CL_ASSERT_EQUAL(5 - 3, 2);
    CL_ASSERT_NOT_EQUAL(5 - 3, 1);
}

CL_TEST_SUITE_BEGIN(MathTests)
CL_TEST_SUITE_TEST(test_addition)
CL_TEST_SUITE_TEST(test_subtraction)
CL_TEST_SUITE_END


CL_TEST(test_string_comparison)
{
    CL_ASSERT_STRING_EQUAL("hello", "hello");
    CL_ASSERT_STRING_NOT_EQUAL("hello", "world");
    CL_ASSERT_STRING_NOT_EQUAL("hello", "HELLO");
}

CL_TEST(test_string_length)
{
    char *str = "hello";
    CL_ASSERT_EQUAL(strlen(str), 5);
    CL_ASSERT_NOT_EQUAL(strlen(str), 4);
}

CL_TEST(test_null_pointer)
{
    int *ptr = NULL;
    CL_ASSERT_NULL(ptr);
    int value = 5;
    CL_ASSERT_NOT_NULL(&value);
}

CL_TEST(test_expected_failure)
{
    CL_ASSERT_EQUAL(2 + 2, 5);
    CL_ASSERT_NOT_EQUAL(2 + 2, 4);
    CL_ASSERT_STRING_EQUAL("hola", "hola");
}


CL_TEST_SUITE_BEGIN(StringTests)
CL_TEST_SUITE_TEST(test_string_comparison)
CL_TEST_SUITE_TEST(test_string_length)
CL_TEST_SUITE_END

CL_TEST_SUITE_BEGIN(PointerTests)
CL_TEST_SUITE_TEST(test_null_pointer)
CL_TEST_SUITE_TEST(test_expected_failure)
CL_TEST_SUITE_END

int main()
{
    CL_RUN_TEST_SUITE(MathTests);
    CL_RUN_TEST_SUITE(StringTests);
    CL_RUN_TEST_SUITE(PointerTests);

    CL_RUN_ALL_TESTS();

    return 0;
}
