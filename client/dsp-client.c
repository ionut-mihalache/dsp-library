#include <stdbool.h>
#include <string.h>

#include "dsp.h"

#include "client-call.h"
#include "client-connect.h"
#include "commons.h"
#include "dsp-client.h"
#include "log.h"
#include "macros.h"
#include "system-values.h"

void sendConnectRequest(struct ClientReturnInfo *p_ReturnInfo,
                        struct ClientConnectInfo *p_ConnectInfo,
                        struct ClientConnectRequestInformation *p_RequestInfo) {
    p_ConnectInfo->m_SendConnectRequest(p_ReturnInfo, p_ConnectInfo,
                                        p_RequestInfo);
}

void sendDisconnectRequest(
    struct ClientConnectInfo *p_ConnectInfo,
    struct ConnectResponseInformation *p_requestResponseInfo) {
    p_ConnectInfo->m_SendDisconnectRequest(p_ConnectInfo,
                                           p_requestResponseInfo);
}

void callFn(struct ClientCallInfo *p_CallInfo, void *p_CallData) {
    struct CommunicationInfo cInfo;

    cInfo.m_Q = &(p_CallInfo->m_Q);
    cInfo.m_Data = p_CallData;

    p_CallInfo->m_CallFn(&cInfo);
};

void returnFn(void *p_ReturnData, struct ClientReturnInfo *p_ReturnInfo) {
    struct CommunicationInfo cInfo;

    cInfo.m_Q = &(p_ReturnInfo->m_Q);
    cInfo.m_Data = p_ReturnData;

    p_ReturnInfo->m_ReturnFn(&cInfo);
}

int32_t setCallData(int p_Type, void *p_CallInfo, uint8_t *p_Data,
                    uint32_t p_Size) {
    int32_t rc = 0;

    switch (p_Type) {
    case SMBQ: {
        struct SMBCall *callInfo = (struct SMBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case EMBQ: {
        struct EMBCall *callInfo = (struct EMBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case QMBQ: {
        struct QMBCall *callInfo = (struct QMBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case HMBQ: {
        struct HMBCall *callInfo = (struct HMBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case MBQ: {
        struct MBCall *callInfo = (struct MBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case DMBQ: {
        struct DMBCall *callInfo = (struct DMBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case HGBQ: {
        struct HGBCall *callInfo = (struct HGBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case GBQ: {
        struct GBCall *callInfo = (struct GBCall *)p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    default:
        DIE(true, "Type not recognized");
    }

    return rc;
}

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId) {
    int rc;
    aqua_file_handle installShmHandle;
    struct InstallInformation *installInfo;
    uint8_t connected = false;
    uint16_t i;
    struct InstallInfo *installMemZone;

    installShmHandle =
        createShmObject(INSTALL_MZONE "123", O_RDWR,
                        AQUA_S_IRUSR | AQUA_S_IWUSR | AQUA_S_IRGRP |
                            AQUA_S_IWGRP | AQUA_S_IROTH | AQUA_S_IWOTH,
                        sizeof(struct InstallInfo), false);

    createQSimple((aqua_void_t **)&installMemZone, sizeof(struct InstallInfo),
                  AQUA_PROT_READ | AQUA_PROT_WRITE, installShmHandle);

    for (i = 0; i < SERVICES_NUMBER; ++i) {
        if (!installMemZone->m_Info[i].m_Available) {
            continue;
        }

        installInfo = (struct InstallInformation *)&(installMemZone->m_Info[i]);

        LOGF("Current service name: %s\n", installInfo->m_StrId);
        if (!strcmp(installInfo->m_StrId, p_ServiceStrId)) {
            if (installInfo->m_Available) {
                connected = true;
            }
            break;
        }
    }

    if (!connected) {
        ELOGF("Could not connect. Service is not installed or unavailable.\n");
        return;
    }

#if defined(__linux__)
    rc = munmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");
#elif defined(_WIN32)
    DIE(!UnmapViewOfFile(installMemZone),
        "Could not unmap install memory zone");
#else
#endif

    /**
     * Map only the information of the service
     */
#if defined(__linux__)
    installInfo = (struct InstallInformation *)mmap(
        NULL, sizeof(struct InstallInformation), PROT_READ | PROT_WRITE,
        MAP_SHARED, installShmHandle, i * sizeof(struct InstallInformation));
    DIE(installInfo == MAP_FAILED, "Could not map service information");
#elif defined(_WIN32)
    installInfo = (struct InstallInformation *)MapViewOfFile(
        installShmHandle, // handle to map object
        AQUA_PROT_READ | AQUA_PROT_WRITE, 0,
        i * sizeof(struct InstallInformation),
        sizeof(struct InstallInformation));
    DIE(installInfo == NULL, "Could not map service information");
#else
#endif

    configureClientConnectInformation(p_ConnectInfo, installInfo);
    configureClientCallInformation(p_CallInfo, installInfo);
}

void retriveInitInformation(struct ClientConnectInfo *p_ConnectInfo,
                            struct ClientCallInfo *p_CallInfo,
                            const char *p_ServiceStrId) {
    dspConnect(p_ConnectInfo, p_CallInfo, p_ServiceStrId);
}

struct ConnectResponseInformation *
getConnectResponse(struct ClientReturnInfo *p_ReturnInfo) {
    if (*p_ReturnInfo->m_ResponseQueue.m_Metadata.m_Size == 0) {
        return &p_ReturnInfo->m_ResponseQueue.m_Data[0];
    }

    return NULL;
}
