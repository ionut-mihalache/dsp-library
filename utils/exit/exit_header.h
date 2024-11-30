//
// Created by ionut on 01.06.2024.
//

#ifndef DSP_EXIT_HEADER_H
#define DSP_EXIT_HEADER_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CHECK_AND_EXIT(should_exit) do { \
    if (should_exit) {                   \
        fprintf(stdout, "%s(%d)", __func__, __LINE__); \
        fprintf(stdout, "[\'%s\']: %s\n", #should_exit, strerror(errno)); \
        exit(1);                         \
    }                                    \
} while(0)

#define CHECK_AND_EXIT_ERR(should_exit) do { \
    if (should_exit) {                       \
        fprintf(stderr, "%s(%d)", __func__, __LINE__); \
        fprintf(stderr, "[\'%s\']: %s\n", #should_exit, strerror(errno)); \
        exit(1);                             \
    }                                        \
} while(0)

#endif //DSP_EXIT_HEADER_H
