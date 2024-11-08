#ifndef __DSP_H_
#define __DSP_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define SHMEM_PATH "/shared_memory"
#define CONNECT_REQS "/conn-reqs"

void hello();

#endif // __DSP_H_
