add_library(clib_test
       test_lib.c
)

target_include_directories(clib_test PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(clib_test PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_test PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

install(TARGETS clib_test
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)