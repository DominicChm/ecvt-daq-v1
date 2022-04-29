//
// Created by Dominic on 4/29/2022.
//

#ifndef ECVT_DAQ_V1_UTIL_H
#define ECVT_DAQ_V1_UTIL_H
enum class Ext {
    UNKNOWN,
    CSV,
    META,

};

Ext get_ext(const char *fileName) {
    size_t len = strlen(fileName);
    const char *end = fileName + len;
    if (!strcmp(end - 3, "csv")) {
        return Ext::CSV;
    } else if (!strcmp(end - 3, "met")) {
        return Ext::META;
    }
    return Ext::UNKNOWN;
}

size_t get_base(char *dest, const char *fileName, size_t size) {
    size_t i;
    for (i = strlen(fileName); i > 0 && fileName[i] != '.'; i--); // Seek to first '.'
    return strlcpy(dest, fileName, i + 1);
}

#endif //ECVT_DAQ_V1_UTIL_H
