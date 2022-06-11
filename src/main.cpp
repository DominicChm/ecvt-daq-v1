#include "global.h"

#include "commparser.h"
#include <Arduino.h>
#include <sdios.h>
#include <SPI.h>
#include "web.h"
#include "jled.h"
#include "DebouncedButton.h"
#include "Communications.h"
#include <WiFi.h>
#include "SDManager.h"

void setup() {
    using namespace global;

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
#ifndef PASSWORD
    WiFi.softAP(SSID);
#else
    WiFi.softAP(SSID, PASSWORD);
#endif
    debug << "Initializing SD" << endl;
    if (!sd_manager.init()) {
        debug << "SD INIT FAIL!" << endl;
        web::setup_fail();
    }

    debug << "Starting server" << endl;
    web::init();

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

size_t write_csv_line(char *buf, Data *d);

void loop() {
    using namespace global;
    static Data data;

    daq_led.Update();
    sd_manager.loop();
    builtin_led.Update();

    if (parse_data(&data)) {
        //Buffer the new packet
        size_t num_written = write_csv_line(print_buf, &data);
        if (state == State::LOGGING) {
            sd_manager.write((uint8_t *) (print_buf), num_written);
        }

    }

    ws_api::loop();

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
                sd_manager.log_file.printf("%s\n", header);
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
            debug << "Log closed!" << endl;
            daq_led.Off(1).Forever();
            state = State::IDLE;
            break;

    }
}

/* UPDATE WHENEVER DATA DOES */
const char global::header[] = "time, rwSpeed, "
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