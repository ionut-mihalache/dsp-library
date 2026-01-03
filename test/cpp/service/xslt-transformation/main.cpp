#include <iostream>
#include <memory>

extern "C" {
#include "dsp-service.h"
#include "dsp.h"
}

int main() {
    std::unique_ptr<ServiceConnectInfo> connectInfo(new ServiceConnectInfo);
    std::unique_ptr<ServiceCallInfo> callInfo(new ServiceCallInfo);

    dspInstall(connectInfo.get(), callInfo.get(), "xslt-transformation",
               "v0.0.1", SMBQ);

    while (true) {
        std::unique_ptr<ServiceReturnInfo> returnInfo(new ServiceReturnInfo);

        connectInfo->m_ReceiveConnectRequest(returnInfo.get(),
                                             connectInfo.get());
        std::cout << "Connection request received.\n";

        std::unique_ptr<SMBCall> callData(new SMBCall);

        std::cout << "Waiting for call request..." << std::endl;
        receiveCall(callData.get(), callInfo.get());
        std::cout << "Call request received..." << std::endl;

        std::cout << callData->m_CallInfo << std::endl;

        std::unique_ptr<SMBCall> returnData(new SMBCall);

        int size =
            sprintf((char *)returnData->m_CallInfo, "It looks like it works!");
        returnData->m_Metadata.m_Size = size;
        returnData->m_Metadata.m_ConnId = callData->m_Metadata.m_ConnId;

        sendReturn(returnInfo.get(), returnData.get());

        connectInfo->m_ReceiveDisconnectRequest(connectInfo.get());
    }
}
