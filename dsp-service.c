#include "dsp.h"
#include "dsp-service.h"
#include "protocol.h"
#include "utils/commons.h"

#include "utils/log/log.h"

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

    rc = ftruncate(installShdFd, sizeof(struct InstallSharedData));
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
    LOGF("Service initialized.\n");
}

void dspInstall(const char *p_StrId, const char *p_Version) {
    int rc;
    int installShmFd;
    int callQFd;
    uint8_t shouldTruncate = true;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    initService();

    installShmFd = createShmObject(INSTALL_MZONE, O_RDWR, 0600, bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)));

    uint8_t *installMemZone = mmap(0,
            bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)),
            PROT_READ | PROT_WRITE, MAP_SHARED, installShmFd, 0);
    assert(installMemZone != MAP_FAILED);

    int32_t freeIdx = -1;
    uint8_t *freeBytePtr = NULL;
    uint32_t freeByteIdx = 0;

    pthread_spin_lock(&installShdata->m_InstallMZoneLk);
    for (uint8_t i = 0; i < bytesnr; ++i) {
        freeBytePtr = installMemZone + i;

        for (uint8_t j = 7; j > 0; --j) {
            freeByteIdx++;
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
    if (freeIdx < 0) {
        LOGF("Cannot install a new service!\n");
        goto end;
    }

    *freeBytePtr = (*freeBytePtr) | (1 << freeIdx);

    struct InstallInformation *installInfo = installMemZone + bytesnr + freeByteIdx * (sizeof(struct InstallInformation));
    installInfo->m_ProcId = getpid();
    installInfo->m_Available = true;
    uint64_t strIdLen = strlen(p_StrId);
    if (strIdLen > STRING_ID_MAX_LENGTH - 1) {
        ELOGF("Service string id %s is too long. Max length is %i.\n", p_StrId, STRING_ID_MAX_LENGTH - 1);
        return;
    }
    memset(installInfo->m_StrId, 0, STRING_ID_MAX_LENGTH);
    memcpy(installInfo->m_StrId, p_StrId, strIdLen);

    uint64_t versionLen = strlen(p_Version);
    if (versionLen > VERSION_MAX_LENGTH - 1) {
        ELOGF("Version string %s is too long. Max length is %u.\n", p_Version, VERSION_MAX_LENGTH - 1);
        return;
    }
    memset(installInfo->m_Version, 0, VERSION_MAX_LENGTH);
    memcpy(installInfo->m_Version, p_Version, versionLen);


    /**
     * Map memory for the call and return queues
     */
    char *callQName, *returnQName;
    asprintf(&callQName, "%s-%s-call-q", p_StrId, p_Version);
    assert(callQName != NULL);
    asprintf(&returnQName, "%s-%s-return-q", p_StrId, p_Version);
    assert(returnQName != NULL);

    installInfo->m_CallQPushIdx = 0;
    installInfo->m_CallQPopIdx = 0;
    installInfo->m_ReturnQPushIdx = 0;
    installInfo->m_ReturnQPopIdx = 0;

    callQFd = createShmObject(callQName, O_RDWR, 0600, CALLQ_MAX_SIZE * sizeof(struct HMBCall));
    createShmObject(returnQName, O_RDWR, 0600, RETURNQ_MAX_SIZE * sizeof(struct QMBCall));

    installInfo->m_CallQ = mmap(NULL, CALLQ_MAX_SIZE * sizeof(struct HMBCall), PROT_READ, MAP_SHARED, callQFd, 0);
    assert(installInfo->m_CallQ != MAP_FAILED);

end:
    pthread_spin_unlock(&installShdata->m_InstallMZoneLk);

    LOGF("Successfully installed new service: (%s, %s).\n", p_StrId, p_Version);

    return;
}

void dspReturn() {
}
