//
// Created by Dominic on 4/14/2022.
//

#ifndef ECVT_DAQ_V1_LOGGER_H
#define ECVT_DAQ_V1_LOGGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <sdios.h>

#include "Vars.h"
#include "jled.h"
#include "DebouncedButton.h"


namespace logger {
    const uint64_t SPI_SPEED = SD_SCK_MHZ(4);
    const uint8_t PIN_CS = 5;
    JLed daq_led(13);
    DebouncedButton log_btn(22);

    SdFs sd;
    FsFile file;

    enum State {
        IDLE,
        LOGGING,
        ERROR_LOG_INIT,
    };

    void sd_init_failure(Vars *vars, ArduinoOutStream &debug) {
        vars->log_status = ERROR_SD_INIT;

        if (sd.card()->errorCode()) {
            debug << F(
                    "\nSD initialization failed.\n"
                    "Do not reformat the card!\n"
                    "Is the card correctly inserted?\n"
                    "Is chipSelect set to the correct value?\n"
                    "Does another SPI device need to be disabled?\n"
                    "Is there a wiring/soldering problem?\n");
            debug << F("\nerrorCode: ") << hex << showbase;
            debug << int(sd.card()->errorCode());
            debug << F(", errorData: ") << int(sd.card()->errorData());
            debug << dec << noshowbase << endl;
            return;
        } else {
            debug << F("Failure to init, but no error code?");
        }
    }

    void sd_size_failure(Vars *vars, ArduinoOutStream &debug) {
        vars->log_status = ERROR_SD_UNDETERMINED_SIZE;

        debug << F("Can't determine the card size.\n");
        debug << F("Try another SD card or reduce the SPI bus speed.\n");
        debug << F("Edit SPI_SPEED in this program to change it.\n");
    }

    bool init_sd(Vars *vars, ArduinoOutStream &debug) {
        if (!sd.begin(PIN_CS, SPI_SPEED)) {
            sd_init_failure(vars, debug);
            return false;
        }

        if (sd.card()->sectorCount() == 0) {
            sd_size_failure(vars, debug);
            return false;
        }

        debug << F("Card initialized!") << endl;
        return true;
    }

    void fsm(Vars *vars, ArduinoOutStream &debug) {
        static State state;

        daq_led.Update();
        switch (state) {
            case IDLE:
                if (log_btn.isTriggered()) {
                    if (init_sd(vars, debug)) {
                        daq_led.On(1).Forever();
                        state = LOGGING;
                    } else {
                        daq_led.Blink(100, 100).Forever();
                        state = ERROR_LOG_INIT;
                    }
                }
                break;

            case LOGGING:
                if (log_btn.isTriggered()) {
                    daq_led.Off(1).Forever();
                    state = IDLE;
                }
                break;

            case ERROR_LOG_INIT:
                if (log_btn.isTriggered()) {
                    daq_led.Off(1).Forever();
                    state = IDLE;
                }
                break;

        }
    }
}


#endif //ECVT_DAQ_V1_LOGGER_H
