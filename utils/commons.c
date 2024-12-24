#include "commons.h"
#include "../dsp.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "exit/exit_header.h"
#include "log/log.h"
#include "macros/macros.h"

int createShmObject(const char *p_Name, int p_Oflag, mode_t p_Mode,
                    loff_t p_Size, uint8_t p_Unlink) {
    int rc;
    int shmFd;
    uint8_t shouldTruncate = true;

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

end:
    return shmFd;
}
