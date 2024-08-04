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

#    # Parse the source file to find individual test names
#    file(STRINGS ${TEST_SOURCE} TEST_LINES REGEX "CL_TEST\\(.*\\)")
#    foreach (TEST_LINE ${TEST_LINES})
#        string(REGEX REPLACE "CL_TEST\\((.*)\\)" "\\1" INDIVIDUAL_TEST_NAME ${TEST_LINE})
#
#        # Create a custom command to run the test and filter its output
#        add_test(
#                NAME ${TEST_NAME}.${INDIVIDUAL_TEST_NAME}
#                COMMAND ${CMAKE_COMMAND} -E env
#                ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME} --gtest_filter=${INDIVIDUAL_TEST_NAME} --test=$<TARGET_FILE:${TEST_NAME}>
#        )
#
#        # Set properties to parse the test output with flexible whitespace matching
##        can have any number of spaces or tabs between the test name and the result
#        # Set properties to parse the test output with flexible whitespace matching
#        # This regex will match the test name, followed by any number of spaces or tabs, followed by PASS or FAIL,
#        # and ignore anything that comes after
#        # Set properties to parse the test output with more flexible matching
#        set_tests_properties(${TEST_NAME}.${INDIVIDUAL_TEST_NAME} PROPERTIES
#                PASS_REGULAR_EXPRESSION ".*${INDIVIDUAL_TEST_NAME}.*PASS.*"
#                FAIL_REGULAR_EXPRESSION ".*${INDIVIDUAL_TEST_NAME}.*FAIL.*"
#        )
#    endforeach ()


endfunction()

# Find all .c files in the tests directory
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c")

# Create an executable for each test file
foreach (TEST_SOURCE ${TEST_SOURCES})
    add_test_executable(${TEST_SOURCE})
endforeach ()

# Enable testing
enable_testing()