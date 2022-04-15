//
// Created by Dominic on 4/14/2022.
//

#ifndef ECVT_DAQ_V1_VARS_H
#define ECVT_DAQ_V1_VARS_H

enum LogStatus {
    IDLE,
    ERROR_SD_INIT,
    ERROR_SD_UNDETERMINED_SIZE,
    LOGGING,
};

struct Vars {
    LogStatus log_status;
    bool request_flag_clear_err;
    bool request_flag_log_init;

};
#endif //ECVT_DAQ_V1_VARS_H
