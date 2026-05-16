// SPDX-License-Identifier: LGPL-2.1-or-later

//
// Created by ionut on 02.06.2024.
//

#ifndef DSP_COMMONS_H
#define DSP_COMMONS_H

#include "platform-types.h"
#define _FILE_OFFSET_BITS 64

#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>

#define PIPES_DIR ".pipes/"

#define DSP_UNUSED __attribute__((unused))

aqua_size_t alignUp(aqua_size_t base, aqua_size_t alignment);

void createQ(void **ptrRes, aqua_size_t size, int prot, int fd);

void triggerKernelPageInit(void *p_MemoryAddr, size_t p_Size, int p_Prot);

#endif // DSP_COMMONS_H
