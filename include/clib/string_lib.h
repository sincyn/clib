#pragma once

#include "defines.h"
#include "memory_lib.h"

typedef struct
{
    u32 len;
    const char *data;
} str_view;

typedef struct
{
    u32 len;
    char *data;
} str;

// String view functions
str_view str_view_create(const char *data, u32 length);

str_view str_view_substr(const str_view *s, u32 start, u32 end);
bool str_view_equals(const str_view *s1, const str_view *s2);
bool str_view_contains(const str_view *haystack, const str_view *needle);
i32 str_view_index_of(const str_view *haystack, const str_view *needle);
bool str_view_starts_with(const str_view *s, const str_view *prefix);
bool str_view_ends_with(const str_view *s, const str_view *suffix);
str_view str_view_trim(const str_view *s);
str_view str_view_from_int(i64 value);
// Owned string functions
str str_create(const cl_allocator_t *allocator, const char *data, u32 length);
#define str_lit(allocator, data) str_create(allocator, data, sizeof(data) - 1)
str str_from_view(const cl_allocator_t *allocator, const str_view *view);
str str_concat(const cl_allocator_t *allocator, const str_view *s1, const str_view *s2);
str *str_split(const cl_allocator_t *allocator, const str_view *s, char delimiter);
str str_join(const cl_allocator_t *allocator, const str_view *strings, u32 count, const str_view *delimiter);
str str_from_int(const cl_allocator_t *allocator, i64 value);
str str_dup(const cl_allocator_t *allocator, const char *s);
str str_ndup(const cl_allocator_t *allocator, const char *s, u32 length);
bool str_push_char(const cl_allocator_t* allocator, str* s, char c);
str str_with_capacity(const cl_allocator_t* allocator, u32 capacity);
void str_clear(str *s);
void str_destroy(const cl_allocator_t *allocator, str *s);

// Conversion functions
str_view str_as_view(const str *s);
// Compile-time string view literal
#define str_view_lit(data)                                                                                             \
    (str_view) { sizeof(data) - 1, data }
