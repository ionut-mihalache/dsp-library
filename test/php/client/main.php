<?php

$ffi = FFI::cdef(
    "
    enum QType {
        SMBQ, // 0
        EMBQ, // 1
        QMBQ, // 2
        HMBQ, // 3
        MBQ,  // 4
        DMBQ, // 5
        GBQ,  // 6
        DGBQ  // 7
    };

    struct CallMetadata {
        uint32_t m_Size;
        uint32_t m_ConnId;
        bool m_DataReady;
    };

    struct DSPQueueMetadata {
        void *m_FullCond;
        void *m_EmptyCond;
        void *m_Lock;
        uint32_t *m_PushIdxPtr;
        uint32_t *m_PopIdxPtr;
        uint32_t *m_Size;
    };

    struct ConnectResponseInformation {
        char m_ReturnQName[256];
        char m_ReturnRequestQName[256];
        uint32_t m_Id;
    };

    struct ConnectResponseQueue {
        struct DSPQueueMetadata m_Metadata;
        struct ConnectResponseInformation *m_Data;
        uint32_t m_MaxSize;
    };

    struct SMBCall {
        uint8_t m_CallInfo[1 << 16];
        struct CallMetadata m_Metadata;
    };

    struct EMBCall {
        uint8_t m_CallInfo[1 << 17];
        struct CallMetadata m_Metadata;
    };

    struct QMBCall {
        uint8_t m_CallInfo[1 << 18];
        struct CallMetadata m_Metadata;
    };

    struct HMBCall {
        uint8_t m_CallInfo[1 << 19];
        struct CallMetadata m_Metadata;
    };

    struct MBCall {
        uint8_t m_CallInfo[1 << 20];
        struct CallMetadata m_Metadata;
    };

    struct DMBCall {
        uint8_t m_CallInfo[1 << 21];
        struct CallMetadata m_Metadata;
    };

    struct ConnectQueue {
        struct DSPQueueMetadata m_Metadata;
        struct ConnectRequest *m_Data;
    };

    struct DisconnectQueue {
        struct DSPQueueMetadata m_Metadata;
        struct ConnectRequest *m_Data;
    };

    struct DSPQueue {
        struct DSPQueueMetadata m_Metadata;
        void *m_Data;
        uint32_t m_MaxSize;
        int m_Type;
    };

    struct ClientCallInfo {
        struct DSPQueue m_Q;

        int32_t (*m_CallFn)(struct PushInformation *);
    };

    struct ClientReturnInfo {
        struct ConnectResponseQueue m_ResponseQueue;
        struct DSPQueue m_Q;
        struct ConnectResponseInformation m_ConnectResponseInformation;

        int32_t (*m_ReturnFn)(struct PopInformation *);
        int m_QType;
    };

    struct ClientConnectRequestInformation {
        char m_ReturnQName[256];
        char m_RequestResponseQName[256];

        uint32_t m_ReturnQSize;
        uint32_t m_ResponseQSize;
        int m_QType;
    };

    struct ClientConnectInfo {
        struct ConnectQueue m_ConnectQ;
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

    void callFn(struct ClientCallInfo *p_CallInfo, void *p_CallData);

    void returnFn(void *p_ReturnData, struct ClientReturnInfo *p_ReturnInfo);

    void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId);

    int32_t setQMBCallData(struct QMBCall *p_CallInfo, uint8_t *p_Data, uint32_t p_Size);
    int32_t setHMBCallData(struct HMBCall *p_CallInfo, uint8_t *p_Data, uint32_t p_Size);

    struct ConnectResponseInformation *getConnectResponse(struct ClientReturnInfo *p_ReturnInfo);
",
    "/home/user/dsp-library/libdsp.so"
);

function dspConnect($p_Ffi, $connectInfoPtr, $callInfoPtr, $serviceStrId)
{
    $p_Ffi->dspConnect($connectInfoPtr, $callInfoPtr, $serviceStrId);
}

function sendConnectRequest($p_Ffi, $returnInfoPtr, $connectInfoPtr, $requestInfoPtr)
{
    $p_Ffi->sendConnectRequest($returnInfoPtr, $connectInfoPtr, $requestInfoPtr);
}

function setQMBCallData($p_Ffi, $callDataPtr, $dataBuffer, $size)
{
    echo "Prepare call information\n";
    return $p_Ffi->setQMBCallData($callDataPtr, $dataBuffer, $size);
}

function callFn($p_Ffi, $p_CallInfoPtr, $p_CallDataPtr)
{
    echo "Send call request\n";
    $p_Ffi->callFn($p_CallInfoPtr, $p_CallDataPtr);
}

function returnQMB($p_Ffi, $returnDataPtr, $returnInfoPtr)
{
    echo "Receive call request\n";
    $p_Ffi->returnQMB($returnDataPtr, $returnInfoPtr);
}

function returnFn($p_Ffi, $p_ReturnDataPtr, $p_ReturnInfoPtr)
{
    echo "Returned data\n";
    $p_Ffi->returnFn($p_ReturnDataPtr, $p_ReturnInfoPtr);
}

function sendDisconnectRequest($p_Ffi, $connectInfoPtr, $requestInfoPtr)
{
    echo "Send disconnect request\n";
    $p_Ffi->sendDisconnectRequest($connectInfoPtr, $requestInfoPtr);
}

$connectInfo = $ffi->new("struct ClientConnectInfo");
$connectInfoPtr = FFI::addr($connectInfo);

$callInfo = $ffi->new("struct ClientCallInfo");
$callInfoPtr = FFI::addr($callInfo);

dspConnect($ffi, $connectInfoPtr, $callInfoPtr, "xslt-transformation");

$returnInfo = $ffi->new("struct ClientReturnInfo");
$returnInfoPtr = FFI::addr($returnInfo);

$requestInfo = $ffi->new("struct ClientConnectRequestInformation");

$uniqueId = uniqid("", true);

$returnQName = "return-q-" . $uniqueId;
FFI::memset($requestInfo->m_ReturnQName, 0, 256);
FFI::memcpy($requestInfo->m_ReturnQName, $returnQName, strlen($returnQName));
$requestInfo->m_ReturnQSize = 1;

$responseQName = "response-q-" . $uniqueId;
FFI::memset($requestInfo->m_RequestResponseQName, 0, 256);
FFI::memcpy($requestInfo->m_RequestResponseQName, $responseQName, strlen($responseQName));
$requestInfo->m_ResponseQSize = 1;
$requestInfo->m_QType = 2;

$requestInfoPtr = FFI::addr($requestInfo);

sendConnectRequest($ffi, $returnInfoPtr, $connectInfoPtr, $requestInfoPtr);

$callData = $ffi->new("struct QMBCall");
$callDataPtr = FFI::addr($callData);

$callData->m_Metadata->m_ConnId = $returnInfo->m_ConnectResponseInformation->m_Id;

$iiaData = '<?xml version="1.0" encoding="UTF-8"?>

<iias-get-response xmlns="https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd" xmlns:c="https://github.com/erasmus-without-paper/ewp-specs-types-contact/tree/stable-v1" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="&#10;        https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/stable-v7/endpoints/get-response.xsd&#10;        https://raw.githubusercontent.com/erasmus-without-paper/ewp-specs-api-iias/stable-v7/endpoints/get-response.xsd&#10;    "><iia><partner><hei-id>upb.ro</hei-id><iia-id>9e01bc4f-f607-43c1-881a-8329c3e26062</iia-id></partner><partner><hei-id>brussels.uni-foundation.eu</hei-id><iia-id>9e01bc4f-f607-43c1-881a-8329c3e26063</iia-id><signing-contact><c:contact-name>Test contact</c:contact-name><c:email>test.contact@test.ro</c:email></signing-contact></partner><in-effect>false</in-effect><cooperation-conditions><student-studies-mobility-spec><sending-hei-id>brussels.uni-foundation.eu</sending-hei-id><sending-ounit-id>5</sending-ounit-id><receiving-hei-id>upb.ro</receiving-hei-id><receiving-ounit-id>ef711143-b1ab-4412-8493-3476d5ff962f</receiving-ounit-id><receiving-first-academic-year-id>2026/2027</receiving-first-academic-year-id><receiving-last-academic-year-id>2029/2030</receiving-last-academic-year-id><mobilities-per-year>2</mobilities-per-year><recommended-language-skill><language>is</language><cefr-level>B2</cefr-level><subject-area><isced-f-code>0007</isced-f-code><isced-clarification>Engineering, manufacturing and construction</isced-clarification></subject-area></recommended-language-skill><subject-area><isced-f-code>0819</isced-f-code><isced-clarification>Agriculture, not elsewhere classified</isced-clarification></subject-area><blended>false</blended></student-studies-mobility-spec></cooperation-conditions></iia></iias-get-response>';

$iiaDataBuffer = FFI::new("uint8_t[" . (strlen($iiaData)) . "]");
for ($i = 0; $i < strlen($iiaData); ++$i) {
    $iiaDataBuffer[$i] = ord($iiaData[$i]);
}

setQMBCallData($ffi, $callDataPtr, $iiaDataBuffer, strlen($iiaData));

callFn($ffi, $callInfoPtr, $callDataPtr);

$returnData = $ffi->new("struct QMBCall");
$returnDataPtr = FFI::addr($returnData);

returnFn($ffi, $returnDataPtr, $returnInfoPtr);

$result = "";
for ($i = 0; $i < $returnData->m_Metadata->m_Size; ++$i) {
    $result = $result . chr($returnData->m_CallInfo[$i]);
}

echo $result . "\n";

$requestInfoPtr = FFI::addr($returnInfo->m_ConnectResponseInformation);
sendDisconnectRequest($ffi, $connectInfoPtr, $requestInfoPtr);
