set(INCROOT "${PROJECT_SOURCE_DIR}/include/ipc")
set(SRCROOT "${PROJECT_SOURCE_DIR}/src/ipc")

set(SRC
    ${SRCROOT}/shared_memory.cpp
    ${INCROOT}/shared_memory.h

    ${SRCROOT}/serial_port.cpp
    ${INCROOT}/serial_port.h

    ${SRCROOT}/decode_encode.cpp
    ${INCROOT}/decode_encode.h

    ${SRCROOT}/subprocess.cpp
    ${INCROOT}/subprocess.h
)

add_library(ipc STATIC ${SRC})

target_include_directories(ipc PUBLIC 
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(ipc PUBLIC common)

set_target_properties(ipc PROPERTIES LINKER_LANGUAGE CXX)
