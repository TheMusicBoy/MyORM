set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/relation")

set(SRC
    ${SRCROOT}/fetch_info.cpp

)

add_library(relation STATIC ${SRC})

target_include_directories(relation PUBLIC 
    ${PROJECT_SOURCE_DIR}/lib
)

target_link_libraries(relation PUBLIC common protobuf::libprotobuf)

set_target_properties(relation PROPERTIES LINKER_LANGUAGE CXX)
