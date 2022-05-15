//
// Created by Dominic on 5/13/2022.
//

#ifndef ECVT_DAQ_V1_WEB_H
#define ECVT_DAQ_V1_WEB_H

#include "global.h"
#include "ArduinoJson.h"

#define WS_BUFFER_SIZE 1024
#define opt_client AsyncWebSocketClient *client = nullptr

namespace ws_api {
    using namespace global;

    AsyncWebSocket socket("/ws");
    StaticJsonDocument<WS_BUFFER_SIZE> doc;
    char socket_buf[WS_BUFFER_SIZE];


    void json_all(opt_client) {
        serializeJson(doc, socket_buf, 1024);
        doc.clear();

        if (client)
            return client->text(socket_buf);
        else
            return socket.textAll(socket_buf);
    }

    void emit_runs(opt_client) {
        doc["type"] = "event";
        doc["data"] = "runs";

        json_all(client);
    }

    void emit_status(opt_client) {
        doc["type"] = "status";
        doc["data"]["logging"] = sd_manager.status.logging;
        doc["data"]["t_write_last"] = sd_manager.status.t_write_last;
        doc["data"]["t_write_avg"] = sd_manager.status.t_write_avg;
        doc["data"]["data_rate_Bps"] = sd_manager.status.data_rate_Bps;
        doc["data"]["num_overflows"] = sd_manager.status.num_overflows;
        doc["data"]["num_writes"] = sd_manager.status.num_writes;

        json_all(client);
    };

    void emit_frame(const char *data, opt_client) {
        doc["type"] = "status";
        doc["frame"] = data;
        json_all(client);
    };


    void emit_header(opt_client) {
        doc["type"] = "header";
        doc["data"] = header;

        json_all(client);
    }


    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                   AwsEventType type, void *arg, uint8_t *data,
                   size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());

            emit_header(client);
            emit_runs(client);
            emit_status(client);

            client->ping();
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("ws[%s][%u] disconnect\n", server->url(),
                          client->id());
        } else if (type == WS_EVT_ERROR) {
            Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(),
                          client->id(),
                          *((uint16_t *) arg), (char *) data);
        } else if (type == WS_EVT_PONG) {
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(),
                          client->id(),
                          len, (len) ? (char *) data : "");
        } else if (type == WS_EVT_DATA) {
            Serial.println("Ws DATA");
        }
    }

    void loop() {
        static unsigned long last_frame = 0;

        if (millis() - last_frame > 500) {
            last_frame = millis();

            if (strlen(print_buf))
                emit_frame(print_buf);

            emit_status();

            ws_api::emit_runs();

        }
    }
}


namespace http_api {
    using namespace global;

    void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                   AwsEventType type, void *arg, uint8_t *data,
                   size_t len);

    void set_meta(AsyncWebServerRequest *req) {
        char path[256];
        char filename[256];
        snprintf(path, 256, "/r/%s", req->getParam("file")->value().c_str());

        if (!SD.exists(path)) {
            req->send(304, "", "Target file doesn't exist");
            return;
        }

        get_base(filename, req->getParam("file")->value().c_str(), 256);
        snprintf(path, 256, "/r/%s.met", filename);
        if (!SD.exists(path)) {
            req->send(304, "", "Target meta doesn't exist");
            return;
        }
        debug << "SET META"
              << "\tname:" << req->getParam("name")->value()
              << "\tdesc:" << req->getParam("desc")->value()
              << "\tfile:" << req->getParam("file")->value() << endl;

        File f = SD.open(path, "w");
        f.print(req->getParam("name")->value());
        f.print("`");
        f.print(req->getParam("desc")->value());
        req->send(200, "", "OK");
        f.close();

        // Yield to give the ESP some time before rescanning.
        yield();
        sd_manager.init_run_db();
        yield();
        ws_api::emit_runs();
    }

    void stop_log(AsyncWebServerRequest *req) {
        char path[256];
        state = State::RESET;
    }

    void start_log(AsyncWebServerRequest *req) {
        state = State::LOGGING;
    }
}

namespace web {
    using namespace global;

    /* Server Defs */
    AsyncWebServer server(80);

    void init() {
        ws_api::socket.onEvent(ws_api::onWsEvent);
        server.addHandler(&ws_api::socket);
        server.on("/api/setmeta", HTTP_GET, http_api::set_meta);
        server.on("/api/start", HTTP_GET, http_api::start_log);
        server.on("/api/stop", HTTP_GET, http_api::stop_log);

        server.serveStatic("/r.txt", SD, "/r.txt", "no-cache");
        server.serveStatic("/", SD, "/").setDefaultFile("index.html");
        server.serveStatic("/*", SD, "404.html");

    }

    void begin() {
        server.begin();
    }

    [[noreturn]] void setup_fail() {
        server.on("/*", [](AsyncWebServerRequest *req) {
            req->send(500, "text/plain",
                      "SD ERROR! (possibly - no SD inserted, "
                      "no web folder");
        });
        while (1) {
            builtin_led.Update();
            daq_led.Update();
        }
    }
}

#endif //ECVT_DAQ_V1_WEB_H
