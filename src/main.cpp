
#include <Arduino.h>
#include "global.h"
#include <sdios.h>
#include <SPI.h>
#include "web.h"
#include "jled.h"
#include "DebouncedButton.h"
#include "Communications.h"
#include <WiFi.h>
#include "SDManager.h"
#include <CircularBuffer.h>


State state;

/* I/O */
JLed daq_led(PIN_DAQ_LED);
JLed builtin_led(LED_BUILTIN);
DebouncedButton log_btn(PIN_BTN_LOG, 250);
ArduinoOutStream debug(Serial);

/* SD Definitions */
SDManager sd_manager(debug);

/* Declariations */


void setup() {
    // Set LED into a default failure state to easily fail.
    builtin_led.Blink(100, 100).Forever();
    daq_led.Blink(100, 100).Forever();

    /****** SETUP BEGIN *******/
    DATA_SERIAL.begin(DATA_BAUD);
    DATA_SERIAL.setRxBufferSize(1024);
    Serial.begin(115200);

    debug << "CPU Clock: " << getCpuFrequencyMhz() << endl;
    debug << "Heap: " << esp_get_free_heap_size() << endl;

    debug << "Starting AP" << endl;
    WiFi.softAPConfig(
            IPAddress(1, 2, 3, 4),
            IPAddress(1, 2, 3, 4),
            IPAddress(255, 255, 255, 0)
    );
    WiFi.softAP(SSID);
    debug << "Local IP: " << WiFi.localIP() << endl;

    debug << "Initializing SD" << endl;
    if (!sd_manager.init()) {
        debug << "SD INIT FAIL!" << endl;
        web::setup_fail();
    }

    debug << "Setting up server" << endl;
    web::init();

    debug << "Starting server" << endl;
    web::begin();

    // Blink a success message :)
    debug << "Setup finished successfully!" << endl;

    builtin_led.Breathe(500);
    daq_led.Breathe(500).Repeat(4);
    while (daq_led.IsRunning()) {
        daq_led.Update();
        builtin_led.Update();
    }
    builtin_led.Off().Forever();
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
            sd_manager.write((uint8_t *) (print_buf), num_written);
        }
        if (millis() - last_push > 500) {
            last_push = millis();
            print_buf[num_written - 1] = '\0';
            ws_api::emit_frame(print_buf);
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
                sd_manager.write((uint8_t *) &header, strlen(header));
                sd_manager.write((uint8_t *) F("\n"), 1);

                debug << header << "\t" << strlen(header) << endl;
                ws_api::emit_status();
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
            sd_manager.init_run_db();

            ws_api::emit_runs();
            ws_api::emit_status();

            daq_led.Off(1).Forever();
            state = State::IDLE;
            break;

    }
}

/* UPDATE WHENEVER DATA DOES */
const char header[] = "time, rwSpeed, "
                      "eState, eSpeed, ePID, eP, eI, eD, "
                      "pState, pEncoder, pLoadCell, pCurrent, pPID, "
                      "sState, sEncoder, sLoadCell, sCurrent, sPID, "
                      "sEncoderPID, sLoadCellPID, sLoadCellP, sLoadCellI, sLoadCellD";

size_t write_csv_line(char *buf, Data *d) {
    sprintf(buf, "%u,%d,"
                 "%d,%d,%d,%d,%d,%d,"
                 "%d,%d,%d,%d,%d,"
                 "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
            d->time, d->rwSpeed,
            d->eState, d->eSpeed, d->ePID, d->eP, d->eI, d->eD,
            d->pState, d->pEncoder, d->pLoadCell, d->pCurrent, d->pPID,
            d->sState, d->sEncoder, d->sLoadCell, d->sCurrent, d->sPID,
            d->sEncoderPID, d->sLoadCellPID, d->sLoadCellP,
            d->sLoadCellI, d->sLoadCellD);
    return strlen(buf);
}