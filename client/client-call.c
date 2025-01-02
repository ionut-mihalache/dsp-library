#include <string.h>

#include "client-call.h"
#include "commons.h"
#include "macros.h"

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

    pthread_mutex_unlock(p_Queue->m_Lock);

    pthread_cond_broadcast(p_Queue->m_FullCond);

    return rc;
}

static int32_t s_QPushHMB(struct HMBDSPQueue *p_Queue,
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

    pthread_mutex_unlock(p_Queue->m_Lock);

    pthread_cond_broadcast(p_Queue->m_FullCond);

    return rc;
}

int32_t
configureClientCallInformation(struct ClientCallInfo *p_CallInfo,
                               struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    int callQFd;

    callQFd = createShmObject(p_InstallInfo->m_CallQName, O_RDWR, 0600,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), false);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_WRITE, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map callQ");

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_CallFnHMB = s_QPushHMB;

    p_CallInfo->m_CallFnQMB = s_QPushQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Size = &p_InstallInfo->m_CallQSize;
    p_CallInfo->m_QMBQueue.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

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
