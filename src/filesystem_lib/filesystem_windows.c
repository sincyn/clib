/**
 * Abstract Filesystem Library - Windows Implementation
 * Created by Claude on 8/5/2024.
 */

#include "clib/defines.h"
#if defined(CL_PLATFORM_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <shlwapi.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#include "clib/log_lib.h"
#include "filesystem_internal.h"
#pragma comment(lib, "Shlwapi.lib")

bool cl_fs_platform_init(cl_fs_t *fs)
{
    // Windows-specific initialization (if needed)
    return true;
}

void cl_fs_platform_cleanup(cl_fs_t *fs)
{
    // Windows-specific cleanup (if needed)
}

char *cl_fs_platform_normalize_path(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    WCHAR full_path[MAX_PATH];
    char *normalized;

    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);
    if (GetFullPathNameW(wide_path, MAX_PATH, full_path, null) == 0)
    {
        cl_fs_set_last_error(fs, "Failed to normalize path");
        return null;
    }

    normalized = cl_mem_alloc(fs->allocator, MAX_PATH);
    if (normalized == null)
    {
        cl_fs_set_last_error(fs, "Failed to allocate memory for normalized path");
        return null;
    }

    WideCharToMultiByte(CP_UTF8, 0, full_path, -1, normalized, MAX_PATH, null, null);
    return normalized;
}

char *cl_fs_platform_denormalize_path(cl_fs_t *fs, const char *normalized_path)
{
    // For Windows, we'll just return a copy of the normalized path
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
    DWORD access = 0;
    DWORD creation = 0;
    HANDLE handle;

    switch (mode)
    {
    case CL_FILE_MODE_READ:
        access = GENERIC_READ;
        creation = OPEN_EXISTING;
        break;
    case CL_FILE_MODE_WRITE:
        access = GENERIC_WRITE;
        creation = CREATE_ALWAYS;
        break;
    case CL_FILE_MODE_APPEND:
        access = FILE_APPEND_DATA;
        creation = OPEN_ALWAYS;
        break;
    case CL_FILE_MODE_READ_WRITE:
        access = GENERIC_READ | GENERIC_WRITE;
        creation = OPEN_ALWAYS;
        break;
    }

    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    handle = CreateFileW(wide_path, access, FILE_SHARE_READ, null, creation, FILE_ATTRIBUTE_NORMAL, null);
    if (handle == INVALID_HANDLE_VALUE)
    {
        cl_fs_set_last_error(fs, "Failed to open file");
        return null;
    }

    cl_file_t *file = cl_mem_alloc(fs->allocator, sizeof(cl_file_t));
    if (file == null)
    {
        CloseHandle(handle);
        cl_fs_set_last_error(fs, "Failed to allocate memory for file handle");
        return null;
    }

    file->fs = fs;
    file->handle = handle;

    return file;
}

void cl_fs_platform_close_file(cl_file_t *file)
{
    if (file)
    {
        CloseHandle(file->handle);
        cl_mem_free(file->fs->allocator, file);
    }
}

u64 cl_fs_platform_read_file(cl_file_t *file, void *buffer, u64 size)
{
    DWORD bytes_read;
    if (!ReadFile(file->handle, buffer, (DWORD)size, &bytes_read, null))
    {
        cl_fs_set_last_error(file->fs, "Failed to read file");
        return 0;
    }
    return bytes_read;
}

u64 cl_fs_platform_write_file(cl_file_t *file, const void *buffer, u64 size)
{
    DWORD bytes_written;
    if (!WriteFile(file->handle, buffer, (DWORD)size, &bytes_written, null))
    {
        cl_fs_set_last_error(file->fs, "Failed to write file");
        return 0;
    }
    return bytes_written;
}

bool cl_fs_platform_seek_file(cl_file_t *file, i64 offset, i32 origin)
{
    LARGE_INTEGER li;
    li.QuadPart = offset;
    if (SetFilePointerEx(file->handle, li, null, origin) == 0)
    {
        cl_fs_set_last_error(file->fs, "Failed to seek file");
        return false;
    }
    return true;
}

i64 cl_fs_platform_tell_file(cl_file_t *file)
{
    LARGE_INTEGER li;
    li.QuadPart = 0;
    if (SetFilePointerEx(file->handle, li, &li, FILE_CURRENT) == 0)
    {
        cl_fs_set_last_error(file->fs, "Failed to get file position");
        return -1;
    }
    return li.QuadPart;
}

bool cl_fs_platform_create_directory(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    if (!CreateDirectoryW(wide_path, null))
    {
        cl_fs_set_last_error(fs, "Failed to create directory");
        return false;
    }
    return true;
}

bool cl_fs_platform_remove_directory(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    if (!RemoveDirectoryW(wide_path))
    {
        cl_fs_set_last_error(fs, "Failed to remove directory");
        return false;
    }
    return true;
}

bool cl_fs_platform_directory_exists(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    DWORD attrs = GetFileAttributesW(wide_path);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
}

bool cl_fs_platform_remove_file(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    if (!DeleteFileW(wide_path))
    {
        cl_fs_set_last_error(fs, "Failed to remove file");
        return false;
    }
    return true;
}

bool cl_fs_platform_rename(cl_fs_t *fs, const char *old_path, const char *new_path)
{
    WCHAR wide_old_path[MAX_PATH];
    WCHAR wide_new_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, old_path, -1, wide_old_path, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, new_path, -1, wide_new_path, MAX_PATH);

    if (!MoveFileW(wide_old_path, wide_new_path))
    {
        cl_fs_set_last_error(fs, "Failed to rename file/directory");
        return false;
    }
    return true;
}

bool cl_fs_platform_copy(cl_fs_t *fs, const char *src_path, const char *dest_path)
{
    WCHAR wide_src_path[MAX_PATH];
    WCHAR wide_dest_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, src_path, -1, wide_src_path, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, dest_path, -1, wide_dest_path, MAX_PATH);

    if (!CopyFileW(wide_src_path, wide_dest_path, FALSE))
    {
        cl_fs_set_last_error(fs, "Failed to copy file");
        return false;
    }
    return true;
}

bool cl_fs_platform_file_exists(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    DWORD attrs = GetFileAttributesW(wide_path);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

i64 cl_fs_platform_get_file_size(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    WIN32_FILE_ATTRIBUTE_DATA file_info;
    if (!GetFileAttributesExW(wide_path, GetFileExInfoStandard, &file_info))
    {
        cl_fs_set_last_error(fs, "Failed to get file size");
        return -1;
    }

    LARGE_INTEGER size;
    size.LowPart = file_info.nFileSizeLow;
    size.HighPart = file_info.nFileSizeHigh;
    return size.QuadPart;
}

bool cl_fs_platform_get_file_time(cl_fs_t *fs, const char *path, u64 *creation_time, u64 *last_access_time,
                                  u64 *last_write_time)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    WIN32_FILE_ATTRIBUTE_DATA file_info;
    if (!GetFileAttributesExW(wide_path, GetFileExInfoStandard, &file_info))
    {
        cl_fs_set_last_error(fs, "Failed to get file time");
        return false;
    }

    if (creation_time)
        *creation_time = ((u64)file_info.ftCreationTime.dwHighDateTime << 32) | file_info.ftCreationTime.dwLowDateTime;
    if (last_access_time)
        *last_access_time =
            ((u64)file_info.ftLastAccessTime.dwHighDateTime << 32) | file_info.ftLastAccessTime.dwLowDateTime;
    if (last_write_time)
        *last_write_time =
            ((u64)file_info.ftLastWriteTime.dwHighDateTime << 32) | file_info.ftLastWriteTime.dwLowDateTime;

    return true;
}

cl_fs_dir_iterator_t *cl_fs_platform_open_directory(cl_fs_t *fs, const char *path)
{
    cl_fs_dir_iterator_t *iterator = cl_mem_alloc(fs->allocator, sizeof(cl_fs_dir_iterator_t));
    if (iterator == null)
    {
        cl_fs_set_last_error(fs, "Failed to allocate memory for directory iterator");
        return null;
    }

    iterator->fs = fs;
    iterator->handle = null;
    iterator->path = cl_mem_alloc(fs->allocator, strlen(path) + 3); // +3 for "\*" and null terminator
    if (iterator->path == null)
    {
        cl_mem_free(fs->allocator, iterator);
        cl_fs_set_last_error(fs, "Failed to allocate memory for directory path");
        return null;
    }

    sprintf(iterator->path, "%s\\*", path);
    return iterator;
}

cl_fs_dir_entry_t *cl_fs_platform_current_working_directory(cl_fs_t *fs)
{
    char *path = cl_mem_alloc(fs->allocator, MAX_PATH);
    if (path == NULL)
    {
        cl_fs_set_last_error(fs, "Failed to allocate memory for current working directory");
        return NULL;
    }

    // Gets the current path on windows and converts t
    GetCurrentDirectory(MAX_PATH, path);
    // normalizes it
    char *normalized_path = cl_fs_platform_normalize_path(fs, path);
    if (normalized_path == NULL)
    {
        cl_mem_free(fs->allocator, path);
        return NULL;
    }

    cl_fs_dir_entry_t *entry = cl_mem_alloc(fs->allocator, sizeof(cl_fs_dir_entry_t));
    if (entry == NULL)
    {
        cl_fs_set_last_error(fs, "Failed to allocate memory for directory entry");
        cl_mem_free(fs->allocator, path);
        cl_mem_free(fs->allocator, normalized_path);
        return NULL;
    }

    entry->name = normalized_path;
    entry->is_directory = true;
    entry->size = 0;
    entry->last_write_time = 0;

    return entry;
}

bool cl_fs_platform_read_directory(cl_fs_dir_iterator_t *iterator, cl_fs_dir_entry_t *entry)
{
    WIN32_FIND_DATAW find_data;

    if (iterator->handle == null)
    {
        WCHAR wide_path[MAX_PATH];
        MultiByteToWideChar(CP_UTF8, 0, iterator->path, -1, wide_path, MAX_PATH);
        iterator->handle = FindFirstFileW(wide_path, &find_data);
        if (iterator->handle == INVALID_HANDLE_VALUE)
        {
            cl_fs_set_last_error(iterator->fs, "Failed to open directory for iteration");
            return false;
        }
    }
    else
    {
        if (!FindNextFileW(iterator->handle, &find_data))
        {
            if (GetLastError() == ERROR_NO_MORE_FILES)
                return false;
            cl_fs_set_last_error(iterator->fs, "Failed to read directory entry");
            return false;
        }
    }

    static char entry_name[MAX_PATH];
    WideCharToMultiByte(CP_UTF8, 0, find_data.cFileName, -1, entry_name, MAX_PATH, null, null);
    entry->name = entry_name;
    entry->is_directory = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    entry->size = ((i64)find_data.nFileSizeHigh << 32) | find_data.nFileSizeLow;
    entry->last_write_time =
        ((u64)find_data.ftLastWriteTime.dwHighDateTime << 32) | find_data.ftLastWriteTime.dwLowDateTime;

    return true;
}

void cl_fs_platform_close_directory(cl_fs_dir_iterator_t *iterator)
{
    if (iterator)
    {
        if (iterator->handle != null && iterator->handle != INVALID_HANDLE_VALUE)
        {
            FindClose(iterator->handle);
        }
        cl_mem_free(iterator->fs->allocator, iterator->path);
        cl_mem_free(iterator->fs->allocator, iterator);
    }
}

bool cl_fs_platform_set_file_attributes(cl_fs_t *fs, const char *path, cl_file_attribute_t attributes)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    DWORD win_attributes = FILE_ATTRIBUTE_NORMAL;

    if (attributes & CL_FILE_ATTRIBUTE_READONLY)
        win_attributes |= FILE_ATTRIBUTE_READONLY;
    if (attributes & CL_FILE_ATTRIBUTE_HIDDEN)
        win_attributes |= FILE_ATTRIBUTE_HIDDEN;
    if (attributes & CL_FILE_ATTRIBUTE_SYSTEM)
        win_attributes |= FILE_ATTRIBUTE_SYSTEM;
    if (attributes & CL_FILE_ATTRIBUTE_DIRECTORY)
        win_attributes |= FILE_ATTRIBUTE_DIRECTORY;
    if (attributes & CL_FILE_ATTRIBUTE_ARCHIVE)
        win_attributes |= FILE_ATTRIBUTE_ARCHIVE;

    if (!SetFileAttributesW(wide_path, win_attributes))
    {
        cl_fs_set_last_error(fs, "Failed to set file attributes");
        return false;
    }

    return true;
}

cl_file_attribute_t cl_fs_platform_get_file_attributes(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    DWORD win_attributes = GetFileAttributesW(wide_path);
    if (win_attributes == INVALID_FILE_ATTRIBUTES)
    {
        cl_fs_set_last_error(fs, "Failed to get file attributes");
        return CL_FILE_ATTRIBUTE_NORMAL;
    }

    cl_file_attribute_t attributes = CL_FILE_ATTRIBUTE_NORMAL;

    if (win_attributes & FILE_ATTRIBUTE_READONLY)
        attributes |= CL_FILE_ATTRIBUTE_READONLY;
    if (win_attributes & FILE_ATTRIBUTE_HIDDEN)
        attributes |= CL_FILE_ATTRIBUTE_HIDDEN;
    if (win_attributes & FILE_ATTRIBUTE_SYSTEM)
        attributes |= CL_FILE_ATTRIBUTE_SYSTEM;
    if (win_attributes & FILE_ATTRIBUTE_DIRECTORY)
        attributes |= CL_FILE_ATTRIBUTE_DIRECTORY;
    if (win_attributes & FILE_ATTRIBUTE_ARCHIVE)
        attributes |= CL_FILE_ATTRIBUTE_ARCHIVE;

    return attributes;
}

// Helper function to convert Windows error codes to string messages
static void set_last_error_from_windows(cl_fs_t *fs)
{
    DWORD error_code = GetLastError();
    LPVOID error_msg;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, null,
                  error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&error_msg, 0, null);

    cl_fs_set_last_error(fs, (const char *)error_msg);
    LocalFree(error_msg);
}

// Additional helper functions

static FILETIME u64_to_filetime(u64 time)
{
    FILETIME ft;
    ft.dwLowDateTime = (DWORD)(time & 0xFFFFFFFF);
    ft.dwHighDateTime = (DWORD)(time >> 32);
    return ft;
}

static u64 filetime_to_u64(FILETIME ft) { return ((u64)ft.dwHighDateTime << 32) | ft.dwLowDateTime; }

// Function to convert Windows path separators to Unix-style
static void convert_path_separators(char *path)
{
    while (*path)
    {
        if (*path == '\\')
            *path = '/';
        path++;
    }
}

// Additional platform-specific functions that might be useful

bool cl_fs_win_get_current_directory(cl_fs_t *fs, char *buffer, size_t size)
{
    WCHAR wide_buffer[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, wide_buffer) == 0)
    {
        set_last_error_from_windows(fs);
        return false;
    }

    WideCharToMultiByte(CP_UTF8, 0, wide_buffer, -1, buffer, (int)size, null, null);
    convert_path_separators(buffer);
    return true;
}

bool cl_fs_win_set_current_directory(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    if (!SetCurrentDirectoryW(wide_path))
    {
        set_last_error_from_windows(fs);
        return false;
    }
    return true;
}

bool cl_fs_win_get_temp_path(cl_fs_t *fs, char *buffer, size_t size)
{
    WCHAR wide_buffer[MAX_PATH];
    if (GetTempPathW(MAX_PATH, wide_buffer) == 0)
    {
        set_last_error_from_windows(fs);
        return false;
    }

    WideCharToMultiByte(CP_UTF8, 0, wide_buffer, -1, buffer, (int)size, null, null);
    convert_path_separators(buffer);
    return true;
}

bool cl_fs_win_create_hard_link(cl_fs_t *fs, const char *existing_file, const char *new_file)
{
    WCHAR wide_existing_file[MAX_PATH];
    WCHAR wide_new_file[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, existing_file, -1, wide_existing_file, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, new_file, -1, wide_new_file, MAX_PATH);

    if (!CreateHardLinkW(wide_new_file, wide_existing_file, null))
    {
        set_last_error_from_windows(fs);
        return false;
    }
    return true;
}

bool cl_fs_win_create_symbolic_link(cl_fs_t *fs, const char *symlink_file, const char *target_file, bool is_directory)
{
    WCHAR wide_symlink_file[MAX_PATH];
    WCHAR wide_target_file[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, symlink_file, -1, wide_symlink_file, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, target_file, -1, wide_target_file, MAX_PATH);

    DWORD flags = is_directory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0;

    if (!CreateSymbolicLinkW(wide_symlink_file, wide_target_file, flags))
    {
        set_last_error_from_windows(fs);
        return false;
    }
    return true;
}

// Function to get the Windows-specific file attributes
DWORD cl_fs_win_get_file_attributes_raw(cl_fs_t *fs, const char *path)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    DWORD attributes = GetFileAttributesW(wide_path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        set_last_error_from_windows(fs);
    }
    return attributes;
}

// Function to set the Windows-specific file attributes
bool cl_fs_win_set_file_attributes_raw(cl_fs_t *fs, const char *path, DWORD attributes)
{
    WCHAR wide_path[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, path, -1, wide_path, MAX_PATH);

    if (!SetFileAttributesW(wide_path, attributes))
    {
        set_last_error_from_windows(fs);
        return false;
    }
    return true;
}

#endif // CL_PLATFORM_WINDOWS
