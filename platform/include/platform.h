// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __AQUA_PLATFORM_H_
#define __AQUA_PLATFORM_H_

#include "aqua-types.h"
#include "platform-types.h"

struct AQUA_Allocator {
    aqua_void_ptr_t (*memmap)(aqua_void_ptr_t addr, aqua_size_t len,
                              aqua_mem_prot_t prot, aqua_mem_flags_t flags,
                              aqua_file_handle_t handle, aqua_off_t off);
    aqua_int_t (*memunmap)(aqua_void_ptr_t addr, aqua_size_t len);
};

extern struct AQUA_Allocator Allocator;

#endif // __AQUA_PLATFORM_H_
