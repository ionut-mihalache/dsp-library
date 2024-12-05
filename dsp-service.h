#ifndef __DSP_SERVICE_H
#define __DSP_SERVICE_H

#include "dsp.h"

#include <pthread.h>
#include <stdint.h>

#define INSTALL_SHD "/install-shared-data"

struct InstallCommons {
    uint8_t m_IsInitialized;
};

struct InstallSharedData {
    pthread_mutex_t m_InstallMZoneMx;
    pthread_spinlock_t m_InstallMZoneLk;
};

struct ServiceCallInfo {
    struct DSPQueue m_Queue;
    int32_t (*m_CallFn)(struct DSPQueue *);
};

void initService();

int getValue();

void dspInstall(struct ServiceCallInfo *p_CallInfo, const char *p_StrId, const char *p_Version);

void dspReturn();

#endif // __DSP_SERVICE_H
