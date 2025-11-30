#include "commons.h"

#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>

#if defined(__linux__)
#include <errno.h>
#include <sys/mman.h>
#endif

#if defined(_WIN32)
#include <Windows.h>
#endif

#include "dsp.h"
#include "macros.h"
#include "system-values.h"

aqua_file_handle createShmObject(const char *p_Name, int p_Oflag,
                                 aqua_mode_t p_Mode, aqua_object_size_t p_Size,
                                 uint8_t p_Unlink) {
    aqua_file_handle handle;

#if defined(__linux__)
    int rc;
    uint8_t shouldTruncate = true;

    int oldMask = umask(0);

    if (p_Unlink) {
        shm_unlink(p_Name); // TODO: This should not happen all the time.
    }

    handle = shm_open(p_Name, O_CREAT | O_EXCL | p_Oflag, p_Mode);
    if (handle < 0) {
        if (errno == EEXIST) {
            shouldTruncate = false;
            handle = shm_open(p_Name, p_Oflag, p_Mode);
            goto end;
            // DIE(handle < 0, "Could not open shared memory object");
        } else {
            // DIE(handle < 0, "Could not open shared memory object");
            goto end;
        }
    }

    if (!shouldTruncate) {
        goto end;
    }

    // Prepare space to allow installation for SERVICES_NUMBER services at most
    // We need a bit map for fast iteration
    // Get the number of bytes for the bit map
    // The information that we need is an array of pointers to the information
    // that we need
    rc = ftruncate(handle, p_Size);
    DIE(rc != 0, "Could not truncate shared memory object");

    umask(oldMask);
end:
#endif

#if defined(_WIN32)
    handle = CreateFileMapping(INVALID_HANDLE_VALUE, // use paging file
                               p_Mode,               // default security
                               PAGE_READWRITE,       // read/write access
                               0,      // maximum object size (high-order DWORD)
                               p_Size, // maximum object size (low-order DWORD)
                               p_Name); // name of mapping object
    DIE(handle == NULL, "Could not create shared memory object");
#endif

    return handle;
}

aqua_void_t createQ(aqua_void_t **p_QPtrRes, aqua_size_t p_Size,
                    aqua_prot_t p_Prot, aqua_file_handle p_FileHandle) {
#if defined(__linux__)
    *p_QPtrRes =
        mmap(NULL, p_Size, p_Prot, MAP_SHARED | MAP_POPULATE, p_FileHandle, 0);
    DIE(*p_QPtrRes == MAP_FAILED, "Could not map return queue memory");
#endif

#if defined(_WIN32)
    BOOL bRet;

    *p_QPtrRes = MapViewOfFile(p_FileHandle, // handle to map object
                               p_Prot, 0, 0, p_Size);
    DIE(*p_QPtrRes == NULL, "Could not map return queue memory");

    bRet = VirtualLock(*p_QPtrRes, p_Size);
    DIE(bRet == FALSE, "Could not lock memory in RAM");
#endif
}

aqua_void_t triggerKernelPageInit(aqua_void_t *p_MemoryAddr, aqua_size_t p_Size,
                                  int p_Prot) {
    volatile char *accessPtr = (volatile char *)p_MemoryAddr;
    aqua_size_t pageIdx;

    switch (p_Prot) {
    case AQUA_PROT_READ:
        for (aqua_size_t i = 0; i < p_Size; i++) {
            (aqua_void_t) accessPtr[i];
        }
        break;
    case AQUA_PROT_WRITE:
    case AQUA_PROT_READ | AQUA_PROT_WRITE:
        for (pageIdx = 0; pageIdx < p_Size; pageIdx += PAGE_SIZE) {
            accessPtr[pageIdx] = 0;
        }
        break;
    default:
        // In case of any other permission nothing happens for now
        break;
    }
}
