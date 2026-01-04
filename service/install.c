#include <stdio.h>
#include <string.h>

#include "commons.h"
#include "install.h"
#include "dsp.h"
#include "macros.h"
#include "return.h"
#include "system-values.h"

int32_t initializeServiceConnections(struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    uint32_t i;
    struct ConnectionInformation *connInfo;
#if defined(_WIN32)
    char qSyncName[RETURNQ_NAME_MAX_SIZE << 1];
    SIZE_T mutexId = 3;
    SIZE_T eventId = 1006;
#endif

#if defined(__linux__)
    for (i = 0; i < OPENED_CONNECTIONS; ++i) {
        connInfo = &p_InstallInfo->m_Connections[i];

        pthread_mutexattr_t attr;
        rc = pthread_mutexattr_init(&attr);
        DIE(rc != 0, "Could not init mutex attribute");

        rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        DIE(rc != 0, "Could not set pthread shared for mutex attribute");

        rc = pthread_mutex_init(&connInfo->m_ReturnQMutex, &attr);
        DIE(rc != 0, "Could not init connect response lock");

        rc = pthread_mutex_init(&connInfo->m_RequestResponseQMutex, &attr);
        DIE(rc != 0, "Could not init connect response lock");

        rc = pthread_mutexattr_destroy(&attr);
        DIE(rc != 0, "Could not destroy mutex attribute");

        pthread_condattr_t condAttr;

        rc = pthread_condattr_init(&condAttr);
        DIE(rc != 0, "Could not init condition attribute");

        rc = pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
        DIE(rc != 0, "Could not set pthread shared for condition attribute");

        rc = pthread_cond_init(&connInfo->m_ReturnQFullCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for full connect response queue");

        rc = pthread_cond_init(&connInfo->m_ReturnQEmptyCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for empty connect response queue");

        rc =
            pthread_cond_init(&connInfo->m_RequestResponseQFullCond, &condAttr);
        DIE(rc != 0,
            "Could not init condition for full connect response queue");

        rc = pthread_cond_init(&connInfo->m_RequestResponseQEmptyCond,
                               &condAttr);
        DIE(rc != 0,
            "Could not init condition for empty connect response queue");

        rc = pthread_condattr_destroy(&condAttr);
        DIE(rc != 0, "Could not destroy condition attribute object");
    }
#elif defined(_WIN32)
    // At this point we want to initialize the synchronisation arrays
    struct ConnectionSyncInformation *connSyncInfo;
    for (i = 0; i < SYNC_ELEMENTS; ++i) {
        connSyncInfo = &p_InstallInfo->m_ConnectionsSyncData[i];

        InterlockedExchange(&connSyncInfo->m_ReturnQWaitProduce, 0);
        InterlockedExchange(&connSyncInfo->m_ReturnQWaitConsume, 0);

        InterlockedExchange(&connSyncInfo->m_RequestResponseQWaitProduce, 0);
        InterlockedExchange(&connSyncInfo->m_RequestResponseQWaitConsume, 0);

        // Create return queue handles
        snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__", eventId, i);
        DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
            "Could not create return queue produce event");
        connSyncInfo->m_ReturnQProduceCond = eventId;

        eventId++;

        snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__", eventId, i);
        DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
            "Could not create return queue consume event");
        connSyncInfo->m_ReturnQConsumeCond = eventId;

        eventId++;

        // Crete request-response queue handles
        snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__", eventId, i);
        DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
            "Could not create request-response queue produce event");
        connSyncInfo->m_RequestResponseQProduceCond = eventId;

        eventId++;

        snprintf(qSyncName, sizeof(qSyncName), "__aqua_%llu_%u__", eventId, i);
        DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
            "Could not create request-response queue consume event");
        connSyncInfo->m_RequestResponseQConsumeCond = eventId;

        eventId++;
    }
    // for (i = 0; i < OPENED_CONNECTIONS; ++i) {
    //     connInfo = &p_InstallInfo->m_Connections[i];

    //     // TODO: Replace the hardcoded values
    //     // At this point we just create the object which will be opened when
    //     // needed by the service and the client

    //     snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
    //     mutexId);
    //     // connInfo->m_ReturnQMutex = CreateMutex(NULL, FALSE, qSyncName);
    //     DIE(CreateMutex(NULL, FALSE, qSyncName) == NULL,
    //         "Could not create return queue mutex");
    //     connInfo->m_ReturnQMutex = mutexId;

    //     mutexId++;

    //     snprintf(qSyncName, sizeof(qSyncName), "%s-%llu",
    //     "request-response-q",
    //              mutexId);
    //     DIE(CreateMutex(NULL, FALSE, qSyncName) == NULL,
    //         "Could not create request-response queue mutex");
    //     connInfo->m_RequestResponseQMutex = mutexId;

    //     mutexId++;

    //     snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
    //     eventId); DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
    //         "Could not create call queue full event");
    //     connInfo->m_ReturnQFullCond = eventId;

    //     eventId++;

    //     snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", "return-q",
    //     eventId); DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
    //         "Could not create return queue empty event");
    //     connInfo->m_ReturnQEmptyCond = eventId;

    //     eventId++;

    //     snprintf(qSyncName, sizeof(qSyncName), "%s-%llu",
    //     "request-response-q",
    //              eventId);
    //     DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
    //         "Could not create request-response full event");
    //     connInfo->m_RequestResponseQFullCond = eventId;

    //     eventId++;

    //     snprintf(qSyncName, sizeof(qSyncName), "%s-%llu",
    //     "request-response-q",
    //              eventId);
    //     DIE(CreateEvent(NULL, FALSE, FALSE, qSyncName) == NULL,
    //         "Could not create request-response empty event");
    //     connInfo->m_RequestResponseQEmptyCond = eventId;
    // }
#else
#error "Platform not supported by AQUA"
#endif

    return rc;
}

static int32_t
s_SendConnectResponse(struct ServiceReturnInfo *p_ReturnInfo,
                      struct ConnectResponseInformation *p_ResponseInfo) {
    int32_t rc = 0;

    /**
     * Send the response to the client to announce that the communication is
     * established
     */
    USQPUSH(
        &p_ReturnInfo->m_ResponseQueue, p_ReturnInfo->m_ResponseQueue.m_MaxSize,
        do {
            memcpy(&(p_ReturnInfo->m_ResponseQueue.m_Data[currIdx]),
                   p_ResponseInfo, sizeof(struct ConnectResponseInformation));

            memcpy(&p_ReturnInfo->m_ConnectResponseInformation, p_ResponseInfo,
                   sizeof(struct ConnectResponseInformation));
        } while (0));
    // QPUSH(
    //     &p_ReturnInfo->m_ResponseQueue,
    //     p_ReturnInfo->m_ResponseQueue.m_MaxSize, do {
    //         memcpy(
    //             &(p_ReturnInfo->m_ResponseQueue.m_Data
    //                   [*p_ReturnInfo->m_ResponseQueue.m_Metadata.m_PushIdxPtr]),
    //             p_ResponseInfo, sizeof(struct ConnectResponseInformation));

    //         memcpy(&p_ReturnInfo->m_ConnectResponseInformation,
    //         p_ResponseInfo,
    //                sizeof(struct ConnectResponseInformation));
    //     } while (0));

    return rc;
}

static int32_t
s_ReceiveConnectRequest(struct ServiceReturnInfo *p_ReturnInfo,
                        struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    struct ConnectQueue *queue = &p_ConnectInfo->m_ConnectQ;
    struct ConnectResponseInformation responseInfo;

    USQPOP(
        queue, CONNECTQ_MAX_SIZE, do {
            configureServiceReturnInformation(p_ReturnInfo, p_ConnectInfo,
                                              &queue->m_Data[currIdx]);

            memcpy(responseInfo.m_ReturnQName,
                   queue->m_Data[currIdx].m_ReturnQName, RETURNQ_NAME_MAX_SIZE);
            memcpy(responseInfo.m_ReturnRequestQName,
                   queue->m_Data[currIdx].m_RequestResponseQName,
                   RETURNQ_NAME_MAX_SIZE);
            responseInfo.m_Id = queue->m_Data[currIdx].m_ConnectionIdx;
        } while (0));

    s_SendConnectResponse(p_ReturnInfo, &responseInfo);

    return rc;
}

static int32_t
s_ReceiveDisconnectRequest(struct ServiceConnectInfo *p_ConnectInfo) {
    int32_t rc = 0;
    uint32_t connId;
    struct DisconnectQueue *queue = &p_ConnectInfo->m_DisconnectQ;

    USQPOP(
        queue, CONNECTQ_MAX_SIZE,
        do { connId = queue->m_Data[currIdx].m_ConnectionIdx; } while (0));

#if defined(__linux__)
    pthread_spin_lock(p_ConnectInfo->m_ConnectLock);

    rc = munmap(p_ConnectInfo->m_Connections[connId].m_RequestResponseQ,
                p_ConnectInfo->m_Connections[connId].m_RequestResponseQMapSize);
    DIE(rc < 0, "Could not unmap request response queue");
    p_ConnectInfo->m_Connections[connId].m_RequestResponseQ = NULL;

    rc = munmap(p_ConnectInfo->m_Connections[connId].m_ReturnQ,
                p_ConnectInfo->m_Connections[connId].m_ReturnQMapSize);
    DIE(rc < 0, "Could not unmap return queue");
    p_ConnectInfo->m_Connections[connId].m_ReturnQ = NULL;

    rc =
        shm_unlink(p_ConnectInfo->m_Connections[connId].m_RequestResponseQName);
    DIE(rc != 0,
        "Could not unlink request response queue shared memory object");

    rc = shm_unlink(p_ConnectInfo->m_Connections[connId].m_ReturnQName);
    DIE(rc != 0, "Could no unlink return queue shared memory object");

    p_ConnectInfo->m_Connections[connId].m_Connected = false;

    pthread_spin_unlock(p_ConnectInfo->m_ConnectLock);
#elif defined(_WIN32)
    WaitForSingleObject(p_ConnectInfo->m_ConnectLock, INFINITE);

    DIE(!UnmapViewOfFile(
            p_ConnectInfo->m_Connections[connId].m_RequestResponseQ),
        "Could not unmap request response queue");
    p_ConnectInfo->m_Connections[connId].m_RequestResponseQ = NULL;

    DIE(!UnmapViewOfFile(p_ConnectInfo->m_Connections[connId].m_ReturnQ),
        "Could not unmap request response queue");
    p_ConnectInfo->m_Connections[connId].m_ReturnQ = NULL;

    p_ConnectInfo->m_Connections[connId].m_Connected = false;

    // TODO: Check how to handle unlinking for windows

    ReleaseMutex(p_ConnectInfo->m_ConnectLock);
#else
#endif
    return rc;
}

int32_t
configureServiceConnectInformation(struct ServiceConnectInfo *p_ConnectInfo,
                                   struct InstallInformation *p_InstallInfo) {
    int32_t rc = 0;
    aqua_file_handle connectQHandle, disconnectQHandle;
    struct ConnectRequest *connectQ;
    struct ConnectRequest *disconnectQ;
#if defined(_WIN32)
    char qSyncName[RETURNQ_NAME_MAX_SIZE << 1];
#endif

    InterlockedExchange(&p_InstallInfo->m_ConnectQWaitConsume, 0);
    InterlockedExchange(&p_InstallInfo->m_ConnectQWaitProduce, 0);
    InterlockedExchange(&p_InstallInfo->m_ConnectQPushIdxAtomic, 0);
    InterlockedExchange(&p_InstallInfo->m_ConnectQPopIdxAtomic, 0);
    InterlockedExchange(&p_InstallInfo->m_ConnectQSizeAtomic, 0);

    InterlockedExchange(&p_InstallInfo->m_DisconnectQWaitConsume, 0);
    InterlockedExchange(&p_InstallInfo->m_DisconnectQWaitProduce, 0);
    InterlockedExchange(&p_InstallInfo->m_DisconnectQPushIdxAtomic, 0);
    InterlockedExchange(&p_InstallInfo->m_DisconnectQPopIdxAtomic, 0);
    InterlockedExchange(&p_InstallInfo->m_DisconnectQSizeAtomic, 0);

    connectQHandle = createShmObject(
        p_InstallInfo->m_ConnectQName, O_RDWR,
        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
            AQUA_S_IROTH | AQUA_S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    createQ((aqua_void_t **)&connectQ,
            CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
            AQUA_PROT_READ | AQUA_PROT_WRITE, connectQHandle);

    triggerKernelPageInit(connectQ,
                          CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
                          AQUA_PROT_READ | AQUA_PROT_WRITE);

#if defined(__linux__)
    rc = close(connectQHandle);
    DIE(rc != 0, "Could not close connectQHandle");
#elif defined(_WIN32)
    // DIE(!CloseHandle(connectQHandle), "Could not close connectQHandle");
#else
#endif

    disconnectQHandle = createShmObject(
        p_InstallInfo->m_DisconnectQName, O_RDWR,
        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP | AQUA_S_IWGRP |
            AQUA_S_IROTH | AQUA_S_IWOTH,
        CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest), true);

    createQ((aqua_void_t **)&disconnectQ,
            CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
            AQUA_PROT_READ | AQUA_PROT_WRITE, disconnectQHandle);

    triggerKernelPageInit(disconnectQ,
                          CONNECTQ_MAX_SIZE * sizeof(struct ConnectRequest),
                          AQUA_PROT_READ | AQUA_PROT_WRITE);

#if defined(__linux__)
    rc = close(disconnectQHandle);
    DIE(rc != 0, "Could not close disconnectQHandle");
#elif defined(_WIN32)
    // DIE(!CloseHandle(disconnectQHandle), "Could not close
    // disconnectQHandle");
#else
#endif

    p_ConnectInfo->m_Connections = p_InstallInfo->m_Connections;
    p_ConnectInfo->m_ConnectionsSyncData = p_InstallInfo->m_ConnectionsSyncData;

    p_ConnectInfo->m_ReceiveConnectRequest = s_ReceiveConnectRequest;
    p_ConnectInfo->m_ConnectQ.m_Data = connectQ;

    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PushIdxAtomic =
        &p_InstallInfo->m_ConnectQPushIdxAtomic;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_PopIdxAtomic =
        &p_InstallInfo->m_ConnectQPopIdxAtomic;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_WaitConsume =
        &p_InstallInfo->m_ConnectQWaitConsume;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_WaitProduce =
        &p_InstallInfo->m_ConnectQWaitProduce;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_SizeAtomic =
        &p_InstallInfo->m_ConnectQSizeAtomic;

    p_ConnectInfo->m_ReceiveDisconnectRequest = s_ReceiveDisconnectRequest;
    p_ConnectInfo->m_DisconnectQ.m_Data = disconnectQ;

    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PushIdxAtomic =
        &p_InstallInfo->m_DisconnectQPushIdxAtomic;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_PopIdxAtomic =
        &p_InstallInfo->m_DisconnectQPopIdxAtomic;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_WaitConsume =
        &p_InstallInfo->m_DisconnectQWaitConsume;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_WaitProduce =
        &p_InstallInfo->m_DisconnectQWaitProduce;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_SizeAtomic =
        &p_InstallInfo->m_DisconnectQSizeAtomic;

#if defined(__linux__)
    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    DIE(rc != 0, "Could not init mutex attribute");

    rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not set pshread for mutex attribute");

    rc = pthread_mutex_init(&p_InstallInfo->m_ConnectQMutex, &attr);
    DIE(rc != 0, "Could not init connect mutex");

    rc = pthread_mutex_init(&p_InstallInfo->m_DisconnectQMutex, &attr);
    DIE(rc != 0, "Could not init disconnect mutex");

    // rc = pthread_mutex_init(&p_InstallInfo->m_ConnectListLock, &attr);
    // DIE(rc != 0, "Could not init opened connections lock");

    rc = pthread_mutexattr_destroy(&attr);
    DIE(rc != 0, "Could not destroy mutex attribute");

    rc = pthread_spin_init(&p_InstallInfo->m_ConnectListLock,
                           PTHREAD_PROCESS_SHARED);
    DIE(rc != 0, "Could not init opened connections lock");

    p_ConnectInfo->m_ConnectLock = &p_InstallInfo->m_ConnectListLock;

    pthread_condattr_t condAttr;
    pthread_condattr_init(&condAttr);

    pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);

    pthread_cond_init(&p_InstallInfo->m_ConnectQFullCond, &condAttr);
    pthread_cond_init(&p_InstallInfo->m_ConnectQEmptyCond, &condAttr);

    pthread_cond_init(&p_InstallInfo->m_DisconnectQFullCond, &condAttr);
    pthread_cond_init(&p_InstallInfo->m_DisconnectQEmptyCond, &condAttr);

    pthread_condattr_destroy(&condAttr);

    p_ConnectInfo->m_ConnectQ.m_Metadata.m_Lock =
        &p_InstallInfo->m_ConnectQMutex;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_FullCond =
        &p_InstallInfo->m_ConnectQFullCond;
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_ConnectQEmptyCond;

    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_Lock =
        &p_InstallInfo->m_DisconnectQMutex;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_FullCond =
        &p_InstallInfo->m_DisconnectQFullCond;
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_EmptyCond =
        &p_InstallInfo->m_DisconnectQEmptyCond;
#elif defined(_WIN32)
    // WIP: For handles identification in shared userspace memory we use numbers
    //  TODO: Replace the hardcoded values for shared handle ids
    snprintf(qSyncName, sizeof(qSyncName), "%s-%llu", p_InstallInfo->m_StrId,
             8912LLU);
    p_ConnectInfo->m_ConnectLock = CreateMutex(NULL, FALSE, qSyncName);
    DIE(p_ConnectInfo->m_ConnectLock == NULL,
        "Could not create connect list mutex");
    p_InstallInfo->m_ConnectListLock = 8912LLU;

    // Create connect queue handles
    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%s_connect_produce_cond__",
             p_InstallInfo->m_StrId);
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_ProduceCond =
        CreateEvent(NULL, FALSE, FALSE, qSyncName);
    DIE(p_ConnectInfo->m_ConnectQ.m_Metadata.m_ProduceCond == NULL,
        "Could not create connect queue produce event");

    snprintf(qSyncName, sizeof(qSyncName), "__aqua_%s_connect_consume_cond__",
             p_InstallInfo->m_StrId);
    p_ConnectInfo->m_ConnectQ.m_Metadata.m_ConsumeCond =
        CreateEvent(NULL, FALSE, FALSE, qSyncName);
    DIE(p_ConnectInfo->m_ConnectQ.m_Metadata.m_ConsumeCond == NULL,
        "Could not create connect queue consume event");

    // Create disconnect queue handles
    snprintf(qSyncName, sizeof(qSyncName),
             "__aqua_%s_disconnect_produce_cond__", p_InstallInfo->m_StrId);
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_ProduceCond =
        CreateEvent(NULL, FALSE, FALSE, qSyncName);
    DIE(p_ConnectInfo->m_DisconnectQ.m_Metadata.m_ProduceCond == NULL,
        "Could not create disconnect queue produce event");

    snprintf(qSyncName, sizeof(qSyncName),
             "__aqua_%s_disconnect_consume_cond__", p_InstallInfo->m_StrId);
    p_ConnectInfo->m_DisconnectQ.m_Metadata.m_ConsumeCond =
        CreateEvent(NULL, FALSE, FALSE, qSyncName);
    DIE(p_ConnectInfo->m_DisconnectQ.m_Metadata.m_ConsumeCond == NULL,
        "Could not create disconnect queue consume event");
#else
#error "Platform not supported by AQUA"
#endif

    return rc;
}
