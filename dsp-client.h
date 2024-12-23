#ifndef __DSP_CLIENT_H_
#define __DSP_CLIENT_H

struct ClientCallInfo {
    struct DSPQueue m_Queue;
    struct QMBDSPQueue m_QMBQueue;
    struct HMBDSPQueue m_HMBQueue;
    int32_t (*m_CallFn)(struct DSPQueue *);
    int32_t (*m_CallFnQMB)(struct QMBDSPQueue *, struct QMBCall *);
    int32_t (*m_CallFnHMB)(struct HMBDSPQueue *, struct HMBCall *);
};

struct ClientReturnInfo {
    struct ConnectResponseQueue m_ResponseQueue;
    struct QMBDSPQueue m_QMBQueue;
    struct ConnectResponseInformation m_ConnectResponseInformation;
    int32_t (*m_ReturnFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
};

struct ClientConnectRequestInformation {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_RequestResponseQName[RETURNQ_NAME_MAX_SIZE];

    uint32_t m_ReturnQSize;
    uint32_t m_ResponseQSize;
};

struct ClientConnectInfo {
    struct ConnectQueue m_Queue;
    struct DisconnectQueue m_DisconnectQ;
    struct ConnectionInformation *m_Connections;
    pthread_spinlock_t *m_ConnectLock;
    int32_t (*m_SendConnectRequest)(struct ClientReturnInfo *,
                                    struct ClientConnectInfo *,
                                    struct ClientConnectRequestInformation *);
    int32_t (*m_SendDisconnectRequest)(struct ClientConnectInfo *,
                                       struct ConnectResponseInformation *);
};

void sendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                        struct ClientConnectInfo *p_ConnectInfo,
                        struct ClientConnectRequestInformation *p_RequestInfo);
void sendDisconnectRequest(
    struct ClientConnectInfo *p_ConnectInfo,
    struct ConnectResponseInformation *p_requestResponseInfo);

void pushQ(struct ClientCallInfo *p_CallInfo);
void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData);
void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData);

int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size);
int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size);

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId);

void retriveInitInformation(struct ClientConnectInfo *p_ConnectInfo,
                            struct ClientCallInfo *p_CallInfo,
                            const char *p_ServiceStrId);

struct ConnectResponseInformation *
getConnectResponse(struct ClientReturnInfo *p_ReturnInfo);

#endif // __DSP_CLIENT_H
