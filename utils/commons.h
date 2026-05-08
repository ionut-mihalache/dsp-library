//
// Created by ionut on 02.06.2024.
//

#ifndef DSP_COMMONS_H
#define DSP_COMMONS_H

#define _FILE_OFFSET_BITS 64

#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>

#define PIPES_DIR ".pipes/"

#define DSP_UNUSED __attribute__((unused))

int createShmObject(const char *name, int oflag, mode_t mode, loff_t size,
                    uint8_t unlink);

void createQ(void **ptrRes, size_t size, int prot, int fd);

void triggerKernelPageInit(void *p_MemoryAddr, size_t p_Size, int p_Prot);

#endif // DSP_COMMONS_H
