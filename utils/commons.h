//
// Created by ionut on 02.06.2024.
//

#ifndef DSP_COMMONS_H
#define DSP_COMMONS_H

#define _FILE_OFFSET_BITS 64

#include <stdint.h>

#include "system-types.h"

#define PIPES_DIR ".pipes/"

#define DSP_UNUSED __attribute__((unused))

#ifdef _WIN32
#define PAGE_SIZE 4096
#endif

aqua_file_handle createShmObject(const char *name, int oflag, aqua_mode_t mode,
                                 aqua_object_size size, uint8_t unlink);

void createQ(void **ptrRes, aqua_size_t size, aqua_prot_t prot,
             aqua_file_handle fileHandle);

void triggerKernelPageInit(void *memoryAddr, aqua_size_t size, int prot);

#endif // DSP_COMMONS_H
