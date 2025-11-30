//
// Created by ionut on 02.06.2024.
//

#ifndef AQUA_DSP_COMMONS_H
#define AQUA_DSP_COMMONS_H

#define _FILE_OFFSET_BITS 64

#include <stdint.h>

#include "dsp.h"

#define PIPES_DIR ".pipes/"

#define DSP_UNUSED __attribute__((unused))

aqua_file_handle createShmObject(const char *name, int oflag, aqua_mode_t mode,
                                 aqua_object_size_t size, uint8_t unlink);

aqua_void_t createQ(aqua_void_t **ptrRes, aqua_size_t size, aqua_prot_t prot,
                    aqua_file_handle fileHandle);

aqua_void_t triggerKernelPageInit(aqua_void_t *memoryAddr, aqua_size_t size,
                                  int prot);

#endif // AQUA_DSP_COMMONS_H
