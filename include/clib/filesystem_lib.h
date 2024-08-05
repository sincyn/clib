/**
 * Abstract Filesystem Library
 * Created by Claude on 8/5/2024.
 * Updated by Assistant on 8/5/2024.
 */

#pragma once

#include "defines.h"
#include "memory_lib.h"

// Forward declarations
typedef struct cl_fs cl_fs_t;
typedef struct cl_file cl_file_t;

// Filesystem types
typedef enum
{
    CL_FS_TYPE_LOCAL,
    CL_FS_TYPE_SSH,
    CL_FS_TYPE_FTP,
    CL_FS_TYPE_HTTP
} cl_fs_type_t;

// Filesystem configuration
typedef struct
{
    cl_fs_type_t type;
    const char *root_path;
    // Additional fields can be added for specific filesystem types
} cl_fs_config_t;

// File open modes
typedef enum
{
    CL_FILE_MODE_READ,
    CL_FILE_MODE_WRITE,
    CL_FILE_MODE_APPEND,
    CL_FILE_MODE_READ_WRITE
} cl_file_mode_t;

// File attributes
typedef enum
{
    CL_FILE_ATTRIBUTE_NORMAL = 0,
    CL_FILE_ATTRIBUTE_READONLY = 1 << 0,
    CL_FILE_ATTRIBUTE_HIDDEN = 1 << 1,
    CL_FILE_ATTRIBUTE_SYSTEM = 1 << 2,
    CL_FILE_ATTRIBUTE_DIRECTORY = 1 << 3,
    CL_FILE_ATTRIBUTE_ARCHIVE = 1 << 4,
} cl_file_attribute_t;

// Directory entry information
typedef struct
{
    const char *name;
    bool is_directory;
    i64 size;
    u64 last_write_time;
} cl_fs_dir_entry_t;

// Directory iterator
typedef struct cl_fs_dir_iterator cl_fs_dir_iterator_t;

// Filesystem operations
cl_fs_t *cl_fs_init(cl_allocator_t *allocator, const cl_fs_config_t *config);
void cl_fs_destroy(cl_fs_t *fs);

// Path operations
char *cl_fs_normalize_path(cl_fs_t *fs, const char *path);
char *cl_fs_denormalize_path(cl_fs_t *fs, const char *normalized_path);

// File operations
cl_file_t *cl_fs_open_file(cl_fs_t *fs, const char *path, cl_file_mode_t mode);
void cl_fs_close_file(cl_file_t *file);
u64 cl_fs_read_file(cl_file_t *file, void *buffer, u64 size);
u64 cl_fs_write_file(cl_file_t *file, const void *buffer, u64 size);
bool cl_fs_seek_file(cl_file_t *file, i64 offset, i32 origin);
i64 cl_fs_tell_file(cl_file_t *file);

// Directory operations
bool cl_fs_create_directory(cl_fs_t *fs, const char *path);
bool cl_fs_remove_directory(cl_fs_t *fs, const char *path);
bool cl_fs_directory_exists(cl_fs_t *fs, const char *path);

// File/directory management
bool cl_fs_remove_file(cl_fs_t *fs, const char *path);
bool cl_fs_rename(cl_fs_t *fs, const char *old_path, const char *new_path);
bool cl_fs_copy(cl_fs_t *fs, const char *src_path, const char *dest_path);
bool cl_fs_file_exists(cl_fs_t *fs, const char *path);
bool cl_fs_set_file_attributes(cl_fs_t *fs, const char *path, cl_file_attribute_t attributes);
cl_file_attribute_t cl_fs_get_file_attributes(cl_fs_t *fs, const char *path);

// File information
i64 cl_fs_get_file_size(cl_fs_t *fs, const char *path);
bool cl_fs_get_file_time(cl_fs_t *fs, const char *path, u64 *creation_time, u64 *last_access_time,
                         u64 *last_write_time);

// Directory listing
cl_fs_dir_iterator_t *cl_fs_open_directory(cl_fs_t *fs, const char *path);
bool cl_fs_read_directory(cl_fs_dir_iterator_t *iterator, cl_fs_dir_entry_t *entry);
void cl_fs_close_directory(cl_fs_dir_iterator_t *iterator);

// Error handling
const char *cl_fs_get_last_error(cl_fs_t *fs);
