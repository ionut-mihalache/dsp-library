#include <string.h>

#include "commons.h"
#include "install.h"
#include "log.h"
#include "macros.h"
#include "return.h"

int32_t initializeServiceConnections(struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    uint32_t i;
    struct ConnectionInformation *connInfo;

    for (i = 0; i < OPENED_CONNECTIONS; ++i) {
        connInfo = &p_InstallInfo->m_Connections[i];

        pthread_mutexattr_t attr;
        rc = pthread_mutexattr_init(&attr);
        DIE(rc != 0, "Could not init mutex attribute");

        rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        DIE(rc != 0, "Could not set pthread shared for mutex attribute");

        rc = pthread_mutex_init(&connInfo->m_ReturnQMutex, &attr);
        DIE(rc != 0, "Could not init connect response lock");

        rc = pthread_mutex_init(&connInfo->m_RequestResponseQMutex, &attr);
        DIE(rc != 0, "Could not init connect response lock");

        rc = pthread_mutexattr_destroy(&attr);
        DIE(rc != 0, "Could not destroy mutex attribute");

        pthread_condattr_t condAttr;

        rc = pthread_condattr_init(&condAttr);
        DIE(rc != 0, "Could not init condition attribute");

        rc = pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
        DIE(rc != 0, "Could not set pthread shared for condition attribute");

        rc = pthread_cond_init(&connInfo->m_ReturnQFullCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for full connect response queue");

        rc = pthread_cond_init(&connInfo->m_ReturnQEmptyCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for empty connect response queue");

        rc =
            pthread_cond_init(&connInfo->m_RequestResponseQFullCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for full connect response queue");

        rc = pthread_cond_init(&connInfo->m_RequestResponseQEmptyCond,
                               &condAttr);
        DIE(rc != 0,
            "Could not init condition for empty connect response queue");

        rc = pthread_condattr_destroy(&condAttr);
        DIE(rc != 0, "Could not destroy condition attribute object");
    }

    return rc;
}

static int32_t
s_SendConnectResponse(struct ServiceReturnInfo *p_ReturnInfo,
                      struct ConnectResponseInformation *p_ResponseInfo) {
    int32_t rc = 0;

    /**
     * Send the response to the client to announce that the communication is
     * established
     */
    QPUSH(
        &p_ReturnInfo->m_ResponseQueue, p_ReturnInfo->m_ResponseQueue.m_MaxSize,
        do {
            memcpy(&p_ReturnInfo->m_ResponseQueue
                        .m_Data[*p_ReturnInfo->m_ResponseQueue.m_Metadata
                                     .m_PushIdxPtr],
                   p_ResponseInfo, sizeof(struct ConnectResponseInformation));

            memcpy(&p_ReturnInfo->m_ConnectResponseInformation, p_ResponseInfo,
                   sizeof(struct ConnectResponseInformation));
        } while (0));

    return rc;
}

static int32_t
s_ReceiveConnectRequest(struct ServiceReturnInfo *p_ReturnInfo,
                        struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    struct ConnectQueue *queue = &p_ConnectInfo->m_ConnectQ;
    struct ConnectResponseInformation responseInfo;

    QPOP(
        queue, CONNECTQ_MAX_SIZE, do {
            configureServiceReturnInformation(
                p_ReturnInfo, p_ConnectInfo,
                &queue->m_Data[*queue->m_Metadata.m_PopIdxPtr]);

            memcpy(responseInfo.m_ReturnQName,
                   queue->m_Data[*queue->m_Metadata.m_PopIdxPtr].m_ReturnQName,
                   RETURNQ_NAME_MAX_SIZE);
            memcpy(responseInfo.m_ReturnRequestQName,
                   queue->m_Data[*queue->m_Metadata.m_PopIdxPtr]
                       .m_RequestResponseQName,
                   RETURNQ_NAME_MAX_SIZE);
            responseInfo.m_Id =
                queue->m_Data[*queue->m_Metadata.m_PopIdxPtr].m_ConnectionIdx;
        } while (0));

    s_SendConnectResponse(p_ReturnInfo, &responseInfo);

    return rc;
}

static int32_t
s_ReceiveDisconnectRequest(struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    uint32_t idx;
    uint32_t connId;
    struct DisconnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    QPOP(
        queue, CONNECTQ_MAX_SIZE, do {
            idx = *queue->m_Metadata.m_PopIdxPtr;
            connId = queue->m_Data[idx].m_ConnectionIdx;
        } while (0));

    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);

    rc = munmap(p_ConnectInfo->m_Connections[connId].m_RequestResponseQ,
                p_ConnectInfo->m_Connections[connId].m_RequestResponseQMapSize);
    DIE(rc < 0, "Could not unmap request response queue");
    p_ConnectInfo->m_Connections[connId].m_RequestResponseQ = NULL;

    rc = munmap(p_ConnectInfo->m_Connections[connId].m_ReturnQ,
                p_ConnectInfo->m_Connections[connId].m_ReturnQMapSize);
    DIE(rc < 0, "Could not unmap return queue");
    p_ConnectInfo->m_Connections[connId].m_ReturnQ = NULL;

    rc =
        shm_unlink(p_ConnectInfo->m_Connections[connId].m_RequestResponseQName);
    DIE(rc != 0,
        "Could not unlink request response queue shared memory object");

    rc = shm_unlink(p_ConnectInfo->m_Connections[connId].m_ReturnQName);
    DIE(rc != 0, "Could no unlink return queue shared memory object");

    p_ConnectInfo->m_Connections[connId].m_Connected = false;

    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);

    return rc;
}

int32_t
configureServiceConnectInformation(struct ServiceConnectInfo *p_ConnectInfo,
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
        p_InstallInfo->m_ConnectQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Could not map connect queue memory");

    rc = close(connectQFd);
    DIE(rc != 0, "Could not close connectQFd");

    disconnectQFd = createShmObject(
        p_InstallInfo->m_DisconnectQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    struct ConnectRequest *disconnectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
             PROT_READ | PROT_WRITE, MAP_SHARED, disconnectQFd, 0);
    DIE(disconnectQ == MAP_FAILED, "Could not map disconnect queue memory");

    rc = close(disconnectQFd);
    DIE(rc != 0, "Could not close disconnectQFd");

    p_ConnectInfo->m_ReceiveConnectRequest = s_ReceiveConnectRequest;
    p_ConnectInfo->m_ConnectQ.m_Data = connectQ;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PushIdxPtr =
        &p_InstallInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PopIdxPtr =
        &p_InstallInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_Size =
        &p_InstallInfo->m_ConnectQSize;
    p_ConnectInfo->m_Connections = p_InstallInfo->m_Connections;

    p_ConnectInfo->m_ReceiveDisconnectRequest = s_ReceiveDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PushIdxPtr =
        &p_InstallInfo->m_DisconnectQPushIdx;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PopIdxPtr =
        &p_InstallInfo->m_DisconnectQPopIdx;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_Size =
        &p_InstallInfo->m_DisconnectQSize;

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

    p_ConnectInfo->m_ConnectQ.m_Metadata.m_Lock =
        &p_InstallInfo->m_ConnectQMutex;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_FullCond =
        &p_InstallInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_ConnectQEmptyCond;

    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_Lock =
        &p_InstallInfo->m_DisconnectQMutex;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_FullCond =
        &p_InstallInfo->m_DisconnectQFullCond;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_DisconnectQEmptyCond;

    return rc;
}
