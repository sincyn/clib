add_library(clib_containers
        cl_ht.c
        cl_da.c
        cl_hs.c
)

target_include_directories(clib_containers PUBLIC
        $<BUILD_INTERFACE:${CLIB_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(clib_containers
        clib_memory
        clib_string
)


set_target_properties(clib_containers PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
)

if (BUILD_SHARED_LIBS)
    set_target_properties(clib_containers PROPERTIES
            POSITION_INDEPENDENT_CODE ON
    )
endif ()

install(TARGETS clib_containers
        EXPORT clib_all_targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
)