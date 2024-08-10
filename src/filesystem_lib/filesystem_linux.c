/**
 * Abstract Filesystem Library - Linux Implementation
 * Created by Assistant on 8/5/2024.
 */

#include "clib/defines.h"
#if defined(CL_PLATFORM_LINUX)
#define _XOPEN_SOURCE 500
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "clib/log_lib.h"
#include "filesystem_internal.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef FTW_DEPTH
#define FTW_DEPTH 4
#endif

#ifndef FTW_PHYS
#define FTW_PHYS 1
#endif


bool cl_fs_platform_init(cl_fs_t *fs)
{
    // Apple-specific initialization (if needed)
    return true;
}

void cl_fs_platform_cleanup(cl_fs_t *fs)
{
    // Apple-specific cleanup (if needed)
}


char *cl_fs_platform_normalize_path(cl_fs_t *fs, const char *path)
{
    char *normalized = cl_mem_alloc(fs->allocator, PATH_MAX);
    if (normalized == null)
    {
        cl_fs_set_last_error(fs, "Failed to allocate memory for normalized path");
        return null;
    }

    char *resolved = realpath(path, normalized);
    if (resolved == null)
    {
        // If realpath fails, we'll implement our own normalization
        char *p = normalized;
        const char *s = path;
        size_t len = 0;

        // Handle absolute paths
        if (*s == '/')
        {
            *p++ = '/';
            len++;
            s++;
        }

        while (*s)
        {
            if (*s == '/')
            {
                s++;
                continue;
            }

            const char *next = s;
            while (*next && *next != '/')
                next++;

            if (next - s == 2 && s[0] == '.' && s[1] == '.')
            {
                // Handle ".."
                while (len > 0 && p[-1] != '/')
                {
                    p--;
                    len--;
                }
                if (len > 0)
                {
                    p--;
                    len--;
                }
            }
            else if (next - s == 1 && s[0] == '.')
            {
                // Skip "."
            }
            else
            {
                // Copy directory or file name
                while (s < next)
                {
                    *p++ = *s++;
                    len++;
                }
                if (*next)
                {
                    *p++ = '/';
                    len++;
                }
            }
            s = next;
        }

        // Remove trailing slash if not root
        if (len > 1 && p[-1] == '/')
            p--;

        *p = '\0';
    }

    return normalized;
}

char *cl_fs_platform_denormalize_path(cl_fs_t *fs, const char *normalized_path)
{
    // For Linux, we'll just return a copy of the normalized path
    // as denormalization is not typically needed on Unix-like systems
    char *denormalized = cl_mem_alloc(fs->allocator, strlen(normalized_path) + 1);
    if (denormalized == null)
    {
        cl_fs_set_last_error(fs, "Failed to allocate memory for denormalized path");
        return null;
    }
    strcpy(denormalized, normalized_path);
    return denormalized;
}

cl_file_t *cl_fs_platform_open_file(cl_fs_t *fs, const char *path, cl_file_mode_t mode)
{
    int flags = 0;
    mode_t create_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    switch (mode)
    {
    case CL_FILE_MODE_READ:
        flags = O_RDONLY;
        break;
    case CL_FILE_MODE_WRITE:
        flags = O_WRONLY | O_CREAT | O_TRUNC;
        break;
    case CL_FILE_MODE_APPEND:
        flags = O_WRONLY | O_CREAT | O_APPEND;
        break;
    case CL_FILE_MODE_READ_WRITE:
        flags = O_RDWR | O_CREAT;
        break;
    }

    int fd = open(path, flags, create_mode);
    if (fd == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return null;
    }

    cl_file_t *file = cl_mem_alloc(fs->allocator, sizeof(cl_file_t));
    if (file == null)
    {
        close(fd);
        cl_fs_set_last_error(fs, "Failed to allocate memory for file handle");
        return null;
    }

    file->fs = fs;
    file->handle = (void *)(intptr_t)fd;

    return file;
}

void cl_fs_platform_close_file(cl_file_t *file)
{
    if (file)
    {
        close((int)(intptr_t)file->handle);
        cl_mem_free(file->fs->allocator, file);
    }
}

u64 cl_fs_platform_read_file(cl_file_t *file, void *buffer, u64 size)
{
    ssize_t bytes_read = read((int)(intptr_t)file->handle, buffer, size);
    if (bytes_read == -1)
    {
        cl_fs_set_last_error(file->fs, strerror(errno));
        return 0;
    }
    return (u64)bytes_read;
}

u64 cl_fs_platform_write_file(cl_file_t *file, const void *buffer, u64 size)
{
    ssize_t bytes_written = write((int)(intptr_t)file->handle, buffer, size);
    if (bytes_written == -1)
    {
        cl_fs_set_last_error(file->fs, strerror(errno));
        return 0;
    }
    return (u64)bytes_written;
}

bool cl_fs_platform_seek_file(cl_file_t *file, i64 offset, i32 origin)
{
    if (lseek((int)(intptr_t)file->handle, offset, origin) == -1)
    {
        cl_fs_set_last_error(file->fs, strerror(errno));
        return false;
    }
    return true;
}

i64 cl_fs_platform_tell_file(cl_file_t *file)
{
    off_t position = lseek((int)(intptr_t)file->handle, 0, SEEK_CUR);
    if (position == -1)
    {
        cl_fs_set_last_error(file->fs, strerror(errno));
        return -1;
    }
    return (i64)position;
}

bool cl_fs_platform_create_directory(cl_fs_t *fs, const char *path)
{
    if (mkdir(path, 0755) == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }
    return true;
}


static int remove_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    (void)sb; // Unused parameter
    (void)typeflag; // Unused parameter
    (void)ftwbuf; // Unused parameter

    int status = remove(fpath);
    if (status != 0)
    {
        cl_log_error("Failed to remove %s: %s", fpath, strerror(errno));
    }
    return status;
}

bool cl_fs_platform_remove_directory(cl_fs_t *fs, const char *path)
{
    if (nftw(path, remove_callback, 64, FTW_DEPTH | FTW_PHYS) == -1)
    {
        int err = errno;
        cl_fs_set_last_error(fs, strerror(err));
        cl_log_error("Failed to remove directory %s: %s (errno: %d)", path, strerror(err), err);
        return false;
    }

    return true;
}

bool cl_fs_platform_directory_exists(cl_fs_t *fs, const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode))
    {
        return true;
    }
    if (errno != ENOENT)
    {
        cl_fs_set_last_error(fs, strerror(errno));
    }
    return false;
}

bool cl_fs_platform_remove_file(cl_fs_t *fs, const char *path)
{
    if (unlink(path) == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }
    return true;
}

bool cl_fs_platform_rename(cl_fs_t *fs, const char *old_path, const char *new_path)
{
    if (rename(old_path, new_path) == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }
    return true;
}

bool cl_fs_platform_copy(cl_fs_t *fs, const char *src_path, const char *dest_path)
{
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }

    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1)
    {
        close(src_fd);
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }

    char buffer[8192];
    ssize_t bytes_read, bytes_written;
    bool success = true;

    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0)
    {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read)
        {
            cl_fs_set_last_error(fs, "Failed to write to destination file");
            success = false;
            break;
        }
    }

    if (bytes_read == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        success = false;
    }

    close(src_fd);
    close(dest_fd);
    return success;
}


bool cl_fs_platform_file_exists(cl_fs_t *fs, const char *path)
{
    (void)fs; // Unused parameter
    struct stat st;
    return (stat(path, &st) == 0 && S_ISREG(st.st_mode));
}

i64 cl_fs_platform_get_file_size(cl_fs_t *fs, const char *path)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        return (i64)st.st_size;
    }
    cl_fs_set_last_error(fs, strerror(errno));
    return -1;
}

bool cl_fs_platform_get_file_time(cl_fs_t *fs, const char *path, u64 *creation_time, u64 *last_access_time,
                                  u64 *last_write_time)
{
    struct stat st;
    if (stat(path, &st) == 0)
    {
        if (creation_time)
            *creation_time = (u64)st.st_ctime; // Note: This is change time, not creation time
        if (last_access_time)
            *last_access_time = (u64)st.st_atime;
        if (last_write_time)
            *last_write_time = (u64)st.st_mtime;
        return true;
    }
    cl_fs_set_last_error(fs, strerror(errno));
    return false;
}

cl_fs_dir_iterator_t *cl_fs_platform_open_directory(cl_fs_t *fs, const char *path)
{
    DIR *dir = opendir(path);
    if (dir == null)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return null;
    }

    cl_fs_dir_iterator_t *iterator = cl_mem_alloc(fs->allocator, sizeof(cl_fs_dir_iterator_t));
    if (iterator == null)
    {
        closedir(dir);
        cl_fs_set_last_error(fs, "Failed to allocate memory for directory iterator");
        return null;
    }

    iterator->fs = fs;
    iterator->handle = dir;

    iterator->path = cl_mem_alloc(fs->allocator, strlen(path) + 1);
    if (iterator->path == null)
    {
        closedir(dir);
        cl_mem_free(fs->allocator, iterator);
        cl_fs_set_last_error(fs, "Failed to allocate memory for directory path");
        return null;
    }
    strcpy(iterator->path, path);

    return iterator;
}

bool cl_fs_platform_read_directory(cl_fs_dir_iterator_t *iterator, cl_fs_dir_entry_t *entry)
{
    struct dirent *dir_entry;
    errno = 0;
    if ((dir_entry = readdir((DIR *)iterator->handle)) == null)
    {
        if (errno != 0)
        {
            cl_fs_set_last_error(iterator->fs, strerror(errno));
        }
        return false;
    }

    entry->name = dir_entry->d_name;
    entry->is_directory = (dir_entry->d_type == DT_DIR);

    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/%s", iterator->path, entry->name);

    struct stat st;
    if (stat(full_path, &st) == 0)
    {
        entry->size = (i64)st.st_size;
        entry->last_write_time = (u64)st.st_mtime;
    }
    else
    {
        cl_fs_set_last_error(iterator->fs, strerror(errno));
        entry->size = -1;
        entry->last_write_time = 0;
    }

    return true;
}


void cl_fs_platform_close_directory(cl_fs_dir_iterator_t *iterator)
{
    if (iterator)
    {
        closedir((DIR *)iterator->handle);
        cl_mem_free(iterator->fs->allocator, iterator->path);
        cl_mem_free(iterator->fs->allocator, iterator);
    }
}

bool cl_fs_platform_set_file_attributes(cl_fs_t *fs, const char *path, cl_file_attribute_t attributes)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }

    mode_t mode = st.st_mode;

    if (attributes & CL_FILE_ATTRIBUTE_READONLY)
    {
        mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    }
    else
    {
        mode |= S_IWUSR;
    }

    if (chmod(path, mode) == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return false;
    }

    return true;
}

cl_file_attribute_t cl_fs_platform_get_file_attributes(cl_fs_t *fs, const char *path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        cl_fs_set_last_error(fs, strerror(errno));
        return CL_FILE_ATTRIBUTE_NORMAL;
    }

    cl_file_attribute_t attributes = CL_FILE_ATTRIBUTE_NORMAL;

    if (!(st.st_mode & S_IWUSR))
    {
        attributes |= CL_FILE_ATTRIBUTE_READONLY;
    }

    if (S_ISDIR(st.st_mode))
    {
        attributes |= CL_FILE_ATTRIBUTE_DIRECTORY;
    }

    return attributes;
}

#endif // CL_PLATFORM_LINUX
