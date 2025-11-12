#include <string.h>
#include <sys/mman.h>

#include "client-connect.h"
#include "commons.h"
#include "macros.h"

static int32_t s_ReturnFnSMBHelper(void *p_ReturnData,
                                   struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct SMBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct SMBCall));

    return rc;
}

static int32_t s_ReturnFnEMBHelper(void *p_ReturnData,
                                   struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct EMBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct EMBCall));

    return rc;
}

static int32_t s_ReturnFnQMBHelper(void *p_ReturnData,
                                   struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct QMBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_ReturnFnHMBHelper(void *p_ReturnData,
                                   struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct HMBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_ReturnFnMBHelper(void *p_ReturnData,
                                  struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct MBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct MBCall));

    return rc;
}

static int32_t s_ReturnFnDMBHelper(void *p_ReturnData,
                                   struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct DMBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct DMBCall));

    return rc;
}

static int32_t s_ReturnFnHGBHelper(void *p_ReturnData,
                                   struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct HGBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct HGBCall));

    return rc;
}

static int32_t s_ReturnFnGBHelper(void *p_ReturnData,
                                  struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct GBCall *qData = p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct GBCall));

    return rc;
}

static int32_t s_ReturnFnA(struct DSPQueue *p_Queue, void *p_ReturnData,
                           uint32_t p_QMaxSize,
                           int32_t (*p_Fn)(void *, struct DSPQueue *)) {
    int32_t rc = 0;

    QPOP(
        p_Queue, p_QMaxSize,
        do { rc = p_Fn(p_ReturnData, p_Queue); } while (0));

    return rc;
}

static int32_t s_ReturnFnSMB(struct SMBCall *p_ReturnData,
                             struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnSMBHelper);
}

static int32_t s_ReturnFnEMB(struct EMBCall *p_ReturnData,
                             struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnEMBHelper);
}

static int32_t s_ReturnFnQMB(struct QMBCall *p_ReturnData,
                             struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnQMBHelper);
}

static int32_t s_ReturnFnHMB(struct HMBCall *p_ReturnData,
                             struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnHMBHelper);
}

static int32_t s_ReturnFnMB(struct MBCall *p_ReturnData,
                            struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnMBHelper);
}

static int32_t s_ReturnFnDMB(struct DMBCall *p_ReturnData,
                             struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnDMBHelper);
}

static int32_t s_ReturnFnHGB(struct HGBCall *p_ReturnData,
                             struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnHGBHelper);
}

static int32_t s_ReturnFnGB(struct GBCall *p_ReturnData,
                            struct DSPQueue *p_Queue) {
    return s_ReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                       s_ReturnFnGBHelper);
}

static int32_t s_QPop(struct CommunicationInfo *p_CInfo) {
    switch (p_CInfo->m_Q->m_Type) {
    case SMBQ:
        return s_ReturnFnSMB((struct SMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case EMBQ:
        return s_ReturnFnEMB((struct EMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case QMBQ:
        return s_ReturnFnQMB((struct QMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case HMBQ:
        return s_ReturnFnHMB((struct HMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case MBQ:
        return s_ReturnFnMB((struct MBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case DMBQ:
        return s_ReturnFnDMB((struct DMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case HGBQ:
        return s_ReturnFnHGB((struct HGBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case GBQ:
        return s_ReturnFnGB((struct GBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    default:
        /**
         * TODO
         */
        return (-1);
    }
}

static int32_t s_ProcessConnectionRequest(
    uint32_t p_ConnId, struct ClientReturnInfo *p_ReturnInfo,
    struct ConnectRequest *p_ConnectRequest,
    struct ClientConnectInfo *p_ConnectInfo,
    struct ClientConnectRequestInformation *p_ConnectInformation) {
    int requestResponseQFd;
    int returnQFd;
    int32_t rc = 0;
    int qFlag;
    int qProt;
    mode_t qMode;
    size_t qSize;
    void *returnQ;

    /**
     * With the connection index found we need to construct the request for the
     * service
     */
    p_ConnectRequest->m_ConnectionIdx = p_ConnId;

    memcpy(p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQName,
           p_ConnectInformation->m_ReturnQName,
           strlen(p_ConnectInformation->m_ReturnQName));
    memcpy(p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQName,
           p_ConnectInformation->m_RequestResponseQName,
           strlen(p_ConnectInformation->m_RequestResponseQName));

    memset(p_ConnectRequest->m_ReturnQName, 0, RETURNQ_NAME_MAX_SIZE);
    memcpy(p_ConnectRequest->m_ReturnQName, p_ConnectInformation->m_ReturnQName,
           strlen(p_ConnectInformation->m_ReturnQName));

    memset(p_ConnectRequest->m_RequestResponseQName, 0, RETURNQ_NAME_MAX_SIZE);
    memcpy(p_ConnectRequest->m_RequestResponseQName,
           p_ConnectInformation->m_RequestResponseQName,
           strlen(p_ConnectInformation->m_RequestResponseQName));

    p_ConnectRequest->m_ReturnQType = p_ConnectInformation->m_QType;

    p_ReturnInfo->m_Q.m_MaxSize = RETURNQ_MAX_SIZE;
    p_ConnectRequest->m_ReturnQSize =
        RETURNQ_MAX_SIZE; // CHECK: possibly user specified

    p_ReturnInfo->m_ResponseQueue.m_MaxSize = RETURN_RESPONSEQ_MAX_SIZE;
    p_ConnectRequest->m_ResponseQSize =
        RETURN_RESPONSEQ_MAX_SIZE; // CHECK: possibly user specified

    requestResponseQFd = createShmObject(
        p_ConnectInformation->m_RequestResponseQName, O_RDWR,
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

    switch (p_ConnectInformation->m_QType) {
    case SMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct SMBCall);
        qProt = PROT_READ;

        break;
    case EMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct EMBCall);
        qProt = PROT_READ;

        break;
    case QMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct QMBCall);
        qProt = PROT_READ;

        break;
    case HMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct HMBCall);
        qProt = PROT_READ;

        break;
    case MBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct MBCall);
        qProt = PROT_READ;

        break;
    case DMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct DMBCall);
        qProt = PROT_READ;

        break;
    case HGBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct HGBCall);
        qProt = PROT_READ;

        break;
    case GBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct GBCall);
        qProt = PROT_READ;

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    returnQFd = createShmObject(p_ConnectInformation->m_ReturnQName, qFlag,
                                qMode, qSize, true);

    createQ(&returnQ, qSize, qProt, returnQFd);

    triggerKernelPageInit(returnQ, qSize, qProt);

    rc = close(returnQFd);
    DIE(rc != 0, "Could not close returnQFd");

    p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPushIdx = 0;
    p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPopIdx = 0;
    p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQSize = 0;

    p_ReturnInfo->m_ResponseQueue.m_Data = requestResponseQ;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_FullCond =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQFullCond;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_EmptyCond =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQEmptyCond;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Lock =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQMutex;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPushIdx;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPopIdx;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Size =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQSize;

    p_ReturnInfo->m_Q.m_Data = returnQ;
    p_ReturnInfo->m_Q.m_Metadata.m_FullCond =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQFullCond;
    p_ReturnInfo->m_Q.m_Metadata.m_EmptyCond =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQEmptyCond;
    p_ReturnInfo->m_Q.m_Metadata.m_Lock =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQMutex;
    p_ReturnInfo->m_Q.m_Metadata.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQPushIdx;
    p_ReturnInfo->m_Q.m_Metadata.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQPopIdx;
    p_ReturnInfo->m_Q.m_Metadata.m_Size =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQSize;
    p_ReturnInfo->m_Q.m_Type = p_ConnectInformation->m_QType;

    p_ReturnInfo->m_ReturnFn = s_QPop;

    return rc;
}

static int32_t
s_SendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                     struct ClientConnectInfo *p_ConnectInfo,
                     struct ClientConnectRequestInformation *p_RequestInfo) {
    int32_t rc = 0;
    uint32_t idx;
    uint32_t connId;
    struct ConnectQueue *queue = &p_ConnectInfo->m_ConnectQ;

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

    QPUSH(
        queue, CONNECTQ_MAX_SIZE, do {
            idx = *queue->m_Metadata.m_PushIdxPtr;
            s_ProcessConnectionRequest(connId, p_ReturnInfo,
                                       &queue->m_Data[idx], p_ConnectInfo,
                                       p_RequestInfo);
        } while (0));

    /**
     * Wait for the response from the service to announce that the communication
     * is established
     */
    QPOP(
        &p_ReturnInfo->m_ResponseQueue, p_ReturnInfo->m_ResponseQueue.m_MaxSize,
        do {
            /**
             * WIP: Add the information to the response queue. Now the signal is
             * enough
             */
            idx = *p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxPtr;

            memcpy(&p_ReturnInfo->m_ConnectResponseInformation,
                   &p_ReturnInfo->m_ResponseQueue.m_Data[idx],
                   sizeof(struct ConnectResponseInformation));
        } while (0));

    return rc;
}

static int32_t
s_SendDisconnectRequest(struct ClientConnectInfo *p_ConnectInfo,
                        struct ConnectResponseInformation *p_ResponseInfo) {
    int32_t rc = 0;
    uint32_t idx;
    uint32_t connId;

    struct DisconnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    QPUSH(
        queue, CONNECTQ_MAX_SIZE, do {
            idx = *queue->m_Metadata.m_PushIdxPtr;

            connId = p_ResponseInfo->m_Id;

            queue->m_Data[idx].m_ConnectionIdx = connId;
        } while (0));

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
    p_ConnectInfo->m_ConnectQ.m_Data = connectQ;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PushIdxPtr =
        &p_InstallInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PopIdxPtr =
        &p_InstallInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_Size =
        &p_InstallInfo->m_ConnectQSize;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_Lock =
        &p_InstallInfo->m_ConnectQMutex;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_FullCond =
        &p_InstallInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_ConnectQEmptyCond;
    p_ConnectInfo->m_ConnectLock = &p_InstallInfo->m_ConnectListLock;

    p_ConnectInfo->m_SendDisconnectRequest = s_SendDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PushIdxPtr =
        &p_InstallInfo->m_DisconnectQPushIdx;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PopIdxPtr =
        &p_InstallInfo->m_DisconnectQPopIdx;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_Size =
        &p_InstallInfo->m_DisconnectQSize;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_Lock =
        &p_InstallInfo->m_DisconnectQMutex;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_FullCond =
        &p_InstallInfo->m_DisconnectQFullCond;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_DisconnectQEmptyCond;

    return rc;
}
