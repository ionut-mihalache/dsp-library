// SPDX-License-Identifier: LGPL-2.1-or-later

#include "dsp-service.h"

#include <stdbool.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "aqua-types.h"
#include "call.h"
#include "commons.h"
#include "dsp.h"
#include "install.h"
#include "log.h"
#include "macros.h"
#include "platform.h"
#include "system-values.h"

static struct InstallSharedData *installShdata = NULL;

void initService() {
    int rc;
    aqua_file_handle_t installShdFd;

    installShdFd = SharedMemoryObject.create(
        INSTALL_MZONE, AQUA_FILE_PERM_RDWR,
        AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
            AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
            AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE,
        sizeof(struct InstallSharedData), true);
    DIE(installShdFd < 0, "Could not create install shared memory object");

    installShdata = Allocator.memmap(0, sizeof(struct InstallSharedData),
                                     AQUA_MEM_PROT_READ | AQUA_MEM_PROT_WRITE,
                                     AQUA_MEM_SHARED, installShdFd, 0);
    DIE(installShdata == MAP_FAILED || installShdata == NULL,
        "Could not mmap install shared data object");

    triggerKernelPageInit(installShdata, sizeof(struct InstallSharedData),
                          PROT_READ | PROT_WRITE);

    rc = close(installShdFd);
    DIE(rc != 0, "Could not close installShdFd");

    rc = pthread_spin_init(&installShdata->m_InstallMZoneLk,
                           PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not init install shared spinlock");
}

static aqua_size_t sf_GetInstallArenaSize() {
    aqua_size_t mapGranularity = Memory.getMapGranularity();

    aqua_size_t alignedHeaderSize =
        alignUp(sizeof(struct InstallInfo), mapGranularity);

    aqua_size_t alignedServiceSize =
        alignUp(sizeof(struct InstallInformation), mapGranularity);

    return alignedHeaderSize + alignedServiceSize * SERVICES_NUMBER;
}

static aqua_size_t sf_GetServiceOff(uint16_t i) {
    aqua_size_t mapGranularity = Memory.getMapGranularity();

    size_t alignedHeaderSize =
        alignUp(sizeof(struct InstallInfo), mapGranularity);

    aqua_size_t alignedServiceSize =
        alignUp(sizeof(struct InstallInformation), mapGranularity);

    return alignedHeaderSize + i * alignedServiceSize;
}

void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                struct ServiceCallInfo *p_CallInfo, const char *p_StrId,
                const char *p_Version, int p_CallQType) {
    int rc;
    aqua_file_handle_t installShmFd;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    initService();

    aqua_size_t installArenaSize = sf_GetInstallArenaSize();

    installShmFd = SharedMemoryObject.create(
        INSTALL_MZONE, AQUA_FILE_PERM_RDWR,
        AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
            AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
            AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE,
        installArenaSize, true);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone =
        Allocator.memmap(installShdata, installArenaSize,
                         AQUA_MEM_PROT_READ | AQUA_MEM_PROT_WRITE,
                         AQUA_MEM_SHARED, installShmFd, 0);
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

    rc = Allocator.memunmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");

    /**
     * Map only the information of the service
     */
    struct InstallInformation *installInfo = Allocator.memmap(
        NULL,
        alignUp(sizeof(struct InstallInformation), Memory.getMapGranularity()),
        AQUA_MEM_PROT_READ | AQUA_MEM_PROT_WRITE, AQUA_MEM_SHARED, installShmFd,
        sf_GetServiceOff(freeByteIdx));
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
