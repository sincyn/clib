add_library(clib_socket
        win_socket.c
        posix_socket.c
        socket_lib.c
)

target_include_directories(clib_socket PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(clib_socket PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_socket PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

if (WIN32)
    if (WIN32)
        target_link_libraries(clib_socket PRIVATE ws2_32)
    endif ()

endif ()

install(TARGETS clib_socket
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)