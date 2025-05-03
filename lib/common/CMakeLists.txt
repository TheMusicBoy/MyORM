set(SRCROOT "${PROJECT_SOURCE_DIR}/lib/common")

set(SRC
    ${SRCROOT}/public.h

    ${SRCROOT}/atomic_intrusive_ptr.cpp
    ${SRCROOT}/atomic_intrusive_ptr.h
    ${SRCROOT}/config.cpp
    ${SRCROOT}/config.h
    ${SRCROOT}/exception.cpp
    ${SRCROOT}/exception.h
    ${SRCROOT}/format.cpp
    ${SRCROOT}/format.h
    ${SRCROOT}/getopts.cpp
    ${SRCROOT}/getopts.h
    ${SRCROOT}/intrusive_ptr.cpp
    ${SRCROOT}/intrusive_ptr.h
    ${SRCROOT}/logging.cpp
    ${SRCROOT}/logging.h
    ${SRCROOT}/periodic_executor.cpp
    ${SRCROOT}/periodic_executor.h
    ${SRCROOT}/program.cpp
    ${SRCROOT}/program.h
    ${SRCROOT}/refcounted.cpp
    ${SRCROOT}/refcounted.h
    ${SRCROOT}/threadpool.cpp
    ${SRCROOT}/threadpool.h
    ${SRCROOT}/weak_ptr.cpp
    ${SRCROOT}/weak_ptr.h

)

add_library(common STATIC ${SRC})

target_link_libraries(common nlohmann_json)

target_include_directories(common PUBLIC 
    ${PROJECT_SOURCE_DIR}/lib
)

set_target_properties(common PROPERTIES LINKER_LANGUAGE CXX)
