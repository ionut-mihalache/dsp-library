<?php
$ffi = FFI::cdef(
    "
    struct ConnectResponseInformation {
        char m_ReturnQName[256];
        char m_ReturnRequestQName[256];
        uint32_t m_Id;
    };

    struct ConnectResponseQueue {
        struct ConnectResponseInformation *m_Data;
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint32_t *m_Size;
        uint32_t m_MaxSize;
    };

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

    struct ConnectQueue {
        void *m_Data;
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint32_t *m_Size;
    };

    struct DisconnectQueue {
        void *m_Data;
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint32_t *m_Size;
    };

    struct DSPQueue {
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint8_t *m_Start;
    };

    struct QMBDSPQueue {
        struct QMBCall *m_Data;
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint32_t *m_Size;
        uint32_t m_MaxSize;
    };

    struct HMBDSPQueue {
        struct HMBCall *m_Data;
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
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

    struct ClientReturnInfo {
        struct ConnectResponseQueue m_ResponseQueue;
        struct QMBDSPQueue m_QMBQueue;
        struct ConnectResponseInformation m_ConnectResponseInformation;
        int32_t (*m_ReturnFnQMB)(struct QMBCall *, struct QMBDSPQueue *);
    };

    struct ClientConnectRequestInformation {
        char m_ReturnQName[256];
        char m_RequestResponseQName[256];

        uint32_t m_ReturnQSize;
        uint32_t m_ResponseQSize;
    };

    struct ClientConnectInfo {
        struct ConnectQueue m_Queue;
        struct DisconnectQueue m_DisconnectQ;
        struct ConnectionInformation *m_Connections;
        void *m_ConnectLock;
        int32_t (*m_SendConnectRequest)(struct ClientReturnInfo *,
                                        struct ClientConnectInfo *,
                                        struct ClientConnectRequestInformation *);
        int32_t (*m_SendDisconnectRequest)(struct ClientConnectInfo *);
    };

    void sendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                        struct ClientConnectInfo *p_ConnectInfo,
                        struct ClientConnectRequestInformation *p_RequestInfo);

    void sendDisconnectRequest(struct ClientConnectInfo *p_ConnectInfo, struct ConnectResponseInformation *p_requestResponseInfo);

    void callQMB(struct ClientCallInfo *p_CallInfo, struct QMBCall *p_CallData);
    void callHMB(struct ClientCallInfo *p_CallInfo, struct HMBCall *p_CallData);

    void returnQMB(struct QMBCall *p_ReturnData, struct ClientReturnInfo *p_ReturnInfo);

    void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId);

    int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data, uint32_t p_Size);
    int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data, uint32_t p_Size);

    struct ConnectResponseInformation *getConnectResponse(struct ClientReturnInfo *p_ReturnInfo);
",
    "/home/user/dsp-library/libdsp.so"
);

$connectInfo = $ffi->new("struct ClientConnectInfo");
$connectInfoPtr = FFI::addr($connectInfo);

$callInfo = $ffi->new("struct ClientCallInfo");
$callInfoPtr = FFI::addr($callInfo);

$ffi->dspConnect($connectInfoPtr, $callInfoPtr, "xslt-transformation");

$returnInfo = $ffi->new("struct ClientReturnInfo");
$returnInfoPtr = FFI::addr($returnInfo);

$requestInfo = $ffi->new("struct ClientConnectRequestInformation");

$uniqueId = uniqid();

$returnQName = "return-q-" . $uniqueId;
FFI::memset($requestInfo->m_ReturnQName, 0, 256);
FFI::memcpy($requestInfo->m_ReturnQName, $returnQName, strlen($returnQName));
$requestInfo->m_ReturnQSize = 1;

$responseQName = "response-q-" . $uniqueId;
FFI::memset($requestInfo->m_RequestResponseQName, 0, 256);
FFI::memcpy($requestInfo->m_RequestResponseQName, $responseQName, strlen($responseQName));
$requestInfo->m_ResponseQSize = 1;

$requestInfoPtr = FFI::addr($requestInfo);

$ffi->sendConnectRequest($returnInfoPtr, $connectInfoPtr, $requestInfoPtr);

$callData = $ffi->new("struct QMBCall");
$callDataPtr = FFI::addr($callData);

$data = "Hello World!";
$dataBuffer = FFI::new("uint8_t[" . (strlen($data)) . "]");
for ($i = 0; $i < strlen($data); ++$i) {
    $dataBuffer[$i] = ord($data[$i]);
}

$iiaData = '<iias-get-response xmlns="https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd" xmlns:c="https://github.com/erasmus-without-paper/ewp-specs-types-contact/tree/stable-v1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="&#10;        https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd&#10;        https://raw.githubusercontent.com/erasmus-without-paper/ewp-specs-api-iias/stable-v7/endpoints/get-response.xsd&#10;    "><iia><partner><hei-id>upb.ro</hei-id><iia-id>15476857</iia-id></partner><partner><hei-id>Not found</hei-id><iia-id>15476857</iia-id><signing-contact><c:contact-name>GÜLDEN ALBUNAR</c:contact-name><c:email>intoffice@thk.edu.tr</c:email></signing-contact></partner><in-effect>true</in-effect><cooperation-conditions><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>12</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>4</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B1</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B1</cefr-level></recommended-language-skill><subject-area><isced-f-code>0710</isced-f-code><isced-clarification>,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><total-days-per-year>12</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0413</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0714</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>6</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0716</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0714</isced-f-code></subject-area><total-days-per-year>30</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>THK UNIVERSITY</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ERASMUS+ Office Campus Bucuresti</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>3</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>15</total-days-per-year></staff-training-mobility-spec><staff-training-mobility-spec><sending-hei-id>Not found</sending-hei-id><sending-ounit-id>ERASMUS+ Office Campus Bucuresti</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>THK UNIVERSITY</receiving-ounit-id><receiving-first-academic-year-id>2023/2024</receiving-first-academic-year-id><receiving-last-academic-year-id>2028/2029</receiving-last-academic-year-id><mobilities-per-year>3</mobilities-per-year><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><recommended-language-skill><language>en</language><cefr-level>B2</cefr-level></recommended-language-skill><subject-area><isced-f-code>0714</isced-f-code><isced-clarification>,,</isced-clarification></subject-area><subject-area><isced-f-code>0413</isced-f-code></subject-area><subject-area><isced-f-code>0716</isced-f-code></subject-area><total-days-per-year>15</total-days-per-year></staff-training-mobility-spec></cooperation-conditions></iia></iias-get-response>';

$iiaDataBuffer = FFI::new("uint8_t[" . (strlen($iiaData)) . "]");
for ($i = 0; $i < strlen($iiaData); ++$i) {
    $iiaDataBuffer[$i] = ord($iiaData[$i]);
}

$ffi->setQMBCallData($callDataPtr, $iiaDataBuffer, strlen($iiaData));
$ffi->callQMB($callInfoPtr, $callDataPtr);

$returnData = $ffi->new("struct QMBCall");
$returnDataPtr = FFI::addr($returnData);

$ffi->returnQMB($returnDataPtr, $returnInfoPtr);

$result = "";
for ($i = 0; $i < $returnData->m_Size; ++$i) {
    $result = $result . chr($returnData->m_CallInfo[$i]);
}

echo $result . "\n";

$requestInfoPtr = FFI::addr($returnInfo->m_ConnectResponseInformation);
$ffi->sendDisconnectRequest($connectInfoPtr, $requestInfoPtr);
