//
// Created by ionut on 02.06.2024.
//

#ifndef DSP_COMMONS_H
#define DSP_COMMONS_H

#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <stdint.h>
#include <sys/uio.h>
#include <unistd.h>

#define PIPES_DIR ".pipes/"

#define DSP_UNUSED __attribute__((unused))

uint64_t strHashFn(const char *p_Str);

uint64_t hash64bit(unsigned long key);

int fullRead(int p_FD, void *p_Buf, size_t p_NBytes);

int fullWrite(int p_FD, const void *p_Buf, size_t p_NBytes);

int fullPipeSplice(int p_InFd, int p_OutFd, size_t p_NBytes);

void createPipe(const char *p_PipeName, __mode_t p_Mode);

void openPipe(char *p_PipeName, int p_Flags, int *p_OutFd);

int fullReadv(int p_FD, struct iovec *p_IOVec, int p_IOVCount);

int fullWritev(int p_FD, struct iovec *p_IOVec, int p_IOVCount);

int createShmObject(const char *p_Name, int p_Oflag, mode_t p_Mode,
                    loff_t p_Size, uint8_t p_Unlink);

#endif // DSP_COMMONS_H
