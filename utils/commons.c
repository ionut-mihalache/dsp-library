#include "commons.h"
#include "dsp.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "exit/exit_header.h"
#include "log/log.h"
#include "macros/macros.h"

uint32_t murmur3Hash_32(const char *key, uint32_t len, uint32_t seed) {
    static const uint32_t c1 = 0xcc9e2d51;
    static const uint32_t c2 = 0x1b873593;
    static const uint32_t r1 = 15;
    static const uint32_t r2 = 13;
    static const uint32_t m = 5;
    static const uint32_t n = 0xe6546b64;

    uint32_t hash = seed;

    const int nblocks = len / 4;
    const uint32_t *blocks = (const uint32_t *)key;
    int i;
    for (i = 0; i < nblocks; i++) {
        uint32_t k = blocks[i];
        k *= c1;
        k = (k << r1) | (k >> (32 - r1));
        k *= c2;

        hash ^= k;
        hash = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }

    const uint8_t *tail = (const uint8_t *)(key + nblocks * 4);
    uint32_t k1 = 0;

    switch (len & 3) {
    case 3:
        k1 ^= tail[2] << 16;
    case 2:
        k1 ^= tail[1] << 8;
    case 1:
        k1 ^= tail[0];

        k1 *= c1;
        k1 = (k1 << r1) | (k1 >> (32 - r1));
        k1 *= c2;
        hash ^= k1;
    }

    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);

    return hash;
}

int createShmObject(const char *p_Name, int p_Oflag, mode_t p_Mode,
                    loff_t p_Size, uint8_t p_Unlink) {
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
}
