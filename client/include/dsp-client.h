#ifndef AQUA_DSP_CLIENT_H_
#define AQUA_DSP_CLIENT_H_

#include "dsp.h"
#include "locking.h"

struct ClientCallInfo {
    struct DSPQueue m_Q;

    int32_t (*m_CallFn)(struct CommunicationInfo *);
};

struct ClientReturnInfo {
    struct ConnectResponseQueue m_ResponseQueue;
    struct DSPQueue m_Q;
    struct ConnectResponseInformation m_ConnectResponseInformation;

    int32_t (*m_ReturnFn)(struct CommunicationInfo *);

    enum QType m_QType;
};

struct ClientConnectRequestInformation {
    char m_ReturnQName[RETURNQ_NAME_MAX_SIZE];
    char m_RequestResponseQName[RETURNQ_NAME_MAX_SIZE];

    uint32_t m_ReturnQSize;
    uint32_t m_ResponseQSize;
    enum QType m_QType;
};

struct ClientConnectInfo {
    struct ConnectQueue m_ConnectQ;
    struct DisconnectQueue m_DisconnectQ;
    struct ConnectionInformation *m_Connections;
    aqua_spinlock_t *m_ConnectLock;
    int32_t (*m_SendConnectRequest)(struct ClientReturnInfo *,
                                    struct ClientConnectInfo *,
                                    struct ClientConnectRequestInformation *);
    int32_t (*m_SendDisconnectRequest)(struct ClientConnectInfo *,
                                       struct ConnectResponseInformation *);
};

AQUA_API_EXPORT void
sendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                   struct ClientConnectInfo *p_ConnectInfo,
                   struct ClientConnectRequestInformation *p_RequestInfo);
AQUA_API_EXPORT void
sendDisconnectRequest(struct ClientConnectInfo *p_ConnectInfo,
                      struct ConnectResponseInformation *p_requestResponseInfo);

AQUA_API_EXPORT void callFn(struct ClientCallInfo *p_CallInfo,
                            void *p_CallData);

AQUA_API_EXPORT void returnFn(void *p_ReturnData,
                              struct ClientReturnInfo *p_ReturnInfo);

AQUA_API_EXPORT int32_t setCallData(int p_Type, void *p_CallInfo,
                                    uint8_t *p_Data, uint32_t p_Size);

AQUA_API_EXPORT void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                                struct ClientCallInfo *p_CallInfo,
                                const char *p_ServiceStrId);

void retriveInitInformation(struct ClientConnectInfo *p_ConnectInfo,
                            struct ClientCallInfo *p_CallInfo,
                            const char *p_ServiceStrId);

struct ConnectResponseInformation *
getConnectResponse(struct ClientReturnInfo *p_ReturnInfo);

#endif // AQUA_DSP_CLIENT_H_
