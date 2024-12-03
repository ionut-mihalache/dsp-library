#ifndef __DSP_CLIENT_H_
#define __DSP_CLIENT_H

struct ClientCallInfo {
    struct DSPQueue p_Queue;
    int32_t (*m_CallFn)(char *);
};

void increment();
void dspConnect(struct ClientCallInfo *p_CallInfo, const char* p_ServiceStrId);

#endif // __DSP_CLIENT_H
