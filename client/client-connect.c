#include <string.h>

#include "client-connect.h"
#include "commons.h"
#include "macros.h"

static int32_t m_ReturnFnQMB(struct QMBCall *p_ReturnData,
                             struct QMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == 0) {
        pthread_cond_wait(p_Queue->m_FullCond, p_Queue->m_Lock);
    }

    memcpy(p_ReturnData, &p_Queue->m_Data[*p_Queue->m_PopIdxPtr],
           sizeof(struct QMBCall));

    (*p_Queue->m_PopIdxPtr) = ((*p_Queue->m_PopIdxPtr) + 1) % RETURNQ_MAX_SIZE;
    (*p_Queue->m_Size)--;

    pthread_cond_broadcast(p_Queue->m_EmptyCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

static int32_t s_ProcessConnectionRequest(
    struct ClientReturnInfo *p_ReturnInfo,
    struct ConnectRequest *p_ConnectRequest,
    struct ClientConnectInfo *p_ConnectInfo,
    struct ClientConnectRequestInformation *p_ConnectInformation) {
    int requestResponseQFd;
    int returnQFd;
    int32_t rc = 0;
    uint32_t connId;

    /**
     *  search for a free spot in the opened connections for the service
     *  send the request to the service with the connection index
     *  WIP: wait for the service to establish the connection on its side
     */
    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);
    for (connId = 0; connId < OPENED_CONNECTIONS; ++connId) {
        if (!p_ConnectInfo->m_Connections[connId].m_Connected) {
            p_ConnectInfo->m_Connections[connId].m_Connected = true;
            break;
        }
    }
    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);

    /**
     * With the connection index found we need to construct the request for the
     * service
     */
    p_ConnectRequest->m_ConnectionIdx = connId;

    memcpy(p_ConnectInfo->m_Connections[connId].m_RequestResponseQName,
           p_ConnectInformation->m_ReturnQName,
           strlen(p_ConnectInformation->m_ReturnQName));
    memcpy(p_ConnectInfo->m_Connections[connId].m_ReturnQName,
           p_ConnectInformation->m_RequestResponseQName,
           strlen(p_ConnectInformation->m_RequestResponseQName));

    memset(p_ConnectRequest->m_ReturnQName, 0, RETURNQ_NAME_MAX_SIZE);
    memcpy(p_ConnectRequest->m_ReturnQName, p_ConnectInformation->m_ReturnQName,
           strlen(p_ConnectInformation->m_ReturnQName));

    memset(p_ConnectRequest->m_RequestResponseQName, 0, RETURNQ_NAME_MAX_SIZE);
    memcpy(p_ConnectRequest->m_RequestResponseQName,
           p_ConnectInformation->m_RequestResponseQName,
           strlen(p_ConnectInformation->m_RequestResponseQName));

    p_ReturnInfo->m_QMBQueue.m_MaxSize = RETURNQ_MAX_SIZE;
    p_ConnectRequest->m_ReturnQSize =
        RETURNQ_MAX_SIZE; // CHECK: possibly user specified

    p_ReturnInfo->m_ResponseQueue.m_MaxSize = RETURN_RESPONSEQ_MAX_SIZE;
    p_ConnectRequest->m_ResponseQSize =
        RETURN_RESPONSEQ_MAX_SIZE; // CHECK: possibly user specified

    requestResponseQFd =
        createShmObject(p_ConnectInformation->m_RequestResponseQName, O_RDWR,
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
                        p_ConnectInformation->m_ResponseQSize *
                            sizeof(struct ConnectResponseInformation),
                        true);
    DIE(requestResponseQFd < 0, "Could not create shared memory object");

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
        p_ConnectInformation->m_ReturnQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        p_ConnectInformation->m_ReturnQSize * sizeof(struct QMBCall), true);

    struct QMBCall *returnQ =
        mmap(NULL, p_ConnectInformation->m_ReturnQSize * sizeof(struct QMBCall),
             PROT_READ, MAP_SHARED, returnQFd, 0);
    DIE(returnQ == MAP_FAILED, "Could not map return queue memory");

    rc = close(returnQFd);
    DIE(rc != 0, "Could not close returnQFd");

    p_ConnectInfo->m_Connections[connId].m_RequestResponseQPushIdx = 0;
    p_ConnectInfo->m_Connections[connId].m_RequestResponseQPopIdx = 0;
    p_ConnectInfo->m_Connections[connId].m_RequestResponseQSize = 0;

    p_ReturnInfo->m_ResponseQueue.m_Data = requestResponseQ;
    p_ReturnInfo->m_ResponseQueue.m_FullCond =
        &p_ConnectInfo->m_Connections[connId].m_RequestResponseQFullCond;
    p_ReturnInfo->m_ResponseQueue.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connId].m_RequestResponseQEmptyCond;
    p_ReturnInfo->m_ResponseQueue.m_Lock =
        &p_ConnectInfo->m_Connections[connId].m_RequestResponseQMutex;
    p_ReturnInfo->m_ResponseQueue.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connId].m_RequestResponseQPushIdx;
    p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connId].m_RequestResponseQPopIdx;
    p_ReturnInfo->m_ResponseQueue.m_Size =
        &p_ConnectInfo->m_Connections[connId].m_RequestResponseQSize;

    p_ReturnInfo->m_QMBQueue.m_Data = returnQ;
    p_ReturnInfo->m_QMBQueue.m_FullCond =
        &p_ConnectInfo->m_Connections[connId].m_ReturnQFullCond;
    p_ReturnInfo->m_QMBQueue.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connId].m_ReturnQEmptyCond;
    p_ReturnInfo->m_QMBQueue.m_Lock =
        &p_ConnectInfo->m_Connections[connId].m_ReturnQMutex;
    p_ReturnInfo->m_QMBQueue.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connId].m_ReturnQPushIdx;
    p_ReturnInfo->m_QMBQueue.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connId].m_ReturnQPopIdx;
    p_ReturnInfo->m_QMBQueue.m_Size =
        &p_ConnectInfo->m_Connections[connId].m_ReturnQSize;

    p_ReturnInfo->m_ReturnFnQMB = m_ReturnFnQMB;

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
    idx = *p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr;

    memcpy(&p_ReturnInfo->m_ConnectResponseInformation,
           &p_ReturnInfo->m_ResponseQueue.m_Data[idx],
           sizeof(struct ConnectResponseInformation));

    (*p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr) =
        ((*p_ReturnInfo->m_ResponseQueue.m_PopIdxPtr) + 1) %
        p_ReturnInfo->m_ResponseQueue.m_MaxSize;
    (*p_ReturnInfo->m_ResponseQueue.m_Size)--;

    pthread_cond_broadcast(p_ReturnInfo->m_ResponseQueue.m_EmptyCond);

    pthread_mutex_unlock(p_ReturnInfo->m_ResponseQueue.m_Lock);

    return rc;
}

static int32_t
s_SendDisconnectRequest(struct ClientConnectInfo *p_ConnectInfo,
                        struct ConnectResponseInformation *p_ResponseInfo) {
    int32_t rc = 0;
    uint32_t idx;
    uint32_t connId;

    struct DisconnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    pthread_mutex_lock(queue->m_Lock);
    while (*queue->m_Size == CONNECTQ_MAX_SIZE) {
        pthread_cond_wait(queue->m_EmptyCond, queue->m_Lock);
    }

    idx = *queue->m_PushIdxPtr;

    connId = p_ResponseInfo->m_Id;

    queue->m_Data[idx].m_ConnectionIdx = connId;
    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);

    // p_ReturnInfo->

    // rc = munmap(p_ConnectInfo->m_Connections[connId].m_RequestResponseQ,
    //             p_ConnectInfo->m_Connections[connId].m_RequestResponseQMapSize);
    // DIE(rc < 0, "Could not unmap request response queue");

    // rc = munmap(p_ConnectInfo->m_Connections[connId].m_ReturnQ,
    //             p_ConnectInfo->m_Connections[connId].m_ReturnQMapSize);
    // DIE(rc < 0, "Could not unmap return queue");

    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);

    (*queue->m_PushIdxPtr) = ((*queue->m_PushIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*queue->m_Size)++;

    pthread_cond_broadcast(queue->m_FullCond);

    pthread_mutex_unlock(queue->m_Lock);

    return rc;
}

int32_t
configureClientConnectInformation(struct ClientConnectInfo *p_ConnectInfo,
                                  struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    int connectQFd, disconnectQFd;

    /**
     * TODO: Implement successfull connection functionality
     */

    connectQFd = createShmObject(
        p_InstallInfo->m_ConnectQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), false);

    struct ConnectRequest *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Could not map connectQ");

    rc = close(connectQFd);
    DIE(rc != 0, "Could not close connectQFd");

    disconnectQFd = createShmObject(
        p_InstallInfo->m_DisconnectQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), false);

    struct ConnectRequest *disconnectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, disconnectQFd, 0);
    DIE(disconnectQ == MAP_FAILED, "Could not map disconnect queue memory");

    rc = close(disconnectQFd);
    DIE(rc != 0, "Could not close disconnectQFd");

    p_ConnectInfo->m_SendConnectRequest = s_SendConnectRequest;
    p_ConnectInfo->m_Connections = p_InstallInfo->m_Connections;
    p_ConnectInfo->m_Queue.m_Data = connectQ;
    p_ConnectInfo->m_Queue.m_PushIdxPtr = &p_InstallInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_Queue.m_PopIdxPtr = &p_InstallInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_Queue.m_Size = &p_InstallInfo->m_ConnectQSize;
    p_ConnectInfo->m_Queue.m_Lock = &p_InstallInfo->m_ConnectQMutex;
    p_ConnectInfo->m_Queue.m_FullCond = &p_InstallInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_Queue.m_EmptyCond = &p_InstallInfo->m_ConnectQEmptyCond;
    p_ConnectInfo->m_ConnectLock = &p_InstallInfo->m_ConnectListLock;

    p_ConnectInfo->m_SendDisconnectRequest = s_SendDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;
    p_ConnectInfo->m_DisconnectQ.m_PushIdxPtr =
        &p_InstallInfo->m_DisconnectQPushIdx;
    p_ConnectInfo->m_DisconnectQ.m_PopIdxPtr =
        &p_InstallInfo->m_DisconnectQPopIdx;
    p_ConnectInfo->m_DisconnectQ.m_Size = &p_InstallInfo->m_DisconnectQSize;
    p_ConnectInfo->m_DisconnectQ.m_Lock = &p_InstallInfo->m_DisconnectQMutex;
    p_ConnectInfo->m_DisconnectQ.m_FullCond =
        &p_InstallInfo->m_DisconnectQFullCond;
    p_ConnectInfo->m_DisconnectQ.m_EmptyCond =
        &p_InstallInfo->m_DisconnectQEmptyCond;

    return rc;
}