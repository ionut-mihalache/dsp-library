// SPDX-License-Identifier: LGPL-2.1-or-later

#include "platform.h"

// #include <stdlib.h>
#include <sys/mman.h>
#include <stdio.h>

static void *memmap() {
    printf("%s\n", "It looks like it works...");
    return NULL;
}

static void memunmap() {
    return;
}

struct AQUA_Allocator Allocator = {
    .memmap = memmap,
    .memunmap = memunmap,
};
