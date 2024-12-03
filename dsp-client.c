#include "dsp-client.h"
#include "dsp.h"
#include "protocol.h"
#include "utils/commons.h"

#include "utils/log/log.h"

#include <stdbool.h>
#include <string.h>
#include <sys/shm.h>

static char* ptr = NULL;

static int32_t s_QPush(struct DSPQueue *p_Queue) {
    return 0;
}

void dspConnect(struct ClientCallInfo *p_CallInfo, const char* p_ServiceStrId)
{
    int rc;
    int installShmFd;
    struct InstallInformation* installInfo;
    uint8_t connected = false;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    installShmFd = createShmObject(INSTALL_MZONE, O_RDONLY, 0600, bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)));

    uint8_t* installMemZone = mmap(0,
        bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)),
        PROT_READ, MAP_SHARED, installShmFd, 0);
    assert(installMemZone != MAP_FAILED);

    for (uint16_t i = 0; i < SERVICES_NUMBER; ++i) {
        installInfo = installMemZone + bytesnr + i * sizeof(struct InstallInformation);

        LOGF("The current service string id is %s.\n", installInfo->m_StrId);
        if (!strcmp(installInfo->m_StrId, p_ServiceStrId)) {
            if (installInfo->m_Available) {
                connected = true;
            }
            break;
        }
    }

    if (!connected) {
        LOGF("Could not connect. Service is not installed or unavailable.\n");
        return;
    }

    /**
     * TODO: Implement successfull connection functionality
     */
    LOGF("Connected to \'%s\' with version \'%s\'.\n", p_ServiceStrId, installInfo->m_Version);
}
