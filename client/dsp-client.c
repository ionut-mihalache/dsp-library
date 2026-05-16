// SPDX-License-Identifier: LGPL-2.1-or-later

#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>

#include "aqua-types.h"
#include "dsp.h"

#include "client-call.h"
#include "client-connect.h"
#include "commons.h"
#include "dsp-client.h"
#include "log.h"
#include "macros.h"
#include "platform.h"
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

static aqua_size_t sf_GetInstallArenaSize() {
    aqua_size_t mapGranularity = Memory.getMapGranularity();

    aqua_size_t alignedHeaderSize =
        alignUp(sizeof(struct InstallInfo), mapGranularity);

    aqua_size_t alignedServiceSize =
        alignUp(sizeof(struct InstallInformation), mapGranularity);

    return alignedHeaderSize + alignedServiceSize * SERVICES_NUMBER;
}

static aqua_size_t sf_GetServiceOff(uint16_t i) {
    aqua_size_t mapGranularity = Memory.getMapGranularity();

    size_t alignedHeaderSize =
        alignUp(sizeof(struct InstallInfo), mapGranularity);

    aqua_size_t alignedServiceSize =
        alignUp(sizeof(struct InstallInformation), mapGranularity);

    return alignedHeaderSize + i * alignedServiceSize;
}

static struct InstallInformation *sf_GetService(struct InstallInfo *info,
                                                uint16_t i) {
    return (struct InstallInformation *)((aqua_u8_t *)info +
                                         sf_GetServiceOff(i));
}

void dspConnect(struct ClientConnectInfo *p_ConnectInfo,
                struct ClientCallInfo *p_CallInfo, const char *p_ServiceStrId) {
    int rc;
    aqua_file_handle_t installShmFd;
    struct InstallInformation *installInfo;
    uint8_t connected = false;
    uint16_t i;

    aqua_size_t installArenaSize = sf_GetInstallArenaSize();

    installShmFd = SharedMemoryObject.create(
        INSTALL_MZONE, AQUA_FILE_PERM_RDWR,
        AQUA_FILE_MODE_USER_READ | AQUA_FILE_MODE_USER_WRITE |
            AQUA_FILE_MODE_GROUP_READ | AQUA_FILE_MODE_GROUP_WRITE |
            AQUA_FILE_MODE_OTHER_READ | AQUA_FILE_MODE_OTHER_WRITE,
        installArenaSize, false);
    DIE(installShmFd < 0,
        "Could not open install memory zone shared memory object");

    struct InstallInfo *installMemZone = Allocator.memmap(
        NULL, installArenaSize, AQUA_MEM_PROT_READ | AQUA_MEM_PROT_WRITE,
        AQUA_MEM_SHARED, installShmFd, 0);

    DIE(installMemZone == MAP_FAILED, "Could not mmap install memory zone");

    for (i = 0; i < SERVICES_NUMBER; ++i) {
        installInfo = sf_GetService(installMemZone, i);

        if (!installInfo->m_Available) {
            continue;
        }

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

    rc = Allocator.memunmap(installMemZone, sizeof(struct InstallInfo));
    DIE(rc != 0, "Could not unmap install memory zone");

    /**
     * Map only the information of the service
     */
    installInfo = Allocator.memmap(
        NULL,
        alignUp(sizeof(struct InstallInformation), Memory.getMapGranularity()),
        AQUA_MEM_PROT_READ | AQUA_MEM_PROT_WRITE, AQUA_MEM_SHARED, installShmFd,
        sf_GetServiceOff(i));
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
