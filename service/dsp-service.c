#include "dsp-service.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "call.h"
#include "commons.h"
#include "dsp.h"
#include "install.h"
#include "log.h"
#include "macros.h"
#include "protocol.h"

static struct InstallSharedData *installShdata = NULL;

void initService() {
    int rc;
    int installShdFd;

    installShdFd = createShmObject(INSTALL_MZONE, O_RDWR,
                                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                       S_IROTH | S_IWOTH,
                                   sizeof(struct InstallSharedData), true);
    DIE(installShdFd < 0, "Could not create install shared memory object");

    installShdata = mmap(0, sizeof(struct InstallSharedData),
                         PROT_READ | PROT_WRITE, MAP_SHARED, installShdFd, 0);
    DIE(installShdata == MAP_FAILED || installShdata == NULL,
        "Could not mmap install shared data object");

    rc = close(installShdFd);
    DIE(rc != 0, "Could not close installShdFd");

    rc = pthread_spin_init(&installShdata->m_InstallMZoneLk,
                           PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not init install shared spinlock");
}

void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                struct ServiceCallInfo *p_CallInfo, const char *p_StrId,
                const char *p_Version, int p_CallQType) {
    int rc;
    int installShmFd;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    initService();

    installShmFd = createShmObject(INSTALL_MZONE, O_RDWR,
                                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                       S_IROTH | S_IWOTH,
                                   sizeof(struct InstallInfo), true);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone =
        mmap(installShdata, sizeof(struct InstallInfo), PROT_READ | PROT_WRITE,
             MAP_SHARED, installShmFd, 0);
    DIE(installMemZone == MAP_FAILED, "Could not mmap install memory zone");

    int32_t freeIdx = -1;
    uint8_t *freeBytePtr = NULL;
    uint32_t freeByteIdx = 0;

    pthread_spin_lock(&installShdata->m_InstallMZoneLk);
    for (uint8_t i = 0; i < bytesnr; ++i) {
        freeBytePtr = &installMemZone->m_InstallMap[i];

        for (uint8_t j = 7; j > 0; --j) {
            freeByteIdx++;

            if (((*freeBytePtr) & (1 << j)) == 0) {
                /**
                 * We set the bit index for the current byte
                 */
                freeIdx = j;
                goto check_free_index;
            }
        }
    }

check_free_index:
    if (freeIdx < 0) {
        ELOGF("Cannot install a new service!\n");
        goto spin_lock_unlock;
    }

    *freeBytePtr = (*freeBytePtr) | (1 << freeIdx);

spin_lock_unlock:
    pthread_spin_unlock(&installShdata->m_InstallMZoneLk);

    rc = munmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");

    /**
     * Map only the information of the service
     */
    struct InstallInformation *installInfo = (struct InstallInformation *)mmap(
        NULL, sizeof(struct InstallInformation), PROT_READ | PROT_WRITE,
        MAP_SHARED, installShmFd,
        freeByteIdx * sizeof(struct InstallInformation));
    DIE(installInfo == MAP_FAILED, "Could not map service information");

    rc = close(installShmFd);
    DIE(rc != 0, "Could not close installShmFd");

    installInfo->m_ProcId = getpid();
    installInfo->m_Available = true;
    uint64_t strIdLen = strlen(p_StrId);
    if (strIdLen > STRING_ID_MAX_LENGTH - 1) {
        ELOGF("Service string id %s is too long. Max length is %i.\n", p_StrId,
              STRING_ID_MAX_LENGTH - 1);
        return;
    }
    memset(installInfo->m_StrId, 0, STRING_ID_MAX_LENGTH);
    memcpy(installInfo->m_StrId, p_StrId, strIdLen);

    uint64_t versionLen = strlen(p_Version);
    if (versionLen > VERSION_MAX_LENGTH - 1) {
        ELOGF("Version string %s is too long. Max length is %u.\n", p_Version,
              VERSION_MAX_LENGTH - 1);
        return;
    }
    memset(installInfo->m_Version, 0, VERSION_MAX_LENGTH);
    memcpy(installInfo->m_Version, p_Version, versionLen);

    /**
     * Map memory for the call and return queues
     */
    if (strIdLen + versionLen + 10 > CALLQ_NAME_MAX_SIZE) {
        ELOGF("Could not create call queue.\n");
        return;
    }
    memset(installInfo->m_CallQName, 0, CALLQ_NAME_MAX_SIZE);
    sprintf(installInfo->m_CallQName, "%s-%s-call-q", p_StrId, p_Version);

    if (strIdLen + versionLen + 10 > RETURNQ_NAME_MAX_SIZE) {
        ELOGF("Could not create call queue.\n");
        return;
    }

    memset(installInfo->m_ConnectQName, 0, CONNECTQ_NAME_MAX_SIZE);
    sprintf(installInfo->m_ConnectQName, "%s-%s-connect-q", p_StrId, p_Version);
    memset(installInfo->m_DisconnectQName, 0, CONNECTQ_NAME_MAX_SIZE);
    sprintf(installInfo->m_DisconnectQName, "%s-%s-disconnect-q", p_StrId,
            p_Version);

    installInfo->m_CallQPushIdx = 0;
    installInfo->m_CallQPopIdx = 0;
    installInfo->m_CallQSize = 0;
    installInfo->m_CallQType = p_CallQType;

    initializeServiceConnections(installInfo);
    configureServiceConnectInformation(p_ConnectInfo, installInfo);
    configureServiceCallInformation(p_CallInfo, installInfo);

    LOGF("Successfully installed new service: (%s, %s).\n", p_StrId, p_Version);
}

void receiveCall(void *p_CallData, struct ServiceCallInfo *p_CallInfo) {
    struct CommunicationInfo cInfo;

    cInfo.m_Q = &(p_CallInfo->m_Q);
    cInfo.m_Data = p_CallData;

    p_CallInfo->m_ReceiveCallFn(&cInfo);
}

void sendReturn(struct ServiceReturnInfo *p_ReturnInfo, void *p_ReturnData) {
    struct CommunicationInfo cInfo;

    cInfo.m_Q = &(p_ReturnInfo->m_Q);
    cInfo.m_Data = p_ReturnData;

    p_ReturnInfo->m_SendReturnFn(&cInfo);
}

void dspReturn() {}
