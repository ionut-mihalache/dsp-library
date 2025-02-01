#include <string.h>

#include "call.h"
#include "commons.h"
#include "log.h"
#include "macros.h"

static int32_t s_QPopQMB(struct QMBCall *p_CallInfo,
                         struct QMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    QPOP(
        p_Queue, QMB_Q_MAX_SIZE, do {
            // LOGF("push idx[%u] - pop idx[%u] - size[%u] - connId[%u]\n",
            //      *p_Queue->m_Metadata.m_PushIdxPtr,
            //      *p_Queue->m_Metadata.m_PopIdxPtr, *p_Queue->m_Metadata.m_Size,
            //      p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr]
            //          .m_Metadata.m_ConnId);
            memcpy(p_CallInfo,
                   &p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr],
                   sizeof(struct QMBCall));
        } while (0));

    return rc;
}

static int32_t s_QPopHMB(struct HMBCall *p_CallInfo,
                         struct HMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    QPOP(
        p_Queue, HMB_Q_MAX_SIZE, do {
            memcpy(p_CallInfo,
                   &p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr],
                   sizeof(struct HMBCall));
        } while (0));

    return rc;
}

int32_t
configureServiceCallInformation(struct ServiceCallInfo *p_CallInfo,
                                struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    int callQFd;

    callQFd = createShmObject(p_InstallInfo->m_CallQName, O_RDWR,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
                                  S_IWOTH,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), true);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_READ, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map call queue memory");

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_ReceiveCallFnHMB = s_QPopHMB;
    // p_CallInfo->m_HMBQueue.m_Data = callQ;
    // p_CallInfo->m_HMBQueue.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    // p_CallInfo->m_HMBQueue.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    // p_CallInfo->m_HMBQueue.m_Size = &p_InstallInfo->m_CallQSize;

    p_CallInfo->m_ReceiveCallFnQMB = s_QPopQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_Metadata.m_PushIdxPtr =
        &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_Metadata.m_PopIdxPtr =
        &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pshread for mutex attribute");

    rc = pthread_mutex_init(&p_InstallInfo->m_CallQMutex, &attr);
    DIE(rc != 0, "Could not init call mutex");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);

    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&p_InstallInfo->m_CallQFullCond, &condAttr);
    pthread_cond_init(&p_InstallInfo->m_CallQEmptyCond, &condAttr);

    pthread_condattr_destroy(&condAttr);

    p_CallInfo->m_HMBQueue.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_HMBQueue.m_Metadata.m_FullCond =
        &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_HMBQueue.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_CallQEmptyCond;

    p_CallInfo->m_QMBQueue.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_Metadata.m_FullCond =
        &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_CallQEmptyCond;

    return rc;
}
