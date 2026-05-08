#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "dsp.h"

#include "client-call.h"
#include "client-connect.h"
#include "commons.h"
#include "dsp-client.h"
#include "log.h"
#include "macros.h"

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
        struct SMBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case EMBQ: {
        struct EMBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case QMBQ: {
        struct QMBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case HMBQ: {
        struct HMBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case MBQ: {
        struct MBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case DMBQ: {
        struct DMBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case HGBQ: {
        struct HGBCall *callInfo = p_CallInfo;
        memcpy(callInfo->m_CallInfo, p_Data, p_Size);
        callInfo->m_Metadata.m_Size = p_Size;

        break;
    }
    case GBQ: {
        struct GBCall *callInfo = p_CallInfo;
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
    int installShmFd;
    struct InstallInformation *installInfo;
    uint8_t connected = false;
    uint16_t i;

    installShmFd = createShmObject(INSTALL_MZONE, O_RDWR,
                                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                       S_IROTH | S_IWOTH,
                                   sizeof(struct InstallInfo), false);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone =
        mmap(NULL, sizeof(struct InstallInfo), PROT_READ | PROT_WRITE,
             MAP_SHARED, installShmFd, 0);
    DIE(installMemZone == MAP_FAILED, "Could not mmap install memory zone");

    for (i = 0; i < SERVICES_NUMBER; ++i) {
        if (!installMemZone->m_Info[i].m_Available) {
            continue;
        }

        installInfo = (struct InstallInformation *)&(installMemZone->m_Info[i]);

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

    rc = munmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");

    /**
     * Map only the information of the service
     */
    installInfo = (struct InstallInformation *)mmap(
        NULL, sizeof(struct InstallInformation), PROT_READ | PROT_WRITE,
        MAP_SHARED, installShmFd, i * sizeof(struct InstallInformation));
    DIE(installInfo == MAP_FAILED, "Could not map service information");

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
