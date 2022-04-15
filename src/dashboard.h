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
    bool initialized;
    DebouncedButton log_btn;

    enum State {
        IDLE,
        LOGGING,
        ERROR_LOG_INIT,
    };

    // Not realllyyyy an FSM, but we pretend it is :)
    void fsm(Vars *vars, ArduinoOutStream &debug) {
        static State state;
        vars->request_flag_log_init = log_btn.isTriggered();
    }
}
#endif //ECVT_DAQ_V1_DASHBOARD_H
