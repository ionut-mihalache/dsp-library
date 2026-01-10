#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <thread>

extern "C" {
#include "dsp-service.h"
#include "dsp.h"
}

template <typename K, typename V> class ConcurrentHashMap {
  private:
    std::unordered_map<K, V> map;
    mutable std::mutex mtx;

  public:
    void insert(const K &key, const V &value) {
        std::lock_guard<std::mutex> lock(mtx);
        map[key] = value;
    }

    bool get(const K &key, V &value) const {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = map.find(key);
        if (it == map.end())
            return false;
        value = it->second;
        return true;
    }

    void erase(const K &key) {
        std::lock_guard<std::mutex> lock(mtx);
        map.erase(key);
    }
};

static std::shared_ptr<ServiceConnectInfo> connectInfo;
static std::shared_ptr<ServiceCallInfo> callInfo;
static ConcurrentHashMap<int, std::shared_ptr<ServiceReturnInfo>> connections;

int main() {
    connectInfo = std::make_shared<ServiceConnectInfo>();
    callInfo = std::make_shared<ServiceCallInfo>();

    dspInstall(connectInfo.get(), callInfo.get(), "xslt-transformation",
               "v0.0.1", SMBQ);

    auto handleConnect = [&]() {
        while (true) {
            std::shared_ptr<ServiceReturnInfo> returnInfo =
                std::make_shared<ServiceReturnInfo>();

            connectInfo->m_ReceiveConnectRequest(returnInfo.get(),
                                                 connectInfo.get());
            std::cout << "Connection request received.\n";

            uint32_t connId = returnInfo->m_ConnectResponseInformation.m_Id;

            connections.insert(connId, returnInfo);
        }
    };

    auto handleDisconnect = [&]() {
        while (true) {
            connectInfo->m_ReceiveDisconnectRequest(connectInfo.get());
        }
    };

    auto handleCall = [&](std::unique_ptr<SMBCall> callData) {
        // std::cout << callData->m_CallInfo << std::endl;

        std::unique_ptr<SMBCall> returnData(new SMBCall);

        int size = snprintf(
            (char *)returnData->m_CallInfo, sizeof(returnData->m_CallInfo),
            "It looks like it works! - %u", callData->m_Metadata.m_ConnId);
        returnData->m_Metadata.m_Size = size;
        returnData->m_Metadata.m_ConnId = callData->m_Metadata.m_ConnId;

        std::shared_ptr<ServiceReturnInfo> returnInfo;
        if (!connections.get(callData->m_Metadata.m_ConnId, returnInfo)) {
            std::cout << "Could not get connection "
                      << callData->m_Metadata.m_ConnId << std::endl;
        }
        sendReturn(returnInfo.get(), returnData.get());
        std::cout << "Return sent to client" << std::endl;
    };

    std::thread connectThread(handleConnect);
    std::thread disconnectThread(handleDisconnect);

    while (true) {
        std::unique_ptr<SMBCall> callData(new SMBCall);

        std::cout << "Waiting for call request..." << std::endl;
        receiveCall(callData.get(), callInfo.get());
        std::cout << "Call request received..." << std::endl;

        // handleCall(std::move(callData));

        std::thread callThread(handleCall, std::move(callData));
        callThread.detach();
    }

    connectThread.join();
    disconnectThread.join();
}
