#ifndef __DSP_CLIENT_H_
#define __DSP_CLIENT_H

struct ClientCallInfo {
    struct DSPQueue m_Queue;
    struct QMBDSPQueue m_QMBQueue;
    struct HMBDSPQueue m_HMBQueue;
    int32_t (*m_CallFn)(struct DSPQueue *);
    int32_t (*m_CallFnQMB)(struct QMBDSPQueue *, struct QMBCall *);
    int32_t (*m_CallFnHMB)(struct HMBDSPQueue *, struct HMBCall *);
    int32_t (*m_ReturnFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
};

void pushQ(struct ClientCallInfo *callInfo);
void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData);
void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData);

int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size);
int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data,
                       uint32_t p_Size);

void dspConnect(struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId);

#endif // __DSP_CLIENT_H
