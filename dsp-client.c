#include "dsp.h"

#include "dsp-client.h"
#include "protocol.h"
#include "utils/commons.h"

#include "utils/log/log.h"
#include "utils/macros/macros.h"

#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>

static int32_t s_ProcessConnectionRequest(
    struct ClientReturnInfo *p_ReturnInfo,
    struct ConnectRequest *p_ConnectRequest,
    struct ClientConnectInfo *p_ConnectInfo,
    struct ClientConnectRequestInformation *p_ConnectInformation) {
    int requestResponseQFd;
    int returnQFd;
    int32_t rc = 0;
    uint32_t connectionIdx;

    /**
     *  search for a free spot in the opened connections for the service
     *  send the request to the service with the connection index
     *  WIP: wait for the service to establish the connection on its side
     */
    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);
    for (connectionIdx = 0; connectionIdx < OPENED_CONNECTIONS;
         ++connectionIdx) {
        if (!p_ConnectInfo->m_Connections[connectionIdx].m_Connected) {
            break;
        }
    }
    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);

    /**
     * With the connection index found we need to construct the request for the
     * service
     */
    p_ConnectRequest->m_ConnectionIdx = connectionIdx;
    p_ConnectInfo->m_Connections[connectionIdx].m_Connected = true;

    memcpy(p_ConnectRequest->m_ReturnQName, p_ConnectInformation->m_ReturnQName,
           min(strlen(p_ConnectInformation->m_ReturnQName),
               RETURNQ_NAME_MAX_SIZE));

    memcpy(p_ConnectRequest->m_RequestResponseQName,
           p_ConnectInformation->m_RequestResponseQName,
           min(strlen(p_ConnectInformation->m_RequestResponseQName),
               RETURNQ_NAME_MAX_SIZE));

    p_ReturnInfo->m_QMBQueue.m_MaxSize = 1;
    p_ConnectRequest->m_ReturnQSize =
        1; // TODO: possibly change this to another (non-hardcoded) value

    p_ReturnInfo->m_ResponseQueue.m_MaxSize = 1;
    p_ConnectRequest->m_ResponseQSize =
        1; // TODO: possibly change this to another (non-hardcoded) value

    requestResponseQFd = createShmObject(
        p_ConnectInformation->m_RequestResponseQName, O_RDWR, 0600,
        p_ConnectInformation->m_ResponseQSize *
            sizeof(struct ConnectResponseInformation),
        true);

    struct ConnectResponseInformation *requestResponseQ =
        mmap(NULL,
             p_ConnectInformation->m_ResponseQSize *
                 sizeof(struct ConnectResponseInformation),
             PROT_READ, MAP_SHARED, requestResponseQFd, 0);
    DIE(requestResponseQ == MAP_FAILED,
        "Could not map request response queue memory");

    rc = close(requestResponseQFd);
    DIE(rc != 0, "Could not close requestResponseQFd");

    returnQFd = createShmObject(
        p_ConnectInformation->m_ReturnQName, O_RDWR, 0600,
        p_ConnectInformation->m_ReturnQSize * sizeof(struct QMBCall), true);

    struct QMBCall *returnQ =
        mmap(NULL, p_ConnectInformation->m_ReturnQSize * sizeof(struct QMBCall),
             PROT_READ, MAP_SHARED, returnQFd, 0);
    DIE(returnQ == MAP_FAILED, "Could not map return queue memory");

    rc = close(returnQFd);
    DIE(rc != 0, "Could not close returnQFd");

    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPushIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPopIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQSize = 0;

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

    p_ReturnInfo->m_ReturnFnQMB = NULL; // TODO: This has to be a valid value

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pthread shared for mutex attribute");

    rc = pthread_mutex_init(p_ReturnInfo->m_ResponseQueue.m_Lock, &attr);
    DIE(rc != 0, "Could not init connect response lock");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    pthread_condattr_t condAttr;

    rc = pthread_condattr_init(&condAttr);
    DIE(rc != 0, "Could not init condition attribute");

    rc = pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pthread shared for condition attribute");

    rc = pthread_cond_init(p_ReturnInfo->m_ResponseQueue.m_FullCond, &condAttr);
    DIE(rc != 0, "Could not init condition for full connect response queue");

    rc =
        pthread_cond_init(p_ReturnInfo->m_ResponseQueue.m_EmptyCond, &condAttr);
    DIE(rc != 0, "Could not init condition for empty connect response queue");

    rc = pthread_condattr_destroy(&condAttr);
    DIE(rc != 0, "Could not destroy condition attribute object");

    return rc;
}

static int32_t
s_SendDisconnectRequest(struct ClientConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;

    return rc;
}

static int32_t
s_SendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                     struct ClientConnectInfo *p_ConnectInfo,
                     struct ClientConnectRequestInformation *p_RequestInfo) {
    int32_t rc = 0;
    uint32_t idx;
    struct ConnectQueue *queue = &p_ConnectInfo->m_Queue;

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == CONNECTQ_MAX_SIZE) {
        pthread_cond_wait(queue->m_EmptyCond, queue->m_Lock);
    }

    idx = *queue->m_PushIdxPtr;
    s_ProcessConnectionRequest(p_ReturnInfo, &queue->m_Data[idx], p_ConnectInfo,
                               p_RequestInfo);

    (*queue->m_PushIdxPtr) = ((*queue->m_PushIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*queue->m_Size)++;

    pthread_cond_broadcast(queue->m_FullCond);

    pthread_mutex_unlock(queue->m_Lock);

    /**
     * Wait for the response from the service to announce that the communication
     * is established
     */
    pthread_mutex_lock(p_ReturnInfo->m_ResponseQueue.m_Lock);
    while (*p_ReturnInfo->m_ResponseQueue.m_Size == 0) {
        pthread_cond_wait(p_ReturnInfo->m_ResponseQueue.m_FullCond,
                          p_ReturnInfo->m_ResponseQueue.m_Lock);
    }

    /**
     * WIP: Add the information to the response queue. Now the signal is enough
     */

    (*p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr) =
        ((*p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr) + 1) %
        p_ReturnInfo->m_ResponseQueue.m_MaxSize;
    (*p_ReturnInfo->m_ResponseQueue.m_Size)--;

    pthread_cond_broadcast(p_ReturnInfo->m_ResponseQueue.m_EmptyCond);

    pthread_mutex_unlock(p_ReturnInfo->m_ResponseQueue.m_Lock);

    return rc;
}

static int32_t s_QPushQMB(struct QMBDSPQueue *p_Queue,
                          struct QMBCall *p_CallInfo) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == QMB_Q_MAX_SIZE) {
        pthread_cond_wait(p_Queue->m_EmptyCond, p_Queue->m_Lock);
    }

    memcpy(&p_Queue->m_Data[*p_Queue->m_PushIdxPtr], p_CallInfo,
           sizeof(struct QMBCall));

    (*p_Queue->m_PushIdxPtr) = ((*p_Queue->m_PushIdxPtr) + 1) % QMB_Q_MAX_SIZE;
    (*p_Queue->m_Size)++;

    pthread_cond_broadcast(p_Queue->m_FullCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

__attribute_used__ static int32_t s_QPushHMB(struct HMBDSPQueue *p_Queue,
                                             struct HMBCall *p_CallInfo) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == HMB_Q_MAX_SIZE) {
        pthread_cond_wait(p_Queue->m_EmptyCond, p_Queue->m_Lock);
    }

    memcpy(&p_Queue->m_Data[*p_Queue->m_PushIdxPtr], p_CallInfo,
           sizeof(struct HMBCall));

    (*p_Queue->m_PushIdxPtr) = ((*p_Queue->m_PushIdxPtr) + 1) % HMB_Q_MAX_SIZE;
    (*p_Queue->m_Size)++;

    pthread_cond_broadcast(p_Queue->m_FullCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

void sendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                        struct ClientConnectInfo *p_ConnectInfo,
                        struct ClientConnectRequestInformation *p_RequestInfo) {
    p_ConnectInfo->m_SendConnectRequest(p_ReturnInfo, p_ConnectInfo,
                                        p_RequestInfo);
}

void pushQ(struct ClientCallInfo *p_CallInfo) {
    p_CallInfo->m_CallFn(&p_CallInfo->m_Queue);
}

void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData) {
    p_CallInfo->m_CallFnQMB(&p_CallInfo->m_QMBQueue, p_CallData);
}

void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData) {
    p_CallInfo->m_CallFnHMB(&p_CallInfo->m_HMBQueue, p_CallData);
}

int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size) {
    int32_t rc = 0;

    memcpy(p_CallInfo->m_CallInfo, p_Data, p_Size);
    p_CallInfo->m_Size = p_Size;

    return rc;
}

int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size) {
    int32_t rc = 0;

    memcpy(p_CallInfo->m_CallInfo, p_Data, p_Size);
    p_CallInfo->m_Size = p_Size;

    return rc;
}

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId) {
    int rc;
    int installShmFd;
    int callQFd, connectQFd;
    struct InstallInformation *installInfo;
    uint8_t connected = false;
    uint16_t i;

    installShmFd = createShmObject(INSTALL_MZONE, O_RDWR, 0600,
                                   sizeof(struct InstallInfo), false);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone =
        mmap(NULL, sizeof(struct InstallInfo), PROT_READ | PROT_WRITE,
             MAP_SHARED, installShmFd, 0);
    DIE(installMemZone == MAP_FAILED, "Could not mmap install memory zone");

    for (i = 0; i < SERVICES_NUMBER; ++i) {
        if (!installMemZone->m_Info[i].m_Available) {
            continue;
        }

        installInfo = (struct InstallInformation *)&(installMemZone->m_Info[i]);

        if (!strcmp(installInfo->m_StrId, p_ServiceStrId)) {
            if (installInfo->m_Available) {
                connected = true;
            }
            break;
        }
    }

    if (!connected) {
        ELOGF("Could not connect. Service is not installed or unavailable.\n");
        return;
    }

    rc = munmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");

    /**
     * Map only the information of the service
     */
    installInfo = (struct InstallInformation *)mmap(
        NULL, sizeof(struct InstallInformation), PROT_READ | PROT_WRITE,
        MAP_SHARED, installShmFd, i * sizeof(struct InstallInformation));
    DIE(installInfo == MAP_FAILED, "Could not map service information");

    /**
     * TODO: Implement successfull connection functionality
     */

    connectQFd = createShmObject(
        installInfo->m_ConnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), false);

    struct ConnectRequest *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Could not map connectQ");

    rc = close(connectQFd);
    DIE(rc != 0, "Could not close connectQFd");

    p_ConnectInfo->m_SendConnectRequest = s_SendConnectRequest;
    p_ConnectInfo->m_Connections = installInfo->m_Connections;
    p_ConnectInfo->m_Queue.m_Data = connectQ;
    p_ConnectInfo->m_Queue.m_PushIdxPtr = &installInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_Queue.m_PopIdxPtr = &installInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_Queue.m_Size = &installInfo->m_ConnectQSize;
    p_ConnectInfo->m_Queue.m_Lock = &installInfo->m_ConnectQMutex;
    p_ConnectInfo->m_Queue.m_FullCond = &installInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_Queue.m_EmptyCond = &installInfo->m_ConnectQEmptyCond;
    p_ConnectInfo->m_ConnectLock = &installInfo->m_ConnectListLock;

    callQFd = createShmObject(installInfo->m_CallQName, O_RDWR, 0600,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), false);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_WRITE, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map callQ");

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_CallFnQMB = s_QPushQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Size = &installInfo->m_CallQSize;
    p_CallInfo->m_QMBQueue.m_Lock = &installInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_FullCond = &installInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_EmptyCond = &installInfo->m_CallQEmptyCond;

    // p_CallInfo->m_CallFnHMB = s_QPushHMB;
    // p_CallInfo->m_HMBQueue.m_Data = callQ;
    // p_CallInfo->m_HMBQueue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    // p_CallInfo->m_HMBQueue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;
    // p_CallInfo->m_HMBQueue.m_Size = &installInfo->m_CallQSize;
    // p_CallInfo->m_HMBQueue.m_Lock = &installInfo->m_CallQMutex;
    // p_CallInfo->m_HMBQueue.m_FullCond = &installInfo->m_CallQFullCond;
    // p_CallInfo->m_HMBQueue.m_EmptyCond = &installInfo->m_CallQEmptyCond;

    LOGF("Connected to \'%s\' with version \'%s\'.\n", p_ServiceStrId,
         installInfo->m_Version);
}

void retriveInitInformation(struct ClientConnectInfo *p_ConnectInfo,
                            struct ClientCallInfo *p_CallInfo,
                            const char *p_ServiceStrId) {
    dspConnect(p_ConnectInfo, p_CallInfo, p_ServiceStrId);
}
