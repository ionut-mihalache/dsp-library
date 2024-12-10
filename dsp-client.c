#include "dsp.h"

#include "dsp-client.h"
#include "protocol.h"
#include "utils/commons.h"

#include "utils/log/log.h"
#include "utils/macros/macros.h"

#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>

static int32_t s_HandleResponse(struct ConnectResponseInformation *p_Response,
                                struct ConnectResponseInformation *p_Queue) {
    int32_t rc = 0;
    (void)p_Response; // TODO
    (void)p_Queue; // TODO

    return rc;
}

static int32_t
s_SendConnectRequest(struct ConnectQueue *p_Queue,
                     struct ConnectRequestInformation *p_RequestInfo) {
    int32_t rc = 0;
    uint32_t idx;
    int returnRequestQFd = -1;
    struct ConnectInformation *connectInfo;

    pthread_mutex_lock(p_Queue->m_Lock);
    while (*p_Queue->m_Size == CONNECTQ_MAX_SIZE) {
        pthread_cond_wait(p_Queue->m_EmptyCond, p_Queue->m_Lock);
    }

    idx = *p_Queue->m_PushIdxPtr;

    connectInfo = &p_Queue->m_Data[idx];

    memcpy(
        connectInfo->m_ReturnQName, p_RequestInfo->m_ReturnQName,
        min(strlen(p_RequestInfo->m_ReturnQName), RETURNQ_NAME_MAX_SIZE - 1));

    memcpy(connectInfo->m_ReturnRequestQName,
           p_RequestInfo->m_ReturnRequestQName,
           min(strlen(p_RequestInfo->m_ReturnRequestQName),
               RETURNQ_NAME_MAX_SIZE - 1));

    struct ConnectResponseInformation *returnRequestQ =
        mmap(NULL,
             p_RequestInfo->m_ResponseQSize *
                 sizeof(struct ConnectResponseInformation),
             PROT_READ, MAP_SHARED, returnRequestQFd, 0);
    DIE(returnRequestQ == MAP_FAILED,
        "Could not map memory for connect request response");

    /**
     * TODO: Add returnRequestQFd initialization
     */
    rc = close(returnRequestQFd);
    DIE(rc != 0, "Could not close return request file");

    // p_RequestInfo->m_ResponseQ = returnRequestQ;

    connectInfo->m_ReturnQSize = p_RequestInfo->m_ReturnQSize;
    connectInfo->m_Connected = false;
    connectInfo->m_ConnectionError = 0;

    p_RequestInfo->m_Connected = &connectInfo->m_Connected;
    p_RequestInfo->m_ConnectError = &connectInfo->m_ConnectionError;

    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pthread shared for mutex attribute");

    rc = pthread_mutex_init(&connectInfo->m_ReturnResponseQMutex, &attr);
    DIE(rc != 0, "Could not init connect response lock");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    pthread_condattr_t condAttr;

    rc = pthread_condattr_init(&condAttr);
    DIE(rc != 0, "Could not init condition attribute");

    rc = pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pthread shared for condition attribute");

    rc = pthread_cond_init(&connectInfo->m_ReturnResponseQFullCond, &condAttr);
    DIE(rc != 0, "Could not init condition for full connect response queue");

    rc = pthread_cond_init(&connectInfo->m_ReturnResponseQEmptyCond, &condAttr);
    DIE(rc != 0, "Could not init condition for empty connect response queue");

    rc = pthread_condattr_destroy(&condAttr);
    DIE(rc != 0, "Could not destroy condition attribute object");

    p_RequestInfo->m_HandleResponse = s_HandleResponse;

    (*p_Queue->m_PushIdxPtr) =
        ((*p_Queue->m_PushIdxPtr) + 1) % CONNECTQ_MAX_SIZE;
    (*p_Queue->m_Size)++;

    pthread_cond_broadcast(p_Queue->m_FullCond);

    pthread_mutex_unlock(p_Queue->m_Lock);

    return rc;
}

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

void sendConnectRequest(struct ClientConnectInfo *p_ConnectInfo,
                        struct ConnectRequestInformation *p_RequestInfo) {
    p_ConnectInfo->m_SendConnectRequest(&p_ConnectInfo->m_Queue, p_RequestInfo);
}

void pushQ(struct ClientCallInfo *p_CallInfo) {
    p_CallInfo->m_CallFn(&p_CallInfo->m_Queue);
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

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId) {
    int rc;
    int installShmFd;
    int callQFd, connectQFd;
    struct InstallInformation *installInfo;
    uint8_t connected = false;
    uint16_t i;

    installShmFd = createShmObject(INSTALL_MZONE, O_RDWR, 0600,
                                   sizeof(struct InstallInfo), false);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone =
        mmap(NULL, sizeof(struct InstallInfo), PROT_READ | PROT_WRITE,
             MAP_SHARED, installShmFd, 0);
    DIE(installMemZone == MAP_FAILED, "Could not mmap install memory zone");

    for (i = 0; i < SERVICES_NUMBER; ++i) {
        if (!installMemZone->m_Info[i].m_Available) {
            continue;
        }

        installInfo = (struct InstallInformation *)&(installMemZone->m_Info[i]);

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

    rc = munmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");

    /**
     * Map only the information of the service
     */
    installInfo = (struct InstallInformation *)mmap(
        NULL, sizeof(struct InstallInformation), PROT_READ | PROT_WRITE,
        MAP_SHARED, installShmFd, i * sizeof(struct InstallInformation));
    DIE(installInfo == MAP_FAILED, "Could not map service information");

    /**
     * TODO: Implement successfull connection functionality
     */

    connectQFd = createShmObject(
        installInfo->m_ConnectQName, O_RDWR, 0600,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectInformation), false);

    struct ConnectInformation *connectQ =
        mmap(NULL, CONNECTQ_MAX_SIZE * sizeof(struct ConnectInformation),
             PROT_READ | PROT_WRITE, MAP_SHARED, connectQFd, 0);
    DIE(connectQ == MAP_FAILED, "Could not map connectQ");

    rc = close(connectQFd);
    DIE(rc != 0, "Coudl not close connectQFd");

    p_ConnectInfo->m_SendConnectRequest = s_SendConnectRequest;
    p_ConnectInfo->m_Queue.m_Data = connectQ;
    p_ConnectInfo->m_Queue.m_PushIdxPtr = &installInfo->m_ConnectQPushIdx;
    p_ConnectInfo->m_Queue.m_PopIdxPtr = &installInfo->m_ConnectQPopIdx;
    p_ConnectInfo->m_Queue.m_Size = &installInfo->m_ConnectQSize;
    p_ConnectInfo->m_Queue.m_Lock = &installInfo->m_ConnectQMutex;
    p_ConnectInfo->m_Queue.m_FullCond = &installInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_Queue.m_EmptyCond = &installInfo->m_ConnectQEmptyCond;

    callQFd = createShmObject(installInfo->m_CallQName, O_RDWR, 0600,
                              QMB_Q_MAX_SIZE * sizeof(struct QMBCall), false);

    struct QMBCall *callQ = mmap(NULL, QMB_Q_MAX_SIZE * sizeof(struct QMBCall),
                                 PROT_WRITE, MAP_SHARED, callQFd, 0);
    DIE(callQ == MAP_FAILED, "Could not map callQ");

    rc = close(callQFd);
    DIE(rc != 0, "Could not close callQFd");

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

    LOGF("Connected to \'%s\' with version \'%s\'.\n", p_ServiceStrId,
         installInfo->m_Version);
}
