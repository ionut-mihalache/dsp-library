#ifndef __DSP_H_
#define __DSP_H_

#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/user.h>
#include <unistd.h>

#define SHMEM_PATH "/shared_memory"
#define CONNECT_REQS "/conn-reqs"
#define INSTALL_MZONE "/install-zone"
#define SERVICES_NUMBER 1024 // needs to be a power of 2

#define STRING_ID_MAX_LENGTH 128
#define VERSION_MAX_LENGTH 16

#define FUNC_NAME_MAX_LENGTH 32

#define QMB 1 << 18
#define HMB 1 << 19
#define MB 1 << 20
#define DMB 1 << 21
#define GB 1 << 30
#define DGB 1LL << 31

#define CALLQ_MAX_SIZE 1024
#define RETURNQ_MAX_SIZE 1024
#define CALLQ_NAME_MAX_SIZE 1056
#define RETURNQ_NAME_MAX_SIZE 1056

struct InstallInformation {
    char m_StrId[STRING_ID_MAX_LENGTH];
    char m_Version[VERSION_MAX_LENGTH];
    char m_CallQName[CALLQ_NAME_MAX_SIZE];
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    pthread_mutex_t m_CallQMutex;
    pthread_mutex_t m_ReturnQMutex;
    uint32_t m_CallQPushIdx, m_CallQPopIdx;
    uint32_t m_ReturnQPushIdx, m_ReturnQPopIdx;
    pid_t m_ProcId;
    uint8_t m_Available;
};

struct QMBCall {
    char m_CallInfo[QMB];
    uint32_t m_Size;
};

struct HMBCall {
    char m_CallInfo[HMB];
    uint32_t m_Size;
};

struct MBCall {
    char m_CallInfo[MB];
    uint32_t m_Size;
};

struct DMBCall {
    char m_CallInfo[DMB];
    uint32_t m_Size;
};

struct HGBCall {
    char m_CallInfo[GB];
    uint32_t m_Size;
};

struct GBCall {
    char m_CallInfo[DGB];
    uint32_t m_Size;
};

struct DSPCall {};

struct DSPReturn {};

struct DSPQueue {
    uint32_t *m_PushIdxPtr;
    uint32_t *m_PopIdxPtr;
    char *m_Start;
};

void hello();

#endif // __DSP_H_
