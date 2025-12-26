#include <chrono>
#include <ratio>
#include <stdio.h>
#include <cstdint>
#include <cstring>
#include <memory>

extern "C" {
#include "dsp.h"
#include "dsp-client.h"
}

int main() {
    std::unique_ptr<ClientConnectInfo> connectInfo(new ClientConnectInfo);
    if (connectInfo == nullptr) {
        fprintf(stderr, "There was an error with allocating memory for client "
                        "information.\n");
        return 0;
    }

    std::unique_ptr<ClientCallInfo> callInfo(new ClientCallInfo);
    if (callInfo == nullptr) {
        fprintf(stderr, "There was an error with allocating memory for client "
                        "call information.\n");
        return 0;
    }

    dspConnect(connectInfo.get(), callInfo.get(), "xslt-transformation");

    std::unique_ptr<ClientReturnInfo> returnInfo(new ClientReturnInfo);
    if (returnInfo == nullptr) {
        fprintf(stderr, "There was an error with allocating memory for client"
                        "return information.\n");
        return 0;
    }

    std::unique_ptr<ClientConnectRequestInformation> requestInfo(
        new ClientConnectRequestInformation);
    if (requestInfo == nullptr) {
        fprintf(stderr, "There was an error with allocating memory for client"
                        "call information.\n");
        return 0;
    }

    sprintf(requestInfo->m_ReturnQName, "%s", "return-q-5656565656");
    requestInfo->m_ReturnQSize = 1;

    sprintf(requestInfo->m_RequestResponseQName, "%s", "response-q-5656565656");
    requestInfo->m_ResponseQSize = 1;
    requestInfo->m_QType = SMBQ;

    sendConnectRequest(returnInfo.get(), connectInfo.get(), requestInfo.get());

    std::unique_ptr<SMBCall> callData(new SMBCall);
    if (callData == nullptr) {
        fprintf(stderr, "There was an error with allocating memory for client"
                        "call data.\n");
        return 0;
    }

    callData->m_Metadata.m_ConnId =
        returnInfo->m_ConnectResponseInformation.m_Id;

    auto iiaData =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n<iias-get-response "
        "xmlns=\"https://github.com/erasmus-without-paper/ewp-specs-api-iias/"
        "blob/stable-v7/endpoints/get-response.xsd\" "
        "xmlns:c=\"https://github.com/erasmus-without-paper/"
        "ewp-specs-types-contact/tree/stable-v1\" "
        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
        "xsi:schemaLocation=\"&#10;        "
        "https://github.com/erasmus-without-paper/ewp-specs-api-iias/blob/"
        "stable-v7/endpoints/get-response.xsd&#10;        "
        "https://raw.githubusercontent.com/erasmus-without-paper/"
        "ewp-specs-api-iias/stable-v7/endpoints/get-response.xsd&#10;    "
        "\"><iia><partner><hei-id>upb.ro</"
        "hei-id><iia-id>9e01bc4f-f607-43c1-881a-8329c3e26062</iia-id></"
        "partner><partner><hei-id>brussels.uni-foundation.eu</"
        "hei-id><iia-id>9e01bc4f-f607-43c1-881a-8329c3e26063</"
        "iia-id><signing-contact><c:contact-name>Test "
        "contact</c:contact-name><c:email>test.contact@test.ro</c:email></"
        "signing-contact></partner><in-effect>false</"
        "in-effect><cooperation-conditions><student-studies-mobility-spec><"
        "sending-hei-id>brussels.uni-foundation.eu</"
        "sending-hei-id><sending-ounit-id>5</"
        "sending-ounit-id><receiving-hei-id>upb.ro</"
        "receiving-hei-id><receiving-ounit-id>ef711143-b1ab-4412-8493-"
        "3476d5ff962f</"
        "receiving-ounit-id><receiving-first-academic-year-id>2026/2027</"
        "receiving-first-academic-year-id><receiving-last-academic-year-id>"
        "2029/2030</receiving-last-academic-year-id><mobilities-per-year>2</"
        "mobilities-per-year><recommended-language-skill><language>is</"
        "language><cefr-level>B2</cefr-level><subject-area><isced-f-code>0007</"
        "isced-f-code><isced-clarification>Engineering, manufacturing and "
        "construction</isced-clarification></subject-area></"
        "recommended-language-skill><subject-area><isced-f-code>0819</"
        "isced-f-code><isced-clarification>Agriculture, not elsewhere "
        "classified</isced-clarification></subject-area><blended>false</"
        "blended></student-studies-mobility-spec></cooperation-conditions></"
        "iia></iias-get-response>";

    setCallData(SMBQ, callData.get(), (uint8_t *)iiaData, strlen(iiaData));

    auto start = std::chrono::high_resolution_clock::now();
    callFn(callInfo.get(), callData.get());
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::milli> duration = end - start;

    fprintf(stdout, "Call duration: %lf\n", duration.count());

    std::unique_ptr<SMBCall> returnData(new SMBCall);
    if (returnData == nullptr) {
        fprintf(stderr, "There was an error with allocating memory for client"
                        "call data.\n");
        return 0;
    }

    start = std::chrono::high_resolution_clock::now();
    returnFn(returnData.get(), returnInfo.get());
    end = std::chrono::high_resolution_clock::now();

    duration = end - start;

    fprintf(stdout, "Return duration: %lf\n", duration.count());

    sendDisconnectRequest(connectInfo.get(),
                          &(returnInfo->m_ConnectResponseInformation));

    fprintf(stdout, "%s\n", returnData->m_CallInfo);
    return 0;
}
