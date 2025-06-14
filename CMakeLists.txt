cmake_minimum_required(VERSION 3.15)
project(MyORM)

# Enable CTest
include(CTest)
enable_testing()

# Export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

# Build settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Building ---
add_subdirectory(thirdparty)
add_subdirectory(lib)
# add_subdirectory(master)
add_subdirectory(core)
add_subdirectory(migrator)
add_subdirectory(tests)

add_custom_target(build_finished ALL
    COMMENT "Build almost finished...")

add_custom_command(TARGET build_finished POST_BUILD
    COMMAND cp ${PROJECT_BINARY_DIR}/compile_commands.json ${PROJECT_SOURCE_DIR}/
)
message("Compile commands exported...")
