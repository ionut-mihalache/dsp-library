#ifndef __DSP_H_
#define __DSP_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/user.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define SHMEM_PATH "/shared_memory"
#define CONNECT_REQS "/conn-reqs"
#define INSTALL_MZONE "/install-zone"
#define SERVICES_NUMBER 1024  // needs to be a power of 2

#define STRING_ID_MAX_LENGTH 128
#define VERSION_MAX_LENGTH 16

#define FUNC_NAME_MAX_LENGTH 32

#define QMB 1 << 18
#define HMB 1 << 19
#define MB 1 << 20
#define DMB 1 << 21
#define GB 1 << 30
#define DGB 1LL << 31

struct InstallInformation {
    char m_StrId[STRING_ID_MAX_LENGTH];
    char m_Version[VERSION_MAX_LENGTH];
    pid_t m_ProcId;
    uint8_t m_Available;
};

struct QMBCall {
    char m_CallInfo[QMB];
};

struct HMBCall {
    char m_CallInfo[HMB];
};

struct MBCall {
    char m_CallInfo[MB];
};

struct DMBCall {
    char m_CallInfo[DMB];
};

struct HGBCall {
    char m_CallInfo[GB];
};

struct GBCall {
    char m_CallInfo[DGB];
};

struct DSPCall {

};

struct DSPReturn {};

void hello();

#endif // __DSP_H_
