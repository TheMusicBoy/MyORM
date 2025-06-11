find_package(Protobuf REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(GRPCPP REQUIRED grpc++_unsecure)

# Находим исполняемый файл protoc и плагин grpc
find_program(PROTOC protoc)
find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin)

function(generate_protobuf_grpc TARGET_NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})
    
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "generate_protobuf_grpc: SOURCES не указаны")
    endif()
    
    # Создаем единую директорию для всех сгенерированных файлов
    set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
    file(MAKE_DIRECTORY ${GEN_DIR})
    
    set(PROTO_SRCS)
    set(PROTO_HDRS)
    
    foreach(PROTO_FILE ${ARG_SOURCES})
        get_filename_component(ABS_PROTO_FILE ${PROTO_FILE} ABSOLUTE)
        get_filename_component(PROTO_NAME_WE ${PROTO_FILE} NAME_WE)
        file(RELATIVE_PATH REL_PROTO_PATH ${PROJECT_SOURCE_DIR} ${ABS_PROTO_FILE})
        get_filename_component(REL_PROTO_DIR ${REL_PROTO_PATH} DIRECTORY)
        
        # Выходные файлы в общей директории, с сохранением относительного пути
        set(OUTPUT_DIR "${GEN_DIR}/${REL_PROTO_DIR}")
        file(MAKE_DIRECTORY ${OUTPUT_DIR})
        
        set(PROTO_CC "${OUTPUT_DIR}/${PROTO_NAME_WE}.pb.cc")
        set(PROTO_H "${OUTPUT_DIR}/${PROTO_NAME_WE}.pb.h")
        set(GRPC_PROTO_CC "${OUTPUT_DIR}/${PROTO_NAME_WE}.grpc.pb.cc")
        set(GRPC_PROTO_H "${OUTPUT_DIR}/${PROTO_NAME_WE}.grpc.pb.h")
        
        list(APPEND PROTO_SRCS ${PROTO_CC} ${GRPC_PROTO_CC})
        list(APPEND PROTO_HDRS ${PROTO_H} ${GRPC_PROTO_H})
        
        # Генерация protobuf файлов из источника
        add_custom_command(
            OUTPUT ${PROTO_CC} ${PROTO_H} ${GRPC_PROTO_CC} ${GRPC_PROTO_H}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
            COMMAND ${PROTOC}
                --grpc_out=${GEN_DIR}
                --cpp_out=${GEN_DIR}
                --proto_path=${PROJECT_SOURCE_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                ${REL_PROTO_PATH}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            DEPENDS ${ABS_PROTO_FILE}
            COMMENT "Generating gRPC protobuf files from ${PROTO_FILE}"
        )
    endforeach()
    
    # Создаем библиотеку
    add_library(${TARGET_NAME} STATIC ${PROTO_SRCS} ${PROTO_HDRS})
    
    target_include_directories(${TARGET_NAME} PUBLIC
        ${GEN_DIR}  # Путь к сгенерированным файлам
        ${PROTOBUF_INCLUDE_DIRS}
        ${GRPC_INCLUDE_DIRS}
    )
    
    target_link_libraries(${TARGET_NAME} 
        PUBLIC 
        ${PROTOBUF_LIBRARIES}
        ${GRPC_LIBRARIES}
        ${GRPCPP_LIBRARIES}
    )
    
    if(ARG_DEPENDS)
        add_dependencies(${TARGET_NAME} ${ARG_DEPENDS})
        target_link_libraries(${TARGET_NAME} PUBLIC ${ARG_DEPENDS})
    endif()
endfunction()

function(generate_protobuf_simple TARGET_NAME)
    cmake_parse_arguments(ARG "" "" "SOURCES;DEPENDS" ${ARGN})
    
    if(NOT ARG_SOURCES)
        message(FATAL_ERROR "generate_protobuf_simple: SOURCES не указаны")
    endif()
    
    # Создаем единую директорию для всех сгенерированных файлов
    set(GEN_DIR "${CMAKE_BINARY_DIR}/generated")
    file(MAKE_DIRECTORY ${GEN_DIR})
    
    set(PROTO_SRCS)
    set(PROTO_HDRS)
    
    foreach(PROTO_FILE ${ARG_SOURCES})
        get_filename_component(ABS_PROTO_FILE ${PROTO_FILE} ABSOLUTE)
        get_filename_component(PROTO_NAME_WE ${PROTO_FILE} NAME_WE)
        file(RELATIVE_PATH REL_PROTO_PATH ${PROJECT_SOURCE_DIR} ${ABS_PROTO_FILE})
        get_filename_component(REL_PROTO_DIR ${REL_PROTO_PATH} DIRECTORY)
        
        # Выходные файлы в общей директории, с сохранением относительного пути
        set(OUTPUT_DIR "${GEN_DIR}/${REL_PROTO_DIR}")
        file(MAKE_DIRECTORY ${OUTPUT_DIR})
        
        set(PROTO_CC "${OUTPUT_DIR}/${PROTO_NAME_WE}.pb.cc")
        set(PROTO_H "${OUTPUT_DIR}/${PROTO_NAME_WE}.pb.h")
        
        list(APPEND PROTO_SRCS ${PROTO_CC})
        list(APPEND PROTO_HDRS ${PROTO_H})
        
        # Генерация protobuf файлов из источника
        add_custom_command(
            OUTPUT ${PROTO_CC} ${PROTO_H}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
            COMMAND ${PROTOC}
                --cpp_out=${GEN_DIR}
                --proto_path=${PROJECT_SOURCE_DIR}
                ${REL_PROTO_PATH}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            DEPENDS ${ABS_PROTO_FILE}
            COMMENT "Generating protobuf files from ${PROTO_FILE}"
        )
    endforeach()
    
    # Создаем библиотеку
    add_library(${TARGET_NAME} STATIC ${PROTO_SRCS} ${PROTO_HDRS})
    
    target_include_directories(${TARGET_NAME} PUBLIC
        ${GEN_DIR}  # Путь к сгенерированным файлам
        ${PROTOBUF_INCLUDE_DIRS}
    )
    
    target_link_libraries(${TARGET_NAME} 
        PUBLIC 
        ${PROTOBUF_LIBRARIES}
    )
    
    if(ARG_DEPENDS)
        add_dependencies(${TARGET_NAME} ${ARG_DEPENDS})
        target_link_libraries(${TARGET_NAME} PUBLIC ${ARG_DEPENDS})
    endif()
endfunction()
