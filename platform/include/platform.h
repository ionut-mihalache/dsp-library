// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __AQUA_PLATFORM_H_
#define __AQUA_PLATFORM_H_

#include "aqua-sync.h"
#include "aqua-types.h"
#include "platform-types.h"

struct AQUA_Memory {
    aqua_size_t (*getPageSize)();
    aqua_size_t (*getMapGranularity)();
};

struct AQUA_Allocator {
    aqua_void_ptr_t (*memmap)(aqua_void_ptr_t addr, aqua_size_t len,
                              aqua_mem_prot_t prot, aqua_mem_flags_t flags,
                              aqua_file_handle_t handle, aqua_off_t off);
    aqua_int_t (*memunmap)(aqua_void_ptr_t addr, aqua_size_t len);
};

struct AQUA_Sync {
    aqua_void_t (*createMutex)(aqua_mutex_t *mutex, const char *name);
    aqua_void_t (*createCond)(aqua_cond_t *cond, const char *name);
    aqua_void_t (*createSemaphore)(aqua_sem_t *sem, const char *name);
    aqua_void_t (*destroyMutex)(aqua_mutex_t *mutex);
    aqua_void_t (*destroyCond)(aqua_cond_t *cond);
    aqua_void_t (*destroySemaphore)(aqua_sem_t *sem);

    aqua_void_t (*mutexLock)(aqua_mutex_t *mutex);
    aqua_void_t (*mutexUnlock)(aqua_mutex_t *mutex);
    aqua_void_t (*condWait)(aqua_cond_t *cond, aqua_mutex_t *mutex);
    aqua_void_t (*condBroadcast)(aqua_cond_t *cond);
};

struct AQUA_SharedMemoryObject {
    aqua_file_handle_t (*create)(const char *name, aqua_file_flags_t flags,
                                 aqua_file_mode_t mode, aqua_off_t size,
                                 aqua_bool_t unlink);
    aqua_void_t (*destroy)();
};

extern struct AQUA_Memory Memory;
extern struct AQUA_Allocator Allocator;
extern struct AQUA_Sync Sync;
extern struct AQUA_SharedMemoryObject SharedMemoryObject;

#endif // __AQUA_PLATFORM_H_
