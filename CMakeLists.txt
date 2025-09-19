cmake_minimum_required(VERSION 3.25.1)

project(dsp-library C)

set(CMAKE_C_STANDARD 23)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_COMPILER clang)
# set(CMAKE_C_COMPILER gcc)

set(LIB_VERSION "0.0.2")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/object")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(OBJECT_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/object")

set(SRC_FILES
    service/dsp-service.c
    service/install.c
    service/call.c
    service/return.c
    client/dsp-client.c
    client/client-connect.c
    client/client-call.c
    utils/commons.c
)

set(HEADER_FILES
    service/include/dsp-service.h
    service/include/install.h
    service/include/call.h
    service/include/return.h
    client/include/dsp-client.h
    client/include/client-connect.h
    client/include/client-call.h
    utils/commons.h
    utils/hashmap/hashmap.h
    utils/macros/macros.h
    utils/exit/exit_header.h
    utils/log/log.h
    include/dsp.h
    include/protocol.h
)

add_library(dsp SHARED
    ${SRC_FILES}
    ${HEADER_FILES}
)

set_target_properties(dsp PROPERTIES
    VERSION ${LIB_VERSION}
)

target_include_directories(dsp PUBLIC
    utils
    utils/macros
    utils/hashmap
    utils/exit
    utils/log
    include
    service/include
    client/include
)

target_compile_options(dsp PRIVATE
    -Wall
    -Wextra
    -Werror
    -O3
)

target_compile_definitions(dsp PRIVATE
    __COMPILE_MODE_DEBUG__
    _GNU_SOURCE
)

add_custom_command(TARGET dsp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/$<TARGET_FILE_NAME:dsp>"
        "${CMAKE_CURRENT_SOURCE_DIR}/$<TARGET_LINKER_FILE_NAME:dsp>"
)

add_custom_command(TARGET dsp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E remove
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/$<TARGET_LINKER_FILE_NAME:dsp>"
)

add_custom_command(
    TARGET dsp
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json"
        "${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json"
)
