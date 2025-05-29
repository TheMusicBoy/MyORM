add_subdirectory(proto)
add_subdirectory(service)

# set(SRCROOT "${PROJECT_SOURCE_DIR}/master")

# set(SRC
#     ${SRCROOT}/bin/main.cpp
# )

# if (NOT "${SRC}" STREQUAL "")
#     message(STATUS "Building main executable...")
#     add_executable(main ${SRC})
#     target_include_directories(main PUBLIC 
#         ${PROJECT_SOURCE_DIR}/lib 
#         ${SRCROOT}
#         ${SRCROOT}/proto  # Add proto directory for include access
#     )
#     target_link_libraries(main PUBLIC 
#         common 
#         nlohmann_json 
#         rpc 
#         proto_gen
#     )
#     set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
# else()
#     message(WARNING "Main executable is not built")
# endif()
