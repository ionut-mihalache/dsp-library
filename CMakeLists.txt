cmake_minimum_required(VERSION 3.25.1)

project(dsp-library C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER clang)
# set(CMAKE_C_COMPILER gcc)

set(LIB_VERSION "0.0.2")
set(LIB_NAME dsp)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_library(${LIB_NAME} SHARED
        service/dsp-service.c
        service/install.c
        service/call.c
        service/return.c
        client/dsp-client.c
        client/client-connect.c
        client/client-call.c
        utils/commons.c
)

set_target_properties(${LIB_NAME} PROPERTIES
        VERSION ${LIB_VERSION}
        SOVERSION ${LIB_VERSION}
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${LIB_NAME} PUBLIC
        utils
        utils/macros
        utils/hashmap
        utils/exit
        utils/log
        include
        service/include
        client/include
)

target_compile_options(${LIB_NAME} PRIVATE
        -Wall -Wextra -Werror -O3
)

target_compile_definitions(${LIB_NAME} PRIVATE
        __COMPILE_MODE_DEBUG__
        _GNU_SOURCE
)

# Symlink libdsp.so -> lib/libdsp.so.X.Y.Z în build root
#add_custom_command(
#        TARGET ${LIB_NAME}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E create_symlink
#        lib/$<TARGET_FILE_NAME:${LIB_NAME}>
#        ${CMAKE_BINARY_DIR}/libdsp.so
#        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#        COMMENT "Creating libdsp.so symlink in build root"
#)
add_custom_command(
        TARGET ${LIB_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        lib/$<TARGET_FILE_NAME:${LIB_NAME}>
        ${CMAKE_BINARY_DIR}/libdsp.so
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Creating libdsp.so in build root"
)
