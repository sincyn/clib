# Get all .c files in the current directory
file(GLOB EXAMPLE_SOURCES "*.c")

# Create an executable for each source file
foreach(SOURCE_FILE ${EXAMPLE_SOURCES})
    # Get the filename without extension
    get_filename_component(EXAMPLE_NAME ${SOURCE_FILE} NAME_WE)

    # Create the executable
    add_executable(${EXAMPLE_NAME} ${SOURCE_FILE})

    # Link with clib_all (without the clib:: namespace)
    target_link_libraries(${EXAMPLE_NAME} PRIVATE clib_all)

    # Set the C standard for the example
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
            C_STANDARD 17
            C_STANDARD_REQUIRED ON
    )

    # Try to enable C23 if available
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${EXAMPLE_NAME} PRIVATE -std=c2x)
    elseif(MSVC)
        target_compile_options(${EXAMPLE_NAME} PRIVATE /std:c17)  # MSVC doesn't support C23 yet, use C17
    endif()
endforeach()