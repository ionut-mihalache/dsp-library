// SPDX-License-Identifier: LGPL-2.1-or-later

#include "commons.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/user.h>

#include "macros/macros.h"
#include "platform-types.h"
#include "platform.h"
#include "system-values.h"

aqua_size_t alignUp(aqua_size_t p_Base, aqua_size_t p_Alignment) {
    return (p_Base + p_Alignment - 1) & ~(p_Alignment - 1);
}

void createQ(void **p_QPtrRes, aqua_size_t p_Size, int p_Prot, int p_Fd) {
    // *p_QPtrRes = mmap(NULL, p_Size, p_Prot, MAP_SHARED | MAP_POPULATE, p_Fd,
    // 0);
    *p_QPtrRes =
        Allocator.memmap(NULL, p_Size, p_Prot, AQUA_MEM_SHARED, p_Fd, 0);
    DIE(*p_QPtrRes == MAP_FAILED, "Could not map return queue memory");
}

void triggerKernelPageInit(void *p_MemoryAddr, size_t p_Size, int p_Prot) {
    volatile char *accessPtr = (volatile char *)p_MemoryAddr;
    size_t pageIdx;
    switch (p_Prot) {
    case PROT_READ:
        for (size_t i = 0; i < p_Size; i++) {
            (void)accessPtr[i];
        }
        break;
    case PROT_WRITE:
    case PROT_READ | PROT_WRITE:
        for (pageIdx = 0; pageIdx < p_Size; pageIdx += PAGE_SIZE) {
            accessPtr[pageIdx] = 0;
        }
        break;
    default:
        // In case of any other permission nothing happens for now
        break;
    }
}
