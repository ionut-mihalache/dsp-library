#include <iostream>
#include <memory>

extern "C" {
#include "dsp-service.h"
#include "dsp.h"
}

int main() {
    std::unique_ptr<ServiceConnectInfo> connectInfo(new ServiceConnectInfo);
    std::unique_ptr<ServiceCallInfo> callInfo(new ServiceCallInfo);

    std::cout << "Installing... ";
    dspInstall(connectInfo.get(), callInfo.get(), "xslt-tranformation",
               "v0.0.1", SMBQ);
    std::cout << "Installation successful" << std::endl;

    while (true) {
        std::unique_ptr<ServiceReturnInfo> returnInfo(new ServiceReturnInfo);

        connectInfo->m_ReceiveConnectRequest(returnInfo.get(),
                                             connectInfo.get());

        std::unique_ptr<SMBCall> callData(new SMBCall);

        receiveCall(callData.get(), callInfo.get());

        std::cout << callData->m_CallInfo << std::endl;
    }

    // std::cout << "Hello World" << std::endl;
}
