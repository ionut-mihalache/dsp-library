#include <string.h>

#include "client-call.h"
#include "commons.h"
#include "log.h"
#include "macros.h"

static int32_t s_QPushQMB(struct QMBDSPQueue *p_Queue,
                          struct QMBCall *p_CallData) {
    int32_t rc = 0;

    // LOGF("Start call.\n");
    QPUSH(
        p_Queue, QMB_Q_MAX_SIZE, do {
            // LOGF("push idx[%u] - pop idx[%u] - size[%u]\n", *p_Queue->m_Metadata.m_PushIdxPtr,
            //      *p_Queue->m_Metadata.m_PopIdxPtr, *p_Queue->m_Metadata.m_Size);
            memcpy(&p_Queue->m_Data[*p_Queue->m_Metadata.m_PushIdxPtr],
                   p_CallData, sizeof(struct QMBCall));
        } while (0));
    // LOGF("End call.\n");

    return rc;
}

static int32_t s_QPushHMB(struct HMBDSPQueue *p_Queue,
                          struct HMBCall *p_CallData) {
    int32_t rc = 0;

    QPUSH(
        p_Queue, HMB_Q_MAX_SIZE, do {
            memcpy(&p_Queue->m_Data[*p_Queue->m_Metadata.m_PushIdxPtr],
                   p_CallData, sizeof(struct HMBCall));
        } while (0));

    return rc;
}

int32_t
configureClientCallInformation(struct ClientCallInfo *p_CallInfo,
                               struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    int callQFd;

    callQFd = createShmObject(p_InstallInfo->m_CallQName, O_RDWR,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
                                  S_IWOTH,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), false);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_WRITE, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map callQ");

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_CallFnHMB = s_QPushHMB;

    p_CallInfo->m_CallFnQMB = s_QPushQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_Metadata.m_PushIdxPtr =
        &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_Metadata.m_PopIdxPtr =
        &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;
    p_CallInfo->m_QMBQueue.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_Metadata.m_FullCond =
        &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_CallQEmptyCond;

    // p_CallInfo->m_CallFnHMB = s_QPushHMB;
    // p_CallInfo->m_HMBQueue.m_Data = callQ;
    // p_CallInfo->m_HMBQueue.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    // p_CallInfo->m_HMBQueue.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    // p_CallInfo->m_HMBQueue.m_Size = &p_InstallInfo->m_CallQSize;
    // p_CallInfo->m_HMBQueue.m_Lock = &p_InstallInfo->m_CallQMutex;
    // p_CallInfo->m_HMBQueue.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    // p_CallInfo->m_HMBQueue.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    // LOGF("Connected to \'%s\' with version \'%s\'.\n", p_ServiceStrId,
    //      p_InstallInfo->m_Version);

    return rc;
}
