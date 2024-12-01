#ifndef __DSP_CLIENT_H_
#define __DSP_CLIENT_H

struct ClientCallInfo {
    int (*m_CallFn)(char *);
};

void increment();
void dspConnect(const char* p_ServiceStrId);

#endif // __DSP_CLIENT_H
