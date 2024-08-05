#include <stdio.h>

/**
 * Comprehensive Filesystem Library Tests
 * Created by Assistant on 8/5/2024.
 */

#include <string.h>
#include <unistd.h>
#include "clib/filesystem_lib.h"
#include "clib/log_lib.h"
#include "clib/memory_lib.h"
#include "clib/test_lib.h"

static cl_fs_t *fs;
static cl_allocator_t *allocator;

#define TEST_DIR "test_filesystem"
#define TEST_FILE "test_file.txt"
#define TEST_FILE_CONTENT "Hello, Filesystem!"
#define LARGE_FILE_SIZE (10 * 1024 * 1024) // 10 MB

void setup() {
    allocator = cl_allocator_create(&(cl_allocator_config_t){.type = CL_ALLOCATOR_TYPE_PLATFORM});
    cl_fs_config_t config = {.type = CL_FS_TYPE_LOCAL, .root_path = "."};
    fs = cl_fs_init(allocator, &config);
    CL_ASSERT_NOT_NULL(fs);
}

void teardown() {
    cl_fs_destroy(fs);
    cl_allocator_destroy(allocator);
}

CL_TEST(test_directory_operations) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        CL_LOG_ERROR("Failed to get current working directory");
        return;
    }
    CL_LOG_INFO("Current working directory: %s", cwd);

    char full_test_dir[1024];
    snprintf(full_test_dir, sizeof(full_test_dir), "%s/%s", cwd, TEST_DIR);

    // Create directory
    bool result = cl_fs_create_directory(fs, full_test_dir);
    if (!result) {
        CL_LOG_ERROR("Failed to create directory: %s", cl_fs_get_last_error(fs));
    }
    CL_ASSERT(result);

    result = cl_fs_directory_exists(fs, full_test_dir);
    if (!result) {
        CL_LOG_ERROR("Directory does not exist after creation: %s", cl_fs_get_last_error(fs));
    }
    CL_ASSERT(result);

    // Create nested directory
    char nested_dir[1024];
    snprintf(nested_dir, sizeof(nested_dir), "%s/nested", full_test_dir);
    result = cl_fs_create_directory(fs, nested_dir);
    if (!result) {
        CL_LOG_ERROR("Failed to create nested directory: %s", cl_fs_get_last_error(fs));
    }
    CL_ASSERT(result);

    result = cl_fs_directory_exists(fs, nested_dir);
    if (!result) {
        CL_LOG_ERROR("Nested directory does not exist after creation: %s", cl_fs_get_last_error(fs));
    }
    CL_ASSERT(result);

    // Remove nested directory
    result = cl_fs_remove_directory(fs, nested_dir);
    if (!result) {
        CL_LOG_ERROR("Failed to remove nested directory: %s", cl_fs_get_last_error(fs));
    }
    CL_ASSERT(result);

    result = cl_fs_directory_exists(fs, nested_dir);
    if (result) {
        CL_LOG_ERROR("Nested directory still exists after removal");
    }
    CL_ASSERT(!result);

    // Remove main directory
    result = cl_fs_remove_directory(fs, full_test_dir);
    if (!result) {
        CL_LOG_ERROR("Failed to remove main directory: %s", cl_fs_get_last_error(fs));
    }
    CL_ASSERT(result);

    result = cl_fs_directory_exists(fs, full_test_dir);
    if (result) {
        CL_LOG_ERROR("Main directory still exists after removal");
    }
    CL_ASSERT(!result);
}

CL_TEST(test_file_operations) {
    // Write file
    cl_file_t *file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_WRITE);
    CL_ASSERT_NOT_NULL(file);
    u64 written = cl_fs_write_file(file, TEST_FILE_CONTENT, strlen(TEST_FILE_CONTENT));
    CL_ASSERT_EQUAL(written, strlen(TEST_FILE_CONTENT));
    cl_fs_close_file(file);

    // Read file
    file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_READ);
    CL_ASSERT_NOT_NULL(file);
    char buffer[256] = {0};
    u64 read = cl_fs_read_file(file, buffer, sizeof(buffer));
    CL_ASSERT_EQUAL(read, strlen(TEST_FILE_CONTENT));
    CL_ASSERT_STRING_EQUAL(buffer, TEST_FILE_CONTENT);
    cl_fs_close_file(file);

    // Append to file
    file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_APPEND);
    CL_ASSERT_NOT_NULL(file);
    written = cl_fs_write_file(file, " Appended content.", 18);
    CL_ASSERT_EQUAL(written, 18);
    cl_fs_close_file(file);

    // Read appended file
    file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_READ);
    CL_ASSERT_NOT_NULL(file);
    memset(buffer, 0, sizeof(buffer));
    read = cl_fs_read_file(file, buffer, sizeof(buffer));
    CL_ASSERT_EQUAL(read, strlen(TEST_FILE_CONTENT) + 18);
    CL_ASSERT_STRING_EQUAL(buffer, "Hello, Filesystem! Appended content.");
    cl_fs_close_file(file);

    // Remove file
    bool result = cl_fs_remove_file(fs, TEST_FILE);
    CL_ASSERT(result);
    CL_ASSERT(!cl_fs_file_exists(fs, TEST_FILE));
}

CL_TEST(test_file_seek_tell) {
    cl_file_t *file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_WRITE);
    CL_ASSERT_NOT_NULL(file);
    cl_fs_write_file(file, TEST_FILE_CONTENT, strlen(TEST_FILE_CONTENT));
    cl_fs_close_file(file);

    file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_READ);
    CL_ASSERT_NOT_NULL(file);

    // Seek from start
    bool seek_result = cl_fs_seek_file(file, 7, SEEK_SET);
    CL_ASSERT(seek_result);
    i64 position = cl_fs_tell_file(file);
    CL_ASSERT_EQUAL(position, 7);

    // Read from current position
    char buffer[256] = {0};
    u64 read = cl_fs_read_file(file, buffer, sizeof(buffer));
    CL_ASSERT_EQUAL(read, strlen(TEST_FILE_CONTENT) - 7);
    CL_ASSERT_STRING_EQUAL(buffer, "Filesystem!");

    // Seek from current position
    seek_result = cl_fs_seek_file(file, -5, SEEK_CUR);
    CL_ASSERT(seek_result);
    position = cl_fs_tell_file(file);
    CL_ASSERT_EQUAL(position, strlen(TEST_FILE_CONTENT) - 5);

    // Seek from end
    seek_result = cl_fs_seek_file(file, -6, SEEK_END);
    CL_ASSERT(seek_result);
    position = cl_fs_tell_file(file);
    CL_ASSERT_EQUAL(position, strlen(TEST_FILE_CONTENT) - 6);

    cl_fs_close_file(file);
    cl_fs_remove_file(fs, TEST_FILE);
}

CL_TEST(test_rename_copy) {
    cl_file_t *file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_WRITE);
    CL_ASSERT_NOT_NULL(file);
    cl_fs_write_file(file, TEST_FILE_CONTENT, strlen(TEST_FILE_CONTENT));
    cl_fs_close_file(file);

    const char *new_name = "renamed_file.txt";
    bool rename_result = cl_fs_rename(fs, TEST_FILE, new_name);
    CL_ASSERT(rename_result);
    CL_ASSERT(!cl_fs_file_exists(fs, TEST_FILE));
    CL_ASSERT(cl_fs_file_exists(fs, new_name));

    const char *copy_name = "copied_file.txt";
    bool copy_result = cl_fs_copy(fs, new_name, copy_name);
    CL_ASSERT(copy_result);
    CL_ASSERT(cl_fs_file_exists(fs, copy_name));

    cl_fs_remove_file(fs, new_name);
    cl_fs_remove_file(fs, copy_name);
}

CL_TEST(test_file_info) {
    cl_file_t *file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_WRITE);
    CL_ASSERT_NOT_NULL(file);
    cl_fs_write_file(file, TEST_FILE_CONTENT, strlen(TEST_FILE_CONTENT));
    cl_fs_close_file(file);

    i64 size = cl_fs_get_file_size(fs, TEST_FILE);
    CL_ASSERT_EQUAL(size, strlen(TEST_FILE_CONTENT));

    u64 creation_time, last_access_time, last_write_time;
    bool time_result = cl_fs_get_file_time(fs, TEST_FILE, &creation_time, &last_access_time, &last_write_time);
    CL_ASSERT(time_result);
    CL_ASSERT(creation_time > 0);
    CL_ASSERT(last_access_time > 0);
    CL_ASSERT(last_write_time > 0);

    cl_fs_remove_file(fs, TEST_FILE);
}

CL_TEST(test_directory_listing) {
    cl_fs_create_directory(fs, TEST_DIR);

    const char *files[] = {"file1.txt", "file2.txt", "file3.txt"};
    for (int i = 0; i < 3; i++) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", TEST_DIR, files[i]);
        cl_file_t *file = cl_fs_open_file(fs, path, CL_FILE_MODE_WRITE);
        cl_fs_write_file(file, TEST_FILE_CONTENT, strlen(TEST_FILE_CONTENT));
        cl_fs_close_file(file);
    }

    cl_fs_dir_iterator_t *iterator = cl_fs_open_directory(fs, TEST_DIR);
    CL_ASSERT_NOT_NULL(iterator);

    int file_count = 0;
    cl_fs_dir_entry_t entry;
    while (cl_fs_read_directory(iterator, &entry)) {
        if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) {
            continue;
        }
        file_count++;
        CL_ASSERT(!entry.is_directory);
        CL_ASSERT_EQUAL(entry.size, strlen(TEST_FILE_CONTENT));
        CL_ASSERT(entry.last_write_time > 0);
    }

    cl_fs_close_directory(iterator);
    CL_ASSERT_EQUAL(file_count, 3);

    for (int i = 0; i < 3; i++) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", TEST_DIR, files[i]);
        cl_fs_remove_file(fs, path);
    }
    cl_fs_remove_directory(fs, TEST_DIR);
}

CL_TEST(test_large_file_operations) {
    cl_file_t *file = cl_fs_open_file(fs, "large_file.bin", CL_FILE_MODE_WRITE);
    CL_ASSERT_NOT_NULL(file);

    char *buffer = cl_mem_alloc(allocator, LARGE_FILE_SIZE);
    CL_ASSERT_NOT_NULL(buffer);
    memset(buffer, 'A', LARGE_FILE_SIZE);

    u64 written = cl_fs_write_file(file, buffer, LARGE_FILE_SIZE);
    CL_ASSERT_EQUAL(written, LARGE_FILE_SIZE);
    cl_fs_close_file(file);

    file = cl_fs_open_file(fs, "large_file.bin", CL_FILE_MODE_READ);
    CL_ASSERT_NOT_NULL(file);

    char *read_buffer = cl_mem_alloc(allocator, LARGE_FILE_SIZE);
    CL_ASSERT_NOT_NULL(read_buffer);

    u64 read = cl_fs_read_file(file, read_buffer, LARGE_FILE_SIZE);
    CL_ASSERT_EQUAL(read, LARGE_FILE_SIZE);
    CL_ASSERT_EQUAL(memcmp(buffer, read_buffer, LARGE_FILE_SIZE), 0);

    cl_fs_close_file(file);
    cl_fs_remove_file(fs, "large_file.bin");

    cl_mem_free(allocator, buffer);
    cl_mem_free(allocator, read_buffer);
}

CL_TEST(test_error_handling) {
    // Test opening a non-existent file
    cl_file_t *file = cl_fs_open_file(fs, "non_existent_file.txt", CL_FILE_MODE_READ);
    CL_ASSERT_NULL(file);
    CL_ASSERT_STRING_NOT_EQUAL(cl_fs_get_last_error(fs), "");

    // Test creating a directory with invalid characters
    bool result = cl_fs_create_directory(fs, "/invalid\\dir:name");
    CL_ASSERT(!result);
    CL_ASSERT_STRING_NOT_EQUAL(cl_fs_get_last_error(fs), "");

    // Test writing to a read-only file
    cl_fs_create_directory(fs, TEST_DIR);
    char readonly_file[256];
    snprintf(readonly_file, sizeof(readonly_file), "%s/readonly.txt", TEST_DIR);
    file = cl_fs_open_file(fs, readonly_file, CL_FILE_MODE_WRITE);
    cl_fs_close_file(file);

    // Set the file as read-only
    cl_fs_set_file_attributes(fs, readonly_file, CL_FILE_ATTRIBUTE_READONLY);

    file = cl_fs_open_file(fs, readonly_file, CL_FILE_MODE_WRITE);
    CL_ASSERT_NULL(file);
    CL_ASSERT_STRING_NOT_EQUAL(cl_fs_get_last_error(fs), "");

    // Clean up
    cl_fs_set_file_attributes(fs, readonly_file, CL_FILE_ATTRIBUTE_NORMAL);
    cl_fs_remove_file(fs, readonly_file);
    cl_fs_remove_directory(fs, TEST_DIR);
}

CL_TEST(test_file_attributes) {
    cl_file_t *file = cl_fs_open_file(fs, TEST_FILE, CL_FILE_MODE_WRITE);
    CL_ASSERT_NOT_NULL(file);
    cl_fs_write_file(file, TEST_FILE_CONTENT, strlen(TEST_FILE_CONTENT));
    cl_fs_close_file(file);

    // Test setting and getting attributes
    bool result = cl_fs_set_file_attributes(fs, TEST_FILE, CL_FILE_ATTRIBUTE_READONLY);
    CL_ASSERT(result);

    cl_file_attribute_t attrs = cl_fs_get_file_attributes(fs, TEST_FILE);
    CL_ASSERT(attrs & CL_FILE_ATTRIBUTE_READONLY);

    // Test removing readonly attribute
    result = cl_fs_set_file_attributes(fs, TEST_FILE, CL_FILE_ATTRIBUTE_NORMAL);
    CL_ASSERT(result);

    attrs = cl_fs_get_file_attributes(fs, TEST_FILE);
    CL_ASSERT(!(attrs & CL_FILE_ATTRIBUTE_READONLY));

    cl_fs_remove_file(fs, TEST_FILE);
}

CL_TEST(test_path_normalization) {
    const char *test_paths[] = {
        "./test/../folder/./file.txt",
        "folder//double_slash/file.txt",
        "../parent/file.txt",
        "/absolute/path/to/file.txt",
        "relative/path/to/file.txt"
    };

    for (int i = 0; i < 5; i++) {
        char *normalized = cl_fs_normalize_path(fs, test_paths[i]);
        CL_ASSERT_NOT_NULL(normalized);
        CL_LOG_INFO("Original: %s", test_paths[i]);
        CL_LOG_INFO("Normalized: %s", normalized);

        char *denormalized = cl_fs_denormalize_path(fs, normalized);
        CL_ASSERT_NOT_NULL(denormalized);
        CL_LOG_INFO("Denormalized: %s", denormalized);

        // Check that normalizing the denormalized path gives the same result
        char *renormalized = cl_fs_normalize_path(fs, denormalized);
        CL_ASSERT_STRING_EQUAL(normalized, renormalized);

        cl_mem_free(allocator, normalized);
        cl_mem_free(allocator, denormalized);
        cl_mem_free(allocator, renormalized);
    }
}

CL_TEST_SUITE_BEGIN(FilesystemTests)
    CL_TEST_SUITE_TEST(test_directory_operations)
    CL_TEST_SUITE_TEST(test_file_operations)
    CL_TEST_SUITE_TEST(test_file_seek_tell)
    CL_TEST_SUITE_TEST(test_rename_copy)
    CL_TEST_SUITE_TEST(test_file_info)
    CL_TEST_SUITE_TEST(test_directory_listing)
    CL_TEST_SUITE_TEST(test_large_file_operations)
    CL_TEST_SUITE_TEST(test_error_handling)
    CL_TEST_SUITE_TEST(test_file_attributes)
    CL_TEST_SUITE_TEST(test_path_normalization)
CL_TEST_SUITE_END

int main() {
    cl_log_init_default(CL_LOG_INFO);

    setup();
    CL_RUN_TEST_SUITE(FilesystemTests);
    CL_RUN_ALL_TESTS();
    teardown();

    cl_log_cleanup();
    return 0;
}