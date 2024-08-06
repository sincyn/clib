#include "clib/string_lib.h"
#include <string.h>

// String view functions
str_view str_view_create(const char *data, u32 length) { return (str_view){length, data}; }

str_view str_view_substr(const str_view *s, u32 start, u32 end)
{
    if (start >= s->len || end > s->len || start > end)
    {
        return (str_view){0, NULL};
    }
    return (str_view){end - start, s->data + start};
}

bool str_view_equals(const str_view *s1, const str_view *s2)
{
    if (s1->len != s2->len)
    {
        return false;
    }
    return cl_mem_compare(s1->data, s2->data, s1->len) == 0;
}

bool str_view_contains(const str_view *haystack, const str_view *needle)
{
    return str_view_index_of(haystack, needle) != -1;
}

i32 str_view_index_of(const str_view *haystack, const str_view *needle)
{
    if (needle->len > haystack->len)
    {
        return -1;
    }
    for (u32 i = 0; i <= haystack->len - needle->len; i++)
    {
        if (cl_mem_compare(haystack->data + i, needle->data, needle->len) == 0)
        {
            return i;
        }
    }
    return -1;
}

bool str_view_starts_with(const str_view *s, const str_view *prefix)
{
    if (prefix->len > s->len)
    {
        return false;
    }
    return cl_mem_compare(s->data, prefix->data, prefix->len) == 0;
}

bool str_view_ends_with(const str_view *s, const str_view *suffix)
{
    if (suffix->len > s->len)
    {
        return false;
    }
    return cl_mem_compare(s->data + s->len - suffix->len, suffix->data, suffix->len) == 0;
}


str_view str_view_trim(const str_view *s)
{
    u32 start = 0;
    u32 end = s->len;

    while (start < end &&
           (s->data[start] == ' ' || s->data[start] == '\t' || s->data[start] == '\n' || s->data[start] == '\r'))
    {
        start++;
    }

    while (
        end > start &&
        (s->data[end - 1] == ' ' || s->data[end - 1] == '\t' || s->data[end - 1] == '\n' || s->data[end - 1] == '\r'))
    {
        end--;
    }

    return (str_view){end - start, s->data + start};
}

// Owned string functions
str str_create(const cl_allocator_t *allocator, const char *data, u32 length)
{
    str s;
    s.len = length;
    s.data = (char *)cl_mem_alloc(allocator, length + 1);
    cl_mem_copy(s.data, data, length);
    s.data[length] = '\0';
    return s;
}

str str_from_view(const cl_allocator_t *allocator, const str_view *view)
{
    return str_create(allocator, view->data, view->len);
}

str str_concat(const cl_allocator_t *allocator, const str_view *s1, const str_view *s2)
{
    str result;
    result.len = s1->len + s2->len;
    result.data = (char *)cl_mem_alloc(allocator, result.len + 1);
    cl_mem_copy(result.data, s1->data, s1->len);
    cl_mem_copy(result.data + s1->len, s2->data, s2->len);
    result.data[result.len] = '\0';
    return result;
}

str *str_split(const cl_allocator_t *allocator, const str_view *s, char delimiter)
{
    u32 count = 1;
    for (u32 i = 0; i < s->len; i++)
    {
        if (s->data[i] == delimiter)
        {
            count++;
        }
    }

    str *result = cl_mem_alloc(allocator, (count + 1) * sizeof(str));
    u32 start = 0;
    u32 index = 0;

    for (u32 i = 0; i < s->len; i++)
    {
        if (s->data[i] == delimiter)
        {
            result[index++] = str_create(allocator, s->data + start, i - start);
            start = i + 1;
        }
    }

    result[index++] = str_create(allocator, s->data + start, s->len - start);
    result[index] = (str){0, NULL}; // Null-terminate the array
    return result;
}

str str_join(const cl_allocator_t *allocator, const str_view *strings, u32 count, const str_view *delimiter)
{
    u32 total_len = 0;
    for (u32 i = 0; i < count; i++)
    {
        total_len += strings[i].len;
    }
    total_len += (count - 1) * delimiter->len;

    str result;
    result.len = total_len;
    result.data = (char *)cl_mem_alloc(allocator, total_len + 1);

    char *ptr = result.data;
    for (u32 i = 0; i < count; i++)
    {
        cl_mem_copy(ptr, strings[i].data, strings[i].len);
        ptr += strings[i].len;
        if (i < count - 1)
        {
            cl_mem_copy(ptr, delimiter->data, delimiter->len);
            ptr += delimiter->len;
        }
    }
    result.data[total_len] = '\0';
    return result;
}

void str_clear(str *s)
{
    s->len = 0;
    s->data[0] = '\0';
}

void str_destroy(const cl_allocator_t *allocator, str *s)
{
    cl_mem_free(allocator, s->data);
    s->data = NULL;
    s->len = 0;
}

// Conversion functions
str_view str_as_view(const str *s) { return (str_view){s->len, s->data}; }
