cmake_minimum_required(VERSION 3.12)
project(clib VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 17)  # Fallback to C17 if C23 is not available
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)

# Add compile options
if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

if (ENABLE_ASAN)
    if (MSVC)
        add_compile_options(/fsanitize=address)
    else ()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif ()
endif ()

# Define include directory
set(CLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Add subdirectories for each library
add_subdirectory(src/log_lib)
add_subdirectory(src/memory_lib)
add_subdirectory(src/socket_lib)
add_subdirectory(src/test_lib)
add_subdirectory(src/thread_lib)
add_subdirectory(src/time_lib)
add_subdirectory(src/string_lib)

# Create the unified clib_all library
add_library(clib_all INTERFACE)
target_link_libraries(clib_all INTERFACE
        clib_log
        clib_memory
        clib_socket
        clib_test
        clib_thread
        clib_time
        clib_string
)

target_include_directories(clib_all INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Create an alias target with the clib:: namespace
add_library(clib::clib_all ALIAS clib_all)

# Install rules
install(TARGETS clib_all
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

install(EXPORT clib_all_targets
        FILE clib_all_targets.cmake
        NAMESPACE clib::
        DESTINATION lib/cmake/clib
)

# Add tests
if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

# Add examples
add_subdirectory(examples)