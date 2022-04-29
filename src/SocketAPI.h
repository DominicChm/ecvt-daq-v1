//
// Created by Dominic on 4/23/2022.
//

#ifndef ECVT_DAQ_V1_SOCKETAPI_H
#define ECVT_DAQ_V1_SOCKETAPI_H

#include <AsyncWebSocket.h>

namespace SocketAPI {

    void emitHeader(AsyncWebSocketClient *client, const char *header) {
        client->printf(R"({"type":"header", "data": "%s"})", header);
    }

    void emitRunEvent(AsyncWebSocketClient *client, const char *event_name) {
        client->printf(R"({"type":"event", "data": "%s"})", event_name);
    }
}
#endif //ECVT_DAQ_V1_SOCKETAPI_H
