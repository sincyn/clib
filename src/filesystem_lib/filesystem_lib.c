/**
 * Abstract Filesystem Library Implementation
 * Created by Claude on 8/5/2024.
 * Updated by Assistant on 8/5/2024.
 */

#include "filesystem_internal.h"
#include "clib/log_lib.h"
#include <string.h>

cl_fs_t *cl_fs_init(cl_allocator_t *allocator, const cl_fs_config_t *config) {
    if (allocator == null || config == null) {
        cl_log_error("Invalid allocator or config provided to cl_fs_init");
        return null;
    }

    cl_fs_t *fs = cl_mem_alloc(allocator, sizeof(cl_fs_t));
    if (fs == null) {
        cl_log_error("Failed to allocate memory for filesystem");
        return null;
    }

    fs->allocator = allocator;
    fs->config = *config;
    fs->last_error[0] = '\0';
    fs->platform_data = null;

    if (!cl_fs_platform_init(fs)) {
        cl_mem_free(allocator, fs);
        return null;
    }

    cl_log_debug("Filesystem initialized with type: %d", config->type);
    return fs;
}

void cl_fs_destroy(cl_fs_t *fs) {
    if (fs) {
        cl_fs_platform_cleanup(fs);
        cl_mem_free(fs->allocator, fs);
        cl_log_debug("Filesystem destroyed");
    }
}

char *cl_fs_normalize_path(cl_fs_t *fs, const char *path) {
    return cl_fs_platform_normalize_path(fs, path);
}

char *cl_fs_denormalize_path(cl_fs_t *fs, const char *normalized_path) {
    return cl_fs_platform_denormalize_path(fs, normalized_path);
}

cl_file_t *cl_fs_open_file(cl_fs_t *fs, const char *path, cl_file_mode_t mode) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return null;
    }
    cl_file_t *file = cl_fs_platform_open_file(fs, normalized_path, mode);
    cl_mem_free(fs->allocator, normalized_path);
    return file;
}

void cl_fs_close_file(cl_file_t *file) {
    cl_fs_platform_close_file(file);
}

u64 cl_fs_read_file(cl_file_t *file, void *buffer, u64 size) {
    return cl_fs_platform_read_file(file, buffer, size);
}

u64 cl_fs_write_file(cl_file_t *file, const void *buffer, u64 size) {
    return cl_fs_platform_write_file(file, buffer, size);
}

bool cl_fs_seek_file(cl_file_t *file, i64 offset, i32 origin) {
    return cl_fs_platform_seek_file(file, offset, origin);
}

i64 cl_fs_tell_file(cl_file_t *file) {
    return cl_fs_platform_tell_file(file);
}

bool cl_fs_create_directory(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_create_directory(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

bool cl_fs_remove_directory(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_remove_directory(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

bool cl_fs_directory_exists(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_directory_exists(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

bool cl_fs_remove_file(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_remove_file(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

bool cl_fs_rename(cl_fs_t *fs, const char *old_path, const char *new_path) {
    char *normalized_old_path = cl_fs_normalize_path(fs, old_path);
    char *normalized_new_path = cl_fs_normalize_path(fs, new_path);
    if (normalized_old_path == null || normalized_new_path == null) {
        cl_mem_free(fs->allocator, normalized_old_path);
        cl_mem_free(fs->allocator, normalized_new_path);
        return false;
    }
    bool result = cl_fs_platform_rename(fs, normalized_old_path, normalized_new_path);
    cl_mem_free(fs->allocator, normalized_old_path);
    cl_mem_free(fs->allocator, normalized_new_path);
    return result;
}

bool cl_fs_copy(cl_fs_t *fs, const char *src_path, const char *dest_path) {
    char *normalized_src_path = cl_fs_normalize_path(fs, src_path);
    char *normalized_dest_path = cl_fs_normalize_path(fs, dest_path);
    if (normalized_src_path == null || normalized_dest_path == null) {
        cl_mem_free(fs->allocator, normalized_src_path);
        cl_mem_free(fs->allocator, normalized_dest_path);
        return false;
    }
    bool result = cl_fs_platform_copy(fs, normalized_src_path, normalized_dest_path);
    cl_mem_free(fs->allocator, normalized_src_path);
    cl_mem_free(fs->allocator, normalized_dest_path);
    return result;
}

bool cl_fs_file_exists(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_file_exists(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

i64 cl_fs_get_file_size(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return -1;
    }
    i64 result = cl_fs_platform_get_file_size(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

bool cl_fs_get_file_time(cl_fs_t *fs, const char *path, u64 *creation_time, u64 *last_access_time, u64 *last_write_time) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_get_file_time(fs, normalized_path, creation_time, last_access_time, last_write_time);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

cl_fs_dir_iterator_t *cl_fs_open_directory(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return null;
    }
    cl_fs_dir_iterator_t *iterator = cl_fs_platform_open_directory(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return iterator;
}

bool cl_fs_read_directory(cl_fs_dir_iterator_t *iterator, cl_fs_dir_entry_t *entry) {
    return cl_fs_platform_read_directory(iterator, entry);
}

void cl_fs_close_directory(cl_fs_dir_iterator_t *iterator) {
    cl_fs_platform_close_directory(iterator);
}

bool cl_fs_set_file_attributes(cl_fs_t *fs, const char *path, cl_file_attribute_t attributes) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return false;
    }
    bool result = cl_fs_platform_set_file_attributes(fs, normalized_path, attributes);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

cl_file_attribute_t cl_fs_get_file_attributes(cl_fs_t *fs, const char *path) {
    char *normalized_path = cl_fs_normalize_path(fs, path);
    if (normalized_path == null) {
        return CL_FILE_ATTRIBUTE_NORMAL;
    }
    cl_file_attribute_t result = cl_fs_platform_get_file_attributes(fs, normalized_path);
    cl_mem_free(fs->allocator, normalized_path);
    return result;
}

const char *cl_fs_get_last_error(cl_fs_t *fs) {
    return fs ? fs->last_error : "Invalid filesystem handle";
}

void cl_fs_set_last_error(cl_fs_t *fs, const char *error) {
    if (fs) {
        strncpy(fs->last_error, error, MAX_ERROR_LENGTH - 1);
        fs->last_error[MAX_ERROR_LENGTH - 1] = '\0';
    }
}