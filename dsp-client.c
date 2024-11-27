#include "dsp.h"
#include "dsp-client.h"
#include "protocol.h"

#include "log/log.h"

#include <sys/shm.h>
#include <string.h>
#include <stdbool.h>

static char *ptr = NULL;

void dspConnect(const char *p_ServiceStrId) {
    int rc;
    int installShmFd;
    uint8_t connected = false;
    uint8_t bytesnr = SERVICES_NUMBER >> 3;

    // shm_unlink(INSTALL_MZONE); // TODO: This should not happen all the time.
    installShmFd = shm_open(INSTALL_MZONE, O_CREAT | O_EXCL | O_RDONLY, 0600);
    if (installShmFd < 0) {
        if (errno == EEXIST) {
            installShmFd = shm_open(INSTALL_MZONE, O_RDONLY, 0600);
            assert(installShmFd >= 0);
        } else {
            LOGF("Could not connect. The install memory zone is not initialized.\n");
            return;
        }
    }

    uint8_t *installMemZone = mmap(0,
            bytesnr + (SERVICES_NUMBER * sizeof(struct InstallInformation)),
            PROT_READ | PROT_WRITE, MAP_SHARED, installShmFd, 0);
    assert(installMemZone != MAP_FAILED);

    for (uint16_t i = 0; i < SERVICES_NUMBER; ++i) {
        struct InstallInformation *installInfo = installMemZone + bytesnr + i * sizeof(struct InstallInformation);

        if (!strcmp(installInfo->m_StrId, p_ServiceStrId)) {
            if (installInfo->m_Available) {
                connected = true;
            }
            break;
        }
    }

    if (!connected) {
        LOGF("Could not connect. Service is not installed or unavailable");
    }

    /**
     * TODO: Implement successfull connection functionality
     */
}
