include(${PROJECT_SOURCE_DIR}/cmake/ProtoGen.cmake)

add_protobuf_library(object_base
    PROTO_FILES
        object_base.proto
        test_object.proto
    OUTPUT_DIR
        ${PROJECT_SOURCE_DIR}/master/proto
    PROTO_PATH
        ${PROJECT_SOURCE_DIR}/lib/rpc/proto
        ${Protobuf_INCLUDE_DIR}
)

add_dependencies(object_base rpc_proto)
target_include_directories(object_base PUBLIC 
    ${PROJECT_SOURCE_DIR}/lib/rpc/proto
)
target_link_libraries(object_base PUBLIC rpc_proto)
