add_library(clib_thread
        posix_thread.c
        thread_lib.c
        win_thread.c
)

target_include_directories(clib_thread PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(clib_thread PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_thread PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

if (NOT WIN32)
    find_package(Threads REQUIRED)
    target_link_libraries(clib_thread PRIVATE Threads::Threads)
endif ()

target_link_libraries(clib_thread PRIVATE clib_log clib_memory)

install(TARGETS clib_thread
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)