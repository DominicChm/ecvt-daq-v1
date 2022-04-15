//
// Created by Dominic on 4/14/2022.
//

#ifndef ECVT_DAQ_V1_DASHBOARD_H
#define ECVT_DAQ_V1_DASHBOARD_H

#include <Arduino.h>
#include <sdios.h>
#include "DebouncedButton.h"
#include "Vars.h"

namespace dashboard {
    enum State {
        IDLE,
        LOGGING,
        ERROR_LOG_INIT,
    };

    void fsm(Vars *vars, ArduinoOutStream &debug) {
        static State state;

    }
}
#endif //ECVT_DAQ_V1_DASHBOARD_H
