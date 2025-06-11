set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/rpc")

include(ProtoGen)

generate_protobuf_grpc(rpc_proto
SOURCES
    ${SRCROOT}/proto/http.proto
    ${SRCROOT}/proto/query.proto
DEPENDS
    relation_proto
)

set(SRC
    ${SRCROOT}/http_server.cpp
    ${SRCROOT}/service_rpc.cpp

)

add_library(rpc STATIC ${SRC})

target_include_directories(rpc PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(rpc PUBLIC common nlohmann_json protobuf::libprotobuf)

set_target_properties(rpc PROPERTIES LINKER_LANGUAGE CXX)
