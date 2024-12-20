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

int32_t processConnectRequest(struct ServiceReturnInfo *p_ReturnInfo,
                              struct ConnectRequest *p_Request,
                              struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    int returnQFd;
    int requestResponseQFd;
    uint32_t connectionIdx;

    connectionIdx = p_Request->m_ConnectionIdx;

    /**
     * WIP: Consider saving the request response and return queues names
     */

    returnQFd = createShmObject(
        p_Request->m_ReturnQName, O_RDWR, 0600,
        p_Request->m_ReturnQSize * sizeof(struct QMBCall), false);

    requestResponseQFd = createShmObject(
        p_Request->m_RequestResponseQName, O_RDWR, 0600,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        false);

    struct QMBCall *returnQ =
        mmap(NULL, p_Request->m_ReturnQSize * sizeof(struct QMBCall),
             PROT_WRITE, MAP_SHARED, returnQFd, 0);
    DIE(returnQ == MAP_FAILED, "Could not map return queue memory");

    struct ConnectResponseInformation *requestResponseQ = mmap(
        NULL,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        PROT_WRITE, MAP_SHARED, requestResponseQFd, 0);
    DIE(requestResponseQ == MAP_FAILED,
        "Could not map request response queue memory");

    rc = close(returnQFd);
    DIE(rc != 0, "Could not close return queue shared object file descriptor");

    rc = close(requestResponseQFd);
    DIE(rc != 0,
        "Could not close request response queue shared object file descriptor");

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPushIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPopIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSize = 0;

    p_ReturnInfo->m_QMBQueue.m_Data = returnQ;

    p_ReturnInfo->m_QMBQueue.m_FullCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQFullCond;
    p_ReturnInfo->m_QMBQueue.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQEmptyCond;
    p_ReturnInfo->m_QMBQueue.m_Lock =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMutex;

    p_ReturnInfo->m_QMBQueue.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPushIdx;
    p_ReturnInfo->m_QMBQueue.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPopIdx;
    p_ReturnInfo->m_QMBQueue.m_Size =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSize;

    p_ReturnInfo->m_ResponseQueue.m_MaxSize = p_Request->m_ReturnQSize;

    p_ReturnInfo->m_SendReturnFnQMB =
        NULL; // TODO: This has to be a valid value

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pthread shared for mutex attribute");

    rc = pthread_mutex_init(p_ReturnInfo->m_QMBQueue.m_Lock, &attr);
    DIE(rc != 0, "Could not init connect response lock");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    pthread_condattr_t condAttr;

    rc = pthread_condattr_init(&condAttr);
    DIE(rc != 0, "Could not init condition attribute");

    rc = pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pthread shared for condition attribute");

    rc = pthread_cond_init(p_ReturnInfo->m_QMBQueue.m_FullCond, &condAttr);
    DIE(rc != 0, "Could not init condition for full connect response queue");

    rc = pthread_cond_init(p_ReturnInfo->m_QMBQueue.m_EmptyCond, &condAttr);
    DIE(rc != 0, "Could not init condition for empty connect response queue");

    rc = pthread_condattr_destroy(&condAttr);
    DIE(rc != 0, "Could not destroy condition attribute object");

    p_ReturnInfo->m_ResponseQueue.m_Data = requestResponseQ;

    p_ReturnInfo->m_ResponseQueue.m_FullCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQFullCond;
    p_ReturnInfo->m_ResponseQueue.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connectionIdx]
             .m_RequestResponseQEmptyCond;
    p_ReturnInfo->m_ResponseQueue.m_Lock =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQMutex;

    p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPushIdx;
    p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPopIdx;
    p_ReturnInfo->m_ResponseQueue.m_Size =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQSize;
    p_ReturnInfo->m_ResponseQueue.m_MaxSize = p_Request->m_ResponseQSize;

    return rc;
}

static int32_t
s_ReceiveConnectRequest(struct ServiceReturnInfo *p_ReturnInfo,
                        struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    struct ConnectQueue *queue = &p_ConnectInfo->m_Queue;
    uint32_t connectId;

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == 0) {
        pthread_cond_wait(queue->m_FullCond, queue->m_Lock);
    }

    processConnectRequest(p_ReturnInfo, &queue->m_Data[*queue->m_PopIdxPtr],
                          p_ConnectInfo);

    connectId = queue->m_Data[*queue->m_PopIdxPtr].m_ConnectionIdx;

    (*queue->m_PopIdxPtr) = ((*queue->m_PopIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*queue->m_Size)--;

    pthread_cond_broadcast(queue->m_EmptyCond);

    pthread_mutex_unlock(queue->m_Lock);

    /**
     * Send the response to the client to announce that the communication is
     * established
     */
    pthread_mutex_lock(p_ReturnInfo->m_ResponseQueue.m_Lock);
    while (*p_ReturnInfo->m_ResponseQueue.m_Size ==
           p_ReturnInfo->m_ResponseQueue.m_MaxSize) {
        pthread_cond_wait(p_ReturnInfo->m_ResponseQueue.m_FullCond,
                          p_ReturnInfo->m_ResponseQueue.m_Lock);
    }

    /**
     * WIP: Add the information to the response queue. Now the signal is enough
     */
    p_ReturnInfo->m_ResponseQueue
        .m_Data[*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr]
        .m_Id = connectId;

    (*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr) =
        ((*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr) + 1) %
        p_ReturnInfo->m_ResponseQueue.m_MaxSize;
    (*p_ReturnInfo->m_ResponseQueue.m_Size)++;

    pthread_cond_broadcast(p_ReturnInfo->m_ResponseQueue.m_FullCond);

    pthread_mutex_unlock(p_ReturnInfo->m_ResponseQueue.m_Lock);

    return rc;
}

static int32_t
s_ReceiveDisconnectRequest(struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    uint32_t idx;
    struct ConnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);
    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == 0) {
        pthread_cond_wait(queue->m_FullCond, queue->m_Lock);
    }

    idx = *queue->m_PopIdxPtr;
    LOGF("Received disconnect request for connection id: %u.\n",
         queue->m_Data[idx].m_ConnectionIdx);

    (*queue->m_PopIdxPtr) = ((*queue->m_PopIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*queue->m_Size)--;

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
    int callQFd, connectQFd, disconnectQFd;
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
    sprintf(installInfo->m_DisconnectQName, "%s-%s-disconnect-q", p_StrId,
            p_Version);

    installInfo->m_CallQPushIdx = 0;
    installInfo->m_CallQPopIdx = 0;
    installInfo->m_CallQSize = 0;

    installInfo->m_ConnectQPushIdx = 0;
    installInfo->m_ConnectQPopIdx = 0;
    installInfo->m_ConnectQSize = 0;

    installInfo->m_DisconnectQPushIdx = 0;
    installInfo->m_DisconnectQPopIdx = 0;
    installInfo->m_DisconnectQSize = 0;

    connectQFd = createShmObject(
        installInfo->m_ConnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Could not map connect queue memory");

    rc = close(connectQFd);
    DIE(rc != 0, "Could not close connectQFd");

    disconnectQFd = createShmObject(
        installInfo->m_DisconnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *disconnectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, disconnectQFd, 0);
    DIE(disconnectQ == MAP_FAILED, "Could not map disconnect queue memory");

    rc = close(disconnectQFd);
    DIE(rc != 0, "Could not close disconnectQFd");

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

    p_ConnectInfo->m_ReceiveDisconnectRequest = s_ReceiveDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;
    p_ConnectInfo->m_DisconnectQ.m_PushIdxPtr =
        &installInfo->m_DisconnectQPushIdx;
    p_ConnectInfo->m_DisconnectQ.m_PopIdxPtr =
        &installInfo->m_DisconnectQPopIdx;
    p_ConnectInfo->m_DisconnectQ.m_Size = &installInfo->m_DisconnectQSize;

    p_CallInfo->m_ReceiveCallFnQMB = s_QPopQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Size = &installInfo->m_CallQSize;

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pshread for mutex attribute");

    rc = pthread_mutex_init(&installInfo->m_CallQMutex, &attr);
    DIE(rc != 0, "Could not init call mutex");

    rc = pthread_mutex_init(&installInfo->m_ConnectQMutex, &attr);
    DIE(rc != 0, "Could not init connect mutex");

    rc = pthread_mutex_init(&installInfo->m_DisconnectQMutex, &attr);
    DIE(rc != 0, "Could not init disconnect mutex");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    rc = pthread_spin_init(&installInfo->m_ConnectListLock,
                           PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not init opened connections lock");

    p_ConnectInfo->m_ConnectLock = &installInfo->m_ConnectListLock;

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);

    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&installInfo->m_CallQFullCond, &condAttr);
    pthread_cond_init(&installInfo->m_CallQEmptyCond, &condAttr);

    pthread_cond_init(&installInfo->m_ConnectQFullCond, &condAttr);
    pthread_cond_init(&installInfo->m_ConnectQEmptyCond, &condAttr);

    pthread_cond_init(&installInfo->m_DisconnectQFullCond, &condAttr);
    pthread_cond_init(&installInfo->m_DisconnectQEmptyCond, &condAttr);

    pthread_condattr_destroy(&condAttr);

    p_ConnectInfo->m_Queue.m_Lock = &installInfo->m_ConnectQMutex;
    p_ConnectInfo->m_Queue.m_FullCond = &installInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_Queue.m_EmptyCond = &installInfo->m_ConnectQEmptyCond;

    p_ConnectInfo->m_DisconnectQ.m_Lock = &installInfo->m_DisconnectQMutex;
    p_ConnectInfo->m_DisconnectQ.m_FullCond =
        &installInfo->m_DisconnectQFullCond;
    p_ConnectInfo->m_DisconnectQ.m_EmptyCond =
        &installInfo->m_DisconnectQEmptyCond;

    p_CallInfo->m_QMBQueue.m_Lock = &installInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_FullCond = &installInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_EmptyCond = &installInfo->m_CallQEmptyCond;

end:
    pthread_spin_unlock(&installShdata->m_InstallMZoneLk);

    LOGF("Successfully installed new service: (%s, %s).\n", p_StrId, p_Version);
}

void dspReturn() {}
