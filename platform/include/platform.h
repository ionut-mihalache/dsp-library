// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __AQUA_PLATFORM_H_
#define __AQUA_PLATFORM_H_

struct AQUA_Allocator {
    void *(*memmap)();
    void (*memunmap)();
};

extern struct AQUA_Allocator Allocator;

#endif // __AQUA_PLATFORM_H_
