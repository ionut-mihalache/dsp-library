#ifndef __DSP_H_
#define __DSP_H_

#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHMEM_PATH "/shared_memory"

void hello();
void increment();
int getValue();

void connect();
void install();

#endif // __DSP_H_