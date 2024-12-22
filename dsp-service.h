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
    struct QMBDSPQueue m_QMBQueue;
    struct HMBDSPQueue m_HMBQueue;
    int32_t (*m_CallFn)(struct DSPQueue *);
    int32_t (*m_ReceiveCallFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
    int32_t (*m_ReceiveCallFnHMB)(struct HMBCall *, struct HMBDSPQueue *);
};

struct ServiceReturnInfo {
    struct ConnectResponseQueue m_ResponseQueue;
    struct QMBDSPQueue m_QMBQueue;
    int32_t (*m_SendReturnFnQMB)(struct QMBDSPQueue *, struct QMBCall *);
};

struct ServiceConnectInfo {
    struct ConnectQueue m_Queue;
    struct DisconnectQueue m_DisconnectQ;
    struct ConnectionInformation *m_Connections;
    pthread_spinlock_t *m_ConnectLock;
    int32_t (*m_ReceiveConnectRequest)(struct ServiceReturnInfo *,
                                       struct ServiceConnectInfo *);
    int32_t (*m_ReceiveDisconnectRequest)(struct ServiceConnectInfo *);
};

void initService();

void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                struct ServiceCallInfo *p_CallInfo, const char *p_StrId,
                const char *p_Version);

void dspReturn();

#endif // __DSP_SERVICE_H
