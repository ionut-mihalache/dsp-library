#include <iostream>
#include <memory>
#include <string>
#include <cstdio>

extern "C" {
#include "dsp-service.h"
#include "dsp.h"
}

#include "SaxonCHE/include/saxonc/SaxonProcessor.h"
#include "SaxonCHE/include/saxonc/Xslt30Processor.h"

string mf_GetXmlTransformed(
        const string& xmlBytes,
        const string& xsltBytes) {

    SaxonProcessor saxon(false); // false = HE (fără licență)
    XsltProcessor* xslt = saxon.newXsltProcessor();

    XdmNode* xmlNode = saxon.parseXmlFromString(xmlBytes.c_str());
    xslt->compileFromString(xsltBytes.c_str());
    xslt->setSource(xmlNode);

    const char* result = xslt->transformToString();

    string output = result ? result : "";

    delete xslt;
    delete xmlNode;

    return output;
}

int main() {
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    printf("Session ID: %lu\n", sessionId);

    std::unique_ptr<ServiceConnectInfo> connectInfo(new ServiceConnectInfo);
    std::unique_ptr<ServiceCallInfo> callInfo(new ServiceCallInfo);

    std::cout << "Installing... ";
    dspInstall(connectInfo.get(), callInfo.get(), "xslt-transformation",
               "v0.0.1", SMBQ);
    std::cout << "Installation successful" << std::endl;

    while (true) {
        std::unique_ptr<ServiceReturnInfo> returnInfo(new ServiceReturnInfo);

        std::cout << "Waiting for connection request...";
        connectInfo->m_ReceiveConnectRequest(returnInfo.get(),
                                             connectInfo.get());
        std::cout << "Connection request received.\n";

        std::unique_ptr<SMBCall> callData(new SMBCall);

        receiveCall(callData.get(), callInfo.get());

        std::cout << callData->m_CallInfo << std::endl;

        std::unique_ptr<SMBCall> returnData(new SMBCall);

        int size = sprintf((char *)returnData->m_CallInfo, "It looks like it works!");
        returnData->m_Metadata.m_Size = size;
        returnData->m_Metadata.m_ConnId = callData->m_Metadata.m_ConnId;

        sendReturn(returnInfo.get(), returnData.get());
    }

    // std::cout << "Hello World" << std::endl;
}

// #define WIN32_LEAN_AND_MEAN
// #include <windows.h>
// #include <stdio.h>

// int main() {
//     const char* name = "Local\\region-name";
//     DWORD size = 4096;

//     HANDLE hMap = CreateFileMappingA(
//         INVALID_HANDLE_VALUE,
//         NULL,
//         PAGE_READWRITE,
//         0,
//         size,
//         name
//     );

//     if (!hMap) {
//         printf("CreateFileMapping failed: %lu\n", GetLastError());
//         return 1;
//     }

//     printf("CreateFileMapping OK\n");

//     void* ptr = MapViewOfFile(
//         hMap,
//         FILE_MAP_READ | FILE_MAP_WRITE,
//         0, 0,
//         size
//     );

//     if (!ptr) {
//         printf("MapViewOfFile failed: %lu\n", GetLastError());
//         return 1;
//     }

//     printf("MapViewOfFile OK\n");

//     strcpy((char*)ptr, "test");
//     printf("Shared memory says: %s\n", (char*)ptr);

//     UnmapViewOfFile(ptr);
//     CloseHandle(hMap);
//     return 0;
// }

// #include <windows.h>
// #include <stdio.h>

// int main() {
//     // Requested size: 1.22 GB
//     UINT64 requestedSize = 1312821248;

//     // Get system allocation granularity
//     SYSTEM_INFO si;
//     GetSystemInfo(&si);
//     SIZE_T gran = si.dwAllocationGranularity;

//     // Round up size to allocation granularity
//     SIZE_T mapSize = (requestedSize + gran - 1) & ~(gran - 1);

//     printf("Requested size: %llu bytes\n", requestedSize);
//     printf("Aligned size:   %llu bytes\n", mapSize);
//     printf("Allocation granularity: %lu bytes\n", (unsigned long)gran);

//     // Split 64-bit size into high and low DWORDs
//     DWORD dwLow  = (DWORD)(mapSize & 0xFFFFFFFF);
//     DWORD dwHigh = (DWORD)(mapSize >> 32);

//     // Create file mapping (backed by page file)
//     HANDLE hMap = CreateFileMapping(
//         INVALID_HANDLE_VALUE,
//         NULL,               // default security
//         PAGE_READWRITE,
//         dwHigh,
//         dwLow,
//         "Local\\install-zone"
//     );

//     if (!hMap) {
//         fprintf(stderr, "CreateFileMapping failed: %lu\n", GetLastError());
//         return 1;
//     }

//     // Map view
//     LPVOID ptr = MapViewOfFile(
//         hMap,
//         FILE_MAP_ALL_ACCESS,
//         0, 0,
//         mapSize
//     );

//     if (!ptr) {
//         fprintf(stderr, "MapViewOfFile failed: %lu\n", GetLastError());
//         CloseHandle(hMap);
//         return 1;
//     }

//     printf("Successfully mapped %llu bytes at %p\n", mapSize, ptr);

//     // Example usage
//     ((char*)ptr)[0] = 42;
//     ((char*)ptr)[requestedSize-1] = 99;

//     // Clean up
//     UnmapViewOfFile(ptr);
//     CloseHandle(hMap);

//     return 0;
// }
