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
    int32_t (*m_ReturnFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
};

struct ClientConnectInfo {
    struct ConnectQueue m_Queue;
    struct ConnectionInformation *m_Connections;
    int32_t (*m_SendConnectRequest)(struct ClientReturnInfo *,
                                    struct ClientConnectInfo *,
                                    struct ConnectRequestInformation *);
};

void sendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                        struct ClientConnectInfo *p_ConnectInfo,
                        struct ConnectRequestInformation *p_RequestInfo);
void pushQ(struct ClientCallInfo *p_CallInfo);
void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData);
void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData);

int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size);
int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size);

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId);

void retriveInitInformation(struct ClientCallInfo *p_ConnectInfo,
                            struct ClientCallInfo *p_CallInfo,
                            const char *p_ServiceStrId);

#endif // __DSP_CLIENT_H
