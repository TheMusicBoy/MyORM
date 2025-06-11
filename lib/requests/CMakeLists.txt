set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/requests")

include(ProtoGen)

generate_protobuf_grpc(requests_proto
SOURCES
    ${SRCROOT}/proto/query.proto
DEPENDS
    relation_proto
)

set(SRC
    ${SRCROOT}/query.cpp
)

add_library(requests STATIC ${SRC})

target_link_libraries(requests PUBLIC common requests_proto)

set_target_properties(requests PROPERTIES LINKER_LANGUAGE CXX)
