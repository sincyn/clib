/**
 * Abstract Filesystem Library - Internal Definitions
 * Created by Claude on 8/5/2024.
 * Updated by Assistant on 8/5/2024.
 */

#pragma once

#include "clib/filesystem_lib.h"

#define MAX_ERROR_LENGTH 256
#define MAX_PATH_LENGTH 4096

struct cl_fs
{
    cl_allocator_t *allocator;
    cl_fs_config_t config;
    char last_error[MAX_ERROR_LENGTH];
    void *platform_data; // Platform-specific data
};

struct cl_file
{
    cl_fs_t *fs;
    void *handle; // Platform-specific file handle
};

struct cl_fs_dir_iterator
{
    cl_fs_t *fs;
     char *path;
    void *handle; // Platform-specific directory handle
};

// Platform-independent internal functions
void cl_fs_set_last_error(cl_fs_t *fs, const char *error);

// Platform-specific function declarations
bool cl_fs_platform_init(cl_fs_t *fs);
void cl_fs_platform_cleanup(cl_fs_t *fs);

char *cl_fs_platform_normalize_path(cl_fs_t *fs, const char *path);
char *cl_fs_platform_denormalize_path(cl_fs_t *fs, const char *normalized_path);

cl_file_t *cl_fs_platform_open_file(cl_fs_t *fs, const char *path, cl_file_mode_t mode);
void cl_fs_platform_close_file(cl_file_t *file);
u64 cl_fs_platform_read_file(cl_file_t *file, void *buffer, u64 size);
u64 cl_fs_platform_write_file(cl_file_t *file, const void *buffer, u64 size);
bool cl_fs_platform_seek_file(cl_file_t *file, i64 offset, i32 origin);
i64 cl_fs_platform_tell_file(cl_file_t *file);

bool cl_fs_platform_create_directory(cl_fs_t *fs, const char *path);
bool cl_fs_platform_remove_directory(cl_fs_t *fs, const char *path);
bool cl_fs_platform_directory_exists(cl_fs_t *fs, const char *path);

bool cl_fs_platform_remove_file(cl_fs_t *fs, const char *path);
bool cl_fs_platform_rename(cl_fs_t *fs, const char *old_path, const char *new_path);
bool cl_fs_platform_copy(cl_fs_t *fs, const char *src_path, const char *dest_path);
bool cl_fs_platform_file_exists(cl_fs_t *fs, const char *path);

i64 cl_fs_platform_get_file_size(cl_fs_t *fs, const char *path);
bool cl_fs_platform_get_file_time(cl_fs_t *fs, const char *path, u64 *creation_time, u64 *last_access_time,
                                  u64 *last_write_time);

cl_fs_dir_iterator_t *cl_fs_platform_open_directory(cl_fs_t *fs, const char *path);
bool cl_fs_platform_read_directory(cl_fs_dir_iterator_t *iterator, cl_fs_dir_entry_t *entry);
void cl_fs_platform_close_directory(cl_fs_dir_iterator_t *iterator);

bool cl_fs_platform_set_file_attributes(cl_fs_t *fs, const char *path, cl_file_attribute_t attributes);
cl_file_attribute_t cl_fs_platform_get_file_attributes(cl_fs_t *fs, const char *path);
