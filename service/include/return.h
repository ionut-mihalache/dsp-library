#ifndef AQUA_DSP_SERVICE_RETURN_H
#define AQUA_DSP_SERVICE_RETURN_H

#include "dsp-service.h"

int32_t
configureServiceReturnInformation(struct ServiceReturnInfo *p_ReturnInfo,
                                  struct ServiceConnectInfo *p_ConnectInfo,
                                  struct ConnectRequest *p_Request);

#endif // AQUA_DSP_SERVICE_RETURN_H
