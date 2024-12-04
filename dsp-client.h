#ifndef __DSP_CLIENT_H_
#define __DSP_CLIENT_H

struct ClientCallInfo {
    struct DSPQueue m_Queue;
    int32_t (*m_CallFn)(struct DSPQueue *);
};

void pushQ(struct ClientCallInfo *callInfo);

void increment();
void dspConnect(struct ClientCallInfo *p_CallInfo, const char* p_ServiceStrId);

#endif // __DSP_CLIENT_H
