cmake_minimum_required(VERSION 3.25.1)

project(dsp-library C)

set(CMAKE_C_STANDARD 23)

set(SRC_FILES
    dsp.c)

set(HEADER_FILES
    dsp.h
    dsp-client.h
    dsp-service.h
    protocol.h)

add_library(dsp SHARED ${HEADER_FILES} ${SRC_FILES})
