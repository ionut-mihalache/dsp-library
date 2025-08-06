#include <string.h>

#include "client-call.h"
#include "commons.h"
#include "log.h"
#include "macros.h"

static int32_t s_QPushA(struct DSPQueue *p_Queue, enum QType p_QType,
                        void *p_CallData, uint32_t p_QMaxSize,
                        int32_t (*p_Fn)(struct DSPQueue *, void *)) {
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
        QPUSH(p_Queue, p_QMaxSize, do { p_Fn(p_Queue, p_CallData); } while (0));
        break;
    case HMBQ:
        QPUSH(p_Queue, p_QMaxSize, do { p_Fn(p_Queue, p_CallData); } while (0));
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

static int32_t s_QMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct DSPQueue *queue = p_Queue;
    struct QMBCall *callData = p_CallData;

    memcpy(&queue->m_Data[*queue->m_Metadata.m_PushIdxPtr], callData,
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_QPushQMB(struct DSPQueue *p_Queue,
                          struct QMBCall *p_CallData) {
    return s_QPushA(p_Queue, QMBQ, p_CallData, QMB_Q_MAX_SIZE, s_QMBPushHelper);
}

static int32_t s_HMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct DSPQueue *queue = p_Queue;
    struct HMBCall *callData = p_CallData;

    memcpy(&queue->m_Data[*queue->m_Metadata.m_PushIdxPtr], callData,
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_QPushHMB(struct DSPQueue *p_Queue,
                          struct HMBCall *p_CallData) {
    return s_QPushA(p_Queue, HMBQ, p_CallData, HMB_Q_MAX_SIZE, s_HMBPushHelper);
}

static int32_t s_QPush(struct PushInformation *p_PushInfo) {
    switch (p_PushInfo->m_QType) {
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
        return s_QPushQMB(p_PushInfo->m_Q, p_PushInfo->m_CallData);
    case HMBQ:
        return s_QPushHMB(p_PushInfo->m_Q, p_PushInfo->m_CallData);
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

    // p_CallInfo->m_CallFnHMB = s_QPushHMB;

    // p_CallInfo->m_CallFnQMB = s_QPushQMB;
    // p_CallInfo->m_QMBQueue.m_Data = callQ;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_PushIdxPtr =
    //     &p_InstallInfo->m_CallQPushIdx;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_PopIdxPtr =
    //     &p_InstallInfo->m_CallQPopIdx;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_FullCond =
    //     &p_InstallInfo->m_CallQFullCond;
    // p_CallInfo->m_QMBQueue.m_Metadata.m_EmptyCond =
    //     &p_InstallInfo->m_CallQEmptyCond;

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

    p_CallInfo->m_CallFn = s_QPush;

    p_CallInfo->m_Q.m_Data = callQ;
    p_CallInfo->m_Q.m_Metadata.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_Q.m_Metadata.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_Q.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;
    p_CallInfo->m_Q.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_Q.m_Metadata.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_Q.m_Metadata.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    return rc;
}
