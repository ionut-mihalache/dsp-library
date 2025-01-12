#include <string.h>

#include "call.h"
#include "commons.h"
#include "log.h"
#include "macros.h"

static int32_t s_QPopQMB(struct QMBCall *p_CallInfo,
                         struct QMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    rc = pthread_mutex_lock(p_Queue->m_Lock);
    DIE(rc != 0, "Could not lock mutex!");
    while (*p_Queue->m_Size == 0) {
        rc = pthread_cond_wait(p_Queue->m_FullCond, p_Queue->m_Lock);
        DIE(rc != 0, "Could not wait for condition!");
    }

    memcpy(p_CallInfo, &p_Queue->m_Data[*p_Queue->m_PopIdxPtr],
           sizeof(struct QMBCall));

    (*p_Queue->m_PopIdxPtr) = ((*p_Queue->m_PopIdxPtr) + 1) % QMB_Q_MAX_SIZE;
    (*p_Queue->m_Size)--;

    rc = pthread_cond_broadcast(p_Queue->m_EmptyCond);
    DIE(rc != 0, "Could not signal condition!");

    rc = pthread_mutex_unlock(p_Queue->m_Lock);
    DIE(rc != 0, "Could not unlock mutex!");

    return rc;
}

static int32_t s_QPopHMB(struct HMBCall *p_CallInfo,
                         struct HMBDSPQueue *p_Queue) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == 0) {
        pthread_cond_wait(p_Queue->m_FullCond, p_Queue->m_Lock);
    }

    memcpy(p_CallInfo, &p_Queue->m_Data[*p_Queue->m_PopIdxPtr],
           sizeof(struct HMBCall));

    LOGF("%s: Message length: %u. Message: %s.\n", __func__,
         p_Queue->m_Data[*p_Queue->m_PopIdxPtr].m_CallMetadata.m_Size,
         p_Queue->m_Data[*p_Queue->m_PopIdxPtr].m_CallInfo);

    (*p_Queue->m_PopIdxPtr) = ((*p_Queue->m_PopIdxPtr) + 1) % HMB_Q_MAX_SIZE;
    (*p_Queue->m_Size)--;

    pthread_cond_broadcast(p_Queue->m_EmptyCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

int32_t
configureServiceCallInformation(struct ServiceCallInfo *p_CallInfo,
                                struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    int callQFd;

    callQFd = createShmObject(p_InstallInfo->m_CallQName, O_RDWR,
                              S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
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
    p_CallInfo->m_QMBQueue.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Size = &p_InstallInfo->m_CallQSize;

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

    p_CallInfo->m_HMBQueue.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_HMBQueue.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_HMBQueue.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    p_CallInfo->m_QMBQueue.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    return rc;
}
