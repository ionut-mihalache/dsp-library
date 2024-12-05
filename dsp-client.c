#include "dsp.h"

#include "dsp-client.h"
#include "protocol.h"
#include "utils/commons.h"

#include "utils/log/log.h"
#include "utils/macros/macros.h"

#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>

static int32_t s_QPushHMB(struct DSPQueue *p_Queue, struct HMBCall *p_Info) {
    int32_t rc = 0;

    pthread_mutex_lock(p_Queue->m_Lock);
    while ((*p_Queue->m_PushIdxPtr) == 2048) {
        pthread_cond_wait(p_Queue->m_EmptyCond, p_Queue->m_Lock);
    }

    memcpy(p_Queue->m_Start + (*p_Queue->m_PushIdxPtr) * sizeof(struct HMBCall),
           p_Info, sizeof(struct HMBCall));

    (*p_Queue->m_PushIdxPtr)++;

    pthread_cond_broadcast(p_Queue->m_FullCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

static int32_t s_QPush(struct DSPQueue *p_Queue) {
    LOGF("pushIdx: %u, popIdx: %u.\n", *p_Queue->m_PushIdxPtr,
         *p_Queue->m_PopIdxPtr);
    (*p_Queue->m_PushIdxPtr)++;
    (*p_Queue->m_PopIdxPtr)++;

    return 0;
}

void pushQ(struct ClientCallInfo *callInfo) {
    LOGF("Calling %s.\n", __func__);
    callInfo->m_CallFn(&callInfo->m_Queue);
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

    for (uint16_t i = 0; i < SERVICES_NUMBER; ++i) {
        installInfo =
            (struct InstallInformation *)(installMemZone + bytesnr +
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
                              CALLQ_MAX_SIZE * sizeof(struct HMBCall), false);

    char *callQ = mmap(NULL, CALLQ_MAX_SIZE * sizeof(struct HMBCall),
                       PROT_WRITE, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map callQ");

    p_CallInfo->m_CallFn = s_QPush;
    p_CallInfo->m_CallFnHMB = s_QPushHMB;
    p_CallInfo->m_Queue.m_Start = callQ;
    p_CallInfo->m_Queue.m_PushIdxPtr = &installInfo->m_CallQPushIdx;
    p_CallInfo->m_Queue.m_PopIdxPtr = &installInfo->m_CallQPopIdx;

    LOGF("callQ ptr: %p, pushIdxPtr: %p, popIdxPtr: %p.\n", callQ,
         p_CallInfo->m_Queue.m_PushIdxPtr, p_CallInfo->m_Queue.m_PopIdxPtr);
}
