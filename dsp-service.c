#include "dsp-service.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/shm.h>

#include "dsp.h"
#include "protocol.h"
#include "utils/commons.h"
#include "utils/log/log.h"
#include "utils/macros/macros.h"

static struct InstallSharedData *installShdata = NULL;

int32_t processConnectRequest(struct ConnectRequestInformation *p_ResultRequest,
                              struct ConnectRequest *p_Request,
                              struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    int returnQFd;
    int returnRequestQFd;
    uint16_t openConnIdx;

    for (openConnIdx = 0; openConnIdx < OPENED_CONNECTIONS; ++openConnIdx) {
        if (!p_ConnectInfo->m_Connections[openConnIdx].m_Connected) {
            break;
        }
    }

    memcpy(p_ResultRequest->m_ReturnQName, p_Request->m_ReturnQName,
           min(strlen(p_Request->m_ReturnQName), RETURNQ_NAME_MAX_SIZE - 1));

    memcpy(p_ResultRequest->m_RequestResponseQName,
           p_Request->m_RequestResponseQName,
           min(strlen(p_Request->m_RequestResponseQName),
               RETURNQ_NAME_MAX_SIZE - 1));

    /**
     * Check implementation
     */
    returnQFd = createShmObject(p_Request->m_ReturnQName, O_WRONLY, 0600,
                                p_Request->m_ReturnQSize * sizeof(struct QMBCall), true);

    returnRequestQFd = createShmObject(
        p_ResultRequest->m_RequestResponseQName, O_RDONLY, 0600,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        true);

    struct ConnectResponseInformation *returnRequestQ = mmap(
        NULL,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        PROT_READ, MAP_SHARED, returnRequestQFd, 0);
    DIE(returnRequestQ == MAP_FAILED,
        "Could not map memory for connect request response");

    rc = close(returnRequestQFd);
    DIE(rc != 0, "Could not close return request file");

    return rc;
}

static int32_t
s_ReceiveConnectRequest(struct ConnectRequestInformation *p_Request,
                        struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    int returnRequestQFd;
    uint32_t idx;
    struct ConnectQueue *queue = &p_ConnectInfo->m_Queue;

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == 0) {
        pthread_cond_wait(queue->m_FullCond, queue->m_Lock);
    }

    idx = *queue->m_PopIdxPtr;

    processConnectRequest(p_Request, &queue->m_Data[idx], p_ConnectInfo);

    // p_Request->m_ReturnQSize = p_Queue->m_Data[idx].m_ReturnQSize;
    // p_Request->m_Connected = &p_Queue->m_Data[idx].m_Connected;
    // p_Request->m_ConnectError = &p_Queue->m_Data[idx].m_ConnectionError;

    // p_Request->m_ResponseQ.m_Data = returnRequestQ;
    // p_Request->m_ResponseQ.m_FullCond =
    //     &p_Queue->m_Data[idx].m_ReturnResponseQFullCond;
    // p_Request->m_ResponseQ.m_EmptyCond =
    //     &p_Queue->m_Data[idx].m_ReturnResponseQEmptyCond;
    // p_Request->m_ResponseQ.m_Lock =
    //     &p_Queue->m_Data[idx].m_ReturnResponseQMutex;

    (*queue->m_Size)++;

    pthread_cond_broadcast(queue->m_EmptyCond);

    pthread_mutex_unlock(queue->m_Lock);

    return rc;
}

static int32_t s_QPopQMB(struct QMBCall *p_CallInfo,
                         struct QMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == 0) {
        pthread_cond_wait(p_Queue->m_FullCond, p_Queue->m_Lock);
    }

    memcpy(p_CallInfo, &p_Queue->m_Data[*p_Queue->m_PopIdxPtr],
           sizeof(struct QMBCall));

    (*p_Queue->m_PopIdxPtr) = ((*p_Queue->m_PopIdxPtr) + 1) % QMB_Q_MAX_SIZE;
    (*p_Queue->m_Size)--;

    pthread_cond_broadcast(p_Queue->m_EmptyCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

__attribute_used__ static int32_t s_QPopHMB(struct HMBCall *p_CallInfo,
                                            struct HMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == 0) {
        pthread_cond_wait(p_Queue->m_FullCond, p_Queue->m_Lock);
    }

    memcpy(p_CallInfo, &p_Queue->m_Data[*p_Queue->m_PopIdxPtr],
           sizeof(struct HMBCall));

    LOGF("%s: Message length: %u. Message: %s.\n", __func__,
         p_Queue->m_Data[*p_Queue->m_PopIdxPtr].m_Size,
         p_Queue->m_Data[*p_Queue->m_PopIdxPtr].m_CallInfo);

    (*p_Queue->m_PopIdxPtr) = ((*p_Queue->m_PopIdxPtr) + 1) % HMB_Q_MAX_SIZE;
    (*p_Queue->m_Size)--;

    pthread_cond_broadcast(p_Queue->m_EmptyCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

void initService() {
    int rc;
    int installShdFd;

    LOGF("Service init...\n");

    installShdFd = createShmObject(INSTALL_MZONE, O_RDWR, 0600,
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
    LOGF("Service initialized.\n");
}

void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                struct ServiceCallInfo *p_CallInfo, const char *p_StrId,
                const char *p_Version) {
    int rc;
    int installShmFd;
    int callQFd, connectQFd;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    initService();

    installShmFd = createShmObject(INSTALL_MZONE, O_RDWR, 0600,
                                   sizeof(struct InstallInfo), true);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone =
        mmap(NULL, sizeof(struct InstallInfo), PROT_READ | PROT_WRITE,
             MAP_SHARED, installShmFd, 0);
    DIE(installMemZone == MAP_FAILED, "Could not mmap install memory zone");

    rc = close(installShmFd);
    DIE(rc != 0, "Could not close installShmFd");

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

    struct InstallInformation *installInfo =
        &installMemZone->m_Info[freeByteIdx];

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
    sprintf(installInfo->m_CallQName, "%s-%s-call-q", p_StrId, p_Version);

    if (strIdLen + versionLen + 10 > RETURNQ_NAME_MAX_SIZE) {
        ELOGF("Could not create call queue.\n");
        return;
    }
    sprintf(installInfo->m_ConnectQName, "%s-%s-connect-q", p_StrId, p_Version);

    installInfo->m_CallQPushIdx = 0;
    installInfo->m_CallQPopIdx = 0;
    installInfo->m_CallQSize = 0;
    installInfo->m_ConnectQPushIdx = 0;
    installInfo->m_ConnectQPopIdx = 0;
    installInfo->m_ConnectQSize = 0;

    connectQFd = createShmObject(
        installInfo->m_ConnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Coudl not map connect queue memory");

    rc = close(connectQFd);
    DIE(rc != 0, "Could not close connectQFd");

    callQFd = createShmObject(installInfo->m_CallQName, O_RDWR, 0600,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), true);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_READ, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map call queue memory");

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_ConnectInfo->m_ReceiveConnectRequest = s_ReceiveConnectRequest;
    p_ConnectInfo->m_Queue.m_Data = connectQ;
    p_ConnectInfo->m_Queue.m_PushIdxPtr = &installInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_Queue.m_PopIdxPtr = &installInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_Queue.m_Size = &installInfo->m_ConnectQSize;
    p_ConnectInfo->m_Connections = installInfo->m_Connections;

    p_CallInfo->m_ReceiveCallFnQMB = s_QPopQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Size = &installInfo->m_CallQSize;

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute.");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pshread for mutex attribute");

    rc = pthread_mutex_init(&installInfo->m_CallQMutex, &attr);
    DIE(rc != 0, "Could not init call mutex!");

    rc = pthread_mutex_init(&installInfo->m_ConnectQMutex, &attr);
    DIE(rc != 0, "Could not init connect mutex!");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Coudl not destroy mutex attribute");

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);

    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&installInfo->m_CallQFullCond, &condAttr);
    pthread_cond_init(&installInfo->m_CallQEmptyCond, &condAttr);

    pthread_cond_init(&installInfo->m_ConnectQFullCond, &condAttr);
    pthread_cond_init(&installInfo->m_ConnectQEmptyCond, &condAttr);

    pthread_condattr_destroy(&condAttr);

    p_ConnectInfo->m_Queue.m_Lock = &installInfo->m_ConnectQMutex;
    p_ConnectInfo->m_Queue.m_FullCond = &installInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_Queue.m_EmptyCond = &installInfo->m_ConnectQEmptyCond;

    p_CallInfo->m_QMBQueue.m_Lock = &installInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_FullCond = &installInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_EmptyCond = &installInfo->m_CallQEmptyCond;

end:
    pthread_spin_unlock(&installShdata->m_InstallMZoneLk);

    LOGF("Successfully installed new service: (%s, %s).\n", p_StrId, p_Version);
}

void dspReturn() {}
