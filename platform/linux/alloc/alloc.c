// SPDX-License-Identifier: LGPL-2.1-or-later

#include <sys/mman.h>

#include "aqua-types.h"
#include "platform-types.h"
#include "platform.h"

#include "system-values.h"

static int mapProt(aqua_mem_prot_t p_Prot) {
    int prot = 0;

    if (p_Prot & AQUA_MEM_PROT_READ) {
        prot |= PROT_READ;
    }

    if (p_Prot & AQUA_MEM_PROT_WRITE) {
        prot |= PROT_WRITE;
    }

    if (p_Prot & AQUA_MEM_PROT_EXEC) {
        prot |= PROT_EXEC;
    }

    return prot;
}

static int mapFlags(aqua_mem_flags_t p_Flags) {
    int flags = 0;

    if (p_Flags & AQUA_MEM_SHARED) {
        flags |= MAP_SHARED;
    }

    if (p_Flags & AQUA_MEM_PRIVATE) {
        flags |= MAP_PRIVATE;
    }

    if (p_Flags & AQUA_MEM_ANONYMOUS) {
        flags |= MAP_ANONYMOUS;
    }

    return flags;
}

static aqua_void_ptr_t memmap(aqua_void_ptr_t p_Addr, aqua_size_t p_Len,
                              aqua_mem_prot_t p_Prot, aqua_mem_flags_t p_Flags,
                              aqua_file_handle_t p_Fd, aqua_off_t p_Off) {
    int prot = mapProt(p_Prot);
    int flags = mapFlags(p_Flags);

    void *ptr = mmap(p_Addr, p_Len, prot, flags, p_Fd, p_Off);

    return ptr;
}

static aqua_int_t memunmap(aqua_void_ptr_t p_Addr, aqua_size_t p_Len) {
    return munmap(p_Addr, p_Len);
}

struct AQUA_Allocator Allocator = {
    .memmap = memmap,
    .memunmap = memunmap,
};
