#include "dsp-service.h"

#include <stdbool.h>
#include <string.h>

#include "call.h"
#include "commons.h"
#include "dsp.h"
#include "install.h"
#include "log.h"
#include "macros.h"
#include "system-values.h"

static struct InstallSharedData *installShdata = NULL;

void initService(void) {
    int rc;
    aqua_file_handle installShdHandle;
#if defined(_WIN32)
    char qSyncName[RETURNQ_NAME_MAX_SIZE];
#endif

    installShdHandle =
        createShmObject(INSTALL_MZONE, O_RDWR,
                        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP |
                            AQUA_S_IWGRP | AQUA_S_IROTH | AQUA_S_IWOTH,
                        sizeof(struct InstallSharedData), true);

    createQSimple((aqua_void_t **)&installShdata,
                  sizeof(struct InstallSharedData),
                  AQUA_PROT_READ | AQUA_PROT_WRITE, installShdHandle);

    triggerKernelPageInit(installShdata, sizeof(struct InstallSharedData),
                          AQUA_PROT_READ | AQUA_PROT_WRITE);

#if defined(__linux__)
    rc = close(installShdHandle);
    DIE(rc != 0, "Could not close installShdHandle");

    rc = pthread_spin_init(&installShdata->m_InstallMZoneLk,
                           PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not init install shared spinlock");
#elif defined(_WIN32)
    snprintf(qSyncName, sizeof(qSyncName), "%s-%s", "sharedRegion", "lock");
    installShdata->m_InstallMZoneLk = CreateMutex(NULL, FALSE, qSyncName);
    DIE(installShdata->m_InstallMZoneLk == NULL,
        "Could not init install shared lock");
#else
#endif
}

void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                struct ServiceCallInfo *p_CallInfo, const char *p_StrId,
                const char *p_Version, int p_CallQType) {
    int rc;
    aqua_file_handle installShmHandle;
    struct InstallInfo *installMemZone = NULL;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;
    int32_t freeIdx = -1;
    uint8_t *freeBytePtr = NULL;
    uint32_t freeByteIdx = 0;
    struct InstallInformation *installInfo;
    uint64_t strIdLen;
    uint64_t versionLen;

    initService();

    installShmHandle =
        createShmObject(INSTALL_MZONE "123", O_RDWR,
                        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP |
                            AQUA_S_IWGRP | AQUA_S_IROTH | AQUA_S_IWOTH,
                        sizeof(struct InstallInfo), true);

    createQSimple((aqua_void_t **)&installMemZone, sizeof(struct InstallInfo),
                  AQUA_PROT_READ | AQUA_PROT_WRITE, installShmHandle);

#if defined(__linux__)
    pthread_spin_lock(&installShdata->m_InstallMZoneLk);
    for (uint8_t i = 0; i < bytesnr; ++i) {
        freeBytePtr = &installMemZone->m_InstallMap[i];

        for (int8_t j = 7; j > 0; --j) {
            if (((*freeBytePtr) & (1 << j)) == 0) {
                /**
                 * We set the bit index for the current byte
                 */
                freeIdx = j;
                goto check_free_index;
            }

            freeByteIdx++;
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
#elif defined(_WIN32)
    WaitForSingleObject(installShdata->m_InstallMZoneLk, INFINITE);
    for (uint8_t i = 0; i < bytesnr; ++i) {
        freeBytePtr = &installMemZone->m_InstallMap[i];

        for (int8_t j = 7; j > 0; --j) {
            if (((*freeBytePtr) & (1 << j)) == 0) {
                /**
                 * We set the bit index for the current byte
                 */
                freeIdx = j;
                goto check_free_index;
            }
            freeByteIdx++;
        }
    }

check_free_index:
    if (freeIdx < 0) {
        ELOGF("Cannot install a new service!\n");
        goto spin_lock_unlock;
    }

    *freeBytePtr = (*freeBytePtr) | (1 << freeIdx);

spin_lock_unlock:
    ReleaseMutex(installShdata->m_InstallMZoneLk);

    DIE(!UnmapViewOfFile(installMemZone),
        "Could not unmap install memory zone");
#else
#endif

    /**
     * Map only the information of the service
     */
#if defined(__linux__)
    installInfo = (struct InstallInformation *)mmap(
        NULL, sizeof(struct InstallInformation), PROT_READ | PROT_WRITE,
        MAP_SHARED, installShmHandle,
        freeByteIdx * sizeof(struct InstallInformation));
    DIE(installInfo == MAP_FAILED, "Could not map service information");

    rc = close(installShmHandle);
    DIE(rc != 0, "Could not close installShmHandle");

    installInfo->m_ProcId = getpid();
#elif defined(_WIN32)
    LPVOID mapBase;
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    SIZE_T gran = si.dwAllocationGranularity;
    SIZE_T alignedOffset =
        (freeByteIdx * sizeof(struct InstallInformation)) & ~(gran - 1);
    SIZE_T offsetDelta =
        (freeByteIdx * sizeof(struct InstallInformation)) - alignedOffset;

    mapBase = MapViewOfFile(installShmHandle, // handle to map object
                            AQUA_PROT_READ | AQUA_PROT_WRITE,
                            (DWORD)(alignedOffset >> 32),
                            (DWORD)(alignedOffset & 0xFFFFFFFF),
                            offsetDelta + sizeof(struct InstallInformation));
    DIE(installInfo == NULL, "Could not map service information");

    // DIE(!CloseHandle(installShmHandle), "Could not close
    // installShmHandle");

    installInfo = (struct InstallInformation *)((char *)mapBase + offsetDelta);
    installInfo->m_ProcId = GetCurrentProcessId();
#else
#endif

    installInfo->m_Available = true;
    strIdLen = strlen(p_StrId);
    if (strIdLen > STRING_ID_MAX_LENGTH - 1) {
        ELOGF("Service string id %s is too long. Max length is %i.\n", p_StrId,
              STRING_ID_MAX_LENGTH - 1);
        return;
    }
    memset(installInfo->m_StrId, 0, STRING_ID_MAX_LENGTH);
    memcpy(installInfo->m_StrId, p_StrId, strIdLen);

    versionLen = strlen(p_Version);
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

    // InterlockedExchange(&installInfo->m_CallQWaitConsume, 0);
    // InterlockedExchange(&installInfo->m_CallQWaitProduce, 0);
    // InterlockedExchange(&installInfo->m_CallQPushIdxAtomic, 0);
    // InterlockedExchange(&installInfo->m_CallQPopIdxAtomic, 0);
    // InterlockedExchange(&installInfo->m_CallQSizeAtomic, 0);

    installInfo->m_CallQType = (enum QType)p_CallQType;

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
