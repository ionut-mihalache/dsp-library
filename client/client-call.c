// SPDX-License-Identifier: LGPL-2.1-or-later

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include "client-call.h"
#include "commons.h"
#include "macros.h"
#include "platform.h"
#include "system-values.h"

static int32_t s_SMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct SMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct SMBCall));

    return rc;
}

static int32_t s_EMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct EMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct EMBCall));

    return rc;
}

static int32_t s_QMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct QMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct QMBCall));

    return rc;
}

static int32_t s_HMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct HMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct HMBCall));

    return rc;
}

static int32_t s_MBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct MBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct MBCall));

    return rc;
}

static int32_t s_DMBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct DMBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct DMBCall));

    return rc;
}

static int32_t s_HGBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct HGBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct HGBCall));

    return rc;
}

static int32_t s_GBPushHelper(struct DSPQueue *p_Queue, void *p_CallData) {
    int32_t rc = 0;
    struct GBCall *qData = p_Queue->m_Data;

    memcpy(&qData[*p_Queue->m_Metadata.m_PushIdxPtr], p_CallData,
           sizeof(struct GBCall));

    return rc;
}

static int32_t s_QPushA(struct DSPQueue *p_Queue, void *p_CallData,
                        uint32_t p_QMaxSize,
                        int32_t (*p_Fn)(struct DSPQueue *, void *)) {
    int32_t rc = 0;

    QPUSH(
        p_Queue, p_QMaxSize, do { rc = p_Fn(p_Queue, p_CallData); } while (0));

    return rc;
}

static int32_t s_QPushSMB(struct DSPQueue *p_Queue,
                          struct SMBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, SMB_Q_MAX_SIZE, s_SMBPushHelper);
}

static int32_t s_QPushEMB(struct DSPQueue *p_Queue,
                          struct EMBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, EMB_Q_MAX_SIZE, s_EMBPushHelper);
}

static int32_t s_QPushQMB(struct DSPQueue *p_Queue,
                          struct QMBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, QMB_Q_MAX_SIZE, s_QMBPushHelper);
}

static int32_t s_QPushHMB(struct DSPQueue *p_Queue,
                          struct HMBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, HMB_Q_MAX_SIZE, s_HMBPushHelper);
}

static int32_t s_QPushMB(struct DSPQueue *p_Queue, struct MBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, MB_Q_MAX_SIZE, s_MBPushHelper);
}

static int32_t s_QPushDMB(struct DSPQueue *p_Queue,
                          struct DMBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, DMB_Q_MAX_SIZE, s_DMBPushHelper);
}

static int32_t s_QPushHGB(struct DSPQueue *p_Queue,
                          struct HGBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, HGB_Q_MAX_SIZE, s_HGBPushHelper);
}

static int32_t s_QPushGB(struct DSPQueue *p_Queue, struct GBCall *p_CallData) {
    return s_QPushA(p_Queue, p_CallData, GB_Q_MAX_SIZE, s_GBPushHelper);
}

static int32_t s_QPush(struct CommunicationInfo *p_CInfo) {
    switch (p_CInfo->m_Q->m_Type) {
    case SMBQ:
        return s_QPushSMB(p_CInfo->m_Q, (struct SMBCall *)p_CInfo->m_Data);
    case EMBQ:
        return s_QPushEMB(p_CInfo->m_Q, (struct EMBCall *)p_CInfo->m_Data);
    case QMBQ:
        return s_QPushQMB(p_CInfo->m_Q, (struct QMBCall *)p_CInfo->m_Data);
    case HMBQ:
        return s_QPushHMB(p_CInfo->m_Q, (struct HMBCall *)p_CInfo->m_Data);
    case MBQ:
        return s_QPushMB(p_CInfo->m_Q, (struct MBCall *)p_CInfo->m_Data);
    case DMBQ:
        return s_QPushDMB(p_CInfo->m_Q, (struct DMBCall *)p_CInfo->m_Data);
    case HGBQ:
        return s_QPushHGB(p_CInfo->m_Q, (struct HGBCall *)p_CInfo->m_Data);
    case GBQ:
        return s_QPushGB(p_CInfo->m_Q, (struct GBCall *)p_CInfo->m_Data);
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
    int qFlag;
    int qProt;
    mode_t qMode;
    size_t qSize;
    void *callQ;

    switch (p_InstallInfo->m_CallQType) {
    case SMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = SMB_Q_MAX_SIZE * sizeof(struct SMBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case EMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = EMB_Q_MAX_SIZE * sizeof(struct EMBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case QMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = QMB_Q_MAX_SIZE * sizeof(struct QMBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case HMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = HMB_Q_MAX_SIZE * sizeof(struct HMBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case MBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = MB_Q_MAX_SIZE * sizeof(struct MBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case DMBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = DMB_Q_MAX_SIZE * sizeof(struct DMBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case HGBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = HGB_Q_MAX_SIZE * sizeof(struct HGBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    case GBQ:
        qFlag = O_RDWR;
        qMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
        qSize = GB_Q_MAX_SIZE * sizeof(struct GBCall);
        qProt = AQUA_MEM_PROT_WRITE;

        break;
    default:
        /**
         * TODO
         */
        DIE(true, "QType is not recognized");
    }

    callQFd =
        createShmObject(p_InstallInfo->m_CallQName, qFlag, qMode, qSize, false);

    createQ(&callQ, qSize, qProt, callQFd);

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

    p_CallInfo->m_CallFn = s_QPush;

    p_CallInfo->m_Q.m_Data = callQ;
    p_CallInfo->m_Q.m_Metadata.m_PushIdxPtr = &p_InstallInfo->m_CallQPushIdx;
    p_CallInfo->m_Q.m_Metadata.m_PopIdxPtr = &p_InstallInfo->m_CallQPopIdx;
    p_CallInfo->m_Q.m_Metadata.m_Size = &p_InstallInfo->m_CallQSize;
    p_CallInfo->m_Q.m_Metadata.m_Lock = &p_InstallInfo->m_CallQMutex;
    p_CallInfo->m_Q.m_Metadata.m_FullCond = &p_InstallInfo->m_CallQFullCond;
    p_CallInfo->m_Q.m_Metadata.m_EmptyCond = &p_InstallInfo->m_CallQEmptyCond;
    p_CallInfo->m_Q.m_Type = p_InstallInfo->m_CallQType;

    return rc;
}
