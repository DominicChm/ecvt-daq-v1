//
// Created by Dominic on 4/29/2022.
//

#ifndef ECVT_DAQ_V1_UTIL_H
#define ECVT_DAQ_V1_UTIL_H
#include "Arduino.h"

enum class Ext {
    UNKNOWN,
    CSV,
    META,
};

Ext get_ext(const char *fileName);

size_t get_base(char *dest, const char *fileName, size_t size);

#endif //ECVT_DAQ_V1_UTIL_H
