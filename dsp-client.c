#include "dsp.h"

#include "dsp-client.h"
#include "protocol.h"
#include "utils/commons.h"

#include "utils/log/log.h"
#include "utils/macros/macros.h"

#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>

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

    pthread_cond_broadcast(p_Queue->m_FullCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

__attribute_used__ static int32_t s_QPushHMB(struct HMBDSPQueue *p_Queue,
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

    pthread_cond_broadcast(p_Queue->m_FullCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

void pushQ(struct ClientCallInfo *callInfo) {
    LOGF("Calling %s.\n", __func__);
    callInfo->m_CallFn(&callInfo->m_Queue);
}

void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData) {
    p_CallInfo->m_CallFnQMB(&p_CallInfo->m_QMBQueue, p_CallData);
}

void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData) {
    p_CallInfo->m_CallFnHMB(&p_CallInfo->m_HMBQueue, p_CallData);
}

int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size) {
    int32_t rc = 0;

    memcpy(p_CallInfo->m_CallInfo, p_Data, p_Size);
    p_CallInfo->m_Size = p_Size;

    return rc;
}

int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size) {
    int32_t rc = 0;

    memcpy(p_CallInfo->m_CallInfo, p_Data, p_Size);
    p_CallInfo->m_Size = p_Size;

    return rc;
}

void dspConnect(struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId) {
    int installShmFd;
    int callQFd;
    struct InstallInformation *installInfo;
    uint8_t connected = false;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    installShmFd = createShmObject(
        INSTALL_MZONE, O_RDWR, 0600,
        bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)), false);
    DIE(installShmFd < 0, "Could not open shared memory object!");

    uint8_t *installMemZone = mmap(
        NULL, bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)),
        PROT_READ | PROT_WRITE, MAP_SHARED, installShmFd, 0);
    DIE(installMemZone == MAP_FAILED, "Could not mmap installMemZone");

    installMemZone += bytesnr;

    for (uint16_t i = 0; i < SERVICES_NUMBER; ++i) {
        installInfo =
            (struct InstallInformation *)(installMemZone +
                                          i * sizeof(
                                                  struct InstallInformation));

        LOGF("The current service string id is %s.\n", installInfo->m_StrId);
        if (!strcmp(installInfo->m_StrId, p_ServiceStrId)) {
            if (installInfo->m_Available) {
                connected = true;
            }
            break;
        }
    }

    if (!connected) {
        LOGF("Could not connect. Service is not installed or unavailable.\n");
        return;
    }

    /**
     * TODO: Implement successfull connection functionality
     */
    LOGF("Connected to \'%s\' with version \'%s\'.\n", p_ServiceStrId,
         installInfo->m_Version);

    callQFd = createShmObject(installInfo->m_CallQName, O_RDWR, 0600,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), false);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_WRITE, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map callQ");

    p_CallInfo->m_CallFnQMB = s_QPushQMB;
    p_CallInfo->m_QMBQueue.m_Data = callQ;
    p_CallInfo->m_QMBQueue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    p_CallInfo->m_QMBQueue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;
    p_CallInfo->m_QMBQueue.m_Size = &installInfo->m_CallQSize;
    p_CallInfo->m_QMBQueue.m_Lock = &installInfo->m_CallQMutex;
    p_CallInfo->m_QMBQueue.m_FullCond = &installInfo->m_CallQFullCond;
    p_CallInfo->m_QMBQueue.m_EmptyCond = &installInfo->m_CallQEmptyCond;

    // p_CallInfo->m_CallFnHMB = s_QPushHMB;
    // p_CallInfo->m_HMBQueue.m_Data = callQ;
    // p_CallInfo->m_HMBQueue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    // p_CallInfo->m_HMBQueue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;
    // p_CallInfo->m_HMBQueue.m_Size = &installInfo->m_CallQSize;
    // p_CallInfo->m_HMBQueue.m_Lock = &installInfo->m_CallQMutex;
    // p_CallInfo->m_HMBQueue.m_FullCond = &installInfo->m_CallQFullCond;
    // p_CallInfo->m_HMBQueue.m_EmptyCond = &installInfo->m_CallQEmptyCond;
}
