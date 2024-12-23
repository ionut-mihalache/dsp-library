#ifndef __DSP_H_
#define __DSP_H_

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <unistd.h>

#define SHMEM_PATH "/shared_memory"
#define CONNECT_REQS "/conn-reqs"
#define INSTALL_MZONE "/install-zone"
#define SERVICES_NUMBER ((uint32_t)1024) // needs to be a power of 2

#define STRING_ID_MAX_LENGTH ((uint32_t)128)
#define VERSION_MAX_LENGTH ((uint32_t)16)

#define FUNC_NAME_MAX_LENGTH ((uint32_t)32)

#define QMB 1 << 18
#define HMB 1 << 19
#define MB 1 << 20
#define DMB 1 << 21
#define GB 1 << 30
#define DGB 1ULL << 31

#define QMB_Q_MAX_SIZE ((uint32_t)8)
#define HMB_Q_MAX_SIZE ((uint32_t)4)
#define MB_Q_MAX_SIZE ((uint32_t)2)
#define DMB_Q_MAX_SIZE ((uint32_t)1)
#define GB_Q_MAX_SIZE ((uint32_t)1)
#define DGB_Q_MAX_SIZE ((uint32_t)1)

#define CALLQ_MAX_SIZE ((uint32_t)1024)
#define CONNECTQ_MAX_SIZE ((uint32_t)64)
#define RETURNQ_MAX_SIZE ((uint32_t)1024)

#define CALLQ_NAME_MAX_SIZE ((uint32_t)1056)
#define CONNECTQ_NAME_MAX_SIZE ((uint32_t)1056)
#define RETURNQ_NAME_MAX_SIZE ((uint32_t)1056)

#define OPENED_CONNECTIONS ((uint32_t)2048)

struct QMBCall {
    uint8_t m_CallInfo[QMB];
    uint32_t m_Size;
    bool m_DataReady;
};

struct HMBCall {
    uint8_t m_CallInfo[HMB];
    uint32_t m_Size;
    bool m_DataReady;
};

struct MBCall {
    uint8_t m_CallInfo[MB];
    uint32_t m_Size;
    bool m_DataReady;
};

struct DMBCall {
    uint8_t m_CallInfo[DMB];
    uint32_t m_Size;
    bool m_DataReady;
};

struct HGBCall {
    uint8_t m_CallInfo[GB];
    uint32_t m_Size;
    bool m_DataReady;
};

struct GBCall {
    uint8_t m_CallInfo[DGB];
    uint32_t m_Size;
    bool m_DataReady;
};

struct ConnectResponseInformation {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_ReturnRequestQName[RETURNQ_NAME_MAX_SIZE];
    uint32_t m_Id;
};

struct ConnectionInformation {
    // char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    // char m_ReturnRequestQName[RETURNQ_NAME_MAX_SIZE];

    pthread_cond_t m_ReturnQFullCond;
    pthread_cond_t m_ReturnQEmptyCond;
    pthread_cond_t m_RequestResponseQFullCond;
    pthread_cond_t m_RequestResponseQEmptyCond;

    pthread_mutex_t m_ReturnQMutex;
    pthread_mutex_t m_RequestResponseQMutex;

    uint32_t m_ReturnQPushIdx, m_ReturnQPopIdx, m_ReturnQSize;
    uint32_t m_RequestResponseQPushIdx, m_RequestResponseQPopIdx,
        m_RequestResponseQSize;

    int32_t m_ConnectionError;
    bool m_Connected;
};

struct InstallInformation {
    struct ConnectionInformation m_Connections[OPENED_CONNECTIONS];

    char m_CallQName[CALLQ_NAME_MAX_SIZE];
    char m_ConnectQName[CONNECTQ_NAME_MAX_SIZE];
    char m_DisconnectQName[CONNECTQ_NAME_MAX_SIZE];
    char m_StrId[STRING_ID_MAX_LENGTH];
    char m_Version[VERSION_MAX_LENGTH];

    pthread_cond_t m_CallQFullCond;
    pthread_cond_t m_CallQEmptyCond;
    pthread_cond_t m_ConnectQFullCond;
    pthread_cond_t m_ConnectQEmptyCond;
    pthread_cond_t m_DisconnectQFullCond;
    pthread_cond_t m_DisconnectQEmptyCond;

    pthread_mutex_t m_CallQMutex;
    pthread_mutex_t m_ConnectQMutex;
    pthread_mutex_t m_DisconnectQMutex;
    pthread_spinlock_t m_ConnectListLock;

    uint32_t m_CallQPushIdx, m_CallQPopIdx, m_CallQSize;
    uint32_t m_ConnectQPushIdx, m_ConnectQPopIdx, m_ConnectQSize;
    uint32_t m_DisconnectQPushIdx, m_DisconnectQPopIdx, m_DisconnectQSize;

    pid_t m_ProcId;
    uint8_t m_Available;
} __attribute__((aligned(PAGE_SIZE)));

struct InstallInfo {
    struct InstallInformation m_Info[SERVICES_NUMBER >> 3];
    uint8_t m_InstallMap[SERVICES_NUMBER >> 3];
    uint8_t m_BytesNr;
} __attribute__((aligned(PAGE_SIZE)));

struct ConnectResponseQueue {
    struct ConnectResponseInformation *m_Data;
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint32_t *m_Size;
    uint32_t m_MaxSize;
};

struct ConnectRequestInformation {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_RequestResponseQName[RETURNQ_NAME_MAX_SIZE];
    struct ConnectResponseQueue m_ResponseQ;
    int32_t *m_ConnectError;
    int32_t (*m_HandleResponse)(struct ConnectResponseInformation *,
                                struct ConnectResponseInformation *);
    bool *m_Connected;
    uint32_t m_ReturnQSize;
    uint32_t m_ResponseQSize;
};

struct ConnectRequest {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_RequestResponseQName[RETURNQ_NAME_MAX_SIZE];
    uint32_t m_ReturnQSize;
    uint32_t m_ResponseQSize;
    uint32_t m_ConnectionIdx;
};

struct ConnectQueue {
    struct ConnectRequest *m_Data;
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint32_t *m_Size;
};

struct DisconnectQueue {
    struct ConnectRequest *m_Data;
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint32_t *m_Size;
};

// struct DisconnectRequest {
//     uint32_t m_ConnectionIdx;
// };

struct QMBDSPQueue {
    struct QMBCall *m_Data;
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint32_t *m_Size;
    uint32_t m_MaxSize;
};

struct HMBDSPQueue {
    struct HMBCall *m_Data;
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint32_t *m_Size;
};

struct DSPQueue {
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint8_t *m_Start;
};

#endif // __DSP_H_
