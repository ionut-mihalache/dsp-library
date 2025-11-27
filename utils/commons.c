#include "commons.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#ifdef linux
#include <sys/mman.h>
#include <sys/user.h>
#endif

#ifdef _WIN32
#include <Windows.h>
#include <tchar.h>
#endif

#include "log/log.h"
#include "macros/macros.h"

int createShmObject(const char *p_Name, int p_Oflag, mode_t p_Mode,
                    loff_t p_Size, uint8_t p_Unlink) {
#ifdef linux
    int rc;
    int shmFd;
    uint8_t shouldTruncate = true;

    int oldMask = umask(0);

    if (p_Unlink) {
        shm_unlink(p_Name); // TODO: This should not happen all the time.
    }

    shmFd = shm_open(p_Name, O_CREAT | O_EXCL | p_Oflag, p_Mode);
    if (shmFd < 0) {
        if (errno == EEXIST) {
            shouldTruncate = false;
            shmFd = shm_open(p_Name, p_Oflag, p_Mode);
            goto end;
            // DIE(shmFd < 0, "Could not open shared memory object");
        } else {
            // DIE(shmFd < 0, "Could not open shared memory object");
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
    rc = ftruncate(shmFd, p_Size);
    if (rc < 0) {
        ELOGF("There was an error with ftruncate: %s(%d).\n", strerror(errno),
              errno);
    }
    DIE(rc != 0, "Could not truncate shared memory object");

    umask(oldMask);

end:
    return shmFd;
#endif

#ifdef _WIN32
    HANDLE hMapFile;

    hMapFile =
        CreateFileMapping(INVALID_HANDLE_VALUE, // use paging file
                          NULL,                 // default security
                          PAGE_READWRITE,       // read/write access
                          0,       // maximum object size (high-order DWORD)
                          p_Size,  // maximum object size (low-order DWORD)
                          p_Name); // name of mapping object
    DIE(hMapFile == NULL, "Could not create shared memory object");
#endif
}

void createQ(void **p_QPtrRes, size_t p_Size, int p_Prot, int p_Fd) {
#ifdef linux
    *p_QPtrRes = mmap(NULL, p_Size, p_Prot, MAP_SHARED | MAP_POPULATE, p_Fd, 0);
    DIE(*p_QPtrRes == MAP_FAILED, "Could not map return queue memory");
#endif

#ifdef _WIN32
    // TODO: Map the file for windows
#endif
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
