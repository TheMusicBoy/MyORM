include(GoogleTest)
enable_testing()

# Helper function to add a test
function(add_test_ex test_name)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})
    
    # Add executable
    add_executable(${test_name} ${ARG_SOURCES})
    
    # Link dependencies
    if(ARG_DEPENDS)
        target_link_libraries(${test_name} PRIVATE ${ARG_DEPENDS} gtest gtest_main gmock)
    else()
        target_link_libraries(${test_name} PRIVATE gtest gtest_main gmock)
    endif()
    
    # Include directories
    target_include_directories(${test_name} PRIVATE 
        ${PROJECT_SOURCE_DIR}
    )
    
    # Add test to CTest
    add_test(NAME ${test_name} COMMAND ${test_name})
    
    # Set properties for the test
    set_tests_properties(${test_name} PROPERTIES TIMEOUT 30)
endfunction()

# Helper function for test suites (multiple tests in one executable)
function(add_test_suite suite_name)
    cmake_parse_arguments(SUITE "" "" "SOURCES;DEPENDS" ${ARGN})
    
    add_test_ex(${suite_name}
        SOURCES ${SUITE_SOURCES}
        DEPENDS ${SUITE_DEPENDS}
    )
endfunction()
