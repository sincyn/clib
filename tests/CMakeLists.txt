cmake_minimum_required(VERSION 3.12)

include(CTest)

# Function to add a test executable and its individual tests
function(add_test_executable TEST_SOURCE)
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})

    target_link_libraries(${TEST_NAME} PRIVATE clib_all)
    target_include_directories(${TEST_NAME}
            PRIVATE
            ${CLIB_INCLUDE_DIR}
    )

    set_target_properties(${TEST_NAME}
            PROPERTIES
            C_STANDARD 17
            C_STANDARD_REQUIRED ON
    )

    # Add the main test
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})

    # Set properties to parse the test output
    set_tests_properties(${TEST_NAME} PROPERTIES
            PASS_REGULAR_EXPRESSION "All tests passed!"
            FAIL_REGULAR_EXPRESSION "Some tests failed."
    )

endfunction()

# Find all .c files in the tests directory
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# Create an executable for each test file
foreach (TEST_SOURCE ${TEST_SOURCES})
    add_test_executable(${TEST_SOURCE})
endforeach ()

# Enable testing
enable_testing()