#include <stdint.h>
#include <string.h>

#include "client-connect.h"
#include "commons.h"
#include "dsp.h"
#include "macros.h"
#include "system-values.h"

static int32_t s_ReturnFnSMBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                   uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct SMBCall *qData = (struct SMBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct SMBCall));

    return rc;
}

static int32_t s_ReturnFnEMBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                   uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct EMBCall *qData = (struct EMBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct EMBCall));

    return rc;
}

static int32_t s_ReturnFnQMBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                   uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct QMBCall *qData = (struct QMBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct QMBCall));

    return rc;
}

static int32_t s_ReturnFnHMBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                   uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct HMBCall *qData = (struct HMBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct HMBCall));

    return rc;
}

static int32_t s_ReturnFnMBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                  uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct MBCall *qData = (struct MBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct MBCall));

    return rc;
}

static int32_t s_ReturnFnDMBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                   uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct DMBCall *qData = (struct DMBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct DMBCall));

    return rc;
}

static int32_t s_ReturnFnHGBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                   uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct HGBCall *qData = (struct HGBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct HGBCall));

    return rc;
}

static int32_t s_ReturnFnGBHelper(void *p_ReturnData, struct DSPQueue *p_Queue,
                                  uint32_t p_CurrIdx) {
    int32_t rc = 0;
    struct GBCall *qData = (struct GBCall *)p_Queue->m_Data;

    memcpy(p_ReturnData, &qData[p_CurrIdx], sizeof(struct GBCall));

    return rc;
}

static int32_t s_ReturnFnSMB(struct SMBCall *returnData,
                             struct DSPQueue *queue);

static int32_t s_ReturnFnEMB(struct EMBCall *returnData,
                             struct DSPQueue *queue);

static int32_t s_ReturnFnQMB(struct QMBCall *returnData,
                             struct DSPQueue *queue);

static int32_t s_ReturnFnHMB(struct HMBCall *returnData,
                             struct DSPQueue *queue);

static int32_t s_ReturnFnMB(struct MBCall *returnData, struct DSPQueue *queue);

static int32_t s_ReturnFnDMB(struct DMBCall *returnData,
                             struct DSPQueue *queue);

static int32_t s_ReturnFnHGB(struct HGBCall *returnData,
                             struct DSPQueue *queue);

static int32_t s_ReturnFnGB(struct GBCall *returnData, struct DSPQueue *queue);

static int32_t s_ReturnFnA(struct DSPQueue *queue, void *returnData,
                           uint32_t qMaxSize,
                           int32_t (*fn)(void *, struct DSPQueue *, uint32_t));

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
    aqua_file_handle requestResponseQHandle;
    aqua_file_handle returnQHandle;
    int32_t rc = 0;
    int qFlag;
    aqua_prot_t qProt;
    aqua_mode_t qMode;
    aqua_object_size_t qSize;
    void *returnQ;
    struct ConnectResponseInformation *requestResponseQ;
#if defined(_WIN32)
    char qSyncName[RETURNQ_NAME_MAX_SIZE];
#endif

    /**
     * With the connection index found we need to construct the request for the
     * service
     */
    p_ConnectRequest->m_ConnectionIdx = p_ConnId;

    memcpy(p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQName,
           p_ConnectInformation->m_RequestResponseQName,
           strlen(p_ConnectInformation->m_ReturnQName));
    memcpy(p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQName,
           p_ConnectInformation->m_ReturnQName,
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

    requestResponseQHandle =
        createShmObject(p_ConnectInformation->m_RequestResponseQName, O_RDWR,
                        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP |
                            AQUA_S_IWGRP | AQUA_S_IROTH | AQUA_S_IWOTH,
                        p_ConnectInformation->m_ResponseQSize *
                            sizeof(struct ConnectResponseInformation),
                        true);

    createQSimple((aqua_void_t **)&requestResponseQ,
                  p_ConnectInformation->m_ResponseQSize *
                      sizeof(struct ConnectResponseInformation),
                  AQUA_PROT_READ, requestResponseQHandle);

    triggerKernelPageInit(requestResponseQ,
                          p_ConnectInformation->m_ResponseQSize *
                              sizeof(struct ConnectResponseInformation),
                          AQUA_PROT_READ);

#if defined(__linux__)
    rc = close(requestResponseQHandle);
    DIE(rc != 0, "Could not close requestResponseQHandle");
#elif defined(_WIN32)
    // DIE(!CloseHandle(requestResponseQHandle),
    //     "Could not close requestResponseQHandle");
#else
#endif

    switch (p_ConnectInformation->m_QType) {
    case SMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct SMBCall);
        qProt = AQUA_PROT_READ;

        break;
    case EMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct EMBCall);
        qProt = AQUA_PROT_READ;

        break;
    case QMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct QMBCall);
        qProt = AQUA_PROT_READ;

        break;
    case HMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct HMBCall);
        qProt = AQUA_PROT_READ;

        break;
    case MBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct MBCall);
        qProt = AQUA_PROT_READ;

        break;
    case DMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct DMBCall);
        qProt = AQUA_PROT_READ;

        break;
    case HGBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct HGBCall);
        qProt = AQUA_PROT_READ;

        break;
    case GBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_ConnectInformation->m_ReturnQSize * sizeof(struct GBCall);
        qProt = AQUA_PROT_READ;

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    returnQHandle = createShmObject(p_ConnectInformation->m_ReturnQName, qFlag,
                                    qMode, qSize, true);

    createQ(&returnQ, qSize, qProt, returnQHandle);

    triggerKernelPageInit(returnQ, qSize, qProt);

#if defined(__linux__)
    rc = close(returnQHandle);
    DIE(rc != 0, "Could not close returnQHandle");
#elif defined(_WIN32)
    // DIE(!CloseHandle(returnQHandle), "Could not close returnQHandle");
#else
#endif

    // p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPushIdx = 0;
    // p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPopIdx = 0;
    // p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQSize = 0;
    InterlockedExchange(
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPushIdxAtomic,
        0);
    InterlockedExchange(
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPopIdxAtomic,
        0);
    InterlockedExchange(
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQSizeAtomic,
        0);

    p_ReturnInfo->m_ResponseQueue.m_Data = requestResponseQ;

    // Obtain the handles for return queue
    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__",
             p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
                 .m_ReturnQProduceCond,
             p_ConnId % SYNC_ELEMENTS);
    p_ReturnInfo->m_Q.m_Metadata.m_ProduceCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_FullCond =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQFullCond;

    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__",
             p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
                 .m_ReturnQConsumeCond,
             p_ConnId % SYNC_ELEMENTS);
    p_ReturnInfo->m_Q.m_Metadata.m_ConsumeCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_EmptyCond =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQEmptyCond;

    // snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
    //          p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQMutex);
    // p_ReturnInfo->m_Q.m_Metadata.m_Lock =
    //     OpenMutex(MUTEX_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Lock =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQMutex;

    // p_ReturnInfo->m_Q.m_Metadata.m_PushIdxPtr =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQPushIdx;
    // p_ReturnInfo->m_Q.m_Metadata.m_PopIdxPtr =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQPopIdx;
    // p_ReturnInfo->m_Q.m_Metadata.m_Size =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQSize;

    p_ReturnInfo->m_Q.m_Metadata.m_PushIdxAtomic =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQPushIdxAtomic;
    p_ReturnInfo->m_Q.m_Metadata.m_PopIdxAtomic =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQPopIdxAtomic;
    p_ReturnInfo->m_Q.m_Metadata.m_SizeAtomic =
        &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQSizeAtomic;
    p_ReturnInfo->m_Q.m_Metadata.m_WaitConsume =
        &p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
             .m_ReturnQWaitConsume;
    p_ReturnInfo->m_Q.m_Metadata.m_WaitProduce =
        &p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
             .m_ReturnQWaitProduce;

    p_ReturnInfo->m_Q.m_Type = p_ConnectInformation->m_QType;

    p_ReturnInfo->m_Q.m_Data = returnQ;

    p_ReturnInfo->m_ReturnFn = s_QPop;

    // Obtain the handles for request-return queue
    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__",
             p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
                 .m_RequestResponseQProduceCond,
             p_ConnId % SYNC_ELEMENTS);
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_ProduceCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_Q.m_Metadata.m_FullCond =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQFullCond;

    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__",
             p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
                 .m_RequestResponseQConsumeCond,
             p_ConnId % SYNC_ELEMENTS);
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_ConsumeCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_Q.m_Metadata.m_EmptyCond =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQEmptyCond;

    // snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "request-response-q",
    //          p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQMutex);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Lock =
    //     OpenMutex(MUTEX_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_Q.m_Metadata.m_Lock =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_ReturnQMutex;

    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PushIdxPtr =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPushIdx;
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxPtr =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPopIdx;
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Size =
    //     &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQSize;

    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PushIdxAtomic =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPushIdxAtomic;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxAtomic =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQPopIdxAtomic;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_SizeAtomic =
        &p_ConnectInfo->m_Connections[p_ConnId].m_RequestResponseQSizeAtomic;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_WaitConsume =
        &p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
             .m_RequestResponseQWaitConsume;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_WaitProduce =
        &p_ConnectInfo->m_ConnectionsSyncData[p_ConnId % SYNC_ELEMENTS]
             .m_RequestResponseQWaitProduce;

    return rc;
}

static int32_t
s_SendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                     struct ClientConnectInfo *p_ConnectInfo,
                     struct ClientConnectRequestInformation *p_RequestInfo) {
    int32_t rc = 0;
    uint32_t connId;
    struct ConnectQueue *queue = &p_ConnectInfo->m_ConnectQ;

    /**
     *  search for a free spot in the opened connections for the service
     *  send the request to the service with the connection index
     *  WIP: wait for the service to establish the connection on its side
     */
#if defined(__linux__)
    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);
    for (connId = 0; connId < OPENED_CONNECTIONS; ++connId) {
        if (!p_ConnectInfo->m_Connections[connId].m_Connected) {
            p_ConnectInfo->m_Connections[connId].m_Connected = true;
            break;
        }
    }
    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);
#elif defined(_WIN32)
    WaitForSingleObject(p_ConnectInfo->m_ConnectLock, INFINITE);
    for (connId = 0; connId < OPENED_CONNECTIONS; ++connId) {
        if (!p_ConnectInfo->m_Connections[connId].m_Connected) {
            p_ConnectInfo->m_Connections[connId].m_Connected = true;
            break;
        }
    }
    ReleaseMutex(p_ConnectInfo->m_ConnectLock);
#else
#endif

    USQPUSH(
        queue, CONNECTQ_MAX_SIZE, do {
            s_ProcessConnectionRequest(connId, p_ReturnInfo,
                                       &queue->m_Data[currIdx], p_ConnectInfo,
                                       p_RequestInfo);
        } while (0));

    /**
     * Wait for the response from the service to announce that the communication
     * is established
     */
    USQPOP(
        &p_ReturnInfo->m_ResponseQueue, p_ReturnInfo->m_ResponseQueue.m_MaxSize,
        do {
            /**
             * WIP: Add the information to the response queue. Now the signal is
             * enough
             */
            memcpy(&p_ReturnInfo->m_ConnectResponseInformation,
                   &p_ReturnInfo->m_ResponseQueue.m_Data[currIdx],
                   sizeof(struct ConnectResponseInformation));
        } while (0));
    // QPOP(
    //     &p_ReturnInfo->m_ResponseQueue,
    //     p_ReturnInfo->m_ResponseQueue.m_MaxSize, do {
    //         /**
    //          * WIP: Add the information to the response queue. Now the signal
    //          is
    //          * enough
    //          */
    //         idx = *p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxPtr;

    //         memcpy(&p_ReturnInfo->m_ConnectResponseInformation,
    //                &p_ReturnInfo->m_ResponseQueue.m_Data[idx],
    //                sizeof(struct ConnectResponseInformation));
    //     } while (0));

    return rc;
}

static int32_t
s_SendDisconnectRequest(struct ClientConnectInfo *p_ConnectInfo,
                        struct ConnectResponseInformation *p_ResponseInfo) {
    int32_t rc = 0;
    struct DisconnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    USQPUSH(
        queue, CONNECTQ_MAX_SIZE, do {
            queue->m_Data[currIdx].m_ConnectionIdx = p_ResponseInfo->m_Id;
        } while (0));

    return rc;
}

int32_t
configureClientConnectInformation(struct ClientConnectInfo *p_ConnectInfo,
                                  struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    aqua_file_handle connectQHandle, disconnectQHandle;
    struct ConnectRequest *connectQ;
    struct ConnectRequest *disconnectQ;
#if defined(_WIN32)
    char qSyncName[RETURNQ_NAME_MAX_SIZE << 1];
#endif

    /**
     * TODO: Implement successfull connection functionality
     */
    connectQHandle = createShmObject(
        p_InstallInfo->m_ConnectQName, O_RDWR,
        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
            AQUA_S_IROTH | AQUA_S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), false);

    createQSimple((aqua_void_t **)&connectQ,
                  CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
                  AQUA_PROT_READ | AQUA_PROT_WRITE, connectQHandle);

#if defined(__linux__)
    rc = close(connectQHandle);
    DIE(rc != 0, "Could not close connectQHandle");
#elif defined(_WIN32)
    DIE(!CloseHandle(connectQHandle), "Could not close connectQHandle");
#else
#endif

    disconnectQHandle = createShmObject(
        p_InstallInfo->m_DisconnectQName, O_RDWR,
        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
            AQUA_S_IROTH | AQUA_S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), false);

    createQSimple((aqua_void_t **)&disconnectQ,
                  CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
                  AQUA_PROT_READ | AQUA_PROT_WRITE, disconnectQHandle);

#if defined(__linux__)
    rc = close(disconnectQHandle);
    DIE(rc != 0, "Could not close disconnectQHandle");
#elif defined(_WIN32)
    DIE(!CloseHandle(disconnectQHandle), "Could not close disconnectQHandle");
#else
#endif

    p_ConnectInfo->m_Connections = p_InstallInfo->m_Connections;
    p_ConnectInfo->m_ConnectionsSyncData = p_InstallInfo->m_ConnectionsSyncData;

    snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", p_InstallInfo->m_StrId,
             p_InstallInfo->m_ConnectListLock);
    p_ConnectInfo->m_ConnectLock =
        OpenMutex(MUTEX_ALL_ACCESS, FALSE, qSyncName);

    p_ConnectInfo->m_SendConnectRequest = s_SendConnectRequest;
    p_ConnectInfo->m_ConnectQ.m_Data = connectQ;

    p_ConnectInfo->m_ConnectQ.m_Metadata.m_WaitConsume =
        &p_InstallInfo->m_ConnectQWaitConsume;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_WaitProduce =
        &p_InstallInfo->m_ConnectQWaitProduce;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PushIdxAtomic =
        &p_InstallInfo->m_ConnectQPushIdxAtomic;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PopIdxAtomic =
        &p_InstallInfo->m_ConnectQPopIdxAtomic;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_SizeAtomic =
        &p_InstallInfo->m_ConnectQSizeAtomic;

    // Obtain the handles for connect queue
    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%s_connect_produce_cond__",
             p_InstallInfo->m_StrId);
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_ProduceCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%s_connect_consume_cond__",
             p_InstallInfo->m_StrId);
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_ConsumeCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);

    p_ConnectInfo->m_SendDisconnectRequest = s_SendDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;

    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_WaitConsume =
        &p_InstallInfo->m_DisconnectQWaitConsume;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_WaitProduce =
        &p_InstallInfo->m_DisconnectQWaitProduce;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PushIdxAtomic =
        &p_InstallInfo->m_DisconnectQPushIdxAtomic;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PopIdxAtomic =
        &p_InstallInfo->m_DisconnectQPopIdxAtomic;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_SizeAtomic =
        &p_InstallInfo->m_DisconnectQSizeAtomic;

    // Obtain the handles for disconnect queue
    snprintf(qSyncName, sizeof(qSyncName),
             "__aqua_%s_disconnect_produce_cond__", p_InstallInfo->m_StrId);
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_ProduceCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    snprintf(qSyncName, sizeof(qSyncName),
             "__aqua_%s_disconnect_consume_cond__", p_InstallInfo->m_StrId);
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_ConsumeCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);

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

static int32_t
s_ReturnFnA(struct DSPQueue *p_Queue, void *p_ReturnData, uint32_t p_QMaxSize,
            int32_t (*p_Fn)(void *, struct DSPQueue *, uint32_t)) {
    int32_t rc = 0;

    USQPOP(
        p_Queue, p_QMaxSize,
        do { rc = p_Fn(p_ReturnData, p_Queue, currIdx); } while (0));

    // QPOP(
    //     p_Queue, p_QMaxSize,
    //     do { rc = p_Fn(p_ReturnData, p_Queue); } while (0));

    return rc;
}
