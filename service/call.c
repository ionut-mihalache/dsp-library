#include <string.h>

#include "call.h"
#include "commons.h"
#include "log.h"
#include "macros.h"

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

static int32_t s_QPop(struct PopInformation *p_PopInfo) {
    switch (p_PopInfo->m_QType) {
    case SMBQ:
        return s_QPopSMB((struct SMBCall *)p_PopInfo->m_ReturnData,
                         p_PopInfo->m_Q);
    case EMBQ:
        return s_QPopEMB((struct EMBCall *)p_PopInfo->m_ReturnData,
                         p_PopInfo->m_Q);
    case QMBQ:
        return s_QPopQMB((struct QMBCall *)p_PopInfo->m_ReturnData,
                         p_PopInfo->m_Q);
    case HMBQ:
        return s_QPopHMB((struct HMBCall *)p_PopInfo->m_ReturnData,
                         p_PopInfo->m_Q);
    case MBQ:
        return s_QPopMB((struct MBCall *)p_PopInfo->m_ReturnData,
                        p_PopInfo->m_Q);
    case DMBQ:
        return s_QPopDMB((struct DMBCall *)p_PopInfo->m_ReturnData,
                         p_PopInfo->m_Q);
    case HGBQ:
        return s_QPopHGB((struct HGBCall *)p_PopInfo->m_ReturnData,
                         p_PopInfo->m_Q);
    case GBQ:
        return s_QPopGB((struct GBCall *)p_PopInfo->m_ReturnData,
                        p_PopInfo->m_Q);
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
    void *callQ;

    switch (p_InstallInfo->m_CallQType) {
    case SMBQ:
        callQFd = createShmObject(
            p_InstallInfo->m_CallQName, O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            SMB_Q_MAX_SIZE * sizeof(struct SMBCall), true);

        createQ(&callQ, SMB_Q_MAX_SIZE * sizeof(struct SMBCall), PROT_READ,
                callQFd);

        break;
    case EMBQ:
        callQFd = createShmObject(
            p_InstallInfo->m_CallQName, O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            EMB_Q_MAX_SIZE * sizeof(struct EMBCall), true);

        createQ(&callQ, EMB_Q_MAX_SIZE * sizeof(struct EMBCall), PROT_READ,
                callQFd);

        break;
    case QMBQ:
        callQFd = createShmObject(
            p_InstallInfo->m_CallQName, O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            QMB_Q_MAX_SIZE * sizeof(struct QMBCall), true);

        createQ(&callQ, QMB_Q_MAX_SIZE * sizeof(struct QMBCall), PROT_READ,
                callQFd);

        break;
    case HMBQ:
        callQFd = createShmObject(
            p_InstallInfo->m_CallQName, O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            HMB_Q_MAX_SIZE * sizeof(struct HMBCall), true);

        createQ(&callQ, HMB_Q_MAX_SIZE * sizeof(struct HMBCall), PROT_READ,
                callQFd);

        break;
    case MBQ:
        callQFd = createShmObject(p_InstallInfo->m_CallQName, O_RDWR,
                                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                      S_IROTH | S_IWOTH,
                                  MB_Q_MAX_SIZE * sizeof(struct MBCall), true);

        createQ(&callQ, MB_Q_MAX_SIZE * sizeof(struct MBCall), PROT_READ,
                callQFd);

        break;
    case DMBQ:
        callQFd = createShmObject(
            p_InstallInfo->m_CallQName, O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            DMB_Q_MAX_SIZE * sizeof(struct DMBCall), true);

        createQ(&callQ, DMB_Q_MAX_SIZE * sizeof(struct DMBCall), PROT_READ,
                callQFd);

        break;
    case HGBQ:
        callQFd = createShmObject(
            p_InstallInfo->m_CallQName, O_RDWR,
            S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
            HGB_Q_MAX_SIZE * sizeof(struct HGBCall), true);

        createQ(&callQ, HGB_Q_MAX_SIZE * sizeof(struct HGBCall), PROT_READ,
                callQFd);

        break;
    case GBQ:
        callQFd = createShmObject(p_InstallInfo->m_CallQName, O_RDWR,
                                  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                      S_IROTH | S_IWOTH,
                                  GB_Q_MAX_SIZE * sizeof(struct GBCall), true);

        createQ(&callQ, GB_Q_MAX_SIZE * sizeof(struct GBCall), PROT_READ,
                callQFd);

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_ReceiveCallFn = s_QPop;
    p_CallInfo->m_Q.m_Data = callQ;
    p_CallInfo->m_Q.m_Metadata.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_Q.m_Metadata.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_Q.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;
    p_CallInfo->m_Q.m_Type = p_InstallInfo->m_CallQType;

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

    p_CallInfo->m_Q.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_Q.m_Metadata.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_Q.m_Metadata.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;

    return rc;
}
