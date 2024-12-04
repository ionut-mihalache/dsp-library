<?php
    $ffi = FFI::cdef("
        struct DSPQueue {
            uint32_t *m_PushIdxPtr;
            uint32_t *m_PopIdxPtr;
            char *m_Start;
        };

        struct ClientCallInfo {
            struct DSPQueue m_Queue;
            int32_t (*m_CallFn)(char *);
        };

        void pushQ(struct ClientCallInfo *callInfo);
        void dspConnect(struct ClientCallInfo *p_CallInfo, const char* p_ServiceStrId);
    ",
    "/home/user/dsp-library/libdsp.so");

    $callInfo = $ffi->new("struct ClientCallInfo");
    $callInfoPtr = FFI::addr($callInfo);

    $ffi->dspConnect($callInfoPtr, "xslt-transformation");

    // $callInfo->m_CallFn($callInfo->m_Queue);
    $ffi->pushQ($callInfoPtr);

    echo $callInfo->m_Queue->m_PushIdxPtr[0] . "\n";
    echo $callInfo->m_Queue->m_PopIdxPtr[0] . "\n";

    // $callInfo->m_Queue->m_PushIdxPtr[0]++;
    // $callInfo->m_Queue->m_PopIdxPtr[0]++;
