#ifndef __DSP_SERVICE_H
#define __DSP_SERVICE_H

#include "dsp.h"

#define INSTALL_SHD "/install-shared-data"

struct InstallCommons {
    uint8_t m_IsInitialized;
};

struct InstallSharedData {
    pthread_mutex_t m_InstallMZoneMx;
    pthread_spinlock_t m_InstallMZoneLk;
};

struct ServiceCallInfo {
    struct HMBDSPQueue m_HMBQueue;
    struct QMBDSPQueue m_QMBQueue;
    /**
     * v0.0.2
     */
    struct DSPQueue m_Q;

    int32_t (*m_ReceiveCallFnHMB)(struct HMBCall *, struct HMBDSPQueue *);
    int32_t (*m_ReceiveCallFnQMB)(struct QMBCall *, struct QMBDSPQueue *);

    /**
     * v0.0.2
     */
    int32_t (*m_ReceiveCallFn)(struct PopInformation *);
};

struct ServiceReturnInfo {
    struct ConnectResponseQueue m_ResponseQueue;
    struct QMBDSPQueue m_QMBQueue;
    struct ConnectResponseInformation m_ConnectResponseInformation;

    /**
     * v0.0.2
     */
    struct DSPQueue m_Q;

    int32_t (*m_SendReturnFnQMB)(struct DSPQueue *, struct QMBCall *);
    int32_t (*m_SendReturnFnHMB)(struct DSPQueue *, struct HMBCall *);

    /**
     * v0.0.2
     */
    int32_t (*m_SendReturnFn)(struct PushInformation *);
};

struct ServiceConnectInfo {
    struct ConnectQueue m_ConnectQ;
    struct DisconnectQueue m_DisconnectQ;
    struct ConnectionInformation *m_Connections;
    pthread_spinlock_t *m_ConnectLock;
    // pthread_mutex_t *m_ConnectLock;
    int32_t (*m_ReceiveConnectRequest)(struct ServiceReturnInfo *,
                                       struct ServiceConnectInfo *);
    int32_t (*m_ReceiveDisconnectRequest)(struct ServiceConnectInfo *);
};

void initService();

void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                struct ServiceCallInfo *p_CallInfo, const char *p_StrId,
                const char *p_Version, int p_CallQType);

void receiveCall(void *, struct ServiceCallInfo *);
void sendReturn(struct ServiceReturnInfo *, void *);

#endif // __DSP_SERVICE_H
