//
// Created by Dominic on 4/21/2022.
//

#ifndef ECVT_DAQ_V1_COMMUNICATIONS_H
#define ECVT_DAQ_V1_COMMUNICATIONS_H
#include <Arduino.h>
struct Data
{
    uint32_t time;
    int16_t rwSpeed;
    int16_t fBrakePressure;
    int16_t rBrakePressure;
    // Engine
    int8_t eState;
    int16_t eSpeed;
    int16_t ePID;
    int16_t eP;
    int16_t eI;
    int16_t eD;
    // Primary
    int8_t pState;
    int32_t pEncoder;
    int16_t pLoadCellForce;
    int16_t pMotorCurrent;
    int16_t pControllerOutput;
    // Secondary
    int8_t sState;
    int32_t sEncoder;
    int16_t sLoadCellForce;
    int16_t sMotorCurrent;
    int16_t sControllerOutput;
    int16_t sEncoderPID;
    int16_t sLoadCellPID;
    int16_t sLoadCellP;
    int16_t sLoadCellI;
    int16_t sLoadCellD;
} __attribute__((packed));
#endif //ECVT_DAQ_V1_COMMUNICATIONS_H
