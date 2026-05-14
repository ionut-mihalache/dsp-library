// SPDX-License-Identifier: LGPL-2.1-or-later

#include <string.h>
#include <sys/mman.h>

#include "commons.h"
#include "log.h"
#include "macros.h"
#include "return.h"

static int32_t s_SendReturnFnSMBHelper(struct DSPQueue *p_Queue,
                                       void *p_ReturnData) {
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

    qData = p_Queue->m_Data;

    memcpy(&qData[idx], p_ReturnData, sizeof(struct SMBCall));

end:
    return rc;
}

static int32_t s_SendReturnFnEMBHelper(struct DSPQueue *p_Queue,
                                       void *p_ReturnData) {
    int32_t rc = 0;
    struct EMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct EMBCall));

    return rc;
}

static int32_t s_SendReturnFnQMBHelper(struct DSPQueue *p_Queue,
                                       void *p_ReturnData) {
    int32_t rc = 0;
    struct QMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_SendReturnFnHMBHelper(struct DSPQueue *p_Queue,
                                       void *p_ReturnData) {
    int32_t rc = 0;
    struct HMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_SendReturnFnMBHelper(struct DSPQueue *p_Queue,
                                      void *p_ReturnData) {
    int32_t rc = 0;
    struct MBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct MBCall));

    return rc;
}

static int32_t s_SendReturnFnDMBHelper(struct DSPQueue *p_Queue,
                                       void *p_ReturnData) {
    int32_t rc = 0;
    struct DMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct DMBCall));

    return rc;
}

static int32_t s_SendReturnFnHGBHelper(struct DSPQueue *p_Queue,
                                       void *p_ReturnData) {
    int32_t rc = 0;
    struct HGBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct HGBCall));

    return rc;
}

static int32_t s_SendReturnFnGBHelper(struct DSPQueue *p_Queue,
                                      void *p_ReturnData) {
    int32_t rc = 0;
    struct GBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_ReturnData,
           sizeof(struct GBCall));

    return rc;
}

static int32_t s_SendReturnFnA(struct DSPQueue *p_Queue, void *p_RetData,
                               uint32_t p_QMaxSize,
                               int32_t (*p_Fn)(struct DSPQueue *, void *)) {
    int32_t rc = 0;

    QPUSH(p_Queue, p_QMaxSize, do { rc = p_Fn(p_Queue, p_RetData); } while (0));

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
    int returnQFd;
    int requestResponseQFd;
    uint32_t connectionIdx;
    int qFlag;
    int qProt;
    mode_t qMode;
    size_t qSize;
    void *returnQ;

    connectionIdx = p_Request->m_ConnectionIdx;

    requestResponseQFd = createShmObject(
        p_Request->m_RequestResponseQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        false);

    switch (p_Request->m_ReturnQType) {
    case SMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct SMBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case EMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct EMBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case QMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct QMBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case HMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct HMBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case MBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct MBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case DMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct DMBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case HGBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct HGBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    case GBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = p_Request->m_ReturnQSize * sizeof(struct GBCall);
        qProt = PROT_WRITE | PROT_READ;

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    returnQFd =
        createShmObject(p_Request->m_ReturnQName, qFlag, qMode, qSize, false);

    createQ(&returnQ, qSize, qProt, returnQFd);

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMapSize = qSize;

    rc = close(returnQFd);
    DIE(rc != 0, "Could not close returnQFd");

    struct ConnectResponseInformation *requestResponseQ = mmap(
        NULL,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        PROT_WRITE | PROT_READ, MAP_SHARED, requestResponseQFd, 0);
    DIE(requestResponseQ == MAP_FAILED,
        "Could not map request response queue memory");

    rc = close(requestResponseQFd);
    DIE(rc != 0,
        "Could not close request response queue shared object file descriptor");

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPushIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPopIdx = 0;
    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSize = 0;

    p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQ = returnQ;

    p_ReturnInfo->m_Q.m_Data = returnQ;

    p_ReturnInfo->m_Q.m_Metadata.m_FullCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQFullCond;
    p_ReturnInfo->m_Q.m_Metadata.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQEmptyCond;
    p_ReturnInfo->m_Q.m_Metadata.m_Lock =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMutex;

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

    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_FullCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQFullCond;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connectionIdx]
             .m_RequestResponseQEmptyCond;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Lock =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQMutex;

    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPushIdx;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQPopIdx;
    p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Size =
        &p_ConnectInfo->m_Connections[connectionIdx].m_RequestResponseQSize;
    p_ReturnInfo->m_ResponseQueue.m_MaxSize = p_Request->m_ResponseQSize;

    return rc;
}
