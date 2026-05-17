// SPDX-License-Identifier: LGPL-2.1-or-later

#include <unistd.h>

#include "platform.h"
#include "system-values.h"

static aqua_size_t sv_PageSize = 0;

static aqua_size_t sf_GetPageSize() {
    if (__builtin_expect(sv_PageSize == 0, 0)) {
        sv_PageSize = (aqua_size_t)sysconf(_SC_PAGESIZE);
    }

    return sv_PageSize;
}

static aqua_size_t sf_GetMapGranularity() {
    return sf_GetPageSize();
}

static aqua_void_t sf_TriggerPageFaults(aqua_void_t *p_Addr, aqua_size_t p_Size,
                                        aqua_mem_perm_t p_Perm) {
    volatile char *accessPtr = (volatile char *)p_Addr;
    size_t pageIdx;

    switch (p_Perm) {
    case AQUA_MEM_PROT_READ:
        for (size_t i = 0; i < p_Size; i++) {
            (void)accessPtr[i];
        }
        break;
    case AQUA_MEM_PROT_WRITE:
    case AQUA_MEM_PROT_READ | AQUA_MEM_PROT_WRITE:
        for (pageIdx = 0; pageIdx < p_Size; pageIdx += sf_GetPageSize()) {
            accessPtr[pageIdx] = 0;
        }
        break;
    default:
        // In case of any other permission nothing happens for now
        break;
    }
}

struct AQUA_Memory Memory = {
    .getPageSize = sf_GetPageSize,
    .getMapGranularity = sf_GetMapGranularity,
    .triggerPageFaults = sf_TriggerPageFaults,
};
