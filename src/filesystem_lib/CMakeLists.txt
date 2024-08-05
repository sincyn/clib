add_library(clib_filesystem
        filesystem_lib.c
        filesystem_apple.c
)

target_include_directories(clib_filesystem PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(clib_filesystem
        clib_memory
)


set_target_properties(clib_filesystem PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_filesystem PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

install(TARGETS clib_filesystem
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)