add_library(clib_string
        string_lib.c
)

target_include_directories(clib_string PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(clib_string PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_string PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

install(TARGETS clib_string
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)