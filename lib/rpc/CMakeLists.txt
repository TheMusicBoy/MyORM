include(${PROJECT_SOURCE_DIR}/cmake/ProtoGen.cmake)

set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/rpc")

add_protobuf_library(rpc_proto
    PROTO_FILES
        proto/http.proto
    OUTPUT_DIR
        ${PROJECT_SOURCE_DIR}/lib/rpc/proto
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
