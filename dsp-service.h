#ifndef __DSP_SERVICE_H
#define __DSP_SERVICE_H

#include <stdint.h>
#include <pthread.h>

#define INSTALL_SHD "/install-shared-data"

struct InstallCommons {
    uint8_t m_IsInitialized;
};

struct InstallSharedData {
    pthread_mutex_t m_InstallMZoneMx;
    pthread_spinlock_t m_InstallMZoneLk;
};

void initService();

int getValue();

void dspInstall(const char *p_StrId, const char *p_Version);

void dspReturn();

#endif // __DSP_SERVICE_H
