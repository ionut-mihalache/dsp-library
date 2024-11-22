#include "dsp.h"
#include "dsp-service.h"
#include "protocol.h"

#include "log/log.h"

#include <sys/ipc.h>
#include <sys/shm.h>

#include <sys/mman.h>
#include <string.h>
#include <stdbool.h>

static struct InstallCommons *installCommons = NULL;
static struct InstallSharedData *installShdata = NULL;

static char *ptr = NULL;

void initService() {
    int rc;
    int installShdFd;
    int shouldReturn = false;

    LOGF("Service init...\n");

    // shm_unlink(INSTALL_SHD); // TODO: This should not happen all the time.
    installShdFd = shm_open(INSTALL_SHD, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (installShdFd < 0) {
        if (errno == EEXIST) {
            shouldReturn = true;
            installShdFd = shm_open(INSTALL_SHD, O_RDWR, 0600);
            if (installShdFd < 0) {
                ELOGF("Error when creating shared object: %s(%d).\n", strerror(errno), errno);
            }
            assert(installShdFd >= 0);
        }
    }

    if (shouldReturn) {
        goto map_info;
    }

    LOGF("Install shared file descriptor is %d.\n", installShdFd);
    rc = ftruncate(installShdFd, sizeof(struct InstallSharedData));
    LOGF("Install shared data structure file descriptor is %d.\n", sizeof(struct InstallSharedData));
    if (rc < 0) {
        ELOGF("There was an error with ftruncate: %s(%d).\n", strerror(errno), errno);
    }
    assert(rc == 0);

map_info:
    installShdata = mmap(0, sizeof(struct InstallSharedData), PROT_READ | PROT_WRITE, MAP_SHARED, installShdFd, 0);
    assert(installShdata != MAP_FAILED && installShdata != NULL);

    // rc = pthread_mutex_init(&installShdata->m_InstallMZoneMx, NULL);
    // assert(rc == 0);

    rc = pthread_spin_init(&installShdata->m_InstallMZoneLk, PTHREAD_PROCESS_SHARED);
    assert(rc == 0);
    ELOGF("The error message is %s(%d).\n", strerror(errno), errno);
    LOGF("Service initialized.\n");
}

void install() {
    int rc;
    int installShmFd;
    uint8_t shouldTruncate = true;

    initService();

    LOGF("Installing new service...\n");

    // shm_unlink(INSTALL_MZONE); // TODO: This should not happen all the time.
    installShmFd = shm_open(INSTALL_MZONE, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (installShmFd < 0) {
        if (errno == EEXIST) {
            shouldTruncate = false;
            installShmFd = shm_open(INSTALL_MZONE, O_RDWR, 0600);
            assert(installShmFd >= 0);
        }
    }

    if (!shouldTruncate) {
        goto find_free_zone;
    }

    // Prepare space to allow installation for SERVICES_NUMBER services at most
    // We need a bit map for fast iteration
    // Get the number of bytes for the bit map
    // The information that we need is an array of pointers to the information that we need
    uint8_t bytesnr = SERVICES_NUMBER >> 3;
    rc = ftruncate(installShmFd, bytesnr + (SERVICES_NUMBER * sizeof(char *)));
    if (rc < 0) {
        ELOGF("There was an error with ftruncate: %s(%d).\n", strerror(errno), errno);
    }
    assert(rc == 0);

find_free_zone:
;
    uint8_t *installMemZone = mmap(0,
            bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)), 
            PROT_READ | PROT_WRITE, MAP_SHARED, installShmFd, 0);
    assert(installMemZone != MAP_FAILED);

    int32_t freeIdx = -1;
    uint8_t *freeBytePtr = NULL;

    pthread_spin_lock(&installShdata->m_InstallMZoneLk);
    for (uint8_t i = 0; i < bytesnr; ++i) {
        freeBytePtr = (uint8_t *)installMemZone + i * bytesnr;
        for (uint8_t j = 0; j < 8; ++j) {
            /**
             * Get the index for the correct bit inside the service map
             * 
             * E.g: 2 bytes are used for the service map
             * b7 b6 b5 b4 b3 b2 b1 b0 | b7 b6 b5 b4 b3 b2 b1 b0
             */
            if (((*freeBytePtr) & (1 << j)) == 0) {
                /**
                 * We set the bit index for the current byte
                 */
                freeIdx = j;
                goto spin_lock_unlock;
            }
        }
    }

spin_lock_unlock:
    pthread_spin_unlock(&installShdata->m_InstallMZoneLk);

    LOGF("The index for the free memory zone is: %d\n", *freeBytePtr + freeIdx);
    (*freeBytePtr) |= 1 << freeIdx;
    LOGF("Service installed.\n");
}
