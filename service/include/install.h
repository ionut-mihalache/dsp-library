#ifndef AQUA_DSP_SERVICE_INSTALL_H
#define AQUA_DSP_SERVICE_INSTALL_H

#include "dsp-service.h"

int32_t initializeServiceConnections(struct InstallInformation *p_InstallInfo);

int32_t
configureServiceConnectInformation(struct ServiceConnectInfo *p_ConnectInfo,
                                   struct InstallInformation *p_InstallInfo);

#endif // AQUA_DSP_SERVICE_INSTALL_H
