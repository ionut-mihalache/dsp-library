#ifndef __DSP_SERVICE_H
#define __DSP_SERVICE_H

#include <stdint.h>
#include <pthread.h>

#define INSTALL_SHD "/install-shared-data"
#define SERVICES_NUMBER 1024  // needs to be a power of 2
#define INSTALL_MZONE "/install-zone"

#define STRING_ID_MAX_LENGTH 128
#define VERSION_MAX_LENGTH 8

struct InstallInformation {
    char m_StrId[STRING_ID_MAX_LENGTH];
    char m_Version[VERSION_MAX_LENGTH];
    pid_t m_ProcId;
};

struct InstallCommons {
    uint8_t m_IsInitialized;
};

struct InstallSharedData {
    pthread_mutex_t m_InstallMZoneMx;
    pthread_spinlock_t m_InstallMZoneLk;
};

void initService();

int getValue();

void install();

#endif // __DSP_SERVICE_H
