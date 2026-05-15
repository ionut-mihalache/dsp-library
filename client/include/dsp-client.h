// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __DSP_CLIENT_H_
#define __DSP_CLIENT_H_

#include "aqua-sync.h"
#include "dsp.h"

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
    // pthread_spinlock_t *m_ConnectLock;
    aqua_mutex_t *m_ConnectLock;
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

void callFn(struct ClientCallInfo *p_CallInfo, void *p_CallData);

void returnFn(void *p_ReturnData, struct ClientReturnInfo *p_ReturnInfo);

int32_t setCallData(int p_Type, void *p_CallInfo, uint8_t *p_Data,
                    uint32_t p_Size);

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId);

void retriveInitInformation(struct ClientConnectInfo *p_ConnectInfo,
                            struct ClientCallInfo *p_CallInfo,
                            const char *p_ServiceStrId);

struct ConnectResponseInformation *
getConnectResponse(struct ClientReturnInfo *p_ReturnInfo);

#endif // __DSP_CLIENT_H
