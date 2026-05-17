// SPDX-License-Identifier: LGPL-2.1-or-later

#include "commons.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "macros/macros.h"
#include "platform-types.h"
#include "platform.h"
#include "system-values.h"

aqua_size_t alignUp(aqua_size_t p_Base, aqua_size_t p_Alignment) {
    return (p_Base + p_Alignment - 1) & ~(p_Alignment - 1);
}

void createQ(void **p_QPtrRes, aqua_size_t p_Size, int p_Prot, int p_Fd) {
    *p_QPtrRes =
        Allocator.memmap(NULL, p_Size, p_Prot, AQUA_MEM_SHARED, p_Fd, 0);
    DIE(*p_QPtrRes == MAP_FAILED, "Could not map return queue memory");
}
