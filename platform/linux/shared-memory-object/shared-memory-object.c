// SPDX-License-Identifier: LGPL-2.1-or-later

#include <errno.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "aqua-types.h"
#include "platform.h"
#include "system-values.h"

static int mapFlags(aqua_file_flags_t p_Flags) {
    int flags = 0;

    if (p_Flags & AQUA_FILE_PERM_READ) {
        flags |= O_RDONLY;
    }

    if (p_Flags & AQUA_FILE_PERM_WRITE) {
        flags |= O_WRONLY;
    }

    if (p_Flags & AQUA_FILE_PERM_RDWR) {
        flags |= O_RDWR;
    }

    return flags;
}

static mode_t mapMode(aqua_file_mode_t p_Mode) {
    mode_t flags = 0;

    if (p_Mode & AQUA_FILE_MODE_USER_READ) {
        flags |= S_IRUSR;
    }

    if (p_Mode & AQUA_FILE_MODE_USER_WRITE) {
        flags |= S_IWUSR;
    }

    if (p_Mode & AQUA_FILE_MODE_GROUP_READ) {
        flags |= S_IRGRP;
    }

    if (p_Mode & AQUA_FILE_MODE_GROUP_WRITE) {
        flags |= S_IWGRP;
    }

    if (p_Mode & AQUA_FILE_MODE_OTHER_READ) {
        flags |= S_IROTH;
    }

    if (p_Mode & AQUA_FILE_MODE_OTHER_WRITE) {
        flags |= S_IWOTH;
    }

    return flags;
}

static aqua_file_handle_t sf_Create(const char *p_Name,
                                    aqua_file_flags_t p_Flags,
                                    aqua_file_mode_t p_Mode, aqua_off_t p_Size,
                                    aqua_bool_t p_Unlink) {
    // int rc;
    int shmFd;
    uint8_t shouldTruncate = true;
    int flags = mapFlags(p_Flags);
    mode_t mode = mapMode(p_Mode);

    int oldMask = umask(0);

    if (p_Unlink) {
        shm_unlink(p_Name); // TODO: This should not happen all the time.
    }

    shmFd = shm_open(p_Name, O_CREAT | O_EXCL | flags, mode);
    if (shmFd < 0) {
        if (errno == EEXIST) {
            shouldTruncate = false;
            shmFd = shm_open(p_Name, flags, mode);
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
    ftruncate(shmFd, p_Size);
    // if (rc < 0) {
    //     ELOGF("There was an error with ftruncate: %s(%d).\n",
    //     strerror(errno),
    //           errno);
    // }
    // DIE(rc != 0, "Could not truncate shared memory object");

    umask(oldMask);

end:
    return shmFd;

    return -1;
}

static aqua_void_t sf_Destroy() {}

struct AQUA_SharedMemoryObject SharedMemoryObject = {
    .create = sf_Create,
    .destroy = sf_Destroy,
};
