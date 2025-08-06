#include <string.h>

#include "call.h"
#include "commons.h"
#include "log.h"
#include "macros.h"

/**
 * TODO: Return correct error code. This function 'never fails' at the moment.
 */
static int32_t s_QPopA(struct DSPQueue *p_Queue, enum QType p_QType,
                       void *p_CallInfo, uint32_t p_QMaxSize,
                       int32_t (*p_Fn)(void *, void *)) {
    int32_t rc = 0;

    switch (p_QType) {
    case SMBQ:
        /**
         * TODO
         */
        break;
    case EMBQ:
        /**
         * TODO
         */
        break;
    case QMBQ:
        QPOP(p_Queue, p_QMaxSize, do { p_Fn(p_CallInfo, p_Queue); } while (0));

        break;
    case HMBQ:
        QPOP(p_Queue, p_QMaxSize, do { p_Fn(p_CallInfo, p_Queue); } while (0));

        break;
    case MBQ:
        /**
         * TODO
         */
        break;
    case DMBQ:
        /**
         * TODO
         */
        break;
    case GBQ:
        /**
         * TODO
         */
        break;
    case DGBQ:
        /**
         * TODO
         */
        break;
    default:
        /**
         * TODO
         */
        ;
    }

    return rc;
}

static int32_t s_QMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct QMBCall *callInfo = p_CallInfo;

    memcpy(callInfo, &p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_QPopQMB(struct QMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, QMBQ, p_CallInfo, QMB_Q_MAX_SIZE, s_QMBPopHelper);
    // int32_t rc = 0;

    // QPOP(
    //     p_Queue, QMB_Q_MAX_SIZE, do {
    //         memcpy(p_CallInfo,
    //                &p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr],
    //                sizeof(struct QMBCall));
    //     } while (0));

    // return rc;
}

static int32_t s_HMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct QMBCall *callInfo = p_CallInfo;

    memcpy(callInfo, &p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_QPopHMB(struct HMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, HMBQ, p_CallInfo, HMB_Q_MAX_SIZE, s_HMBPopHelper);
    // int32_t rc = 0;

    // QPOP(
    //     p_Queue, HMB_Q_MAX_SIZE, do {
    //         memcpy(p_CallInfo,
    //                &p_Queue->m_Data[*p_Queue->m_Metadata.m_PopIdxPtr],
    //                sizeof(struct HMBCall));
    //     } while (0));

    // return rc;
}

static int32_t s_QPop(struct PopInformation *p_PopInfo) {
    switch (p_PopInfo->m_QType) {
    case SMBQ:
        /**
         * TODO
         */
        return (-1);
    case EMBQ:
        /**
         * TODO
         */
        return (-1);
    case QMBQ:
        return s_QPopQMB(p_PopInfo->m_ReturnData, p_PopInfo->m_Q);
    case HMBQ:
        return s_QPopHMB(p_PopInfo->m_ReturnData, p_PopInfo->m_Q);
    case MBQ:
        /**
         * TODO
         */
        return (-1);
    case DMBQ:
        /**
         * TODO
         */
        return (-1);
    case GBQ:
        /**
         * TODO
         */
        return (-1);
    case DGBQ:
        /**
         * TODO
         */
        return (-1);
    default:
        /**
         * TODO
         */
        return (-1);
    }
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

    // p_CallInfo->m_ReceiveCallFnHMB = s_QPopHMB;
    // p_CallInfo->m_HMBQueue.m_Data = callQ;
    // p_CallInfo->m_HMBQueue.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    // p_CallInfo->m_HMBQueue.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    // p_CallInfo->m_HMBQueue.m_Size = &p_InstallInfo->m_CallQSize;

    // p_CallInfo->m_ReceiveCallFnQMB = s_QPopQMB;
    // p_CallInfo->m_QMBQueue.m_Data = callQ;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_PushIdxPtr =
    //     &p_InstallInfo->m_CallQPushIdx;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_PopIdxPtr =
    //     &p_InstallInfo->m_CallQPopIdx;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;

    p_CallInfo->m_ReceiveCallFn = s_QPop;
    p_CallInfo->m_Q.m_Data = callQ;
    p_CallInfo->m_Q.m_Metadata.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_Q.m_Metadata.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_Q.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;

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

    // p_CallInfo->m_HMBQueue.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    // p_CallInfo->m_HMBQueue.m_Metadata.m_FullCond =
    //     &p_InstallInfo->m_CallQFullCond;
    // p_CallInfo->m_HMBQueue.m_Metadata.m_EmptyCond =
    //     &p_InstallInfo->m_CallQEmptyCond;

    // p_CallInfo->m_QMBQueue.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_FullCond =
    //     &p_InstallInfo->m_CallQFullCond;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_EmptyCond =
    //     &p_InstallInfo->m_CallQEmptyCond;

    p_CallInfo->m_Q.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_Q.m_Metadata.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_Q.m_Metadata.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    return rc;
}
