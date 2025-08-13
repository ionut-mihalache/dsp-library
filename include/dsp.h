#ifndef __DSP_H_
#define __DSP_H_

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

#define SMB (1 << 16)
#define EMB (1 << 17)
#define QMB (1 << 18)
#define HMB (1 << 19)
#define MB (1 << 20)
#define DMB (1 << 21)
#define HGB (1 << 29)
#define GB (1 << 30)

#define SMB_Q_MAX_SIZE ((uint32_t)32)
#define EMB_Q_MAX_SIZE ((uint32_t)16)
#define QMB_Q_MAX_SIZE ((uint32_t)8)
#define HMB_Q_MAX_SIZE ((uint32_t)4)
#define MB_Q_MAX_SIZE ((uint32_t)2)
#define DMB_Q_MAX_SIZE ((uint32_t)1)
#define HGB_Q_MAX_SIZE ((uint32_t)1)
#define GB_Q_MAX_SIZE ((uint32_t)1)

#define CONNECTQ_MAX_SIZE ((uint32_t)128)
#define RETURNQ_MAX_SIZE ((uint32_t)1)
#define RETURN_RESPONSEQ_MAX_SIZE ((uint32_t)1)

#define CALLQ_NAME_MAX_SIZE ((uint32_t)256)
#define CONNECTQ_NAME_MAX_SIZE ((uint32_t)256)
#define RETURNQ_NAME_MAX_SIZE ((uint32_t)256)

#define OPENED_CONNECTIONS ((uint32_t)2048)

enum QType {
    SMBQ, // 0
    EMBQ, // 1
    QMBQ, // 2
    HMBQ, // 3
    MBQ,  // 4
    DMBQ, // 5
    HGBQ, // 6
    GBQ   // 7
};

struct CallMetadata {
    uint32_t m_Size;
    uint32_t m_ConnId;
    bool m_DataReady;
};

struct SMBCall {
    uint8_t m_CallInfo[SMB];
    struct CallMetadata m_Metadata;
};

struct EMBCall {
    uint8_t m_CallInfo[EMB];
    struct CallMetadata m_Metadata;
};

struct QMBCall {
    uint8_t m_CallInfo[QMB];
    struct CallMetadata m_Metadata;
};

struct HMBCall {
    uint8_t m_CallInfo[HMB];
    struct CallMetadata m_Metadata;
};

struct MBCall {
    uint8_t m_CallInfo[MB];
    struct CallMetadata m_Metadata;
};

struct DMBCall {
    uint8_t m_CallInfo[DMB];
    struct CallMetadata m_Metadata;
};

struct HGBCall {
    uint8_t m_CallInfo[HGB];
    struct CallMetadata m_Metadata;
};

struct GBCall {
    uint8_t m_CallInfo[GB];
    struct CallMetadata m_Metadata;
};

struct CommunicationInfo {
    struct DSPQueue *m_Q;
    void *m_Data;
};

struct ConnectResponseInformation {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_ReturnRequestQName[RETURNQ_NAME_MAX_SIZE];
    uint32_t m_Id;
};

struct ConnectionInformation {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_RequestResponseQName[RETURNQ_NAME_MAX_SIZE];

    pthread_cond_t m_ReturnQFullCond;
    pthread_cond_t m_ReturnQEmptyCond;
    pthread_cond_t m_RequestResponseQFullCond;
    pthread_cond_t m_RequestResponseQEmptyCond;

    pthread_mutex_t m_ReturnQMutex;
    pthread_mutex_t m_RequestResponseQMutex;

    void *m_RequestResponseQ, *m_ReturnQ;
    size_t m_RequestResponseQMapSize, m_ReturnQMapSize;

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

    enum QType m_CallQType;

    pid_t m_ProcId;
    uint8_t m_Available;
} __attribute__((aligned(PAGE_SIZE)));

struct InstallInfo {
    struct InstallInformation m_Info[SERVICES_NUMBER];
    uint8_t m_InstallMap[SERVICES_NUMBER >> 3];
    uint8_t m_BytesNr;
} __attribute__((aligned(PAGE_SIZE)));

struct DSPQueueMetadata {
    pthread_cond_t *m_FullCond;
    pthread_cond_t *m_EmptyCond;
    pthread_mutex_t *m_Lock;
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    uint32_t *m_Size;
};

struct ConnectResponseQueue {
    struct DSPQueueMetadata m_Metadata;
    struct ConnectResponseInformation *m_Data;
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
    /**
     * v0.0.2
     */
    enum QType m_ReturnQType;
};

struct ConnectQueue {
    struct DSPQueueMetadata m_Metadata;
    struct ConnectRequest *m_Data;
};

struct DisconnectQueue {
    struct DSPQueueMetadata m_Metadata;
    struct ConnectRequest *m_Data;
};

struct DSPQueue {
    struct DSPQueueMetadata m_Metadata;
    void *m_Data;
    uint32_t m_MaxSize;
    enum QType m_Type;
};

#define QPUSH(p_Queue, p_QMaxSize, p_Code)                                     \
    do {                                                                       \
        pthread_mutex_lock((p_Queue)->m_Metadata.m_Lock);                      \
        while (*(p_Queue)->m_Metadata.m_Size == (p_QMaxSize)) {                \
            pthread_cond_wait((p_Queue)->m_Metadata.m_EmptyCond,               \
                              (p_Queue)->m_Metadata.m_Lock);                   \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PushIdxPtr) =                                \
            ((*(p_Queue)->m_Metadata.m_PushIdxPtr) + 1) % (p_QMaxSize);        \
        (*(p_Queue)->m_Metadata.m_Size)++;                                     \
                                                                               \
        pthread_mutex_unlock((p_Queue)->m_Metadata.m_Lock);                    \
                                                                               \
        pthread_cond_broadcast((p_Queue)->m_Metadata.m_FullCond);              \
    } while (0)

#define QPOP(p_Queue, p_QMaxSize, p_Code)                                      \
    do {                                                                       \
        pthread_mutex_lock((p_Queue)->m_Metadata.m_Lock);                      \
        while (*(p_Queue)->m_Metadata.m_Size == 0) {                           \
            pthread_cond_wait((p_Queue)->m_Metadata.m_FullCond,                \
                              (p_Queue)->m_Metadata.m_Lock);                   \
        }                                                                      \
                                                                               \
        p_Code;                                                                \
                                                                               \
        (*(p_Queue)->m_Metadata.m_PopIdxPtr) =                                 \
            ((*(p_Queue)->m_Metadata.m_PopIdxPtr) + 1) % (p_QMaxSize);         \
        (*(p_Queue)->m_Metadata.m_Size)--;                                     \
                                                                               \
        pthread_mutex_unlock((p_Queue)->m_Metadata.m_Lock);                    \
                                                                               \
        pthread_cond_broadcast((p_Queue)->m_Metadata.m_EmptyCond);             \
    } while (0)

#endif // __DSP_H_
