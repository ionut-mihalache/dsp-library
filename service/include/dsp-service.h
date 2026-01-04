#ifndef AQUA_DSP_SERVICE_H
#define AQUA_DSP_SERVICE_H

#include "dsp.h"

#define INSTALL_SHD "/install-shared-data"

struct InstallCommons {
    uint8_t m_IsInitialized;
};

struct InstallSharedData {
    aqua_mutex_t m_InstallMZoneMx;
    aqua_spinlock_t m_InstallMZoneLk;
};

struct ServiceCallInfo {
    struct DSPQueue m_Q;
    int32_t (*m_ReceiveCallFn)(struct CommunicationInfo *);
};

struct ServiceReturnInfo {
    struct ConnectResponseQueue m_ResponseQueue;
    struct ConnectResponseInformation m_ConnectResponseInformation;
    struct DSPQueue m_Q;

    int32_t (*m_SendReturnFn)(struct CommunicationInfo *);
};

struct ServiceConnectInfo {
    struct ConnectQueue m_ConnectQ;
    struct DisconnectQueue m_DisconnectQ;
    struct ConnectionInformation *m_Connections;
    struct ConnectionSyncInformation *m_ConnectionsSyncData;
    aqua_spinlock_ptr_t m_ConnectLock;
    // aqua_mutex_t *m_ConnectLock;
    int32_t (*m_ReceiveConnectRequest)(struct ServiceReturnInfo *,
                                       struct ServiceConnectInfo *);
    int32_t (*m_ReceiveDisconnectRequest)(struct ServiceConnectInfo *);
};

void initService(void);

AQUA_API_EXPORT void dspInstall(struct ServiceConnectInfo *p_ConnectInfo,
                                struct ServiceCallInfo *p_CallInfo,
                                const char *p_StrId, const char *p_Version,
                                int p_CallQType);

AQUA_API_EXPORT void receiveCall(void *, struct ServiceCallInfo *);
AQUA_API_EXPORT void sendReturn(struct ServiceReturnInfo *, void *);

#endif // AQUA_DSP_SERVICE_H
