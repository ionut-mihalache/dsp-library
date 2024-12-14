<?php
$ffi = FFI::cdef("
    #define HMB 1 << 19

    #define HMB_Q_MAX_SIZE 4

    #define CALLQ_MAX_SIZE 1024
    #define RETURNQ_MAX_SIZE 1024
    #define CALLQ_NAME_MAX_SIZE 1056
    #define RETURNQ_NAME_MAX_SIZE 1056

    typedef struct { int unused; } pthread_cond_t;
    typedef struct { int unused; } pthread_mutex_t;

    struct QMBCall {
        uint8_t m_CallInfo[1 << 18];
        uint32_t m_Size;
        bool m_DataReady;
    };

    struct HMBCall {
        uint8_t m_CallInfo[1 << 19];
        uint32_t m_Size;
        bool m_DataReady;
    };

    struct DSPQueue {
        pthread_cond_t *m_FullCond;
        pthread_cond_t *m_EmptyCond;
        pthread_mutex_t *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint8_t *m_Start;
    };

    struct HMBDSPQueue {
        struct HMBCall *m_Data;
        pthread_cond_t *m_FullCond;
        pthread_cond_t *m_EmptyCond;
        pthread_mutex_t *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint32_t *m_Size;
    };

    struct ClientCallInfo {
        struct DSPQueue m_Queue;
        struct HMBDSPQueue m_HMBQueue;
        int32_t (*m_CallFn)(struct DSPQueue *);
        int32_t (*m_CallFnQMB)(struct QMBDSPQueue *, struct QMBCall *);
        int32_t (*m_CallFnHMB)(struct HMBDSPQueue *, struct HMBCall *);
    };

    void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData);
    void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData);

    void dspConnect(struct ClientCallInfo *p_CallInfo, const char* p_ServiceStrId);

    int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data, uint32_t p_Size);
    int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data, uint32_t p_Size);
",
"/home/user/dsp-library/libdsp.so"
);

$callInfo = $ffi->new("struct ClientCallInfo");
$callInfoPtr = FFI::addr($callInfo);

$ffi->dspConnect($callInfoPtr, "xslt-transformation");

$callData = $ffi->new("struct QMBCall");
$callDataPtr = FFI::addr($callData);

// $data = "Hello World!";
// $dataBuffer = FFI::new("uint8_t[" . (strlen($data)) . "]");
// for ($i = 0; $i < strlen($data); ++$i) {
//     $dataBuffer[$i] = ord($data[$i]);
// }

// $ffi->setQMBCallData($callDataPtr, $dataBuffer, strlen($data));
// $ffi->callQMB($callInfoPtr, $callDataPtr);

$iiaData = '<iias-get-response xmlns="https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd" xmlns:c="https://github.com/erasmus-without-paper/ewp-specs-types-contact/tree/stable-v1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="&#10;        https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd&#10;        https://raw.githubusercontent.com/erasmus-without-paper/ewp-specs-api-iias/stable-v7/endpoints/get-response.xsd&#10;    "><iia><partner><hei-id>upb.ro</hei-id><iia-id>15476857</iia-id></partner><partner><hei-id>Not found</hei-id><iia-id>15476857</iia-id><signing-contact><c:contact-name>GÜLDEN ALBUNAR</c:contact-name><c:email>intoffice@thk.edu.tr</c:email></signing-contact></partner><in-effect>true</in-effect><cooperation-conditions><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>12</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>4</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B1</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B1</cefr-level></recommended-language-skill><subject-area><isced-f-code>0710</isced-f-code><isced-clarification>,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><total-days-per-year>12</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0413</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0714</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0716</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0714</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>3</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>15</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>3</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>15</total-days-per-year></staff-training-mobility-spec></cooperation-conditions></iia></iias-get-response>';

$iiaDataBuffer = FFI::new("uint8_t[" . (strlen($iiaData)) . "]");
for ($i = 0; $i < strlen($iiaData); ++$i) {
    $iiaDataBuffer[$i] = ord($iiaData[$i]);
}

$ffi->setQMBCallData($callDataPtr, $iiaDataBuffer, strlen($iiaData));
$ffi->callQMB($callInfoPtr, $callDataPtr);
