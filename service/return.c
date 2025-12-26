#include <string.h>

#include "commons.h"
#include "dsp.h"
#include "log.h"
#include "macros.h"
#include "system-values.h"
#include "return.h"

static int32_t s_SendReturnFnSMBHelper(struct DSPQueue *p_Queue,
                                       aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    uint32_t idx;
    struct SMBCall *qData;

    if (!p_Queue || !p_Queue->m_Data || !p_Queue->m_Metadata.m_PushIdxPtr ||
        !p_Queue->m_Metadata.m_PopIdxPtr || !p_ReturnData) {
        ELOGF("queue or return data is not pointer is invalid.\n");
        rc = -1;
        goto end;
    }

    idx = *p_Queue->m_Metadata.m_PushIdxPtr;

    qData = (struct SMBCall *)p_Queue->m_Data;

    memcpy(&qData[idx], p_ReturnData, sizeof(struct SMBCall));

end:
    return rc;
}

static int32_t s_SendReturnFnEMBHelper(struct DSPQueue *p_Queue,
                                       aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct EMBCall *qData = (struct EMBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct EMBCall));

    return rc;
}

static int32_t s_SendReturnFnQMBHelper(struct DSPQueue *p_Queue,
                                       aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct QMBCall *qData = (struct QMBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_SendReturnFnHMBHelper(struct DSPQueue *p_Queue,
                                       aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct HMBCall *qData = (struct HMBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_SendReturnFnMBHelper(struct DSPQueue *p_Queue,
                                      aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct MBCall *qData = (struct MBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct MBCall));

    return rc;
}

static int32_t s_SendReturnFnDMBHelper(struct DSPQueue *p_Queue,
                                       aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct DMBCall *qData = (struct DMBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct DMBCall));

    return rc;
}

static int32_t s_SendReturnFnHGBHelper(struct DSPQueue *p_Queue,
                                       aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct HGBCall *qData = (struct HGBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct HGBCall));

    return rc;
}

static int32_t s_SendReturnFnGBHelper(struct DSPQueue *p_Queue,
                                      aqua_void_t *p_ReturnData) {
    int32_t rc = 0;
    struct GBCall *qData = (struct GBCall *)p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct GBCall));

    return rc;
}

static int32_t s_SendReturnFnA(struct DSPQueue *queue, aqua_void_t *retData,
                               uint32_t qMaxSize,
                               int32_t (*fn)(struct DSPQueue *, aqua_void_t *));

static int32_t s_SendReturnFnSMB(struct DSPQueue *queue,
                                 struct SMBCall *returnData);

static int32_t s_SendReturnFnEMB(struct DSPQueue *queue,
                                 struct EMBCall *returnData);

static int32_t s_SendReturnFnQMB(struct DSPQueue *queue,
                                 struct QMBCall *returnData);

static int32_t s_SendReturnFnHMB(struct DSPQueue *queue,
                                 struct HMBCall *returnData);

static int32_t s_SendReturnFnMB(struct DSPQueue *queue,
                                struct MBCall *returnData);

static int32_t s_SendReturnFnDMB(struct DSPQueue *queue,
                                 struct DMBCall *returnData);

static int32_t s_SendReturnFnHGB(struct DSPQueue *queue,
                                 struct HGBCall *returnData);

static int32_t s_SendReturnFnGB(struct DSPQueue *queue,
                                struct GBCall *returnData);

static int32_t s_SendReturnFn(struct CommunicationInfo *p_CInfo) {
    switch (p_CInfo->m_Q->m_Type) {
    case SMBQ:
        return s_SendReturnFnSMB(p_CInfo->m_Q,
                                 (struct SMBCall *)p_CInfo->m_Data);
    case EMBQ:
        return s_SendReturnFnEMB(p_CInfo->m_Q,
                                 (struct EMBCall *)p_CInfo->m_Data);
    case QMBQ:
        return s_SendReturnFnQMB(p_CInfo->m_Q,
                                 (struct QMBCall *)p_CInfo->m_Data);
    case HMBQ:
        return s_SendReturnFnHMB(p_CInfo->m_Q,
                                 (struct HMBCall *)p_CInfo->m_Data);
    case MBQ:
        return s_SendReturnFnMB(p_CInfo->m_Q, (struct MBCall *)p_CInfo->m_Data);
    case DMBQ:
        return s_SendReturnFnDMB(p_CInfo->m_Q,
                                 (struct DMBCall *)p_CInfo->m_Data);
    case HGBQ:
        return s_SendReturnFnHGB(p_CInfo->m_Q,
                                 (struct HGBCall *)p_CInfo->m_Data);
    case GBQ:
        return s_SendReturnFnGB(p_CInfo->m_Q, (struct GBCall *)p_CInfo->m_Data);
    default:
        /**
         * TODO
         */
        return (-1);
    }
}

int32_t
configureServiceReturnInformation(struct ServiceReturnInfo *p_ReturnInfo,
                                  struct ServiceConnectInfo *p_ConnectInfo,
                                  struct ConnectRequest *p_Request) {
    int32_t rc = 0;
    aqua_file_handle returnQHandle;
    aqua_file_handle requestResponseQHandle;
    uint32_t connectionIdx;
    int qFlag;
    aqua_prot_t qProt;
    aqua_mode_t qMode;
    aqua_object_size_t qSize;
    aqua_void_t *returnQ;
    struct ConnectResponseInformation *requestResponseQ;
#if defined(_WIN32)
    char qSyncName[RETURNQ_NAME_MAX_SIZE];
#endif

    connectionIdx = p_Request->m_ConnectionIdx;

    requestResponseQHandle = createShmObject(
        p_Request->m_RequestResponseQName, O_RDWR,
        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
            AQUA_S_IROTH | AQUA_S_IWOTH,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        false);

    switch (p_Request->m_ReturnQType) {
    case SMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct SMBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case EMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct EMBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case QMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct QMBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case HMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct HMBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case MBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct MBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case DMBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct DMBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case HGBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct HGBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    case GBQ:
        qFlag = O_RDWR;
        qMode = AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
                AQUA_S_IROTH | AQUA_S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct GBCall);
        qProt = AQUA_PROT_WRITE | AQUA_PROT_READ;

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    returnQHandle =
        createShmObject(p_Request->m_ReturnQName, qFlag, qMode, qSize, false);

    createQ(&returnQ, qSize, qProt, returnQHandle);

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMapSize = qSize;

#if defined(__linux__)
    rc = close(returnQHandle);
    DIE(rc != 0, "Could not close returnQHandle");
#elif defined(_WIN32)
    DIE(!CloseHandle(returnQHandle), "Could not close returnQHandle");
#else
#endif

    createQSimple((aqua_void_t **)&requestResponseQ,
                  p_Request->m_ResponseQSize *
                      sizeof(struct ConnectResponseInformation),
                  AQUA_PROT_WRITE | AQUA_PROT_READ, requestResponseQHandle);

#if defined(__linux__)
    rc = close(requestResponseQHandle);
    DIE(rc != 0,
        "Could not close request response queue shared object file descriptor");
#elif defined(_WIN32)
    DIE(!CloseHandle(requestResponseQHandle),
        "Could not close request response queue shared object file descriptor");
#else
#endif

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPushIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPopIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSize = 0;

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQ = returnQ;

    p_ReturnInfo->m_Q.m_Data = returnQ;

    // Obtain the handles for return queue
    snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
             p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQFullCond);
    p_ReturnInfo->m_Q.m_Metadata.m_FullCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_Q.m_Metadata.m_FullCond =
    //     &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQFullCond;

    snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
             p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQEmptyCond);
    p_ReturnInfo->m_Q.m_Metadata.m_EmptyCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_Q.m_Metadata.m_EmptyCond =
    //     &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQEmptyCond;

    snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
             p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMutex);
    p_ReturnInfo->m_Q.m_Metadata.m_Lock =
        OpenMutex(MUTEX_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_Q.m_Metadata.m_Lock =
    //     &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMutex;

    p_ReturnInfo->m_Q.m_Metadata.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPushIdx;
    p_ReturnInfo->m_Q.m_Metadata.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPopIdx;
    p_ReturnInfo->m_Q.m_Metadata.m_Size =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSize;

    p_ReturnInfo->m_Q.m_Type = p_Request->m_ReturnQType;

    p_ReturnInfo->m_ResponseQueue.m_MaxSize = p_Request->m_ReturnQSize;

    p_ReturnInfo->m_SendReturnFn = s_SendReturnFn;

    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQMapSize =
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation);
    p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQ =
        requestResponseQ;

    p_ReturnInfo->m_ResponseQueue.m_Data = requestResponseQ;

    // Obtain the handles for request-return queue
    snprintf(
        qSyncName, sizeof(qSyncName), "%s-%llu", "request-response-q",
        p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQFullCond);
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_FullCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_FullCond =
    //     &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQFullCond;

    snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "request-response-q",
             p_ConnectInfo->m_Connections[connectionIdx]
                 .m_RequestResponseQEmptyCond);
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_EmptyCond =
        OpenEvent(EVENT_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_EmptyCond =
    //     &p_ConnectInfo->m_Connections[connectionIdx]
    //          .m_RequestResponseQEmptyCond;

    snprintf(
        qSyncName, sizeof(qSyncName), "%s-%llu", "request-response-q",
        p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQMutex);
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Lock =
        OpenMutex(MUTEX_ALL_ACCESS, FALSE, qSyncName);
    // p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Lock =
    //     &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQMutex;

    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPushIdx;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPopIdx;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Size =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQSize;
    p_ReturnInfo->m_ResponseQueue.m_MaxSize = p_Request->m_ResponseQSize;

    return rc;
}

static int32_t s_SendReturnFnSMB(struct DSPQueue *p_Queue,
                                 struct SMBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnSMBHelper);
}

static int32_t s_SendReturnFnEMB(struct DSPQueue *p_Queue,
                                 struct EMBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnEMBHelper);
}

static int32_t s_SendReturnFnQMB(struct DSPQueue *p_Queue,
                                 struct QMBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnQMBHelper);
}

static int32_t s_SendReturnFnHMB(struct DSPQueue *p_Queue,
                                 struct HMBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnHMBHelper);
}

static int32_t s_SendReturnFnMB(struct DSPQueue *p_Queue,
                                struct MBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnMBHelper);
}

static int32_t s_SendReturnFnDMB(struct DSPQueue *p_Queue,
                                 struct DMBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnDMBHelper);
}

static int32_t s_SendReturnFnHGB(struct DSPQueue *p_Queue,
                                 struct HGBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnHGBHelper);
}

static int32_t s_SendReturnFnGB(struct DSPQueue *p_Queue,
                                struct GBCall *p_ReturnData) {
    return s_SendReturnFnA(p_Queue, p_ReturnData, RETURNQ_MAX_SIZE,
                           s_SendReturnFnGBHelper);
}

static int32_t s_SendReturnFnA(struct DSPQueue *p_Queue, aqua_void_t *p_RetData,
                               uint32_t p_QMaxSize,
                               int32_t (*p_Fn)(struct DSPQueue *,
                                               aqua_void_t *)) {
    int32_t rc = 0;

    QPUSH(p_Queue, p_QMaxSize, do { rc = p_Fn(p_Queue, p_RetData); } while (0));

    return rc;
}
