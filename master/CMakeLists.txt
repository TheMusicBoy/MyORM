set(SRCROOT "${PROJECT_SOURCE_DIR}/master")

set(SRC
    # ${SRCROOT}/service/config.cpp
    # ${SRCROOT}/service/service.cpp
    # ${SRCROOT}/service/file_storage.cpp
    # ${SRCROOT}/service/database_storage.cpp
    # ${SRCROOT}/service/service_rpc.cpp
    ${SRCROOT}/service/main.cpp
)

if (NOT "${SRC}" STREQUAL "")
    message(STATUS "Building main executable...")
    add_executable(main ${SRC})
    target_include_directories(main PUBLIC ${PROJECT_SOURCE_DIR}/lib ${SRCROOT})
    target_link_libraries(main PUBLIC common  nlohmann_json)
    set_target_properties(main PROPERTIES LINKER_LANGUAGE CXX)
else()
    message(WARNING "Main executable is not built")
endif()

# set(SRC
#     ${SRCROOT}/ui/config.cpp
#     ${SRCROOT}/ui/service_rpc.cpp
#     ${SRCROOT}/ui/service.cpp
#     ${SRCROOT}/ui/assets.cpp
#     ${SRCROOT}/ui/main.cpp
# )

# if (NOT "${SRC}" STREQUAL "")
#     message(STATUS "Building UI...")
#     add_executable(ui ${SRC})
#     target_include_directories(ui PUBLIC ${INCROOT} ${SRCROOT})
#     target_link_libraries(ui PUBLIC common ipc rpc nlohmann_json jinja2cpp)
#     set_target_properties(ui PROPERTIES LINKER_LANGUAGE CXX)
# else()
#     message(WARNING "UI is not built")
# endif()

# add_subdirectory(tools)
