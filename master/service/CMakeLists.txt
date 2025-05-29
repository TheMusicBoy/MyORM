set(SRCROOT ${PROJECT_SOURCE_DIR}/master/service)

set(SRC
    ${SRCROOT}/config.cpp 
    ${SRCROOT}/server_rpc.cpp 
    ${SRCROOT}/service.cpp 
    ${SRCROOT}/query/postgresql.cpp
)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building master service...")
    add_library(master_service ${SRC})
    target_include_directories(master_service PUBLIC 
        ${PROJECT_SOURCE_DIR}/lib 
        ${PROJECT_SOURCE_DIR}/master
        ${PROJECT_SOURCE_DIR}/master/proto
    )
    target_link_libraries(master_service PUBLIC 
        common 
        rpc 
        object_base
    )
    set_target_properties(master_service PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Master service is not built")
endif()
