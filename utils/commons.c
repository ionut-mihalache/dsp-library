//
// Created by ionut on 9/7/2024.
//

#include "commons.h"
#include "../dsp.h"

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

#include "exit/exit_header.h"
#include "log/log.h"

uint64_t
strHashFn(const char* p_Str)
{
    static const uint32_t p = 23;
    static const uint32_t m = 1000000009;

    uint64_t hashValue = 0;
    uint64_t pPow = 1;
    const size_t strLen = strlen(p_Str);

    for (size_t i = 0; i < strLen; ++i) {
        hashValue += (hashValue + (p_Str[i] - ' ' + 1) * pPow) % m;
        pPow = (pPow * p) % m;
    }

    return hashValue;
}

uint64_t
hash64bit(unsigned long key)
{
    key = (~key) + (key << 21); // key = (key << 21) - key - 1;
    key = key ^ (key >> 24);
    key = (key + (key << 3)) + (key << 8); // key * 265
    key = key ^ (key >> 14);
    key = (key + (key << 2)) + (key << 4); // key * 21
    key = key ^ (key >> 28);
    key = key + (key << 31);

    return key;
}

/**
 * Read an exact number of bytes from a file descriptor
 * @param p_FD
 * @param p_Buf
 * @param p_NBytes
 * @return
 */
int fullRead(const int p_FD, void* p_Buf, const size_t p_NBytes)
{
    size_t currBytes = 0;

    while (currBytes < p_NBytes) {
        const ssize_t rdBytes = read(p_FD, p_Buf + currBytes, p_NBytes - currBytes);

        if (rdBytes > 0) {
            currBytes += rdBytes;
            LOGF("Current read bytes: %lu. Expected read bytes: %lu\n", currBytes, p_NBytes);
            continue;
        }

        if (rdBytes == 0) {
            return 1;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }

        ELOGF("Received error while reading: %s.\n", strerror(errno));

        return -1;
    }

    return 0;
}

/**
 * Write an exact number of bytes to a file descriptor
 * @param p_FD
 * @param p_Buf
 * @param p_NBytes
 * @return
 */
int fullWrite(const int p_FD, const void* p_Buf, const size_t p_NBytes)
{
    size_t currBytes = 0;

    while (currBytes < p_NBytes) {
        const ssize_t wrBytes = write(p_FD, p_Buf + currBytes, p_NBytes - currBytes);

        if (wrBytes > 0) {
            currBytes += wrBytes;
            LOGF("Current written bytes: %lu. Expected written bytes: %lu\n", currBytes, p_NBytes);
            continue;
        }

        if (wrBytes == 0) {
            return 1;
        }

        return -1;
    }

    return 0;
}

/**
 * Transfer all the bytes from the in pipe to the out pipe
 * @param p_InFd
 * @param p_OutFd
 * @param p_NBytes
 * @return
 */
int fullPipeSplice(const int p_InFd, const int p_OutFd, const size_t p_NBytes)
{
    size_t currBytes = 0;

    while (currBytes < p_NBytes) {
        const ssize_t trBytes = splice(p_InFd, NULL, p_OutFd, NULL, p_NBytes - currBytes, SPLICE_F_MOVE | SPLICE_F_NONBLOCK);

        if (trBytes > 0) {
            currBytes += trBytes;
            LOGF("Expected bytes %lu, Splice bytes %lu\n", p_NBytes, currBytes);
            continue;
        }

        if (trBytes == 0) {
            return 1;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }

        return -1;
    }

    return 0;
}

/**
 * Check if pipe exists and create a new one if it is not already created.
 * @param p_PipeName
 * @param p_Mode
 */
void createPipe(const char* p_PipeName, const __mode_t p_Mode)
{
    if (access(p_PipeName, F_OK) == 0) {
        LOGF("Pipe at path \'%s\' already exists.\n", p_PipeName);
    } else {
        int err = mkfifo(p_PipeName, p_Mode);
        CHECK_AND_EXIT_ERR(err < 0);
        LOGF("Created named pipe at \'%s\'.\n", p_PipeName);

        // Set the proper permissions
        err = chmod(p_PipeName, p_Mode);
        CHECK_AND_EXIT_ERR(err < 0);
    }
}

void openPipe(char* p_PipeName, const int p_Flags, int* p_OutFd)
{
    *p_OutFd = open(p_PipeName, p_Flags);
    CHECK_AND_EXIT_ERR(*p_OutFd < 0);
    LOGF("Opened file %s(%d).\n", p_PipeName, *p_OutFd);
}

/**
 * Read an exact number of iovecs from the file descriptor
 * @param p_FD
 * @param p_IOVec
 * @param p_IOVCount
 * @return
 */
int fullReadv(const int p_FD, struct iovec* p_IOVec, const int p_IOVCount)
{
    int currCnt = 0;

    while (true) {
        ssize_t rdBytes = readv(p_FD, p_IOVec, p_IOVCount - currCnt);

        if (rdBytes > 0) {
            while (currCnt < p_IOVCount && rdBytes >= p_IOVec[currCnt].iov_len)
                rdBytes -= p_IOVec[currCnt++].iov_len;
            if (currCnt == p_IOVCount) {
                break;
            }
            p_IOVec[currCnt].iov_base = (char*)p_IOVec[currCnt].iov_base + rdBytes;
            p_IOVec[currCnt].iov_len -= rdBytes;
            LOGF("Current read iovecs: %d. Expected read iovecs: %d\n", currCnt, p_IOVCount);
            continue;
        }

        if (rdBytes == 0) {
            return 1;
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            continue;
        }

        ELOGF("Received error while reading: %s.\n", strerror(errno));

        return -1;
    }

    return 0;
}

/**
 * Write an exact number of iovecs to the file descriptor
 * @param p_FD
 * @param p_IOVec
 * @param p_IOVCount
 * @return
 */
int fullWritev(const int p_FD, struct iovec* p_IOVec, const int p_IOVCount)
{
    int currCnt = 0;

    while (true) {
        ssize_t wrBytes = writev(p_FD, p_IOVec, p_IOVCount - currCnt);

        if (wrBytes > 0) {
            while (currCnt < p_IOVCount && wrBytes >= p_IOVec[currCnt].iov_len)
                wrBytes -= p_IOVec[currCnt++].iov_len;
            if (currCnt == p_IOVCount)
                break;
            p_IOVec[currCnt].iov_base = (char*)p_IOVec[currCnt].iov_base + wrBytes;
            p_IOVec[currCnt].iov_len -= wrBytes;
            continue;
        }

        if (wrBytes == 0) {
            return 1;
        }

        return -1;
    }

    return 0;
}

int createShmObject(const char* p_Name, int p_Oflag, mode_t p_Mode, loff_t p_Size, uint8_t p_Unlink)
{
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
            assert(shmFd >= 0);
        }
    }

    if (!shouldTruncate) {
        goto end;
    }

    // Prepare space to allow installation for SERVICES_NUMBER services at most
    // We need a bit map for fast iteration
    // Get the number of bytes for the bit map
    // The information that we need is an array of pointers to the information that we need
    rc = ftruncate(shmFd, p_Size);
    if (rc < 0) {
        ELOGF("There was an error with ftruncate: %s(%d).\n", strerror(errno), errno);
    }
    assert(rc == 0);

end:
    return shmFd;
}
