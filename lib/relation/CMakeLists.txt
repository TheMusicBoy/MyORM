set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/relation")

include(ProtoGen)

generate_protobuf_simple(relation_proto
SOURCES
    ${SRCROOT}/proto/orm_core.proto
    ${SRCROOT}/proto/query.proto
)

set(SRC
    ${SRCROOT}/path.cpp
    ${SRCROOT}/base.cpp
    ${SRCROOT}/message.cpp
    ${SRCROOT}/field.cpp
    ${SRCROOT}/config.cpp
    ${SRCROOT}/relation_manager.cpp
)

add_library(relation STATIC ${SRC})

target_include_directories(relation PUBLIC 
    ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(relation PUBLIC common protobuf::libprotobuf relation_proto)

set_target_properties(relation PROPERTIES LINKER_LANGUAGE CXX)
