// SPDX-License-Identifier: LGPL-2.1-or-later

#include <unistd.h>

#include "platform.h"

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

struct AQUA_Memory Memory = {
    .getPageSize = sf_GetPageSize,
    .getMapGranularity = sf_GetMapGranularity,
};
