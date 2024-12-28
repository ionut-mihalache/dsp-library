#include <string.h>

#include "commons.h"
#include "install.h"
#include "macros.h"

int32_t s_ProcessConnectRequest(struct ServiceReturnInfo *p_ReturnInfo,
                                struct ConnectRequest *p_Request,
                                struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    int returnQFd;
    int requestResponseQFd;
    uint32_t connectionIdx;

    connectionIdx = p_Request->m_ConnectionIdx;

    // LOGF("Connected for id: %u.\n", connectionIdx);

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

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQ = returnQ;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMapSize =
        p_Request->m_ReturnQSize * sizeof(struct QMBCall);

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

    if (!p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSyncInit) {
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
        DIE(rc != 0,
            "Could not init condition for full connect response queue");

        rc = pthread_cond_init(p_ReturnInfo->m_QMBQueue.m_EmptyCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for empty connect response queue");

        rc = pthread_condattr_destroy(&condAttr);
        DIE(rc != 0, "Could not destroy condition attribute object");

        p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSyncInit = true;
    }

    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQMapSize =
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation);
    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQ =
        requestResponseQ;

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
    struct ConnectResponseInformation responseInfo;

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == 0) {
        pthread_cond_wait(queue->m_FullCond, queue->m_Lock);
    }

    s_ProcessConnectRequest(p_ReturnInfo, &queue->m_Data[*queue->m_PopIdxPtr],
                            p_ConnectInfo);

    memcpy(responseInfo.m_ReturnQName,
           queue->m_Data[*queue->m_PopIdxPtr].m_ReturnQName,
           RETURNQ_NAME_MAX_SIZE);
    memcpy(responseInfo.m_ReturnRequestQName,
           queue->m_Data[*queue->m_PopIdxPtr].m_RequestResponseQName,
           RETURNQ_NAME_MAX_SIZE);
    responseInfo.m_Id = queue->m_Data[*queue->m_PopIdxPtr].m_ConnectionIdx;

    (*queue->m_PopIdxPtr) = ((*queue->m_PopIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*queue->m_Size)--;

    pthread_mutex_unlock(queue->m_Lock);

    pthread_cond_broadcast(queue->m_EmptyCond);

    /**
     * Send the response to the client to announce that the communication is
     * established
     */
    pthread_mutex_lock(p_ReturnInfo->m_ResponseQueue.m_Lock);
    while (*p_ReturnInfo->m_ResponseQueue.m_Size ==
           p_ReturnInfo->m_ResponseQueue.m_MaxSize) {
        rc = pthread_cond_wait(p_ReturnInfo->m_ResponseQueue.m_EmptyCond,
                               p_ReturnInfo->m_ResponseQueue.m_Lock);
        DIE(rc != 0, "Could not wait for response queue empty condition");
    }

    /**
     * WIP: Add the information to the response queue. Now the signal is enough
     */
    // p_ReturnInfo->m_ResponseQueue
    //     .m_Data[*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr]
    //     .m_Id = connectId;
    memcpy(&p_ReturnInfo->m_ResponseQueue
                .m_Data[*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr],
           &responseInfo, sizeof(struct ConnectResponseInformation));

    (*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr) =
        ((*p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr) + 1) %
        p_ReturnInfo->m_ResponseQueue.m_MaxSize;
    (*p_ReturnInfo->m_ResponseQueue.m_Size)++;

    pthread_mutex_unlock(p_ReturnInfo->m_ResponseQueue.m_Lock);

    pthread_cond_broadcast(p_ReturnInfo->m_ResponseQueue.m_FullCond);

    return rc;
}

static int32_t
s_ReceiveDisconnectRequest(struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    uint32_t idx;
    uint32_t connId;
    struct DisconnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == 0) {
        pthread_cond_wait(queue->m_FullCond, queue->m_Lock);
    }

    idx = *queue->m_PopIdxPtr;

    connId = queue->m_Data[idx].m_ConnectionIdx;

    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);

    rc = munmap(p_ConnectInfo->m_Connections[connId].m_RequestResponseQ,
                p_ConnectInfo->m_Connections[connId].m_RequestResponseQMapSize);
    DIE(rc < 0, "Could not unmap request response queue");

    rc = munmap(p_ConnectInfo->m_Connections[connId].m_ReturnQ,
                p_ConnectInfo->m_Connections[connId].m_ReturnQMapSize);
    DIE(rc < 0, "Could not unmap return queue");

    // rc =
    //     shm_unlink(p_ConnectInfo->m_Connections[connId].m_RequestResponseQName);
    // DIE(rc < 0, "Could not unlink request response queue shared memory
    // object");

    // shm_unlink(p_ConnectInfo->m_Connections[connId].m_ReturnQName);
    // DIE(rc < 0, "Could not unlink return queue shared memory object");

    p_ConnectInfo->m_Connections[connId].m_Connected = false;

    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);

    (*queue->m_PopIdxPtr) = ((*queue->m_PopIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*queue->m_Size)--;

    pthread_mutex_unlock(queue->m_Lock);

    pthread_cond_broadcast(queue->m_EmptyCond);

    return rc;
}

int32_t configureConnectInformation(struct ServiceConnectInfo *p_ConnectInfo,
                                    struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    int connectQFd, disconnectQFd;

    p_InstallInfo->m_ConnectQPushIdx = 0;
    p_InstallInfo->m_ConnectQPopIdx = 0;
    p_InstallInfo->m_ConnectQSize = 0;

    p_InstallInfo->m_DisconnectQPushIdx = 0;
    p_InstallInfo->m_DisconnectQPopIdx = 0;
    p_InstallInfo->m_DisconnectQSize = 0;

    connectQFd = createShmObject(
        p_InstallInfo->m_ConnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Could not map connect queue memory");

    rc = close(connectQFd);
    DIE(rc != 0, "Could not close connectQFd");

    disconnectQFd = createShmObject(
        p_InstallInfo->m_DisconnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *disconnectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, disconnectQFd, 0);
    DIE(disconnectQ == MAP_FAILED, "Could not map disconnect queue memory");

    rc = close(disconnectQFd);
    DIE(rc != 0, "Could not close disconnectQFd");

    p_ConnectInfo->m_ReceiveConnectRequest = s_ReceiveConnectRequest;
    p_ConnectInfo->m_Queue.m_Data = connectQ;
    p_ConnectInfo->m_Queue.m_PushIdxPtr = &p_InstallInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_Queue.m_PopIdxPtr = &p_InstallInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_Queue.m_Size = &p_InstallInfo->m_ConnectQSize;
    p_ConnectInfo->m_Connections = p_InstallInfo->m_Connections;

    p_ConnectInfo->m_ReceiveDisconnectRequest = s_ReceiveDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;
    p_ConnectInfo->m_DisconnectQ.m_PushIdxPtr =
        &p_InstallInfo->m_DisconnectQPushIdx;
    p_ConnectInfo->m_DisconnectQ.m_PopIdxPtr =
        &p_InstallInfo->m_DisconnectQPopIdx;
    p_ConnectInfo->m_DisconnectQ.m_Size = &p_InstallInfo->m_DisconnectQSize;

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pshread for mutex attribute");

    rc = pthread_mutex_init(&p_InstallInfo->m_ConnectQMutex, &attr);
    DIE(rc != 0, "Could not init connect mutex");

    rc = pthread_mutex_init(&p_InstallInfo->m_DisconnectQMutex, &attr);
    DIE(rc != 0, "Could not init disconnect mutex");

    // rc = pthread_mutex_init(&p_InstallInfo->m_ConnectListLock, &attr);
    // DIE(rc != 0, "Could not init opened connections lock");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    rc = pthread_spin_init(&p_InstallInfo->m_ConnectListLock,
                           PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not init opened connections lock");

    p_ConnectInfo->m_ConnectLock = &p_InstallInfo->m_ConnectListLock;

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);

    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&p_InstallInfo->m_ConnectQFullCond, &condAttr);
    pthread_cond_init(&p_InstallInfo->m_ConnectQEmptyCond, &condAttr);

    pthread_cond_init(&p_InstallInfo->m_DisconnectQFullCond, &condAttr);
    pthread_cond_init(&p_InstallInfo->m_DisconnectQEmptyCond, &condAttr);

    pthread_condattr_destroy(&condAttr);

    p_ConnectInfo->m_Queue.m_Lock = &p_InstallInfo->m_ConnectQMutex;
    p_ConnectInfo->m_Queue.m_FullCond = &p_InstallInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_Queue.m_EmptyCond = &p_InstallInfo->m_ConnectQEmptyCond;

    p_ConnectInfo->m_DisconnectQ.m_Lock = &p_InstallInfo->m_DisconnectQMutex;
    p_ConnectInfo->m_DisconnectQ.m_FullCond =
        &p_InstallInfo->m_DisconnectQFullCond;
    p_ConnectInfo->m_DisconnectQ.m_EmptyCond =
        &p_InstallInfo->m_DisconnectQEmptyCond;

    return rc;
}
