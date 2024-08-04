add_library(clib_memory
        arena_allocator.c
        freelist_allocator.c
        memory_lib.c
        platform_allocator.c
)

target_include_directories(clib_memory PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(clib_memory PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_memory PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

install(TARGETS clib_memory
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)