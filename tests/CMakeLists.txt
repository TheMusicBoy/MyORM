set(INCROOT "${PROJECT_SOURCE_DIR}/include")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src")
set(TESTROOT "${PROJECT_SOURCE_DIR}/tests")

# Include the test utilities module
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
include(TestUtils)

# Generate protobuf objects for tests
include(${PROJECT_SOURCE_DIR}/cmake/ProtoGen.cmake)

generate_protobuf_simple(test_objects
SOURCES
    ${TESTROOT}/proto/test_objects.proto
DEPENDS
    relation_proto
)

# Common tests
add_test_ex(common_test
SOURCES 
    ${TESTROOT}/common/format_test.cpp
    ${TESTROOT}/common/logging_test.cpp
    ${TESTROOT}/common/program_test.cpp
DEPENDS
    common
)

# Relation tests
add_test_ex(relation_test
SOURCES 
    ${TESTROOT}/relation/relation_manager_test.cpp
    ${TESTROOT}/relation/path_test.cpp
DEPENDS
    relation
    test_objects
    common
)

# Requests tests
add_test_ex(requests_test
SOURCES 
    ${TESTROOT}/requests/query_create_test.cpp
DEPENDS
    relation
    requests
    test_objects
    common
)

# Query builder tests
add_test_ex(query_builder_test
SOURCES 
    ${TESTROOT}/query_builder/postgres_query_builder_test.cpp
DEPENDS
    relation
    query_builder
    requests
    test_objects
    common
)


message(STATUS "Test framework configured")
