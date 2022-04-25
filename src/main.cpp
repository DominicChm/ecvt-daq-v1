#include <Arduino.h>
#include "SocketAPI.h"
#include <sdios.h>
#include <SPI.h>
#include <SdFat.h>
#include "jled.h"
#include "DebouncedButton.h"
#include "Communications.h"
#include "PrintMessage.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
//#include <ESPmDNS.h>
#include <WiFi.h>
#include "SDManager.h"



#define SSID "daqdrew 🥵🍆💦"

#define DATA_SERIAL Serial2
#define DATA_BAUD 115200
#define DATA_MAGIC_START 0xAA

/* Global Definitions */
ArduinoOutStream debug(Serial);

JLed daq_led(13);
JLed builtin_led(LED_BUILTIN);

DebouncedButton log_btn(22, 250);

AsyncWebServer server(80);
AsyncWebSocket socket("/ws");

/*SD Definitions*/
SDManager<3> sd_manager(debug);

/* Buffers data in a circular buffer to avoid data loss */
static uint8_t sd_buf[512 * 5];
static size_t sd_buf_head;
static size_t sd_buf_tail;
static uint8_t write_buf[512];

/* Declariations */
extern const char header[]; //Defined with the writing function below :)
bool init_sd();

void finalize_write();

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data,
               size_t len);

void fs_handler(AsyncWebServerRequest *req) {
    char filepath[256];
    debug << req->url() << endl;
    snprintf(filepath, 256, "/web%s", req->url().c_str());
    if (sd_manager.sd.exists(filepath)) {
        FsFile f = sd_manager.sd.open(filepath, O_RDONLY);
        //Stream &stream, const String& contentType, size_t len, AwsTemplateProcessor callback
        req->send((Stream &) f, "text/plain", f.size());
    } else {
        req->send(404, "text", "File not found :O\nBlame Andrew >:(");
    }
}

void setup() {
    DATA_SERIAL.begin(DATA_BAUD);
    Serial.begin(115200);

    // Set LED into a default failure state to easily fail.
    builtin_led.Blink(100, 100).Forever();
    daq_led.Blink(100, 100).Forever();

    debug << "Initializing SD" << endl;
    if (!init_sd()) {
        debug << "SD INIT FAIL!" << endl;
        while (true) {
            builtin_led.Update();
            daq_led.Update();
        }
    }

    debug << "Setting up server" << endl;
    socket.onEvent(onWsEvent);
    server.addHandler(&socket);

    server.on("/*", HTTP_GET, fs_handler);


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

    debug << "Starting server" << endl;
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

enum class State {
    IDLE,
    LOGGING,
    ERROR_LOG_INIT,
    RESET,
};

struct {
    char filename[256];
    State logger_state;
} daq_state;

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
            ((uint8_t *) &data)[dp] = b;
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

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data,
               size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        SocketAPI::emitHeader(client, header);
        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *) arg), (char *) data);
    } else if (type == WS_EVT_PONG) {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *) data : "");
    } else if (type == WS_EVT_DATA) {
        Serial.println("DATA");
    }
}

size_t buffer_data(char *buf, Data *data) {
    return sprintf(buf, "%d\n", data->time);
}


void loop() {
    static State state;

    static char print_buf[512];
    Data *dat_ptr;

    daq_led.Update();
    //builtin_led.Update();

    switch (state) {
        case State::IDLE:
            if (log_btn.isTriggered()) {
                debug << "Starting write!" << endl;
                daq_led.On(1).Forever();
                state = State::LOGGING;
            }
            break;

        case State::LOGGING:
            if (log_btn.isTriggered()) {
                finalize_write();
                state = State::RESET;
            }
            if ((dat_ptr = parse_data()) != nullptr) {
                //Buffer the new packet
                debug << dat_ptr->time << endl;
                size_t num_written = buffer_data(print_buf, dat_ptr);
                sd_manager.write((uint8_t *) (print_buf), num_written);
            }
            /* Write a block when the SD isn't busy */

            break;

        case State::ERROR_LOG_INIT:
            if (log_btn.isTriggered()) {
                state = State::RESET;
            }
            break;
        case State::RESET:
            debug << "Logger reset!" << endl;

            daq_led.Off(1).Forever();
            state = State::IDLE;

            break;

    }
}


bool init_sd() {

}

/* UPDATE WHENEVER DATA DOES */
const char header[] = "time, rwSpeed, "
                      "eState, eSpeed, ePID, eP, eI, eD, "
                      "pState, pEncoder, pLoadCell, pCurrent, pPID, "
                      "sState, sEncoder, sLoadCell, sCurrent, sPID, "
                      "sEncoderPID, sLoadCellPID, sLoadCellP, sLoadCellI, sLoadCellD";

void write_csv_line(char *buf, Data *d) {
    sprintf(buf, "LELEL");
}