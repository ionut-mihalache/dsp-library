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

uint32_t murmur3Hash_32(const char *key, uint32_t len, uint32_t seed);

int createShmObject(const char *p_Name, int p_Oflag, mode_t p_Mode,
                    loff_t p_Size, uint8_t p_Unlink);

#endif // DSP_COMMONS_H
