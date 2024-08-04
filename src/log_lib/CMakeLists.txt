add_library(clib_log
        log_lib.c
        console_log.c
        remote_log.c
)

target_include_directories(clib_log PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

set_target_properties(clib_log PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

# Try to enable C23 if available
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(clib_log PRIVATE -std=c2x)
elseif(MSVC)
    # MSVC doesn't support C23 yet, use C17
    target_compile_options(clib_log PRIVATE /std:c17)
endif()

if(BUILD_SHARED_LIBS)
    set_target_properties(clib_log PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif()

install(TARGETS clib_log
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)