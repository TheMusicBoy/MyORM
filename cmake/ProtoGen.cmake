# Function to generate protobuf code and create a library
# Usage:
#   add_protobuf_library(target_name
#       PROTO_FILES file1.proto file2.proto
#       OUTPUT_DIR path/to/output
#       [PROTO_PATH path1 path2 ...]
#   )
function(add_protobuf_library target_name)
    # Parse remaining arguments
    cmake_parse_arguments(PROTO_GEN "" "OUTPUT_DIR" "PROTO_FILES;PROTO_PATH" ${ARGN})
    
    # Validate required arguments
    if(NOT PROTO_GEN_PROTO_FILES)
        message(FATAL_ERROR "PROTO_FILES is required for add_protobuf_library")
    endif()
    
    if(NOT PROTO_GEN_OUTPUT_DIR)
        message(FATAL_ERROR "OUTPUT_DIR is required for add_protobuf_library")
    endif()
    
    # Create output directory if it doesn't exist
    file(MAKE_DIRECTORY ${PROTO_GEN_OUTPUT_DIR})
    
    # Get the protoc executable path
    set(PROTOC_PATH "$<TARGET_FILE:protobuf::protoc>")
    
    # Set up lists for generated sources and headers
    set(PROTO_SRCS)
    set(PROTO_HDRS)
    
    # Process each proto file
    foreach(proto_file ${PROTO_GEN_PROTO_FILES})
        # Get absolute path and filename without extension
        get_filename_component(proto_file_abs ${proto_file} ABSOLUTE)
        get_filename_component(proto_name ${proto_file} NAME_WE)
        get_filename_component(proto_file_name ${proto_file} NAME)
        get_filename_component(proto_dir ${proto_file_abs} DIRECTORY)
        
        # Set output file paths
        set(proto_src "${PROTO_GEN_OUTPUT_DIR}/${proto_name}.pb.cc")
        set(proto_hdr "${PROTO_GEN_OUTPUT_DIR}/${proto_name}.pb.h")
        
        # Add files to lists
        list(APPEND PROTO_SRCS ${proto_src})
        list(APPEND PROTO_HDRS ${proto_hdr})
        
        # Set up import paths for protoc
        set(PROTO_IMPORT_PATHS)
        foreach(import_path ${PROTO_GEN_PROTO_PATH})
            list(APPEND PROTO_IMPORT_PATHS "-I${import_path}")
        endforeach()
        
        # Add the directory containing the proto file as an import path
        list(APPEND PROTO_IMPORT_PATHS "-I${proto_dir}")
        
        # Add custom command to generate C++ code
        add_custom_command(
            OUTPUT ${proto_src} ${proto_hdr}
            COMMAND ${PROTOC_PATH}
            ARGS --cpp_out=${PROTO_GEN_OUTPUT_DIR}
                 ${PROTO_IMPORT_PATHS}
                 -I${CMAKE_CURRENT_SOURCE_DIR}
                 ${proto_file_name}
            WORKING_DIRECTORY ${proto_dir}
            DEPENDS ${proto_file_abs} protobuf::protoc
            COMMENT "Generating C++ code from ${proto_file}"
            VERBATIM
        )
    endforeach()
    
    # Create custom target for generation
    add_custom_target(${target_name}_generation ALL 
                      DEPENDS ${PROTO_SRCS} ${PROTO_HDRS})
    
    # Create library target with generated files
    add_library(${target_name} STATIC ${PROTO_SRCS} ${PROTO_HDRS})
    add_dependencies(${target_name} ${target_name}_generation)
    target_link_libraries(${target_name} PUBLIC protobuf::libprotobuf)
    target_include_directories(${target_name} PUBLIC 
        ${PROTO_GEN_OUTPUT_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endfunction()
