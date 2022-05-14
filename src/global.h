//
// Created by Dominic on 5/13/2022.
//

#ifndef ECVT_DAQ_V1_GLOBAL_H
#define ECVT_DAQ_V1_GLOBAL_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include "iostream/ArduinoStream.h"
#include "jled.h"
#include "DebouncedButton.h"
#include "SDManager.h"

#define SSID "daqdrewwww 🥵🍆💦"

#define DATA_SERIAL Serial2
#define DATA_BAUD 115200
#define DATA_MAGIC_START 0xAA

#define PIN_DAQ_LED 13
#define PIN_BTN_LOG 22

namespace global {
    struct Status {
        bool is_logging;
        uint32_t num_serial_overflows;
        uint32_t num_bytes_read;
    };

    enum class State {
        IDLE,
        LOGGING,
        ERROR_LOG_INIT,
        RESET,
    };

    extern State state;
    extern Status status;

    extern ArduinoOutStream debug;

    extern JLed daq_led;
    extern JLed builtin_led;

    extern DebouncedButton log_btn;

/*SD Definitions*/
    extern SDManager sd_manager;

    extern const char header[]; //Defined with the writing function below :)
}


#endif //ECVT_DAQ_V1_GLOBAL_H
