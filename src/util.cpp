#include "util.h"

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
