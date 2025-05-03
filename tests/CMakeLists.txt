set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")
set(TESTROOT "${PROJECT_SOURCE_DIR}/tests")

# Include the test utilities module
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(TestUtils)

# Example: Uncomment to add a sample test
add_test_ex(common_test
    SOURCES 
        ${TESTROOT}/common/format_test.cpp
        ${TESTROOT}/common/logging_test.cpp
        ${TESTROOT}/common/program_test.cpp
    DEPENDS
        common
)

message(STATUS "Test framework configured")
