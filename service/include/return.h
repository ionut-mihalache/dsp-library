// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef __DSP_SERVICE_RETURN_H
#define __DSP_SERVICE_RETURN_H

#include "dsp-service.h"

int32_t configureServiceReturnInformation(struct ServiceReturnInfo *p_ReturnInfo,
                                   struct ServiceConnectInfo *p_ConnectInfo,
                                   struct ConnectRequest *p_Request);

#endif // __DSP_SERVICE_RETURN_H
