#include <semaphore.h>
#include <string.h>

#include "commons.h"
#include "log.h"
#include "macros.h"
#include "return.h"

sem_t sem;

static int32_t s_SendReturnFnQMB(struct QMBDSPQueue *p_Queue,
                                 struct QMBCall *p_ReturnData) {

    int32_t rc = 0;

    QPUSH(
        p_Queue, RETURNQ_MAX_SIZE, do {
            memcpy(&p_Queue->m_Data[*p_Queue->m_Metadata.m_PushIdxPtr],
                   p_ReturnData, sizeof(struct QMBCall));
        } while (0));

    return rc;
}

int32_t
configureServiceReturnInformation(struct ServiceReturnInfo *p_ReturnInfo,
                                  struct ServiceConnectInfo *p_ConnectInfo,
                                  struct ConnectRequest *p_Request) {
    int32_t rc = 0;
    int returnQFd;
    int requestResponseQFd;
    uint32_t connectionIdx;

    connectionIdx = p_Request->m_ConnectionIdx;

    returnQFd = createShmObject(
        p_Request->m_ReturnQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        p_Request->m_ReturnQSize * sizeof(struct QMBCall), false);

    requestResponseQFd = createShmObject(
        p_Request->m_RequestResponseQName, O_RDWR,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        false);

    struct QMBCall *returnQ =
        mmap(NULL, p_Request->m_ReturnQSize * sizeof(struct QMBCall),
             PROT_WRITE | PROT_READ, MAP_SHARED, returnQFd, 0);
    DIE(returnQ == MAP_FAILED, "Could not map return queue memory");

    struct ConnectResponseInformation *requestResponseQ = mmap(
        NULL,
        p_Request->m_ResponseQSize * sizeof(struct ConnectResponseInformation),
        PROT_WRITE | PROT_READ, MAP_SHARED, requestResponseQFd, 0);
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

    p_ReturnInfo->m_QMBQueue.m_Metadata.m_FullCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQFullCond;
    p_ReturnInfo->m_QMBQueue.m_Metadata.m_EmptyCond =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQEmptyCond;
    p_ReturnInfo->m_QMBQueue.m_Metadata.m_Lock =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQMutex;

    p_ReturnInfo->m_QMBQueue.m_Metadata.m_PushIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPushIdx;
    p_ReturnInfo->m_QMBQueue.m_Metadata.m_PopIdxPtr =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQPopIdx;
    p_ReturnInfo->m_QMBQueue.m_Metadata.m_Size =
        &p_ConnectInfo->m_Connections[connectionIdx].m_ReturnQSize;

    p_ReturnInfo->m_ResponseQueue.m_MaxSize = p_Request->m_ReturnQSize;

    p_ReturnInfo->m_SendReturnFnQMB = s_SendReturnFnQMB;

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