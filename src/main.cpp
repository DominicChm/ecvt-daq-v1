#include <Arduino.h>
#include "util.h"
#include "SocketAPI.h"
#include <sdios.h>
#include <SPI.h>

#include <SD.h>
#include "jled.h"
#include "DebouncedButton.h"
#include "Communications.h"
#include "ESPAsyncWebServer.h"
//#include <ESPmDNS.h>
#include <WiFi.h>
#include "SDManager.h"
#include <CircularBuffer.h>

#define SSID "daqdrewwww 🥵🍆💦"

#define DATA_SERIAL Serial2
#define DATA_BAUD 115200
#define DATA_MAGIC_START 0xAA

size_t orate = 0;

enum class HandlerT {
    FILE,
    RUN_LIST,
};

CircularBuffer<AsyncWebServerRequest *, 20> request_buffer;

/* Global Definitions */
ArduinoOutStream debug(Serial);

JLed daq_led(13);
JLed builtin_led(LED_BUILTIN);

DebouncedButton log_btn(22, 250);

AsyncWebServer server(80);
AsyncWebSocket socket("/ws");

/*SD Definitions*/
SDManager sd_manager(debug);
volatile size_t is_responding = false;
/* Declariations */
extern const char header[]; //Defined with the writing function below :)

enum class State {
    IDLE,
    LOGGING,
    ERROR_LOG_INIT,
    RESET,
};


State state;

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data,
               size_t len);

void api_set_meta(AsyncWebServerRequest *req) {
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
    //req->getParam("name")->value().c_str();
    //req->getParam("desc")->value().c_str();
    req->send(200, "", "OK");
    f.close();

    // Yield to give the ESP some time before rescanning.
    yield();
    sd_manager.scan_runs();
    yield();
    socket.textAll(R"({"type":"event", "data": "runs"})");
}

void api_stop(AsyncWebServerRequest *req) {
    char path[256];
    state = State::RESET;
}

void api_start(AsyncWebServerRequest *req) {
    state = State::LOGGING;
}

[[noreturn]] void setup_fail() {
    server.on("/*", [](AsyncWebServerRequest *req) {
        req->send(500, "text/plain", "SD ERROR! (possibly - no SD inserted, "
                                     "no web folder");
    });
    while (1) {
        builtin_led.Update();
        daq_led.Update();
    }
}

void setup() {
    DATA_SERIAL.begin(DATA_BAUD);
    DATA_SERIAL.setRxBufferSize(1024);
    Serial.begin(115200);
    debug << "CPU Clock: " << getCpuFrequencyMhz() << endl;
    // Set LED into a default failure state to easily fail.
    builtin_led.Blink(100, 100).Forever();
    daq_led.Blink(100, 100).Forever();

    debug << "Setting up server" << endl;
    socket.onEvent(onWsEvent);
    server.addHandler(&socket);

//    debug << "Starting MDNS" << endl;
//    if (!MDNS.begin("ecvt")) {
//        debug << "Error starting mDNS" << endl;
//
//        while (true) {
//            daq_led.Update();
//            builtin_led.Update();
//        }
//    }

    debug << "Starting AP" << endl;
    WiFi.softAPConfig(
            IPAddress(192, 168, 1, 1),
            IPAddress(192, 168, 1, 1),
            IPAddress(255, 255, 255, 0)
    );
    WiFi.softAP(SSID);
    debug << "Local IP: " << WiFi.localIP() << endl;

    debug << "Initializing SD" << endl;
    if (!sd_manager.init()) {
        debug << "SD INIT FAIL!" << endl;
        setup_fail();
    }

    debug << "Starting server" << endl;
    server.on("/api/setmeta", HTTP_GET, api_set_meta);
    server.on("/api/start", HTTP_GET, api_start);
    server.on("/api/stop", HTTP_GET, api_stop);

    server.serveStatic("/r.txt", SD, "/r.txt", "no-cache");
    server.serveStatic("/", SD, "/").setDefaultFile("index.html");
//    server.on("/*", [](AsyncWebServerRequest *req) {
//        req->send(404, "text/html", "Error 404 DaqDrew not found :o");
//    });

    server.begin();

    // Blink a success message :)
    debug << "Setup finished successfully!" << endl;
    builtin_led.Breathe(500);
    daq_led.Breathe(500).Repeat(4);
    while (daq_led.IsRunning()) {
        daq_led.Update();
        builtin_led.Update();
    }
}


Data *parse_data() {
    static Data data{};
    static size_t packet_index;
    static size_t dp;

    static enum {
        IDLE,
        _IDLE,

        PAYLOAD,
        PAYLOAD_,

        TRAILER,
        TRAILER_,
    } state;

    if (DATA_SERIAL.available() > 1000) debug << "WEEWOO" << endl;
    if (!DATA_SERIAL.available()) return nullptr;
    uint8_t b = DATA_SERIAL.read();
    //debug << "DAT" << endl;

    switch (state) {
        case IDLE:
            state = _IDLE;
            packet_index = 0;
        case _IDLE:
            if (b != DATA_MAGIC_START) {
                state = IDLE; //RESET
            } else if (packet_index >= 1) {
                state = PAYLOAD;
            }
            break;

        case PAYLOAD:
            state = PAYLOAD_;
            dp = 0;
        case PAYLOAD_:
            ((uint8_t * ) & data)[dp] = b;
            if (dp >= sizeof(Data) - 1) {
                state = TRAILER;
            }
            break;

        case TRAILER:
            state = TRAILER_;
            dp = 0;
        case TRAILER_:
            if (dp >= 1) {
                state = IDLE;
                return &data;
            }
            break;
    }
    packet_index++;
    dp++;
    return nullptr;
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data,
               size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        SocketAPI::emitHeader(client, header);
        SocketAPI::emitRunEvent(client, "runs");
        if(sd_manager.is_logging()) {
            client->text(R"({"type":"status", "data": {"logging": true}})");
        } else {
            client->text(R"({"type":"status", "data": {"logging": false}})");
        }

        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(),
                      *((uint16_t *) arg), (char *) data);
    } else if (type == WS_EVT_PONG) {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(),
                      len, (len) ? (char *) data : "");
    } else if (type == WS_EVT_DATA) {
        Serial.println("DATA");
    }
}

size_t write_csv_line(char *buf, Data *d);

size_t last_push = 0;

void loop() {

    static char print_buf[512];
    Data *dat_ptr;

    daq_led.Update();
    sd_manager.loop();
    builtin_led.Update();

    if ((dat_ptr = parse_data()) != nullptr) {
        //Buffer the new packet
        //debug << dat_ptr->time << endl;
        //debug << sd_manager.sd_buf_head << "\t" << sd_manager.sd_buf_tail << endl;
        size_t num_written = write_csv_line(print_buf, dat_ptr);
        if (state == State::LOGGING) {
            sd_manager.write((uint8_t * )(print_buf), num_written);
        }
        if (millis() - last_push > 500) {
            last_push = millis();
            print_buf[num_written - 1] = '\0';
            socket.printfAll(R"({"type":"frame", "data": "%s"})", print_buf);
        }
    }

    switch (state) {
        case State::IDLE:
            if (log_btn.isTriggered()) {
                debug << "Starting write!" << endl;
                daq_led.On(1).Forever();
                state = State::LOGGING;
            }
            break;

        case State::LOGGING:
            if (!sd_manager.is_logging()) {
                debug << "Initing log" << endl;
                if (!sd_manager.init_log()) {
                    state = State::ERROR_LOG_INIT;
                    break;
                }
                while (DATA_SERIAL.available()) { DATA_SERIAL.read(); } // Clear buffered data!
                sd_manager.write((uint8_t * ) & header, strlen(header));
                sd_manager.write((uint8_t *) F("\n"), 1);

                debug << header << "\t" << strlen(header) << endl;
                socket.textAll(R"({"type":"status", "data": {"logging": true}})");
            }
            if (log_btn.isTriggered()) {
                state = State::RESET;
            }
            //SD WRITING HANDLED OUTSIDE OF FSM KINDA (ABOVE)
            break;

        case State::ERROR_LOG_INIT:
            if (log_btn.isTriggered()) {
                state = State::RESET;
            }
            break;
        case State::RESET:
            debug << "Logger reset!" << endl;
            sd_manager.close_log();

            sd_manager.scan_runs();

            socket.textAll(R"({"type":"event", "data": "runs"})");
            socket.textAll(R"({"type":"status", "data":{"logging":false}})");

            daq_led.Off(1).Forever();
            state = State::IDLE;
            break;

    }
}

/* UPDATE WHENEVER DATA DOES */
const char header[] = "time, rwSpeed, "
                      "eState, eSpeed, ePID, eP, eI, eD, "
                      "pState, pEncoder, pLoadCellForce, pMotorCurrent, "
                      "pControllerOutput, "
                      "sState, sEncoder, sLoadCellForce, sMotorCurrent, "
                      "sControllerOutput, sEncoderPID, sLoadCellPID, "
                      "sLoadCellP, sLoadCellI, sLoadCellD, "
                      "fBrakePressure, rBrakePressure";

size_t write_csv_line(char *buf, Data *d) {
    sprintf(buf, "%u,%d,"
                 "%d,%d,%d,%d,%d,%d,"
                 "%d,%d,%d,%d,%d,"
                 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,"
                 "%d,%d\n",
            d->time, d->rwSpeed,
            d->eState, d->eSpeed, d->ePID, d->eP, d->eI, d->eD,
            d->pState, d->pEncoder, d->pLoadCellForce, d->pMotorCurrent,
            d->pControllerOutput,
            d->sState, d->sEncoder, d->sLoadCellForce, d->sMotorCurrent,
            d->sControllerOutput, d->sEncoderPID, d->sLoadCellPID,
            d->sLoadCellP, d->sLoadCellI, d->sLoadCellD,
            d->fBrakePressure, d->rBrakePressure);
    return strlen(buf);
}