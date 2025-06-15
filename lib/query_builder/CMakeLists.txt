set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/query_builder")

set(SRC
    ${SRCROOT}/builder_base.cpp
    ${SRCROOT}/builders/postgres.cpp
    ${SRCROOT}/query_organizer_base.cpp
    ${SRCROOT}/organizers/sql_organizer.cpp
)

add_library(query_builder STATIC ${SRC})

target_include_directories(query_builder PUBLIC 
    ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(query_builder PUBLIC common relation requests)

set_target_properties(query_builder PROPERTIES LINKER_LANGUAGE CXX)
