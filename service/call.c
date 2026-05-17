// SPDX-License-Identifier: LGPL-2.1-or-later

#include <string.h>
#include <sys/mman.h>

#include "call.h"
#include "commons.h"
#include "macros.h"
#include "platform.h"
#include "system-values.h"

static int32_t s_SMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct SMBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct SMBCall));

    return rc;
}

static int32_t s_EMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct EMBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct EMBCall));

    return rc;
}

static int32_t s_QMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct QMBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_HMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct HMBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_MBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct MBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct MBCall));

    return rc;
}

static int32_t s_DMBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct DMBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct DMBCall));

    return rc;
}

static int32_t s_HGBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct HGBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct HGBCall));

    return rc;
}

static int32_t s_GBPopHelper(void *p_CallInfo, struct DSPQueue *p_Queue) {
    int32_t rc = 0;
    struct GBCall *qData = p_Queue->m_Data;

    memcpy(p_CallInfo, &qData[*p_Queue->m_Metadata.m_PopIdxPtr],
           sizeof(struct GBCall));

    return rc;
}

static int32_t s_QPopSMB(struct SMBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopEMB(struct EMBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopQMB(struct QMBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopHMB(struct HMBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopMB(struct MBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopDMB(struct DMBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopHGB(struct HGBCall *callInfo, struct DSPQueue *queue);
static int32_t s_QPopGB(struct GBCall *callInfo, struct DSPQueue *queue);

/**
 * TODO: Return correct error code. This function 'never fails' at the moment.
 */
static int32_t s_QPopA(struct DSPQueue *queue, void *callInfo,
                       uint32_t qMaxSize,
                       int32_t (*fn)(void *, struct DSPQueue *));

static int32_t s_QPop(struct CommunicationInfo *p_CInfo) {
    switch (p_CInfo->m_Q->m_Type) {
    case SMBQ:
        return s_QPopSMB((struct SMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case EMBQ:
        return s_QPopEMB((struct EMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case QMBQ:
        return s_QPopQMB((struct QMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case HMBQ:
        return s_QPopHMB((struct HMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case MBQ:
        return s_QPopMB((struct MBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case DMBQ:
        return s_QPopDMB((struct DMBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case HGBQ:
        return s_QPopHGB((struct HGBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
    case GBQ:
        return s_QPopGB((struct GBCall *)p_CInfo->m_Data, p_CInfo->m_Q);
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
    int qFlag;
    int qProt;
    mode_t qMode;
    size_t qSize;
    void *callQ;

    switch (p_InstallInfo->m_CallQType) {
    case SMBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = SMB_Q_MAX_SIZE * sizeof(struct SMBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case EMBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = EMB_Q_MAX_SIZE * sizeof(struct EMBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case QMBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = QMB_Q_MAX_SIZE * sizeof(struct QMBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case HMBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = HMB_Q_MAX_SIZE * sizeof(struct HMBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case MBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = MB_Q_MAX_SIZE * sizeof(struct MBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case DMBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = DMB_Q_MAX_SIZE * sizeof(struct DMBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case HGBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = HGB_Q_MAX_SIZE * sizeof(struct HGBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    case GBQ:
        qFlag = AQUA_FILE_PERM_RDWR;
        qMode = AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
                AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
                AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE;
        qSize = GB_Q_MAX_SIZE * sizeof(struct GBCall);
        qProt = AQUA_MEM_PROT_READ;

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    callQFd = SharedMemoryObject.create(p_InstallInfo->m_CallQName, qFlag,
                                        qMode, qSize, true);

    createQ(&callQ, qSize, qProt, callQFd);

    // triggerKernelPageInit(callQ, qSize, qProt);
    Memory.triggerPageFaults(callQ, qSize, qProt);

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_ReceiveCallFn = s_QPop;
    p_CallInfo->m_Q.m_Data = callQ;
    p_CallInfo->m_Q.m_Metadata.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_Q.m_Metadata.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_Q.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;
    p_CallInfo->m_Q.m_Type = p_InstallInfo->m_CallQType;

    Sync.createMutex(&p_InstallInfo->m_CallQMutex, "");
    Sync.createCond(&p_InstallInfo->m_CallQFullCond, "");
    Sync.createCond(&p_InstallInfo->m_CallQEmptyCond, "");

    // pthread_mutexattr_t attr;
    // rc = pthread_mutexattr_init(&attr);
    // DIE(rc != 0, "Could not init mutex attribute");

    // rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    // DIE(rc != 0, "Could not set pshread for mutex attribute");

    // rc = pthread_mutex_init(&p_InstallInfo->m_CallQMutex, &attr);
    // DIE(rc != 0, "Could not init call mutex");

    // rc = pthread_mutexattr_destroy(&attr);
    // DIE(rc != 0, "Could not destroy mutex attribute");

    // pthread_condattr_t condAttr;
    // pthread_condattr_init(&condAttr);

    // pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    // pthread_cond_init(&p_InstallInfo->m_CallQFullCond, &condAttr);
    // pthread_cond_init(&p_InstallInfo->m_CallQEmptyCond, &condAttr);

    // pthread_condattr_destroy(&condAttr);

    p_CallInfo->m_Q.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_Q.m_Metadata.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_Q.m_Metadata.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    return rc;
}

static int32_t s_QPopSMB(struct SMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, SMB_Q_MAX_SIZE, s_SMBPopHelper);
}

static int32_t s_QPopEMB(struct EMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, EMB_Q_MAX_SIZE, s_EMBPopHelper);
}

static int32_t s_QPopQMB(struct QMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, QMB_Q_MAX_SIZE, s_QMBPopHelper);
}

static int32_t s_QPopHMB(struct HMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, HMB_Q_MAX_SIZE, s_HMBPopHelper);
}

static int32_t s_QPopMB(struct MBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, MB_Q_MAX_SIZE, s_MBPopHelper);
}

static int32_t s_QPopDMB(struct DMBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, DMB_Q_MAX_SIZE, s_DMBPopHelper);
}

static int32_t s_QPopHGB(struct HGBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, HGB_Q_MAX_SIZE, s_HGBPopHelper);
}

static int32_t s_QPopGB(struct GBCall *p_CallInfo, struct DSPQueue *p_Queue) {
    return s_QPopA(p_Queue, p_CallInfo, GB_Q_MAX_SIZE, s_GBPopHelper);
}

/**
 * TODO: Return correct error code. This function 'never fails' at the moment.
 */
static int32_t s_QPopA(struct DSPQueue *p_Queue, void *p_CallInfo,
                       uint32_t p_QMaxSize,
                       int32_t (*p_Fn)(void *, struct DSPQueue *)) {
    int32_t rc = 0;

    QPOP(p_Queue, p_QMaxSize, do { rc = p_Fn(p_CallInfo, p_Queue); } while (0));

    return rc;
}
