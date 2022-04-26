//
// Created by Dominic on 4/23/2022.
//

#ifndef ECVT_DAQ_V1_SOCKETAPI_H
#define ECVT_DAQ_V1_SOCKETAPI_H

#include <AsyncWebSocket.h>
#include "SdFat.h"

namespace SocketAPI {
    SdFs sd;

    void emitHeader(AsyncWebSocketClient *client, const char *header) {
        client->printf(R"({"type":"header", "data": "%s"})", header);
    }
}
#endif //ECVT_DAQ_V1_SOCKETAPI_H
